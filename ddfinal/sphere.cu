#include <optix_world.h>

using namespace optix;

rtDeclareVariable(float4,  sphere, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

template<bool use_robust_method>
__device__
void intersect_sphere(void)
{

    //Because of the transform groups, the ray should be transformed to hit the uniform sphere.
    float3 eye = ray.origin;
    float3 dir = ray.direction;
    
    float final_t = FLT_MAX; //Calculate t's
    float a = dot(dir, dir);
    float b = 2*eye.x*dir.x + 2*eye.y*dir.y + 2*eye.z*dir.z;
    float c = dot(eye, eye);

    float d = b*b - 4*a*c;
    float t0 = t1 = -1;
    if (d >= 0) {
        t0 = (-1*b + sqrtf(d))/(2*a);
        t1 = (-1*b - sqrtf(d))/(2*a);
    }
    
    if (rtPotentialIntersection(t0) && t0 < final_t) 
        final_t = t0;
    
    if (rtPotentialIntersection(t1) && t1 < final_t) 
        final_t = t1;
    
    if(rtPotentialIntersection(final_t))  {
        float3 interPt = eye + dir*t0;
        shading_normal = geometric_normal = normalize(interPt);
        rtReportIntersection(0);
    }

}

float3 normalize(float3 v) {
    return rsqrt(dot(v,v))*v;
}

RT_PROGRAM void intersect(int primIdx)
{
  intersect_sphere<false>();
}


RT_PROGRAM void robust_intersect(int primIdx)
{
  intersect_sphere<true>();
}


RT_PROGRAM void bounds (int, float result[6])
{
  const float3 cen = make_float3( sphere );
  const float3 rad = make_float3( sphere.w );

  optix::Aabb* aabb = (optix::Aabb*)result;
  
  if( rad.x > 0.0f  && !isinf(rad.x) ) {
    aabb->m_min = cen - rad;
    aabb->m_max = cen + rad;
  } else {
    aabb->invalidate();
  }
}

