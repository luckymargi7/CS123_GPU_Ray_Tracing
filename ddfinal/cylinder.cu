#include <optix_world.h>
#include <float.h>

using namespace optix;

rtDeclareVariable(float4,  cylinder, , );

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
void intersect_cylinder(void)
{
    float3 points[2];
    points[0] = float3(0, 0.5, 0);
    points[1] = float3(0,-0.5, 0);
    
    float3 normals[2];
    normals[0] = float3( 0, 1, 0);
    normals[1] = float3( 0,-1, 0);
    
    float3 eye = ray.origin;
    float3 dir = ray.direction;
    float final_t = FLT_MAX;
    int final_i = 0;

    for (int i = 0; i < 2; i++) {
        float3 p = points[i];
        float3 n = normals[i];
        float t = intersectPlane(eye, dir, p, n);

        if (rtPotentialIntersection(t) && t < final_t){
            final_t = t;
            final_i = i;
        }
    }
 
    //Calculate t's
    float a = dir.x*dir.x + dir.z*dir.z;
    float b = 2*eye.x*dir.x + 2*eye.z*dir.z;
    float c = eye.x*eye.x + eye.z*eye.z + 0.25;

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
            float3 normal = float3(interPt.x, 0, interPt.z);
            shading_normal = geometric_normal = normalize(normal);
        }

        rtReportIntersection(0);
    }
    
    
}


RT_PROGRAM void intersect(int primIdx)
{
  intersect_cylinder<false>();
}


RT_PROGRAM void robust_intersect(int primIdx)
{
  intersect_cylinder<true>();
}


RT_PROGRAM void bounds (int, float result[6])
{
  const float3 cen = make_float3( cylinder );
  const float3 rad = make_float3( cylinder.w );

  optix::Aabb* aabb = (optix::Aabb*)result;
  
  if( rad.x > 0.0f  && !isinf(rad.x) ) {
    aabb->m_min = cen - rad;
    aabb->m_max = cen + rad;
  } else {
    aabb->invalidate();
  }
}

