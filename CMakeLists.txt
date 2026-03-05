cmake_minimum_required(VERSION 3.13)

if (EMSCRIPTEN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -DGRAPHICS_API_OPENGL_ES3 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1 -s STACK_SIZE=1mb -s ALLOW_MEMORY_GROWTH=1  --preload-file ${CMAKE_CURRENT_BINARY_DIR}/../../res@res/")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -DGRAPHICS_API_OPENGL_ES3 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1 -s STACK_SIZE=1mb -s ALLOW_MEMORY_GROWTH=1  --preload-file ${CMAKE_CURRENT_BINARY_DIR}/../../res@res/")
  set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so taht you can directly open it.
endif ()

project(dcengine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src dcengine)

add_subdirectory(3rdParty)

