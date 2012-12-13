#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <iostream>
#include <GLUTDisplay.h>
#include <ImageLoader.h>
#include "Scene.h"
#include "commonStructs.h"
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <math.h>

using namespace optix;

//-----------------------------------------------------------------------------
//
// Main driver
//
//-----------------------------------------------------------------------------
void printUsageAndExit( const string& argv0, bool doExit = true )
{
    cerr
    << "Usage  : " << argv0 << " [options]\n"
    << "App options:\n"
    << "  -h  | --help                               Print this usage message\n"
    << "  -t  | --texture-path <path>                Specify path to texture directory\n"
    << "        --dim=<width>x<height>               Set image dimensions\n"
    << endl;

    if ( doExit ) exit(1);
}


int main( int argc, char** argv )
{
    GLUTDisplay::init( argc, argv );

    unsigned int width = 1080u, height = 720u;

    string texture_path;
    for ( int i = 1; i < argc; ++i ) {
        string arg( argv[i] );
        if ( arg == "--help" || arg == "-h" ) {
            printUsageAndExit( argv[0] );
        } else if ( arg.substr( 0, 6 ) == "--dim=" ) {
            string dims_arg = arg.substr(6);
            if ( sutilParseImageDimensions( dims_arg.c_str(), &width, &height ) != RT_SUCCESS ) {
                std::cerr << "Invalid window dimensions: '" << dims_arg << "'" << std::endl;
                printUsageAndExit( argv[0] );
            }
        } else if ( arg == "-t" || arg == "--texture-path" ) {
            if ( i == argc-1 ) {
                printUsageAndExit( argv[0] );
            }
            texture_path = argv[++i];
        } else {
              std::cerr << "Unknown option: '" << arg << "'\n";
              printUsageAndExit( argv[0] );
        }
    }

    if( !GLUTDisplay::isBenchmark() ) printUsageAndExit( argv[0], false );

    if( texture_path.empty() ) {
        texture_path = string( sutilSamplesDir() ) + "/tutorial/data";
    }

    stringstream title;
    title << "Tutorial " << tutnum;
    try {
        Scene scene(texture_path);
        scene.setDimensions( width, height );
        GLUTDisplay::run( title.str(), &scene );
    } catch( Exception& e ){
        sutilReportError( e.getErrorString().c_str() );
        exit(1);
    }
    return 0;
}
