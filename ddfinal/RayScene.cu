/*
   I updated this to be specific to our needs as much as possible
   but basically everything is the same because this was a pretty
   much bare bones implementation of only the absolutely necessary
   things already.  (Much of it was taken from tutorial1.cu)
*/

#include "RayScene.h"

rtDeclareVariable(float3, shading_normal, attribute shading_normal, );//regular normal
rtDeclareVariable(RayData_lighting, rd_lighting, rtPayload, );//ray struct defined in RayScene.h                                                                 
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );//current t dist
rtDeclareVariable(uint2, currRay_index, rtLaunchIndex, );//TODO

//used to create the ray
rtDeclareVariable(unsigned int, lighting_ray_type, , );
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_object, , );

//Define Camera
rtDeclareVariable(float3, eye, , );
rtDeclareVariable(float3, U, , );
rtDeclareVariable(float3, V, , );
rtDeclareVariable(float3, W, , );
rtDeclareVariable(float3, fail_color, , );
rtBuffer<uchar4, 2> output_buffer;

RT_PROGRAM void camera(){
    size_t2 screen = output_buffer.size();

    float2 d = make_float2(currRay_index) / make_float2(screen)*2.f-1.f;
    float3 ray_origin = eye;
    float3 ray_direction = normalized(d.x*U + d.y*V + W);

    optix::Ray ray(ray_origin, ray_direction, lighting_ray_type, scene_epsilon);
    RayData_lighting rd;
    rd.importance = 1.f;
    rd.depth = 0;

    rtTrace(top_object, ray, rd);

    output_buffer[currRay_index] = make_color( rd.result);
}

//Miss Method
rtDeclareVariable(float3, bg_color, , );
RT_PROGRAM void miss(){
    rd_lighting.result = bg_color;
}

//Shading
rtDeclareVariable(float3, Ka, , );
rtDeclareVariable(float3, Kd, , );
rtDeclareVariable(float3, ambient_light_color, , );

RT_PROGRAM void closest_hit_lighting(){
    float3 color = Ka * ambient_light_color;

    rd_lighting.result = color;
}

//Use failure color (if anything goes wrong)
RT_PROGRAM void exception(){
    output_buffer[currRay_index] = make_color(failure_color);
}




