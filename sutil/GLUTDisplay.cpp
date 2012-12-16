
/*
 * Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */


#if defined(__APPLE__)
#  include <GLUT/glut.h>
#  define GL_FRAMEBUFFER_SRGB_EXT           0x8DB9
#  define GL_FRAMEBUFFER_SRGB_CAPABLE_EXT   0x8DBA
#else
#  include <GL/glew.h>
#  if defined(_WIN32)
#    include <GL/wglew.h>
#  endif
#  include <GL/glut.h>
#endif

#include <GLUTDisplay.h>
#include <Mouse.h>
#include <DeviceMemoryLogger.h>

#include <optixu/optixu_math_stream_namespace.h>

#include <iostream>
#include <cstdlib>
#include <cstdio> //sprintf
#include <sstream>

using namespace optix;

//-----------------------------------------------------------------------------
// 
// GLUTDisplay class implementation 
//-----------------------------------------------------------------------------

Mouse*         GLUTDisplay::_mouse                = 0;
PinholeCamera* GLUTDisplay::_camera               = 0;
Scene*   GLUTDisplay::_scene                = 0;

double         GLUTDisplay::_last_frame_time      = 0.0;
unsigned int   GLUTDisplay::_last_frame_count     = 0;
unsigned int   GLUTDisplay::_frame_count          = 0;

bool           GLUTDisplay::_display_fps          = true;
double         GLUTDisplay::_fps_update_threshold = 0.5;
char           GLUTDisplay::_fps_text[32];
float3         GLUTDisplay::_text_color           = make_float3( 0.95f );
float3         GLUTDisplay::_text_shadow_color    = make_float3( 0.10f );

bool           GLUTDisplay::_print_mem_usage      = false;

GLUTDisplay::contDraw_E GLUTDisplay::_app_continuous_mode = CDNone;
GLUTDisplay::contDraw_E GLUTDisplay::_cur_continuous_mode = CDNone;

bool           GLUTDisplay::_display_frames       = true;
bool           GLUTDisplay::_save_frames_to_file  = false;
std::string    GLUTDisplay::_save_frames_basename = "";

std::string    GLUTDisplay::_camera_pose          = "";

int            GLUTDisplay::_initial_window_width  = -1;
int            GLUTDisplay::_initial_window_height = -1;

int            GLUTDisplay::_old_window_height    = -1;
int            GLUTDisplay::_old_window_width     = -1;
int            GLUTDisplay::_old_window_x         = -1;
int            GLUTDisplay::_old_window_y         = -1;

unsigned int   GLUTDisplay::_texId                = 0;
bool           GLUTDisplay::_sRGB_supported       = false;
bool           GLUTDisplay::_use_sRGB             = false;

bool           GLUTDisplay::_initialized          = false;
bool           GLUTDisplay::_benchmark_no_display = false;
unsigned int   GLUTDisplay::_warmup_frames        = 50u;
unsigned int   GLUTDisplay::_timed_frames         = 100u;
double         GLUTDisplay::_warmup_start         = 0;
double         GLUTDisplay::_warmup_time          = 10.0;
double         GLUTDisplay::_benchmark_time       = 10.0;
unsigned int   GLUTDisplay::_benchmark_frame_start= 0;
double         GLUTDisplay::_benchmark_frame_time = 0;
std::string    GLUTDisplay::_title                = "";

double         GLUTDisplay::_progressive_timeout   = -1.;
double         GLUTDisplay::_start_time           = 0.0;

int            GLUTDisplay::_num_devices          = 0;

inline void removeArg( int& i, int& argc, char** argv ) 
{
  char* disappearing_arg = argv[i];
  for(int j = i; j < argc-1; ++j) {
    argv[j] = argv[j+1];
  }
  argv[argc-1] = disappearing_arg;
  --argc;
  --i;
}

