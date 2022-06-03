cmake_minimum_required(VERSION 3.19)
#set(CMAKE_VERBOSE_MAKEFILE ON)
project(02_ShadersVAOVBO)

#############################
#
#  Cmake file for oglbase for linux
#   (... and possibly for MacOS / any other *nix based OSes as well, but this hasn't been tested yet)
#
# How to compile the project:
#  0) make sure you have OpenGL, SDL2 and Glew installed
#  1) create a build directory and cd into it
#      for example: `mkdir build && cd build`
#  2) run this cmake file
#      for example: `cmake ..`
#  3) run `make` in the build directory whenever you have made a change to the source code / assets
#  4) your executable will be placed in the build directory
#
# How to add source files:
#  a) either enable the globbing below (bad idea)
#  b) or add the source file (.cpp) to the SOURCE_FILES variable
#  Then run the cmake file again.
#
# How to add assets:
#  1) add the asset files to the ASSETS variable
#
#############################

# Set the source files
#  Globbing is usually not a good idea, but it may be more comfortable. To enable it, simply change
#  the 'true' below to a 'false' and rerun the cmake file.
if(true)
set(SOURCE_FILES
    02_ShadersVAOVBO/main.cpp
    02_ShadersVAOVBO/MyApp.cpp
    02_ShadersVAOVBO/GLDebugMessageCallback.cpp
)
else()
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.11.0")
    file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS 02_ShadersVAOVBO/*.cpp)
else()
    file(GLOB SOURCE_FILES 02_ShadersVAOVBO/*.cpp 02_ShadersVAOVBO/*/*.cpp)
endif()
endif()

# Make sure that we have the assets ready in the build directory
set(ASSETS
    02_ShadersVAOVBO/wood.jpg
    02_ShadersVAOVBO/myFrag.frag
    02_ShadersVAOVBO/myVert.vert
)

# -------------

# Create our executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# On some linux distros, the FindSDL2_image file is missing, so we ship our own.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Find the required libraries
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(GLEW REQUIRED)

# Set the include directories
target_include_directories(${PROJECT_NAME}
    PUBLIC "02_ShadersVAOVBO"
    PRIVATE ${OPENGL_INCLUDE_DIR}
    PRIVATE ${SDL2_INCLUDE_DIRS}
    PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS}
)

# Link the necessary libraries
target_link_libraries(${PROJECT_NAME}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
)

# Prepend the absolute source path
list(TRANSFORM ASSETS PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

# Copy them over to the build directory
add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${ASSETS}
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
add_dependencies(${PROJECT_NAME} copy_assets)