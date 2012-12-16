if(GLUT_FOUND AND OPENGL_FOUND)
  include_directories(${GLUT_INCLUDE_DIR})
  add_definitions(-DGLUT_FOUND -DGLUT_NO_LIB_PRAGMA)

  # See top level CMakeLists.txt file for documentation of OPTIX_add_sample_executable.
  OPTIX_add_sample_executable( final
    RayScene.cpp
    RayScene.h
    RayScene.cu
    Scene.cpp
    Scene.h
    DataProcessor.cpp
    DataProcessor.h
    main.cpp
    commonStructs.h

    cube.cu
    cone.cu
    cylinder.cu
    sphere.cu
    )
else()
  # GLUT or OpenGL not found
  message("Disabling rayDifferentials, which requires glut and opengl.")
endif()
