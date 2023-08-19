cmake_minimum_required (VERSION 3.10)

project(argus-opencv-video-capture)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
set (CMAKE_CXX_STANDARD 17)

find_package(Argus REQUIRED)
find_package(NVMMAPI REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${ARGUS_INCLUDE_DIRS} ${NVMMAPI_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include)

# deal with NvBuffer and NvBufSurface difference between JetPack v4 and v5 respectively
#
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/argus_camera_settings.cpp)
string(SUBSTRING $ENV{JETSON_JETPACK} 0 1 JETPACK_MAJOR_VERSION)
if (${JETPACK_MAJOR_VERSION} MATCHES 5)
    message("JetPack 5.x detected, the build will use argus_opencv_video_capture_jpv5.cpp")
    list(APPEND SRC_FILES ${PROJECT_SOURCE_DIR}/src/argus_opencv_video_capture_jpv5.cpp)
elseif (${JETPACK_MAJOR_VERSION} MATCHES 4)
    message("JetPack 4.x detected, the build will use argus_opencv_video_capture_jpv4.cpp")
    list(APPEND SRC_FILES ${PROJECT_SOURCE_DIR}/src/argus_opencv_video_capture_jpv4.cpp)
else()
    message(FATAL_ERROR "Unknown version of JetPack detected: " $ENV{JETSON_JETPACK})
endif()

# create a shared library for use with other applications
#
add_library(argus-opencv-videocapture-bp-v1.0 SHARED ${SRC_FILES})
target_link_libraries(argus-opencv-videocapture-bp-v1.0 ${ARGUS_LIBRARIES} ${NVMMAPI_LIBRARIES} ${OpenCV_LIBS})

# build the demo camera application, linked against the above shared library
#
add_executable(argus-csi-camera-demo ${PROJECT_SOURCE_DIR}/src/applications/camera_demo.cpp)
target_link_libraries(argus-csi-camera-demo argus-opencv-videocapture-bp-v1.0)
