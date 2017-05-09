# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (Tutorials)

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
    message(ERROR " GLUT not found!")
endif(NOT GLUT_FOUND)


find_package(OpenGL REQUIRED)




if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

add_subdirectory (include)

# On Visual 2005 and above, this module can set the debug working directory
cmake_policy(SET CMP0026 OLD)

include_directories(
	include/GL/
  include/GL/glm/
  include/glew-1.13.0/include/
  lib/
  .
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
  ${GLUT_LIBRARY}
  GLEW_1130
)

add_executable(main
	main.cpp
)
target_link_libraries(main
	${ALL_LIBS}
)
