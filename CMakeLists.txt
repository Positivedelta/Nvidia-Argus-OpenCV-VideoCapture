cmake_minimum_required (VERSION 3.10)

project(argus-opencv-video-capture)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
set (CMAKE_CXX_STANDARD 17)

find_package(Argus REQUIRED)
find_package(NVMMAPI REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${ARGUS_INCLUDE_DIRS} ${NVMMAPI_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include)

add_executable(camera-demo ${PROJECT_SOURCE_DIR}/src/camera_demo.cpp ${PROJECT_SOURCE_DIR}/src/argus_opencv_video_capture.cpp ${PROJECT_SOURCE_DIR}/src/argus_camera_settings.cpp)

target_link_libraries(camera-demo ${ARGUS_LIBRARIES} ${NVMMAPI_LIBRARIES} ${OpenCV_LIBS})