void GLUTDisplay::printUsage()
{
  std::cerr
    << "Standard options:\n"
    << "  -d  | --dim=<width>x<height>               Set image dimensions\n"
    << "  -D  | --num-devices=<num_devices>          Set desired number of GPUs\n"
    << "  -p  | --pose=\"[<eye>][<lookat>][<up>]vfov\" Camera pose, e.g. --pose=\"[0,0,-1][0,0,0][0,1,0]45.0\"\n"
    << "  -s  | --save-frames[=<file_basename>]      Save each frame to frame_XXXX.ppm or file_basename_XXXX.ppm\n"
    << "  -N  | --no-display                         Don't display the image to the GLUT window\n"
    << "  -M  | --mem-usage                          Print memory usage after every frame\n"
    << "  -b  | --benchmark[=<w>x<t>]                Render and display 'w' warmup and 't' timing frames, then exit\n"
    << "  -bb | --timed-benchmark=<w>x<t>            Render and display 'w' warmup and 't' timing seconds, then exit\n"
    << "  -B  | --benchmark-no-display=<w>x<t>       Render 'w' warmup and 't' timing frames, then exit\n"
    << "  -BB | --timed-benchmark-no-display=<w>x<t> Render 'w' warmup and 't' timing seconds, then exit\n"
    << std::endl;

  std::cerr
    << "Standard mouse interaction:\n"
    << "  left mouse           Camera Rotate/Orbit (when interactive)\n"
    << "  middle mouse         Camera Pan/Truck (when interactive)\n"
    << "  right mouse          Camera Dolly (when interactive)\n"
    << "  right mouse + shift  Camera FOV (when interactive)\n"
    << std::endl;

  std::cerr
    << "Standard keystrokes:\n"
    << "  q Quit\n"
    << "  f Toggle full screen\n"
    << "  r Toggle continuous mode (progressive refinement, animation, or benchmark)\n"
    << "  R Set progressive refinement to never timeout and toggle continuous mode\n"
    << "  b Start/stop a benchmark\n"
    << "  d Toggle frame rate display\n"
    << "  s Save a frame to 'out.ppm'\n"
    << "  m Toggle memory usage printing\n"
    << "  c Print camera pose\n"
    << std::endl;
}

