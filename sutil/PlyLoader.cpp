/*
 * Copyright (c) 2008 - 2011 NVIDIA Corporation.  All rights reserved.
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

#include "PlyLoader.h"
#include "rply-1.01/rply.h"
#include <optixu/optixu.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <iostream>
#include <cstring> //memcpy
#include <algorithm>


//------------------------------------------------------------------------------
// 
//  Helper functions
//
//------------------------------------------------------------------------------

namespace
{
  struct ModelData
  {
    ModelData() : nverts(0), ntris(0), verts(0), tris(0) {}
    optix::Aabb    aabb;
    unsigned       nverts;
    unsigned       ntris;
    optix::float3* verts;
    optix::int3*   tris; 
  };


  std::string getExtension( const std::string& filename )
  {
    std::string::size_type extension_index = filename.find_last_of( "." );
    return extension_index != std::string::npos ?
           filename.substr( extension_index+1 ) :
           std::string();
  }


  int vertexCB( p_ply_argument argument )
  {
    int coord_index;
    ModelData* data;
    ModelData** data_pp = &data;
    ply_get_argument_user_data( argument, reinterpret_cast<void**>( data_pp ), &coord_index );
    switch( coord_index ) {
      case 0: 
        data->verts[ data->nverts ].x = static_cast<float>( ply_get_argument_value( argument ) );
        break;
      case 1: 
        data->verts[ data->nverts ].y = static_cast<float>( ply_get_argument_value( argument ) );
        break;
      case 2: 
        data->verts[ data->nverts ].z = static_cast<float>( ply_get_argument_value( argument ) );
        data->aabb.include( data->verts[ data->nverts ] );
        data->nverts += 1;
        break;
    }
    return 1;
  }


  int faceCB(p_ply_argument argument)
  {
    ModelData* data;
    ModelData** data_pp = &data;
    ply_get_argument_user_data( argument, reinterpret_cast<void**>( data_pp ), NULL );

    int num_verts, which_vertex;
    ply_get_argument_property( argument, NULL, &num_verts, &which_vertex );
    
    switch( which_vertex ) {
      case 0: 
        data->tris[ data->ntris ].x = static_cast<int>( ply_get_argument_value( argument ) );
        break;
      case 1: 
        data->tris[ data->ntris ].y = static_cast<int>( ply_get_argument_value( argument ) );
        break;
      case 2: 
        data->tris[ data->ntris ].z = static_cast<int>( ply_get_argument_value( argument ) );
        data->ntris += 1;
        break;
    }

    return 1;
  }

}

//------------------------------------------------------------------------------
// 
//  PlyLoader implementation
//
//------------------------------------------------------------------------------


PlyLoader::PlyLoader( const std::string&   filename,
                      optix::Context       context,
                      optix::GeometryGroup geometrygroup,
                      optix::Material      material,
                      const char* ASBuilder,
                      const char* ASTraverser,
                      bool large_geom )
: _filename( filename ),
  _context( context ),
  _geometrygroup( geometrygroup ),
  _material( material ),
  _large_geom( large_geom),
  _ASBuilder  (ASBuilder),
  _ASTraverser(ASTraverser)
{
  // Error checking on context and geometrygroup done in ModelLoader

  if( material.get() == 0 ) {
    const std::string ptx_path = std::string( sutilSamplesPtxDir() ) + "/cuda_compile_ptx_generated_phong.cu.ptx";
    _material = context->createMaterial();
    _material->setClosestHitProgram( 0, _context->createProgramFromPTXFile( ptx_path, "closest_hit_radiance" ) );
    _material->setAnyHitProgram    ( 1, _context->createProgramFromPTXFile( ptx_path, "any_hit_shadow" ) );
    _material[ "Kd"           ]->setFloat( 0.50f, 0.50f, 0.50f );
    _material[ "Ks"           ]->setFloat( 0.00f, 0.00f, 0.00f );
    _material[ "Ka"           ]->setFloat( 0.05f, 0.05f, 0.05f );
    _material[ "reflectivity" ]->setFloat( 0.00f, 0.00f, 0.00f );
    _material[ "phong_exp"    ]->setFloat( 1.00f );
  }
}


void PlyLoader::load()
{
  load( optix::Matrix4x4::identity() );
}


void PlyLoader::load( const optix::Matrix4x4& transform )
{
  p_ply ply = ply_open( _filename.c_str(), 0);
  if( !ply ) {
    throw optix::Exception( "Error opening ply file (" + _filename + ")" );
  }

  if( !ply_read_header( ply ) ) {
    throw optix::Exception( "Error parsing ply header (" + _filename + ")" );
  }

  ModelData data;
  int nverts = ply_set_read_cb( ply, "vertex", "x", vertexCB, &data, 0);
  ply_set_read_cb( ply, "vertex", "y", vertexCB, &data, 1);
  ply_set_read_cb( ply, "vertex", "z", vertexCB, &data, 2);

  int ntris = ply_set_read_cb(ply, "face", "vertex_indices", faceCB, &data, 0);

  data.verts = new optix::float3[ nverts ];
  data.tris  = new optix::int3[ ntris ];

  if( !ply_read( ply ) ) {
    throw optix::Exception( "Error parsing ply file (" + _filename + ")" );
  }
  ply_close(ply);

  _aabb = data.aabb;

  /*
  std::cerr << " done.  got " << nverts << " verts and " << ntris << " tris." << std::endl;

  for( int i = 0; i < 10; ++i ) {
    std::cerr << " v: " << data.verts[i] << std::endl;
  }

  for( int i = 0; i < 10; ++i ) {
    std::cerr << " f: " << data.tris[i] << std::endl;
  }
  */

  createGeometryInstance( data.nverts, data.verts, data.ntris, data.tris );
}

