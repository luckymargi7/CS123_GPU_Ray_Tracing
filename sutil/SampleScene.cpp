
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
#  include <OpenGL/gl.h>
#else
#  include <GL/glew.h>
#  if defined(_WIN32)
#    include <GL/wglew.h>
#  endif
#  include <GL/gl.h>
#endif

#include <SampleScene.h>

#include <optixu/optixu_math_stream_namespace.h>
#include <optixu/optixu.h>

#include <iostream>
#include <sstream>
#include <cstdlib>


using namespace optix;

//-----------------------------------------------------------------------------
// 
// SampleScene class implementation 
//
//-----------------------------------------------------------------------------

SampleScene::SampleScene()
 : _camera_changed( true ), _use_vbo_buffer( true ), _num_devices( 0 )
{
  _context = Context::create();
}

SampleScene::InitialCameraData::InitialCameraData( const std::string &camstr)
{
  std::istringstream istr(camstr);
  istr >> eye >> lookat >> up >> vfov;
}

const char* const SampleScene::ptxpath( const std::string& target, const std::string& base )
{
  static std::string path;
  path = std::string(sutilSamplesPtxDir()) + "/" + target + "_generated_" + base + ".ptx";
  return path.c_str();
}

Buffer SampleScene::createOutputBuffer( RTformat format,
                                        unsigned int width,
                                        unsigned int height )
{
  Buffer buffer;

  if ( _use_vbo_buffer )
  {
    /*
      Allocate first the memory for the gl buffer, then attach it to OptiX.
    */
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    size_t element_size;
    _context->checkError(rtuGetSizeForRTformat(format, &element_size));
    glBufferData(GL_ARRAY_BUFFER, element_size * width * height, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    buffer = _context->createBufferFromGLBO(RT_BUFFER_OUTPUT, vbo);
    buffer->setFormat(format);
    buffer->setSize( width, height );
  }
  else {
    buffer = _context->createBuffer( RT_BUFFER_OUTPUT, format, width, height);
  }

  // Set number of devices to be used
  // Default, 0, means not to specify them here, but let OptiX use its default behavior.
  if(_num_devices)
  {
    int max_num_devices    = Context::getDeviceCount();
    int actual_num_devices = std::min( max_num_devices, std::max( 1, _num_devices ) );
    std::vector<int> devs(actual_num_devices);
    for( int i = 0; i < actual_num_devices; ++i ) devs[i] = i;
    _context->setDevices( devs.begin(), devs.end() );
  }

  return buffer;
}

void SampleScene::cleanUp()
{
  _context->destroy();
  _context = 0;
}

void SampleScene::resize(unsigned int width, unsigned int height)
{
  try {
    Buffer buffer = getOutputBuffer();
    buffer->setSize( width, height );

    if(_use_vbo_buffer)
    {
      buffer->unregisterGLBuffer();
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer->getGLBOId());
      glBufferData(GL_PIXEL_UNPACK_BUFFER, buffer->getElementSize() * width * height, 0, GL_STREAM_DRAW);
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
      buffer->registerGLBuffer();
    }

  } catch( Exception& e ){
    sutilReportError( e.getErrorString().c_str() );
    exit(2);
  }

  // Let the user resize any other buffers
  doResize( width, height );
}

