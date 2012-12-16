#include <optix.h>
#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include <float.h>

using namespace optix;

rtDeclareVariable(float4,  cone, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );


__device__ bool withinConstraint(float3 eye, float3 dir, float t) {
    float3 interPt = eye + dir*t;

    float dist = interPt.x*interPt.x + interPt.z*interPt.z;

    return dist <= 0.25;
}

__device__ float instersectPlane(float3 eye, float3 dir, float3 point, float3 normal) {
    //Calculate t's
    float t = (dot(normal, point) - dot(eye, normal))/dot(dir, normal);
    
    return (withinConstraint(eye, dir, t)) ? t : -1;
        
}

template<bool use_robust_method>
__device__
void intersect_cone(void)
{
    float3 point = float3(0, -0.5, 0);
    float3 normal = float3( 0,-1, 0);
    
    float3 eye = ray.origin;
    float3 dir = ray.direction;
    float final_t = FLT_MAX;
    int final_i = 0;

    float t = intersectPlane(eye, dir, point, normal);

    if (rtPotentialIntersection(t) && t < final_t)
        final_t = t;
 
    //Calculate t's
    float a = dot(dir, dir);
    float b = 2*eye.x*dir.x + 2*eye.y*dir.y + 2*eye.z*dir.z;
    float c = dot(eye, eye);

    float d = b*b - 4*a*c;
    float t0 = t1 = -1;
    if (d >= 0) {
        t0 = (-1*b + sqrtf(d))/(2*a);
        t1 = (-1*b - sqrtf(d))/(2*a);
    }

    if (rtPotentialIntersection(t0) && t0 < final_t) {
        final_t = t0;
        final_i = -1;
    }
    
    if (rtPotentialIntersection(t1) && t1 < final_t) {
        final_t = t1;
        final_i = -1;
    }

    if (rtPotentialIntersection(final_t)){
        if (final_i != -1) {
            shading_normal = geometric_normal = normals[final_i];
        } else {
            float3 interPt = eye + dir(final_t);
            float normal_y = sqrtf(interPt.x*interPt.x + interPt.z*interPt.z)/2.0;
            float3 normal = float3(interPt.x, normal_y, interPt.z);
            shading_normal = geometric_normal = normalize(normal);
        }

        rtReportIntersection(0);
    }
}

float3 normalize(float3 v) {
    return rsqrt(dot(v,v))*v;
}

RT_PROGRAM void intersect(int primIdx)
{
  intersect_cone<false>();
}


RT_PROGRAM void robust_intersect(int primIdx)
{
  intersect_cone<true>();
}


RT_PROGRAM void bounds (int, float result[6])
{
  const float3 cen = make_float3( cone );
  const float3 rad = make_float3( cone.w );

  optix::Aabb* aabb = (optix::Aabb*)result;
  
  if( rad.x > 0.0f  && !isinf(rad.x) ) {
    aabb->m_min = cen - rad;
    aabb->m_max = cen + rad;
  } else {
    aabb->invalidate();
  }
}