void GLUTDisplay::init( int& argc, char** argv )
{
  _initialized = true;

  for (int i = 1; i < argc; ++i ) {
    std::string arg( argv[i] );
    if ( arg == "-s" || arg == "--save-frames" ) {
      _save_frames_to_file = true;
      removeArg( i, argc, argv );
    } else if ( arg.substr( 0, 3 ) == "-s=" || arg.substr( 0, 14 )  == "--save-frames=" ) {
      _save_frames_to_file = true;
      _save_frames_basename = arg.substr( arg.find_first_of( '=' ) + 1 );
      removeArg( i, argc, argv );
    } else if ( arg == "-N" || arg == "--no-display" ) {
      _display_frames = false;
      removeArg( i, argc, argv );
    } else if ( arg == "-M" || arg == "--mem-usage" ) {
      _print_mem_usage = true;
      removeArg( i, argc, argv );
    } else if ( arg.substr( 0, 3 ) == "-p=" || arg.substr( 0, 7 ) == "--pose=" ) {
      _camera_pose = arg.substr( arg.find_first_of( '=' ) + 1 );
      std::cerr << " got <<" << _camera_pose << ">>" << std::endl;
      removeArg( i, argc, argv );
    } else if( arg.substr( 0, 3) == "-D=" || arg.substr( 0, 14 ) == "--num-devices=" ) {
      std::string dims_arg = arg.substr( arg.find_first_of( '=' ) + 1 );
      _num_devices = atoi(dims_arg.c_str());
      if ( _num_devices < 1 ) {
        std::cerr << "Invalid num devices: '" << dims_arg << "'" << std::endl;
        printUsage();
        quit(1);
      }
      removeArg( i, argc, argv );
    } else if( arg.substr( 0, 3) == "-d=" || arg.substr( 0, 6 ) == "--dim=" ) {
      std::string dims_arg = arg.substr( arg.find_first_of( '=' ) + 1 );
      unsigned int width, height;
      if ( sutilParseImageDimensions( dims_arg.c_str(), &width, &height ) != RT_SUCCESS ) {
        std::cerr << "Invalid window dimensions: '" << dims_arg << "'" << std::endl;
        printUsage();
        quit(1);
      }
      _initial_window_width = width;
      _initial_window_height = height;
      removeArg( i, argc, argv );
    } else if ( arg == "-b" || arg == "--benchmark" ) {
      _app_continuous_mode = CDBenchmark;
      removeArg( i, argc, argv );
    } else if ( arg == "-B" || arg == "--benchmark-no-display" ) {
      _cur_continuous_mode = CDBenchmark;
      _benchmark_no_display = true;
      removeArg( i, argc, argv );
    } else if ( arg == "-bb" || arg == "--timed-benchmark" ) {
      _app_continuous_mode = CDBenchmarkTimed;
      removeArg( i, argc, argv );
    } else if ( arg == "-BB" || arg == "--timed-benchmark-no-display" ) {
      _cur_continuous_mode = CDBenchmarkTimed;
      _benchmark_no_display = true;
      removeArg( i, argc, argv );
    } else if ( arg.substr( 0, 3 ) == "-b=" || arg.substr( 0, 12 ) == "--benchmark=" ) {
      _app_continuous_mode = CDBenchmark;
      std::string bnd_args = arg.substr( arg.find_first_of( '=' ) + 1 );
      if ( sutilParseImageDimensions( bnd_args.c_str(), &_warmup_frames, &_timed_frames ) != RT_SUCCESS ) {
        std::cerr << "Invalid --benchmark arguments: '" << bnd_args << "'" << std::endl;
        printUsage();
        quit(1);
      }
      removeArg( i, argc, argv );
    } else if ( arg.substr( 0, 3) == "-B=" || arg.substr( 0, 23 ) == "--benchmark-no-display=" ) {
      _cur_continuous_mode = CDBenchmark;
      _benchmark_no_display = true;
      std::string bnd_args = arg.substr( arg.find_first_of( '=' ) + 1 );
      if ( sutilParseImageDimensions( bnd_args.c_str(), &_warmup_frames, &_timed_frames ) != RT_SUCCESS ) {
        std::cerr << "Invalid --benchmark-no-display arguments: '" << bnd_args << "'" << std::endl;
        printUsage();
        quit(1);
      }
      removeArg( i, argc, argv );
    } else if ( arg.substr( 0, 4) == "-bb=" || arg.substr( 0, 18 ) == "--timed-benchmark=" ) {
      _app_continuous_mode = CDBenchmarkTimed;
      std::string bnd_args = arg.substr( arg.find_first_of( '=' ) + 1 );
      if ( sutilParseFloatDimensions( bnd_args.c_str(), &_warmup_time, &_benchmark_time ) != RT_SUCCESS) {
        std::cerr << "Invalid --timed-benchmark (-bb) arguments: '" << bnd_args << "'" << std::endl;
        printUsage();
        quit(1);
      }
      removeArg( i, argc, argv );
    } else if ( arg.substr( 0, 4) == "-BB=" || arg.substr( 0, 29 ) == "--timed-benchmark-no-display=" ) {
      _cur_continuous_mode = CDBenchmarkTimed;
      _benchmark_no_display = true;
      std::string bnd_args = arg.substr( arg.find_first_of( '=' ) + 1 );
      if ( sutilParseFloatDimensions( bnd_args.c_str(), &_warmup_time, &_benchmark_time ) != RT_SUCCESS ) {
        std::cerr << "Invalid --timed-benchmark-no-display (-BB) arguments: '" << bnd_args << "'" << std::endl;
        printUsage();
        quit(1);
      }
      removeArg( i, argc, argv );
    }
  }

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
}

