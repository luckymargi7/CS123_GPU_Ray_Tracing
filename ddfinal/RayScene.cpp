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

    // Lights 
    std::vector<CS123SceneLightData>* oldL = _dp.getLightLists();
    CS123SceneLightData lights[(*oldL).size()];
    for(int i=0; i<(*oldL).size(); i++){
        lights[i] = (*oldL).back();
        (*oldL).pop_back();
    }

    Buffer light_buffer = _context->createBuffer(RT_BUFFER_INPUT);
    light_buffer->setFormat(RT_FORMAT_USER);
    light_buffer->setElementSize(sizeof(CS123SceneLightData));
    light_buffer->setSize(sizeof(lights)/sizeof(lights[0]));
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
                                            
//Graph structure:
// TopGroup(noaccel)->TopTransform(Identity)->ShapeGroup(Sbvh)->AllTransforms(shape specific)
// When we need to rotate or scale the objects then we will change the top transform
void RayScene::createGeometry()
{ 
    //Load Each Basic Shape
    std::string cube_ptx(ptxpath("tutorial","cube.cu"));
    Program cube_bounds = _context->createProgramFromPTXFile(cube_ptx, "cube_bounds");
    Program cube_intersect = _context->createProgramFromPTXFile(cube_ptx, "cube_intersect");
    Geometry cube = _context->createGeometry();
    cube->setPrimitiveCount(1u);
    cube->setBoundingBoxProgram(cube_bounds);
    cube->setIntersectionProgram(cube_intersect);

    std::string cone_ptx(ptxpath("tutorial","cone.cu"));
    Program cone_bounds = _context->createProgramFromPTXFile(cone_ptx, "cone_bounds");
    Program cone_intersect = _context->createProgramFromPTXFile(cone_ptx, "cone_intersect");
    Geometry cone = _context->createGeometry();
    cone->setPrimitiveCount(1u);
    cone->setBoundingBoxProgram(cone_bounds);
    cone->setIntersectionProgram(cone_intersect);

    std::string sphere_ptx(ptxpath("tutorial","sphere.cu"));
    Program sphere_bounds = _context->createProgramFromPTXFile(sphere_ptx, "sphere_bounds");
    Program sphere_intersect = _context->createProgramFromPTXFile(sphere_ptx, "sphere_intersect");
    Geometry sphere = _context->createGeometry();
    sphere->setPrimitiveCount(1u);
    sphere->setBoundingBoxProgram(sphere_bounds);
    sphere->setIntersectionProgram(sphere_intersect);

    std::string cylinder_ptx(ptxpath("tutorial","cylinder.cu"));
    Program cylinder_bounds = _context->createProgramFromPTXFile(cylinder_ptx, "cylinder_bounds");
    Program cylinder_intersect = _context->createProgramFromPTXFile(cylinder_ptx, "cylinder_intersect");
    Geometry cylinder = _context->createGeometry();
    cylinder->setPrimitiveCount(1u);
    cylinder->setBoundingBoxProgram(cylinder_bounds);
    cylinder->setIntersectionProgram(cylinder_intersect);

    //Load Image
    std::vector<RTtransform> transes;
    std::vector<SimplePrimitive>* prims = _dp.getPrimLists();
    CS123SceneGlobalData gdata = _dp.getGlobalData();

    for(int i=0; i<(*prims).size(); i++){
        SimplePrimitive cprim = (*prims).back();
        (*prims).pop_back();

        //make transformation
        RTtransform currt;
        rtTransformCreate(_context, &currt);
        Matrix4x4 om = cprim.transMat;
        const float tm[16] = {om.a,om.b,om.c,om.d,
                              om.e,om.f,om.g,om.h,
                              om.i,om.j,om.k,om.l,
                              om.m,om.n,om.o,om.p };
        rtTransformSetMatrix(currt, 0, tm, 0);

        //make material
        CS123SceneMaterial smatl = cprim.material;
        Material cmatl = _context->createMaterial();
        Program closestHitP = _context->createProgramFromPTXFile(_ptx_path, "closest_hit");
        cmatl->setClosestHitProgram(0,closestHitP);
        /*if(useShadows){
            Program anyHitP = _context->createProgramFromPTXFile(_ptx_path, "any_hit_shadow");
            cmatl->setAnyHitProgram(1,anyHitP);
        }*/
        cmatl["cDiffuse"]->setFloat(gdata.kd*smatl.cDiffuse.r,gdata.kd*smatl.cDiffuse.g,gdata.kd*smatl.cDiffuse.b);
        cmatl["cAmbient"]->setFloat(gdata.ka*smatl.cAmbient.r,gdata.ka*smatl.cAmbient.g,gdata.ka*smatl.cAmbient.b);
        cmatl["cSpecular"]->setFloat(gdata.ks*smatl.cSpecular.r,gdata.ks*smatl.cSpecular.g,gdata.ks*smatl.cSpecular.b);
        cmatl["blend"]->setFloat(smatl.blend);
        cmatl["shininess"]->setFloat(smatl.shininess);
        cmatl["ior"]->setFloat(smatl.ior);
        //we'll do texture when we get the rest working

        GeometryInstance cinstance;
        switch(cprim.type) {
            case PRIMITIVE_CUBE:
                cinstance = _context->createGeometryInstance(cube, &cmatl, &cmatl+1);
                break;                                                      
            case PRIMITIVE_CONE:
                cinstance = _context->createGeometryInstance(cone, &cmatl, &cmatl+1);
                break;
            case PRIMITIVE_CYLINDER:
                cinstance = _context->createGeometryInstance(cylinder, &cmatl, &cmatl+1);
                break;
            case PRIMITIVE_SPHERE:
                cinstance = _context->createGeometryInstance(sphere, &cmatl, &cmatl+1);
                break;
            default:
                cinstance = _context->createGeometryInstance(cube, &cmatl, &cmatl+1);
                break;
        }

        GeometryGroup cgroup = _context->createGeometryGroup();
        cgroup->setChildCount(1);
        cgroup->setChild(0,cinstance);

        rtTransformSetChild(currt, cgroup);
        transes.push_back(currt);
    }

    //Create Scene Group and add all transforms to it
    rtGroup sceneGroup;
    rtGroupCreate(_context, &sceneGroup);
    rtGroupSetChildCount(sceneGroup, transes.size());

    rtAcceleration sceneAcceleration;
    rtAccelerationCreate(_context, &sceneAcceleration);
    rtAccelerationSetBuilder(sceneAcceleration,"Sbvh");
    rtAccelerationSetTraverser(sceneAcceleration,"bvhCompact");
    rtGroupSetAcceleration(sceneGroup,sceneAcceleration);

    for(int i=0; i<transes.size(); i++)
        rtGroupSetChild(sceneGroup, transes[i]);

    //Create the Top Group and its transform
    rtGroup topGroup;
    rtGroupCreate(_context, &topGroup);
    rtGroupSetChildCount(topGroup, 1);
    
    rtAcceleration topAcceleration;
    rtAccelerationCreate(_context, &topAcceleration);
    rtAccelerationSetBuilder(topAcceleration, "NoAccel");
    rtAccelerationSetTraverser(topAcceleration, "NoAccel");
    rtGroupSetAcceleration(topGroup, topAcceleration);

    RTtransform topTrans;
    rtTransformCreate(_context, &topTrans);
    const float tm[16] = {1,0,0,0,
                          0,1,0,0,
                          0,0,1,0,
                          0,0,0,1 };
    rtTransformSetMatrix(topTrans,0,tm,0);
    rtTransformSetChild(topTrans,sceneGroup);

    rtGroupSetChild(topGroup,topTrans);

    //Finish up
    _context["top_object"]->set(topGroup);
    _context["top_shadower"]->set(topGroup);
}


