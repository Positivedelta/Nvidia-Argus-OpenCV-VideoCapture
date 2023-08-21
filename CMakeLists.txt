cmake_minimum_required (VERSION 3.10)

project(argus-opencv-video-capture)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
set (CMAKE_CXX_STANDARD 17)

find_package(Argus REQUIRED)
find_package(NVMMAPI REQUIRED)
find_package(OpenCV REQUIRED)

# notes 1, setup the install base path for the Argus CSI OpenCV camera shared library, include files and the test applications
#       2, setup the RPATH for the test applications in bin/ directory
#       3, choose the /usr/local/bit-parallel path once ready to deploy in production (or edit to suit your needs)
#
#set(BIT_PARALLEL_INSTALL_ROOT "/usr/local/bit-parallel")
set(BIT_PARALLEL_INSTALL_ROOT "${PROJECT_SOURCE_DIR}/bit-parallel")
set(CMAKE_INSTALL_RPATH "${BIT_PARALLEL_INSTALL_ROOT}/lib")

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

# the make -install target for the library and include files
#
install(TARGETS argus-opencv-videocapture-bp-v1.0 DESTINATION ${BIT_PARALLEL_INSTALL_ROOT}/lib)
file(GLOB ARGUS_CAMERA_HPP_FILES "${PROJECT_SOURCE_DIR}/include/*.hpp")
install(FILES ${ARGUS_CAMERA_HPP_FILES} DESTINATION ${BIT_PARALLEL_INSTALL_ROOT}/include/camera)

# build the demo camera application, linked against the above shared library
# add a make -install target
#
add_executable(argus-csi-camera-demo ${PROJECT_SOURCE_DIR}/src/applications/camera_demo.cpp)
target_link_libraries(argus-csi-camera-demo argus-opencv-videocapture-bp-v1.0)
install(TARGETS argus-csi-camera-demo DESTINATION ${BIT_PARALLEL_INSTALL_ROOT}/bin/camera)
