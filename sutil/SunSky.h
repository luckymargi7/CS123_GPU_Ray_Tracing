
// Copyright NVIDIA Corporation 2009
// TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
// *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
// BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES

#pragma once

#include <sutil.h>
#include <optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>


//------------------------------------------------------------------------------
//
// Implements the Preetham analytic sun/sky model ( Preetham, SIGGRAPH 99 )
//
//------------------------------------------------------------------------------
class SUTILCLASSAPI PreethamSunSky
{
protected:
  typedef optix::float3 float3;
public:
  SUTILAPI PreethamSunSky();

  SUTILAPI void setSunTheta( float sun_theta )            { _sun_theta = sun_theta; _dirty = true; }
  SUTILAPI void setSunPhi( float sun_phi)                 { _sun_phi = sun_phi;     _dirty = true; }
  SUTILAPI void setTurbidity( float turbidity )           { _turbidity = turbidity; _dirty = true; }

  SUTILAPI void setUpDir( const float3& up )       { _up = up; _dirty = true; }
  SUTILAPI void setOvercast( float overcast )             { _overcast = overcast;    }
  
  SUTILAPI float  getSunTheta()                           { return _sun_theta; }
  SUTILAPI float  getSunPhi()                             { return _sun_phi;   }
  SUTILAPI float  getTurbidity()                          { return _turbidity; }

  SUTILAPI float         getOvercast()                    { return _overcast;              }
  SUTILAPI float3 getUpDir()                       { return _up;                    }
  SUTILAPI float3 getSunDir()                      { preprocess(); return _sun_dir; }


  // Query the sun color at current sun position and air turbidity ( kilo-cd / m^2 )
  SUTILAPI float3  sunColor();

  // Query the sky color in a given direction ( kilo-cd / m^2 )
  SUTILAPI float3  skyColor( const float3 & direction, bool CEL = false );
  
  // Sample the solid angle subtended by the sun at its current position
  SUTILAPI float3 sampleSun()const;

  // Set precomputed Preetham model variables on the given context:
  //   c[0-4]          : 
  //   inv_divisor_Yxy :
  //   sun_dir         :
  //   sun_color       :
  //   overcast        :
  //   up              :
  SUTILAPI void setVariables( optix::Context context );


private:
  void          preprocess();
  float3 calculateSunColor();


  // Represents one entry from table 2 in the paper
  struct Datum  
  {
    float wavelength;
    float sun_spectral_radiance;
    float k_o;
    float k_wa;
  };
  
  static const float cie_table[38][4];          // CIE spectral sensitivy curves
  static const Datum data[38];                  // Table2


  // Calculate absorption for a given wavelength of direct sunlight
  static float calculateAbsorption( float sun_theta, // Sun angle from zenith
                                    float m,         // Optical mass of atmosphere
                                    float lambda,    // light wavelength
                                    float turbidity, // atmospheric turbidity
                                    float k_o,       // atten coeff for ozone
                                    float k_wa );    // atten coeff for h2o vapor
  
  // Unit conversion helpers
  static float3 XYZ2rgb( const float3& xyz );
  static float3 Yxy2XYZ( const float3& Yxy );
  static float  rad2deg( float rads );

  // Input parameters
  float  _sun_theta;
  float  _sun_phi;
  float  _turbidity;
  float  _overcast;
  float3 _up;
  float3 _sun_color;
  float3 _sun_dir;

  // Precomputation results
  bool   _dirty;
  float3 _c0;
  float3 _c1;
  float3 _c2;
  float3 _c3;
  float3 _c4;
  float3 _inv_divisor_Yxy;
};



  
inline float PreethamSunSky::rad2deg( float rads )
{
  return rads * 180.0f / static_cast<float>( M_PI );
}


inline optix::float3 PreethamSunSky::Yxy2XYZ( const optix::float3& Yxy )
{
  return optix::make_float3(  Yxy.y * ( Yxy.x / Yxy.z ),
                              Yxy.x,
                              ( 1.0f - Yxy.y - Yxy.z ) * ( Yxy.x / Yxy.z ) );
}


inline optix::float3 PreethamSunSky::XYZ2rgb( const optix::float3& xyz)
{
  const float R = optix::dot( xyz, optix::make_float3(  3.2410f, -1.5374f, -0.4986f ) );
  const float G = optix::dot( xyz, optix::make_float3( -0.9692f,  1.8760f,  0.0416f ) );
  const float B = optix::dot( xyz, optix::make_float3(  0.0556f, -0.2040f,  1.0570f ) );
  return optix::make_float3( R, G, B );
}
