# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (OpenGL_VRC)

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
    message(ERROR " GLUT not found!")
endif(NOT GLUT_FOUND)

SET(glui_DIR /usr/lib/lib/cmake/glui/)
find_package(glui CONFIG REQUIRED)

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

#search SOIL


find_path(SOIL_INCLUDE_DIR SOIL.h
	/usr/include
	/usr/local/include
	/opt/local/include
	${CMAKE_SOURCE_DIR}/includes
)

find_library(SOIL_LIBRARY SOIL
	/usr/lib64
  /usr/lib
	/usr/local/lib
	/opt/local/lib
	${CMAKE_SOURCE_DIR}/lib
)

IF(SOIL_INCLUDE_DIR AND SOIL_LIBRARY)
	SET( SOIL_FOUND TRUE )
	SET( SOIL_LIBRARIES ${SOIL_LIBRARY} )
ENDIF(SOIL_INCLUDE_DIR AND SOIL_LIBRARY)
IF(SOIL_FOUND)
	IF(NOT SOIL_FIND_QUIETLY)
	MESSAGE(STATUS "Found SOIL: ${SOIL_LIBRARY}")
	ENDIF(NOT SOIL_FIND_QUIETLY)
ELSE(SOIL_FOUND)
	IF(SOIL_FIND_REQUIRED)
	MESSAGE(FATAL_ERROR "Could not find libSOIL")
	ENDIF(SOIL_FIND_REQUIRED)
ENDIF(SOIL_FOUND)

set(ALL_LIBS
	${OPENGL_LIBRARY}
  ${GLUT_LIBRARY}
  GLEW_1130
  ${SOIL_LIBRARY}
)

add_executable(main main2.cpp)
target_link_libraries(main PUBLIC glui::glui ${ALL_LIBS})
