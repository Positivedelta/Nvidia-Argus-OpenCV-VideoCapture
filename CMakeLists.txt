cmake_minimum_required (VERSION 3.10)

project(argus-opencv-video-capture)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
set (CMAKE_CXX_STANDARD 17)

find_package(Argus REQUIRED)
find_package(NVMMAPI REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${ARGUS_INCLUDE_DIRS} ${NVMMAPI_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include)

# create a shared library for use with other applications
#
add_library(argus-opencv-videocapture SHARED ${PROJECT_SOURCE_DIR}/src/argus_opencv_video_capture.cpp ${PROJECT_SOURCE_DIR}/src/argus_camera_settings.cpp)
target_link_libraries(argus-opencv-videocapture ${ARGUS_LIBRARIES} ${NVMMAPI_LIBRARIES} ${OpenCV_LIBS})

# build the demo camera application, linked against the above shared library
#
add_executable(camera-demo ${PROJECT_SOURCE_DIR}/src/camera_demo.cpp)
target_link_libraries(camera-demo argus-opencv-videocapture)
