#pragma once

#include <string>
#include <optixu/optixpp_namespace.h>
#include <sutil.h>
#include "../ddfinal/Scene.h"

class Mouse;
class PinholeCamera;


//-----------------------------------------------------------------------------
// 
// GLUTDisplay 
//
//-----------------------------------------------------------------------------

class SUTILCLASSAPI GLUTDisplay
{
public:
  enum contDraw_E { CDNone=0, CDProgressive=1, CDAnimated=2, CDBenchmark=3, CDBenchmarkTimed=4 };

  SUTILAPI static void init( int& argc, char** argv );
  SUTILAPI static void run( const std::string& title, Scene* scene, contDraw_E continuous_mode = CDNone );
  SUTILAPI static void printUsage();

  SUTILAPI static void setTextColor( const optix::float3& c )
  { _text_color = c; }

  SUTILAPI static void setTextShadowColor( const optix::float3& c )
  { _text_shadow_color = c; }

  SUTILAPI static void setProgressiveDrawingTimeout( double t )
  { _progressive_timeout = t; }

  SUTILAPI static contDraw_E getContinuousMode() { return _app_continuous_mode; }
  SUTILAPI static void setContinuousMode(contDraw_E continuous_mode);
  SUTILAPI static void setCamera(Scene::InitialCameraData& camera_data);

  SUTILAPI static bool isBenchmark() { return _cur_continuous_mode == CDBenchmark || _cur_continuous_mode == CDBenchmarkTimed ||
    _app_continuous_mode == CDBenchmark || _app_continuous_mode == CDBenchmarkTimed; }

  // Make sure you only call this from the callback functions found in Scene:
  // initScene, trace, getOutputBuffer, cleanUp, resize, doResize, and keyPressed.
  SUTILAPI static void postRedisplay();

  SUTILAPI static void setUseSRGB(bool enabled) { _use_sRGB = enabled; }

private:

  // Draw text to screen at window pos x,y.  To make this public we will need to have
  // a public helper that caches the text for use in the display func
  static void drawText( const std::string& text, float x, float y, void* font );

  // Do the actual rendering to the display
  static void displayFrame();

  // Executed if _benchmark_no_display is true:
  //   - Renders '_warmup_frames' frames without timing
  //   - Renders '_timed_frames' frames for benchmarking
  //   OR
  //   - Renders '_warmup_time' seconds without timing
  //   - Renders '_timed_time' seconds for benchmarking
  //   - Prints results to screen using sutilPrintBenchmark.
  //   - If _save_frames_to_file is set, prints one copy of frame to file:
  //     ${_title}.ppm. _title is set to _save_frames_basename if non-empty.
  static void runBenchmarkNoDisplay();
  
  // Set the current continuous drawing mode, while preserving the app's choice.
  static void setCurContinuousMode(contDraw_E continuous_mode);

  // Cleans up the rendering context and quits.  If there wasn't error cleaning up, the 
  // return code is passed out, otherwise 2 is used as the return code.
  static void quit(int return_code=0);

  // Glut callbacks
  static void idle();
  static void display();
  static void keyPressed(unsigned char key, int x, int y);
  static void mouseButton(int button, int state, int x, int y);
  static void mouseMotion(int x, int y);
  static void resize(int width, int height);

  static Mouse*         _mouse;
  static PinholeCamera* _camera;
  static Scene*   _scene;

  static double         _last_frame_time;
  static unsigned int   _last_frame_count;
  static unsigned int   _frame_count;

  static bool           _display_fps;
  static double         _fps_update_threshold;
  static char           _fps_text[32];
  static optix::float3  _text_color;
  static optix::float3  _text_shadow_color;

  static bool           _print_mem_usage;

  static contDraw_E     _app_continuous_mode;
  static contDraw_E     _cur_continuous_mode;
  static bool           _display_frames;
  static bool           _save_frames_to_file;
  static std::string    _save_frames_basename;

  static std::string    _camera_pose;

  static int            _initial_window_width;
  static int            _initial_window_height;

  static int            _old_window_height;
  static int            _old_window_width;
  static int            _old_window_x;
  static int            _old_window_y;

  static unsigned int   _texId;
  static bool           _sRGB_supported;
  static bool           _use_sRGB;
  static bool           _initialized;

  static bool           _benchmark_no_display;
  static unsigned int   _warmup_frames;
  static unsigned int   _timed_frames;
  static double         _warmup_start; // time when the warmup started
  static double         _warmup_time; // used instead of _warmup_frames to specify a time to run instead of number of frames to run
  static double         _benchmark_time; // like _warmup_time, but for the time to benchmark
  static unsigned int   _benchmark_frame_start;
  static double         _benchmark_frame_time;
  static std::string    _title;

  static double         _progressive_timeout; // how long to do continuous rendering for progressive refinement (ignored when benchmarking or animating)
  static double         _start_time; // time since continuous rendering last started

  static int            _num_devices;
};