void GLUTDisplay::run( const std::string& title, Scene* scene, contDraw_E continuous_mode )
{
  if ( !_initialized ) {
    std::cerr << "ERROR - GLUTDisplay::run() called before GLUTDisplay::init()" << std::endl;
    exit(2);
  }
  _scene = scene;
  _title = title;
  _scene->setNumDevices( _num_devices );

  if( _benchmark_no_display ) {
    runBenchmarkNoDisplay();
    return;
  }

  if( _print_mem_usage ) {
    DeviceMemoryLogger::logDeviceDescription(_scene->getContext(), std::cerr);
    DeviceMemoryLogger::logCurrentMemoryUsage(_scene->getContext(), std::cerr, "Initial memory available: " );
    std::cerr << std::endl;
  }

  // Initialize GLUT and GLEW first. Now initScene can use OpenGL and GLEW.
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
  if( _initial_window_width > 0 && _initial_window_height > 0)
    glutInitWindowSize( _initial_window_width, _initial_window_height );
  else
    glutInitWindowSize( 128, 128 );
  glutInitWindowPosition(100,100);
  glutCreateWindow( _title.c_str() );
  glutHideWindow();
#if !defined(__APPLE__)
  glewInit();
  if (glewIsSupported( "GL_EXT_texture_sRGB GL_EXT_framebuffer_sRGB")) {
    _sRGB_supported = true;
  }
#else
  _sRGB_supported = true;
#endif
#if defined(_WIN32)
  // Turn off vertical sync
  wglSwapIntervalEXT(0);
#endif

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // If _app_continuous_mode was already set to CDBenchmark* on the command line then preserve it.
  setContinuousMode( _app_continuous_mode == CDNone ? continuous_mode : _app_continuous_mode );

  int buffer_width;
  int buffer_height;
  try {
    // Set up scene
    Scene::InitialCameraData camera_data;
    _scene->initScene( camera_data );

    if( _initial_window_width > 0 && _initial_window_height > 0)
      _scene->resize( _initial_window_width, _initial_window_height );

    if ( !_camera_pose.empty() )
      camera_data = Scene::InitialCameraData( _camera_pose );

    // Initialize camera according to scene params
    _camera = new PinholeCamera( camera_data.eye,
                                 camera_data.lookat,
                                 camera_data.up,
                                 -1.0f, // hfov is ignored when using keep vertical
                                 camera_data.vfov,
                                 PinholeCamera::KeepVertical );

    Buffer buffer = _scene->getOutputBuffer();
    RTsize buffer_width_rts, buffer_height_rts;
    buffer->getSize( buffer_width_rts, buffer_height_rts );
    buffer_width  = static_cast<int>(buffer_width_rts);
    buffer_height = static_cast<int>(buffer_height_rts);
    _mouse = new Mouse( _camera, buffer_width, buffer_height );
  } catch( Exception& e ){
    sutilReportError( e.getErrorString().c_str() );
    exit(2);
  }

  // Initialize state
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 1, 0, 1, -1, 1 );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, buffer_width, buffer_height);

  glutShowWindow();

  // reshape window to the correct window resize
  glutReshapeWindow( buffer_width, buffer_height);

  // Set callbacks
  glutKeyboardFunc(keyPressed);
  glutDisplayFunc(display);
  glutMouseFunc(mouseButton);
  glutMotionFunc(mouseMotion);
  glutReshapeFunc(resize);

  // Initialize timer
  sutilCurrentTime( &_last_frame_time );
  _frame_count = 0;
  _last_frame_count = 0;
  _start_time = _last_frame_time;
  if( _cur_continuous_mode == CDBenchmarkTimed ) {
    _warmup_start = _last_frame_time;
    _warmup_frames = 0;
    _timed_frames = 0;
  }
  _benchmark_frame_start = 0;

  // Enter main loop
  glutMainLoop();
}

void GLUTDisplay::setCamera(Scene::InitialCameraData& camera_data)
{
  _camera->setParameters(camera_data.eye,
                         camera_data.lookat,
                         camera_data.up,
                         camera_data.vfov, 
                         camera_data.vfov,
                         PinholeCamera::KeepVertical );
  glutPostRedisplay();  
}

// This is an internal function that does the actual work.
void GLUTDisplay::setCurContinuousMode(contDraw_E continuous_mode)
{
  _cur_continuous_mode = continuous_mode;

  sutilCurrentTime( &_start_time );
  glutIdleFunc( _cur_continuous_mode!=CDNone ? idle : 0 );
  glutPostRedisplay();
}

// This is an API function for the app to specify its desired mode.
void GLUTDisplay::setContinuousMode(contDraw_E continuous_mode)
{
  _app_continuous_mode = continuous_mode;

  // Unless the user has overridden it, progressive implies a finite continuous drawing timeout.
  if(_app_continuous_mode == CDProgressive && _progressive_timeout < 0.0) {
    _progressive_timeout = 10.0;
  }

  setCurContinuousMode(_app_continuous_mode);
}

void GLUTDisplay::postRedisplay()
{
  glutPostRedisplay();
}

