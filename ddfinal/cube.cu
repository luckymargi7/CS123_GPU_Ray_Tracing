#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include <float.h>

using namespace optix;

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float3, texcoord, attribute texcoord, ); 
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 

__device__ bool withinConstraint(float3 eye, float3 dir, float t) {
    float3 interPt = eye + dir*t;

    return fabs(interPt.x) <= 0.5 &&fabs(interPt.y) <= 0.5 && fabs(interPt.y) <= 0.5;
}

__device__ float instersectPlane(float3 eye, float3 dir, float3 point, float3 normal) {
    //Calculate t's
    float t = (dot(normal, point) - dot(eye, normal))/dot(dir, normal);
    
    return (withinConstraint(eye, dir, t)) ? t : -1;
        
}

RT_PROGRAM void cube_intersect(int)
{
    float3 points[6];
    points[0] = float3( 0.5, 0, 0);
    points[1] = float3(-0.5, 0, 0);
    points[2] = float3(0, 0.5, 0);
    points[3] = float3(0,-0.5, 0);
    points[4] = float3(0, 0, 0.5);
    points[5] = float3(0, 0,-0.5);
    
    float3 normals[6];
    normals[0] = float3( 1, 0, 0);
    normals[1] = float3(-1, 0, 0);
    normals[2] = float3( 0, 1, 0);
    normals[3] = float3( 0,-1, 0);
    normals[4] = float3( 0, 0, 1);
    normals[5] = float3( 0, 0,-1);
    
    float3 eye = ray.origin;
    float3 dir = ray.direction;
    float final_t = 0;
    int final_i = 0;

    for (int i = 0; i < 6; i++) {
        float3 p = points[i];
        float3 n = normals[i];
        float t = intersectPlane(eye, dir, p, n);

        if ((i == 0) || (rtPotentialIntersection(t) && t < final_t)){
            final_t = t;
            final_i = i;
        }
    }
 
    if (rtPotentialIntersection(final_t)) {
        shading_normal = geometric_normal = normals[final_i];
        rtReportIntersection(0);
    }

}

RT_PROGRAM void cube_bounds (int, float result[6])
{
    const float3 cubemin = make_float3(-.5,-.5,-.5);
    const float3 cubemax = make_float3(.5,.5,.5);
    optix::Aabb* aabb = (optix::Aabb*)result;
    aabb->set(cubemin, cubemax);
}
