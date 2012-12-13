#include "Scene.h"

float Scene::rand_range(float min, float max)
{
    return min + (max - min) * (float) rand() / (float) RAND_MAX;
}
void Scene::initScene( InitialCameraData& camera_data )
{
    // set up path to ptx file 
    _ptx_path = ptxpath( "scene", "scene.cu" );

    // context 
    _context->setRayTypeCount( 2 );
    _context->setEntryPointCount( 1 );
    _context->setStackSize( 4640 );

    _context["max_depth"]->setInt(100);
    _context["radiance_ray_type"]->setUint(0);
    _context["shadow_ray_type"]->setUint(1);
    _context["frame_number"]->setUint( 0u );
    _context["scene_epsilon"]->setFloat( 1.e-3f );
    _context["importance_cutoff"]->setFloat( 0.01f );
    _context["ambient_light_color"]->setFloat( 0.31f, 0.33f, 0.28f );

    _context["output_buffer"]->set( createOutputBuffer(RT_FORMAT_UNSIGNED_BYTE4, _width, _height) );

    // Ray gen program
    string camera_name = "pinhole_camera";
    Program ray_gen_program = _context->createProgramFromPTXFile( _ptx_path, camera_name );
    _context->setRayGenerationProgram( 0, ray_gen_program );

    // Exception / miss programs
    Program exception_program = _context->createProgramFromPTXFile( _ptx_path, "exception" );
    _context->setExceptionProgram( 0, exception_program );
    _context["bad_color"]->setFloat( 0.0f, 1.0f, 0.0f );

    string miss_name = "miss";
    _context->setMissProgram( 0, _context->createProgramFromPTXFile( _ptx_path, miss_name ) );
    const float3 default_color = make_float3(1.0f, 1.0f, 1.0f);
    //_context["envmap"]->settexturesampler( loadtexture( _context, texpath("cedarcity.hdr"), default_color) );
    _context["bg_color"]->setFloat( make_float3( 0.34f, 0.55f, 0.85f ) );

    // Lights
    BasicLight lights[] = { 
    { make_float3( -5.0f, 60.0f, -16.0f ), make_float3( 1.0f, 1.0f, 1.0f ), 1 }
    };

    Buffer light_buffer = _context->createBuffer(RT_BUFFER_INPUT);
    light_buffer->setFormat(RT_FORMAT_USER);
    light_buffer->setElementSize(sizeof(BasicLight));
    light_buffer->setSize( sizeof(lights)/sizeof(lights[0]) );
    memcpy(light_buffer->map(), lights, sizeof(lights));
    light_buffer->unmap();

    _context["lights"]->set(light_buffer);

    // Set up camera
    camera_data = InitialCameraData( make_float3( 7.0f, 9.2f, -6.0f ), // eye
                                   make_float3( 0.0f, 4.0f,  0.0f ), // lookat
                                   make_float3( 0.0f, 1.0f,  0.0f ), // up
                                   60.0f );                          // vfov

    _context["eye"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );
    _context["U"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );
    _context["V"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );
    _context["W"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );

    // Populate scene hierarchy
    createGeometry();

    // Prepare to run
    _context->validate();
    _context->compile();
}


Buffer Scene::getOutputBuffer()
{
    return _context["output_buffer"]->getBuffer();
}


void Scene::trace( const RayGenCameraData& camera_data )
{
    _context["eye"]->setFloat( camera_data.eye );
    _context["U"]->setFloat( camera_data.U );
    _context["V"]->setFloat( camera_data.V );
    _context["W"]->setFloat( camera_data.W );

    Buffer buffer = _context["output_buffer"]->getBuffer();
    RTsize buffer_width, buffer_height;
    buffer->getSize( buffer_width, buffer_height );

    _context->launch( 0, static_cast<unsigned int>(buffer_width),
                  static_cast<unsigned int>(buffer_height) );
}


void Scene::doResize( unsigned int width, unsigned int height )
{
    // output buffer handled in SampleScene::resize
}

string Scene::texpath( const string& base )
{
    return texture_path + "/" + base;
}

float4 make_plane( float3 n, float3 p )
{
    n = normalize(n);
    float d = -dot(n, p);
    return make_float4( n, d );
}

void Scene::createGeometry()
{
    string cube_ptx( ptxpath( "scene", "cube.cu" ) ); 
    Program cube_bounds = _context->createProgramFromPTXFile( cube_ptx, "cube_bounds" );
    Program cube_intersect = _context->createProgramFromPTXFile( cube_ptx, "cube_intersect" );

    // Create cube
    Geometry cube = _context->createGeometry();
    cube->setPrimitiveCount( 1u );
    cube->setBoundingcubeProgram( cube_bounds );
    cube->setIntersectionProgram( cube_intersect );
    cube["cubemin"]->setFloat( -2.0f, 0.0f, -2.0f );
    cube["cubemax"]->setFloat(  2.0f, 7.0f,  2.0f );

    // Materials
    string cube_chname = "closest_hit_radiance1";

    Material cube_matl = _context->createMaterial();
    Program cube_ch = _context->createProgramFromPTXFile( _ptx_path, cube_chname );
    cube_matl->setClosestHitProgram( 0, cube_ch );
    cube_matl["Ka"]->setFloat( 0.3f, 0.3f, 0.3f );
    cube_matl["Kd"]->setFloat( 0.6f, 0.7f, 0.8f );
    cube_matl["Ks"]->setFloat( 0.8f, 0.9f, 0.8f );
    cube_matl["phong_exp"]->setFloat( 88 );
    cube_matl["reflectivity_n"]->setFloat( 0.2f, 0.2f, 0.2f );

    // Create GIs for each piece of geometry
    std::vector<GeometryInstance> gis;
    gis.push_back( _context->createGeometryInstance( cube, &cube_matl, &cube_matl+1 ) );

    // Place all in group
    GeometryGroup geometrygroup = _context->createGeometryGroup();
    geometrygroup->setChildCount( static_cast<unsigned int>(gis.size()) );
    geometrygroup->setChild( 0, gis[0] );
    geometrygroup->setChild( 1, gis[1] );
    if(chull.get())
    geometrygroup->setChild( 2, gis[2] );
    geometrygroup->setAcceleration( _context->createAcceleration("NoAccel","NoAccel") );

    _context["top_object"]->set( geometrygroup );
    _context["top_shadower"]->set( geometrygroup );
}
