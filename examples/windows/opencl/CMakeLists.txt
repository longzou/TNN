cmake_minimum_required(VERSION 3.1)
project(TNN-demo)
message(${CMAKE_SOURCE_DIR})
message(${TNN_LIB_PATH})

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CXX_STANDARD 11)
set(TNN_OPENCL_ENABLE ON)

if (TNN_DEMO_WITH_OPENCV)
    find_package(OpenCV 4 REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /w")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=c++11 -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -pthread -fPIC")
endif()

set(TNNRoot ${CMAKE_SOURCE_DIR}/../../../)
set(TNNInclude ${TNNRoot}/include)

include_directories(${TNNInclude})
include_directories(${CMAKE_SOURCE_DIR}/../../base)
include_directories(${CMAKE_SOURCE_DIR}/../../utils)
link_directories(${TNN_LIB_PATH})
if (${TNN_OPENVINO_ENABLE})
    link_directories(${TNN_OPENVINO_LIB_PATH})
    add_definitions(-D_OPENVINO_)
endif()

link_libraries(TNN)

add_subdirectory(${TNNRoot}/third_party/gflags ${TNNRoot}/third_party/gflags)
get_target_property(GFLAGS_INCLUDE_DIRS gflags INTERFACE_INCLUDE_DIRECTORIES)
include_directories(BEFORE "${GFLAGS_INCLUDE_DIRS}")
include_directories(${TNNRoot}/third_party/opencl/include)
link_libraries(gflags)

file(GLOB FLAG_SRC "${CMAKE_SOURCE_DIR}/../../linux/src/*.cc")

file(GLOB_RECURSE BASE_SRC
                    "${CMAKE_SOURCE_DIR}/../../base/*.cc")

file(GLOB_RECURSE UTIL_SRC
                   "${CMAKE_SOURCE_DIR}/../../utils/*.cc")

file(GLOB_RECURSE WEBCAM_SRC "${CMAKE_SOURCE_DIR}/../../base/ocr*.cc")
list(REMOVE_ITEM BASE_SRC ${WEBCAM_SRC})

add_definitions(-D_OPENCL_)
if (TNN_DEMO_WITH_OPENCV)
    add_definitions(-D_OPENCV_)
endif()

if(TNN_OPENCL_ENABLE)
    find_package(OpenCL)
    message(${OpenCL_LIBRARY})
    link_libraries(${OpenCL_LIBRARY})
endif()

if(SHARING_MEM_WITH_OPENGL)
    add_definitions(-DSHARING_MEM_WITH_OPENGL)
    add_definitions(-DGL_GLEXT_PROTOTYPES)
    find_package(OpenGL REQUIRED)
    include_directories(${TNNRoot}/third_party/glew/include)
    link_libraries(opengl32)
    link_directories(${TNNRoot}/third_party/glew/lib/Release/x64)
    link_libraries(glew32 glew32s)
endif()
add_executable(demo_opencl_imageclassify ${CMAKE_SOURCE_DIR}/../../linux/src/TNNImageClassify/TNNImageClassify.cc ${BASE_SRC} ${UTIL_SRC} ${FLAG_SRC})

if(SHARING_MEM_WITH_OPENGL)
    add_executable(demo_opencl_imageclassify_opengl ${CMAKE_SOURCE_DIR}/../../linux/src/TNNImageClassifyOpenGL/TNNImageClassifyOpenGL.cc ${BASE_SRC} ${UTIL_SRC} ${FLAG_SRC})
endif()
