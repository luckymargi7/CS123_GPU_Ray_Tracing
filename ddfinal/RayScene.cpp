#include "RayScene.h"
#include "lib/CS123SceneData.h"
#include <QtGlobal>

using namespace std;

void RayScene::initScene( InitialCameraData& camera_data )
{
    // set up path to ptx file 
    //this isn't going to work...
    _ptx_path = ptxpath( "scene", "RayScene.cu" );
    
    // context 
    _context->setRayTypeCount( 2 );
    _context->setEntryPointCount( 1 );
    _context->setStackSize( 4640 );

    _context["max_depth"]->setInt(100);
    _context["radiance_ray_type"]->setUint(0);
    _context["shadow_ray_type"]->setUint(1);
    _context["frame_number"]->setUint( 0u );//we need to figure this out..
    _context["scene_epsilon"]->setFloat( 1.e-3f );
    _context["importance_cutoff"]->setFloat( 0.01f );
    _context["ambient_light_color"]->setFloat( 0.31f, 0.33f, 0.28f );

    _context["output_buffer"]->set( createOutputBuffer(RT_FORMAT_UNSIGNED_BYTE4, _width, _height) );

    // Ray gen program
    Program ray_gen_program = _context->createProgramFromPTXFile( _ptx_path, "camera");
    _context->setRayGenerationProgram( 0, ray_gen_program );

    // Exception / miss programs
    Program exception_program = _context->createProgramFromPTXFile( _ptx_path, "exception" );
    _context->setExceptionProgram( 0, exception_program );
    _context["failure_color"]->setFloat( 0.0f, 1.0f, 0.0f );

    _context->setMissProgram( 0, _context->createProgramFromPTXFile( _ptx_path, "miss") );
    const float3 default_color = make_float3(1.0f, 1.0f, 1.0f);
    //_context["envmap"]->settexturesampler( loadtexture( _context, texpath("cedarcity.hdr"), default_color) );
    _context["bg_color"]->setFloat( make_float3( 0.34f, 0.55f, 0.85f ) );

    // Lights-we need to import our own lights! Then we need to do the steps listed
    //below to pass the information into the context 
    
    //So the data processor is _dp and it has a getLightList() method that
    //returns a QList of CS123SceneLightData which can be used to fill
    //this light buffer.

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


Buffer RayScene::getOutputBuffer()
{
    return _context["output_buffer"]->getBuffer();
}


void RayScene::trace( const RayGenCameraData& camera_data )
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


void RayScene::doResize( unsigned int width, unsigned int height )
{
    // output buffer handled in SampleScene::resize
}

string RayScene::texpath( const string& base )
{
    return texture_path + "/" + base;
}

void RayScene::createCone(SimplePrimitive prim) {
    CS123SceneMaterial matl = prim.material;
    Matrix4x4 transMat = prim.transMat;
    
    string cone_ptx( ptxpath( "scene", "cone.cu" ) ); 
    Program cone_bounds = _context->createProgramFromPTXFile( cone_ptx, "cone_bounds" );
    Program cone_intersect = _context->createProgramFromPTXFile( cone_ptx, "cone_intersect" );
    
    // Create cone
    Geometry cone = _context->createGeometry();
    cone->setPrimitiveCount( 1u );
    cone->setBoundingconeProgram( cone_bounds );
    cone->setIntersectionProgram( cone_intersect );
    cone["conemin"]->setFloat( -0.5f, 0.5f, -0.5f );
    cone["conemax"]->setFloat(  0.5f, 0.5f,  0.5f );

    // Materials
    string cone_chname = "closest_hit_lighting";

    Material cone_matl = _context->createMaterial();
    Program cone_ch = _context->createProgramFromPTXFile( _ptx_path, cone_chname );
    cone_matl->setClosestHitProgram( 0, cone_ch );
    //cone_matl["Ka"]->setFloat( 0.3f, 0.3f, 0.3f );
    //cone_matl["Kd"]->setFloat( 0.6f, 0.7f, 0.8f );
    //cone_matl["Ks"]->setFloat( 0.8f, 0.9f, 0.8f );
    //cone_matl["phong_exp"]->setFloat( 88 );
    //cone_matl["reflectivity_n"]->setFloat( 0.2f, 0.2f, 0.2f );

    // Create GIs for each piece of geometry
    Geometry cone = _context->createGeometryInstance( cone, &cone_matl, &cone_matl+1 );

    // Place all in group
    GeometryGroup geometrygroup = _context->createGeometryGroup();
    geometrygroup->setChildCount(1);
    geometrygroup->setChild( 0, cone );
    geometrygroup->setAcceleration( _context->createAcceleration("NoAccel","NoAccel") );

    Transform transform = _context->createTransform();
    transform.setChild(geometrygroup);

    tNodes.push_back(transform);
}

void RayScene::createGeometry()
{ 
    //this is all us!
    //so the _dp has a getPrimList() method that returns a QList of
    // SimplePrimitives (see CS123SceneData) which can be used to fill
    // in this section.
    
    //Create all geometry instances here

    QList<SimplePrimitive> prims = _dp.getPrimList();
    for(int i = 0; i < prims->size(); i++) {
        SimplePrimitive prim = prims[i];
        Program prim_bounds;
        Program prim_intersect;
        Material prim_matl;
        switch(prim.type) {
            case PRIMITIVE_CUBE:
                createCube(prim, prim_bounds, prim_intersect, prim_matl);
                break;
            case PRIMITIVE_CONE:
                createCone(prim, prim_bounds, prim_intersect, prim_matl);
                break;
            case PRIMITIVE_CYLINDER:
                createCylinder(prim, prim_bounds, prim_intersect, prim_matl);
                break;

            case PRIMITIVE_SPHERE:
                createSphere(prim, prim_bounds, prim_intersect, prim_matl);
                break;

            default:
                break;
        }

    }
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
        vector<GeometryInstance> gis;
        gis.push_back( _context->createGeometryInstance( cube, &cube_matl, &cube_matl+1 ) );

        // Place all in group
        GeometryGroup geometrygroup = _context->createGeometryGroup();
        geometrygroup->setChildCount( static_cast<unsigned int>(gis.size()) );
        geometrygroup->setChild( 0, gis[0] );
        geometrygroup->setChild( 1, gis[1] );
        geometrygroup->setAcceleration( _context->createAcceleration("NoAccel","NoAccel") );

        _context["top_object"]->set( geometrygroup );
        _context["top_shadower"]->set( geometrygroup );
}