void GLUTDisplay::drawText( const std::string& text, float x, float y, void* font )
{
  // Save state
  glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT );

  glDisable( GL_TEXTURE_2D );
  glDisable( GL_LIGHTING );
  glDisable( GL_DEPTH_TEST);

  glColor3fv( &( _text_shadow_color.x) ); // drop shadow
  // Shift shadow one pixel to the lower right.
  glWindowPos2f(x + 1.0f, y - 1.0f);
  for( std::string::const_iterator it = text.begin(); it != text.end(); ++it )
    glutBitmapCharacter( font, *it );

  glColor3fv( &( _text_color.x) );        // main text
  glWindowPos2f(x, y);
  for( std::string::const_iterator it = text.begin(); it != text.end(); ++it )
    glutBitmapCharacter( font, *it );

  // Restore state
  glPopAttrib();
}

void GLUTDisplay::runBenchmarkNoDisplay( )
{
  // Set up scene
  Scene::InitialCameraData initial_camera_data;
  _scene->setUseVBOBuffer( false );
  _scene->initScene( initial_camera_data );

  if( _initial_window_width > 0 && _initial_window_height > 0)
    _scene->resize( _initial_window_width, _initial_window_height );

  if ( !_camera_pose.empty() )
    initial_camera_data = Scene::InitialCameraData( _camera_pose );

  // Initialize camera according to scene params
  _camera = new PinholeCamera( initial_camera_data.eye,
                               initial_camera_data.lookat,
                               initial_camera_data.up,
                               -1.0f, // hfov is ignored when using keep vertical
                               initial_camera_data.vfov,
                               PinholeCamera::KeepVertical );
  _mouse = new Mouse( _camera, _initial_window_width, _initial_window_height );
  _mouse->handleResize( _initial_window_width, _initial_window_height );

  float3 eye, U, V, W;
  _camera->getEyeUVW( eye, U, V, W );
  Scene::RayGenCameraData camera_data( eye, U, V, W );

  // Warmup frames
  if ( _cur_continuous_mode == CDBenchmarkTimed ) {
    // Count elapsed time
    double start_time, finish_time;
    sutilCurrentTime( &start_time );
    _warmup_frames = 0;
    do
    {
      _scene->trace( camera_data );
      sutilCurrentTime( &finish_time );
      _warmup_frames++;
    } while( finish_time-start_time < _warmup_time);
  } else {
    // Count frames
    for( unsigned int i = 0; i < _warmup_frames; ++i ) {
      _scene->trace( camera_data );
    }
  }

  // Timed frames
  double start_time, finish_time;
  sutilCurrentTime( &start_time );
  if ( _cur_continuous_mode == CDBenchmarkTimed ) {
    // Count elapsed time
    _timed_frames = 0;
    do
    {
      _scene->trace( camera_data );
      sutilCurrentTime( &finish_time );
      _timed_frames++;
    } while( finish_time-start_time < _benchmark_time );
  } else
  {
    // Count frames
    for( unsigned int i = 0; i < _timed_frames; ++i ) {
      _scene->trace( camera_data );
    }
    sutilCurrentTime( &finish_time );
  }

  // Save image if necessary
  if( _save_frames_to_file ) {
    std::string filename = _save_frames_basename.empty() ?  _title + ".ppm" : _save_frames_basename+ ".ppm"; 
    Buffer buffer = _scene->getOutputBuffer();
    sutilDisplayFilePPM( filename.c_str(), buffer->get() );
  }

  double total_time = finish_time-start_time;
  sutilPrintBenchmark( _title.c_str(), total_time, _warmup_frames, _timed_frames);
}

