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

class ArgusVideoCapture
{
    private:
        const static inline uint64_t ONE_SECOND_IN_NANOSECONDS = 1000000000U;
        const static inline uint64_t FIVE_SECONDS_IN_NANOSECONDS = 5000000000U;

        const int32_t cameraDeviceIndex, sensorModeIndex;
        Argus::UniqueObj<Argus::CameraProvider> cameraProvider;
        Argus::ICameraProvider* iCameraProvider;
        std::vector<Argus::CameraDevice*> cameraDevices;
        std::vector<Argus::SensorMode*> sensorModes;
        Argus::Size2D<uint32_t> resolution;
        Argus::ICaptureSession* iSession;
//      Argus::IEventProvider* iEventProvider;
//      Argus::UniqueObj<Argus::Request> request;
//      Argus::UniqueObj<Argus::EventQueue> queue;
//      Argus::IEventQueue* iQueue;
//      EGLStream::IFrameConsumer* iFrameConsumer;
        int32_t dmaBufferFd;
        uint32_t captureId;
        uint64_t timestamp;

        Argus::UniqueObj<Argus::CaptureSession> captureSession;
//      Argus::UniqueObj<Argus::OutputStreamSettings> streamSettings;

    public:
        static void displayAttachedCameraInfo();

        ArgusVideoCapture(const int32_t deviceIndex, const int32_t sensorModeIndex);
        ~ArgusVideoCapture();

        cv::Mat grab();
        cv::Size2i getResolution() const;
        uint64_t getTimestamp() const;
        uint32_t getCaptureId() const;

    private:
        static void displayUUID(const Argus::UUID& uuid);
        static void displaySensorModeInfo(Argus::SensorMode* sensorMode, const char* indent);
};

#endif
