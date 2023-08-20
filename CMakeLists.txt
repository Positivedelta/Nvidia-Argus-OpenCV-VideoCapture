cmake_minimum_required (VERSION 3.10)

project(argus-opencv-video-capture)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
set (CMAKE_CXX_STANDARD 17)

find_package(Argus REQUIRED)
find_package(NVMMAPI REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${ARGUS_INCLUDE_DIRS} ${NVMMAPI_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include)

# create a shared library for use with other applications
# notes 1, deal with NvBuffer and NvBufSurface difference between JetPack v4 and v5 respectively
#       2, not detecting JetPack versions less than 4 as there's not much point...
#
add_library(argus-opencv-videocapture-bp-v1.0 SHARED ${PROJECT_SOURCE_DIR}/src/argus_opencv_video_capture.cpp ${PROJECT_SOURCE_DIR}/src/argus_camera_settings.cpp)
target_link_libraries(argus-opencv-videocapture-bp-v1.0 ${ARGUS_LIBRARIES} ${NVMMAPI_LIBRARIES} ${OpenCV_LIBS})
string(SUBSTRING $ENV{JETSON_JETPACK} 0 1 JETPACK_MAJOR_VERSION)
if (${JETPACK_MAJOR_VERSION} MATCHES 4)
    message("JetPack 4.x detected, setting: JETPACK_4_DETECTED")
    target_compile_definitions(argus-opencv-videocapture-bp-v1.0 PRIVATE JETPACK_4_DETECTED)
else()
    message("JetPack 5.x or greater detected, setting: JETPACK_5_OR_GREATER_DETECTED")
    target_compile_definitions(argus-opencv-videocapture-bp-v1.0 PRIVATE JETPACK_5_OR_GREATER_DETECTED)
endif()

# build the demo camera application, linked against the above shared library
#
add_executable(argus-csi-camera-demo ${PROJECT_SOURCE_DIR}/src/applications/camera_demo.cpp)
target_link_libraries(argus-csi-camera-demo argus-opencv-videocapture-bp-v1.0)
