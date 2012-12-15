/* The scene header which is also quite similar to the support version
   the header file.
   */

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

#include "RayScene.h"
#include "commonStructs.h"
#include "DataProcessor.h"

using namespace std;
using namespace optix;

#define FLT_MAX         1e30;

__device__ __inline__ float3 exp( const float3& x )
{
  return make_float3(exp(x.x), exp(x.y), exp(x.z));
}

__device__ __inline__ float3 mix( float3 a, float3 b, float x )
{
  return a*(1-x) + b*x;
}

struct RayData_lighting{
  float3 result;
  float  importance;
  int depth;
};

struct RayData_shadow
{
  float3 attenuation;
};

class RayScene : public Scene
{
    public:
        RayScene(const std::string& texture_path, DataProcessor dp)
            : Scene(), _width(1080u), _height(720u), texture_path( texture_path ), _dp(dp)
        {}
  
        // From Scene
        void   initScene( InitialCameraData& camera_data );
        void   trace( const RayGenCameraData& camera_data );
        void   doResize( unsigned int width, unsigned int height );
        void   setDimensions( const unsigned int w, const unsigned int h ) { _width = w; _height = h; }
        Buffer getOutputBuffer();

    private:
        string texpath( const std::string& base );
        void createGeometry();

        unsigned int _width;
        unsigned int _height;
        DataProcessor _dp;
        string   texture_path;
        string  _ptx_path;
};

#endif //for __SCENE_H__