void PlyLoader::createGeometryInstance( unsigned nverts, optix::float3* verts, unsigned ntris, optix::int3* tris )
{
  if (_large_geom) {
    if( _ASBuilder == std::string("Sbvh") || _ASBuilder == std::string("KdTree")) {
      _ASBuilder = "MedianBvh";
      _ASTraverser = "Bvh";
    }


    RTgeometry geometry;
    unsigned int usePTX32InHost64 = 0;
    rtuCreateClusteredMesh( _context->get(), usePTX32InHost64, &geometry, nverts, 
                           (const float*)verts, ntris, (const unsigned int*)tris, 0 );
    optix::Geometry mesh = optix::Geometry::take(geometry);
   
    optix::GeometryInstance instance = _context->createGeometryInstance( mesh, &_material, &_material+1 );

    optix::Acceleration acceleration = _context->createAcceleration(_ASBuilder, _ASTraverser);
    acceleration->setProperty( "leaf_size", "1" );
    acceleration->markDirty();

    _geometrygroup->setAcceleration( acceleration );
    _geometrygroup->setChildCount( 1u );
    _geometrygroup->setChild( 0, instance );
  } else {
    optix::Buffer vertex_buffer = _context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, nverts );
    memcpy( vertex_buffer->map(), reinterpret_cast<void*>( verts ), sizeof( optix::float3 )*nverts );
    vertex_buffer->unmap();

    optix::Buffer vindex_buffer = _context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3, ntris );
    memcpy( vindex_buffer->map(), reinterpret_cast<void*>( tris ), sizeof( optix::int3 )*ntris );
    vindex_buffer->unmap();

    std::string ptx_path = std::string( sutilSamplesPtxDir() ) + "/cuda_compile_ptx_generated_triangle_mesh_small.cu.ptx";

    optix::Geometry mesh = _context->createGeometry();
    mesh->setPrimitiveCount( ntris );
    mesh->setIntersectionProgram( _context->createProgramFromPTXFile( ptx_path, "mesh_intersect" ) );
    mesh->setBoundingBoxProgram(  _context->createProgramFromPTXFile( ptx_path, "mesh_bounds" ) );
    mesh[ "vertex_buffer" ]->set( vertex_buffer );
    mesh[ "vindex_buffer" ]->set( vindex_buffer );

    optix::GeometryInstance instance = _context->createGeometryInstance( mesh, &_material, &_material+1 );

    optix::Acceleration acceleration = _context->createAcceleration(_ASBuilder, _ASTraverser);
    if ( _ASBuilder   == std::string("Sbvh")           ||
         _ASBuilder   == std::string("TriangleKdTree") ||
         _ASTraverser == std::string( "KdTree")        )
    {
      acceleration->setProperty( "vertex_buffer_name", "vertex_buffer" );
      acceleration->setProperty( "index_buffer_name", "vindex_buffer" );
    }
    acceleration->markDirty();

    _geometrygroup->setAcceleration( acceleration );
    _geometrygroup->setChildCount( 1u );
    _geometrygroup->setChild( 0, instance );
  }
}


bool PlyLoader::isMyFile( const std::string& filename )
{
  return getExtension( filename ) == "ply";
}