void GLUTDisplay::keyPressed(unsigned char key, int x, int y)
{
  try {
    if( _scene->keyPressed(key, x, y) ) {
      glutPostRedisplay();
      return;
    }
  } catch( Exception& e ){
    sutilReportError( e.getErrorString().c_str() );
    exit(2);
  }

  switch (key) {
  case 27: // esc
  case 'q':
    quit();
  case 'f':
    if ( _old_window_width == -1) { // We are in non-fullscreen mode
      _old_window_width  = glutGet(GLUT_WINDOW_WIDTH);
      _old_window_height = glutGet(GLUT_WINDOW_HEIGHT);
      _old_window_x      = glutGet(GLUT_WINDOW_X);
      _old_window_y      = glutGet(GLUT_WINDOW_Y);
      glutFullScreen();
    } else { // We are in fullscreen mode
      glutPositionWindow( _old_window_x, _old_window_y );
      glutReshapeWindow( _old_window_width, _old_window_height );
      _old_window_width = _old_window_height = -1;
    }
    glutPostRedisplay();
    break;

  case 'R':
    setProgressiveDrawingTimeout(0.0);
    // Fall through

  case 'r':
    if(_app_continuous_mode == CDProgressive) {
      if(_cur_continuous_mode == CDProgressive) {
        setCurContinuousMode(CDNone);
      } else if(_cur_continuous_mode == CDNone) {
        setCurContinuousMode(CDProgressive);
      }
      break;
    }
    if(_app_continuous_mode == CDAnimated) {
      if(_cur_continuous_mode == CDAnimated) {
        setCurContinuousMode(CDNone);
      } else if(_cur_continuous_mode == CDNone) {
        setCurContinuousMode(CDAnimated);
      }
      break;
    }
    // Fall through to benchmark mode if the app hasn't specified a kind of continuous to do

  case 'b':
    if(_cur_continuous_mode == CDBenchmarkTimed) {
      // Turn off the benchmark and print the results
      double current_time;
      sutilCurrentTime(&current_time);
      double total_time = current_time-_benchmark_frame_time;
      sutilPrintBenchmark(_title.c_str(), total_time, _warmup_frames, _timed_frames);
      setCurContinuousMode(_app_continuous_mode);
    } else {
      // Turn on the benchmark and set continuous rendering
      std::cerr << "Benchmark started. Press 'b' again to end.\n";
      setCurContinuousMode(CDBenchmarkTimed);
      _benchmark_time = 1e37f; // Do a timed benchmark, but forever.
      _benchmark_frame_start = _frame_count;

      double current_time;
      sutilCurrentTime(&current_time);
      _warmup_start = current_time;
      _benchmark_frame_time = current_time;
      _warmup_frames = 0;
      _warmup_time = 0;
      _timed_frames = 0;
    }
    break;

  case 'd':
    _display_fps = !_display_fps;
    break;

  case 's':
    sutilDisplayFilePPM( "out.ppm", _scene->getOutputBuffer()->get() );
    break;

  case 'm':
    _print_mem_usage =  !_print_mem_usage;
    glutPostRedisplay();
    break;

  case 'c':
    float3 eye, lookat, up;
    float hfov, vfov;

    _camera->getEyeLookUpFOV(eye, lookat, up, hfov, vfov);
    std::cerr << '"' << eye << lookat << up << vfov << '"' << std::endl;
    break;

  default:
    return;
  }
}


void GLUTDisplay::mouseButton(int button, int state, int x, int y)
{
  sutilCurrentTime( &_start_time );
  _mouse->handleMouseFunc( button, state, x, y, glutGetModifiers() );
  if ( state != GLUT_UP )
    _scene->signalCameraChanged();
  glutPostRedisplay();
}


void GLUTDisplay::mouseMotion(int x, int y)
{
  sutilCurrentTime( &_start_time );
  _mouse->handleMoveFunc( x, y );
  _scene->signalCameraChanged();
  if (_app_continuous_mode == CDProgressive) {
    setCurContinuousMode(CDProgressive);
  }
  glutPostRedisplay();
}


void GLUTDisplay::resize(int width, int height)
{
  // disallow size 0
  width  = max(1, width);
  height = max(1, height);

  sutilCurrentTime( &_start_time );
  _scene->signalCameraChanged();
  _mouse->handleResize( width, height );

  try {
    _scene->resize(width, height);
  } catch( Exception& e ){
    sutilReportError( e.getErrorString().c_str() );
    exit(2);
  }

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 1, 0, 1, -1, 1);
  glViewport(0, 0, width, height);
  if (_app_continuous_mode == CDProgressive) {
    setCurContinuousMode(CDProgressive);
  }
  glutPostRedisplay();
}


void GLUTDisplay::idle()
{
  glutPostRedisplay();
}


