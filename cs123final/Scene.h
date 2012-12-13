#ifndef __SCENE_H__
#define __SCENE_H__

#include <optix.h>
#include <optix_math.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>

#include <iostream>
#include <GLUTDisplay.h>
#include <ImageLoader.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <math.h>

#include "Scene.h"
#include "commonStructs.h"

using namespace std;
using namespace optix;

#define FLT_MAX         1e30;

__device__ __inline__ float3 exp( const float3& x )
{
  return make_float3(exp(x.x), exp(x.y), exp(x.z));
}

__device__ __inline__ float step( float min, float value )
{
  return value<min?0:1;
}

__device__ __inline__ float3 mix( float3 a, float3 b, float x )
{
  return a*(1-x) + b*x;
}

__device__ __inline__ float3 schlick( float nDi, const float3& rgb )
{
  float r = fresnel_schlick(nDi, 5, rgb.x, 1);
  float g = fresnel_schlick(nDi, 5, rgb.y, 1);
  float b = fresnel_schlick(nDi, 5, rgb.z, 1);
  return make_float3(r, g, b);
}

__device__ __inline__ uchar4 make_color(const float3& c)
{
    return make_uchar4( static_cast<unsigned char>(__saturatef(c.z)*255.99f),  /* B */
                        static_cast<unsigned char>(__saturatef(c.y)*255.99f),  /* G */
                        static_cast<unsigned char>(__saturatef(c.x)*255.99f),  /* R */
                        255u);                                                 /* A */
}

struct PerRayData_radiance
{
  float3 result;
  float  importance;
  int depth;
};

struct PerRayData_shadow
{
  float3 attenuation;
};


class Scene : public SampleScene
{
    public:
        Scene(const std::string& texture_path)
            : SampleScene(), _width(1080u), _height(720u), texture_path( texture_path )
        {}
  
        // From SampleScene
        void   initScene( InitialCameraData& camera_data );
        void   trace( const RayGenCameraData& camera_data );
        void   doResize( unsigned int width, unsigned int height );
        void   setDimensions( const unsigned int w, const unsigned int h ) { _width = w; _height = h; }
        Buffer getOutputBuffer();
        float rand_range(float min, float max);

    private:
        string texpath( const std::string& base );
        void createGeometry();

        unsigned int _width;
        unsigned int _height;
        string   texture_path;
        string  _ptx_path;
};

#endif //for __SCENE_H__
