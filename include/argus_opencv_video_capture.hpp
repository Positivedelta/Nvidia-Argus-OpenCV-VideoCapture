//
// (c) Bit Parallel Ltd, July 2023
//

#ifndef ALLOTHETIC_ARGUS_OPENCV_VIDEO_CAPTURE_HPP
#define ALLOTHETIC_ARGUS_OPENCV_VIDEO_CAPTURE_HPP

#include <cstdint>
#include <vector>

#include <Argus/Argus.h>
#include <EGLStream/EGLStream.h>
#include <opencv2/opencv.hpp>

#include "argus_camera_settings.hpp"

class ArgusVideoCapture
{
    private:
        const static inline uint64_t ONE_SECOND_IN_NANOSECONDS = 1000000000UL;
        const static inline uint64_t FIVE_SECONDS_IN_NANOSECONDS = 5000000000UL;

        // FIXME! review these properties, some may not be needed
        //
        const int32_t cameraDeviceIndex, sensorModeIndex;
        Argus::UniqueObj<Argus::CameraProvider> cameraProvider;
        Argus::ICameraProvider* iCameraProvider;
        std::vector<Argus::CameraDevice*> cameraDevices;
        std::vector<Argus::SensorMode*> sensorModes;
        Argus::Size2D<uint32_t> resolution;
        Argus::UniqueObj<Argus::CaptureSession> captureSession;
        Argus::ICaptureSession* iSession;
        Argus::UniqueObj<Argus::OutputStream> stream;
        Argus::UniqueObj<EGLStream::FrameConsumer> consumer;
        EGLStream::IFrameConsumer* iFrameConsumer;
        Argus::UniqueObj<Argus::Request> request;
        Argus::ISourceSettings* iSourceSettings;
        Argus::IAutoControlSettings* iAutoControlSettings;
        ArgusCameraSettings argusCameraSettings;
        int32_t dmaBufferFd;
        void *cvImageBuffer;
        uint32_t captureId;
        uint64_t timestamp;

    public:
        static void displayAttachedCameraInfo();

        ArgusVideoCapture(const int32_t deviceIndex, const int32_t sensorModeIndex);
        ~ArgusVideoCapture();

        cv::Mat grab();
        cv::Size2i getResolution() const;
        uint64_t getTimestamp() const;
        uint32_t getCaptureId() const;

        ArgusCameraSettings& getCameraSettings();
        bool restart();

    private:
        static void displayUUID(const Argus::UUID& uuid);
        static void displaySensorModeInfo(Argus::SensorMode* sensorMode, const char* indent);
};

#endif