void GLUTDisplay::displayFrame()
{
  GLboolean sRGB = GL_FALSE;
  if (_use_sRGB && _sRGB_supported) {
    glGetBooleanv( GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &sRGB );
    if (sRGB) {
      glEnable(GL_FRAMEBUFFER_SRGB_EXT);
    }
  }

  // Draw the resulting image
  Buffer buffer = _scene->getOutputBuffer(); 
  RTsize buffer_width_rts, buffer_height_rts;
  buffer->getSize( buffer_width_rts, buffer_height_rts );
  int buffer_width  = static_cast<int>(buffer_width_rts);
  int buffer_height = static_cast<int>(buffer_height_rts);
  RTformat buffer_format = buffer->getFormat();

  if( _save_frames_to_file ) {
    static char fname[128];
    std::string basename = _save_frames_basename.empty() ? "frame" : _save_frames_basename;
    sprintf(fname, "%s_%05d.ppm", basename.c_str(), _frame_count);
    sutilDisplayFilePPM( fname, buffer->get() );
  }

  unsigned int vboId = 0;
  if( _scene->usesVBOBuffer() )
    vboId = buffer->getGLBOId();

  if (vboId)
  {
    if (!_texId)
    {
      glGenTextures( 1, &_texId );
      glBindTexture( GL_TEXTURE_2D, _texId);

      // Change these to GL_LINEAR for super- or sub-sampling
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

      // GL_CLAMP_TO_EDGE for linear filtering, not relevant for nearest.
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glBindTexture( GL_TEXTURE_2D, 0);
    }

    glBindTexture( GL_TEXTURE_2D, _texId );

    // send pbo to texture
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vboId);

    RTsize elementSize = buffer->getElementSize();
    if      ((elementSize % 8) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
    else if ((elementSize % 4) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    else if ((elementSize % 2) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    else                             glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if(buffer_format == RT_FORMAT_UNSIGNED_BYTE4) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, buffer_width, buffer_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
    } else if(buffer_format == RT_FORMAT_FLOAT4) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, buffer_width, buffer_height, 0, GL_RGBA, GL_FLOAT, 0);
    } else if(buffer_format == RT_FORMAT_FLOAT3) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F_ARB, buffer_width, buffer_height, 0, GL_RGB, GL_FLOAT, 0);
    } else if(buffer_format == RT_FORMAT_FLOAT) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE32F_ARB, buffer_width, buffer_height, 0, GL_LUMINANCE, GL_FLOAT, 0);
    } else {
      assert(0 && "Unknown buffer format");
    }

    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );

    glEnable(GL_TEXTURE_2D);

    // Initialize offsets to pixel center sampling.

    float u = 0.5f/buffer_width;
    float v = 0.5f/buffer_height;

    glBegin(GL_QUADS);
    glTexCoord2f(u, v);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, v);
    glVertex2f(1.0f, 0.0f);
    glTexCoord2f(1.0f - u, 1.0f - v);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(u, 1.0f - v);
    glVertex2f(0.0f, 1.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
  } else {
    GLvoid* imageData = buffer->map();
    assert( imageData );

    GLenum gl_data_type = GL_FALSE;
    GLenum gl_format = GL_FALSE;

    switch (buffer_format) {
          case RT_FORMAT_UNSIGNED_BYTE4:
            gl_data_type = GL_UNSIGNED_BYTE;
            gl_format    = GL_BGRA;
            break;

          case RT_FORMAT_FLOAT:
            gl_data_type = GL_FLOAT;
            gl_format    = GL_LUMINANCE;
            break;

          case RT_FORMAT_FLOAT3:
            gl_data_type = GL_FLOAT;
            gl_format    = GL_RGB;
            break;

          case RT_FORMAT_FLOAT4:
            gl_data_type = GL_FLOAT;
            gl_format    = GL_RGBA;
            break;

          default:
            fprintf(stderr, "Unrecognized buffer data type or format.\n");
            exit(2);
            break;
    }

    glDrawPixels( static_cast<GLsizei>( buffer_width),
      static_cast<GLsizei>( buffer_height ),
      gl_format, gl_data_type, imageData);

    buffer->unmap();
  }
  if (_use_sRGB && _sRGB_supported && sRGB) {
    glDisable(GL_FRAMEBUFFER_SRGB_EXT);
  }
}

