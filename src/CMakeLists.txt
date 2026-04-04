cmake_minimum_required(VERSION 3.13)

project(dcengine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    primitives.cpp
    math.cpp
    textures.cpp
    level.cpp
    cam.cpp
    renderer.cpp
    prefabs.cpp
    ui.cpp
    audio.cpp
    player.cpp
    settings.cpp
    )

set(EDITOR_SOURCES
    editor/editor.cpp
    editor/editor_ui.cpp
    editor/cam.cpp
    editor/player.cpp
    editor/editor_main.cpp
    )

if(MSVC)
  # https://github.com/raysan5/raylib/issues/857
  add_compile_definitions(NOVIRTUALKEYCODES NOWINMESSAGES NOWINSTYLES NOSYSMETRICS NOMENUS NOICONS NOKEYSTATES NOSYSCOMMANDS NORASTEROPS NOSHOWWINDOW OEMRESOURCE NOATOM NOCLIPBOARD NOCOLOR NOCTLMGR NODRAWTEXT NOGDI NOKERNEL NOUSER NOMB NOMEMMGR NOMETAFILE NOMINMAX NOMSG NOOPENFILE NOSCROLL NOSERVICE NOSOUND NOTEXTMETRIC NOWH NOWINOFFSETS NOCOMM NOKANJI NOHELP NOPROFILER NODEFERWINDOWPOS NOMCX)
endif()
if (EMSCRIPTEN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -DGRAPHICS_API_OPENGL_ES3 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1 -s STACK_SIZE=1mb -s ALLOW_MEMORY_GROWTH=1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -DGRAPHICS_API_OPENGL_ES3 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1 -s STACK_SIZE=1mb -s ALLOW_MEMORY_GROWTH=1")
  set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so taht you can directly open it.
endif ()
    
add_executable(editor ${SOURCES} ${EDITOR_SOURCES})
add_library(dcengine ${SOURCES})
target_link_libraries(editor PUBLIC raylib glfw eecs edat)
target_link_libraries(dcengine PUBLIC raylib glfw eecs edat)
target_include_directories(editor PUBLIC include)
target_include_directories(dcengine PUBLIC include)