void GLUTDisplay::display()
{
  if( _cur_continuous_mode == CDProgressive && _progressive_timeout > 0.0 ) {
    // If doing progressive refinement, see if we're done
    double current_time;
    sutilCurrentTime( &current_time );
    if( current_time - _start_time > _progressive_timeout ) {
      setCurContinuousMode( CDNone );
      return;
    }
  }

  try {
    // render the scene
    float3 eye, U, V, W;
    _camera->getEyeUVW( eye, U, V, W );
    // Don't be tempted to just start filling in the values outside of a constructor, 
    // because if you add a parameter it's easy to forget to add it here.
    Scene::RayGenCameraData camera_data( eye, U, V, W );
    _scene->trace( camera_data );

    // Always count rendered frames
    ++_frame_count;

    // Not strictly necessary
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    if( _display_frames ) {
      displayFrame();
    }
  } catch( Exception& e ){
    sutilReportError( e.getErrorString().c_str() );
    exit(2);
  }

  // Do not draw text on 1st frame -- issue on linux causes problems with 
  // glDrawPixels call if drawText glutBitmapCharacter is called on first frame.
  if ( _display_fps && _cur_continuous_mode != CDNone && _frame_count > 1 ) {
    // Output fps 
    double current_time;
    sutilCurrentTime( &current_time );
    double dt = current_time - _last_frame_time;
    if( dt > _fps_update_threshold ) {
      sprintf( _fps_text, "fps: %7.2f", (_frame_count - _last_frame_count) / dt );

      _last_frame_time = current_time;
      _last_frame_count = _frame_count;
    } else if( _frame_count == 1 ) {
      sprintf( _fps_text, "fps: %7.2f", 0.f );
    }

    drawText( _fps_text, 10.0f, 10.0f, GLUT_BITMAP_8_BY_13 );
  }

  if( _print_mem_usage ) {
    // Output memory
    std::ostringstream str;
    DeviceMemoryLogger::logCurrentMemoryUsage(_scene->getContext(), str);
    drawText( str.str(), 10.0f, 26.0f, GLUT_BITMAP_8_BY_13 );
  }

  //printf("finished frame: %d\n", _frame_count);
  if( _cur_continuous_mode == CDBenchmark || _cur_continuous_mode == CDBenchmarkTimed ) {
    double current_time;
    sutilCurrentTime(&current_time);

    // Do the timed frames first, since _benchmark_frame_time may be set by the warmup
    // section below and we don't want to double count the frames.
    if ( _cur_continuous_mode == CDBenchmarkTimed ) {
      // Count elapsed time, but only if we have moved out of the warmup phase.
      if (_benchmark_frame_time > 0) {
        _timed_frames++;
        //printf("_timed_frames = %d\n", _timed_frames);
        double total_time = current_time-_benchmark_frame_time;
        if ( total_time > _benchmark_time ) {
          sutilPrintBenchmark(_title.c_str(), total_time, _warmup_frames, _timed_frames);
          setCurContinuousMode( CDNone );
          quit(0); // We only get here for command line benchmarks, which always end.
        }
      }
    } else {
      // Count frames
      if(_frame_count == _benchmark_frame_start+_warmup_frames+_timed_frames) {
        double total_time = current_time-_benchmark_frame_time;
        sutilPrintBenchmark(_title.c_str(), total_time, _warmup_frames, _timed_frames);
        setCurContinuousMode( CDNone );
        quit(0); // We only get here for command line benchmarks, which always end.
      }
    }

    if ( _cur_continuous_mode == CDBenchmarkTimed ) {
      if ( current_time-_warmup_start < _warmup_time) {
        // Under the alloted time, keep counting
        _warmup_frames++;
        //printf("_warmup_frames = %d\n", _warmup_frames);
      } else {
        // Over the alloted time, set the _benchmark_frame_time if it hasn't been set yet.
        if (_benchmark_frame_time == 0) {
          _benchmark_frame_time = current_time;
          // Make sure we account for that last frame
          _warmup_frames++;
          //printf("warmup done (_warmup_frames = %d) after %g seconds.\n", _warmup_frames, current_time-_warmup_start);
        }
      }
    } else {
      // Count frames
      if(_frame_count-1 == _benchmark_frame_start+_warmup_frames) {
        _benchmark_frame_time = current_time; // Start timing.
      }
    }
  }

  // Swap buffers
  glutSwapBuffers();
}

void GLUTDisplay::quit(int return_code)
{
  try {
    if(_scene)
    {
      _scene->cleanUp();
      if (_scene->getContext().get() != 0)
      {
        sutilReportError( "Derived scene class failed to call Scene::cleanUp()" );
        exit(2);
      }
    }
    exit(return_code);
  } catch( Exception& e ) {
    sutilReportError( e.getErrorString().c_str() );
    exit(2);
  }
}
