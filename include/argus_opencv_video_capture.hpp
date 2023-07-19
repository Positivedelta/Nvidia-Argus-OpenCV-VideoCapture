//
// (c) Bit Parallel Ltd, July 2023
//

#ifndef ALLOTHETIC_ARGUS_OPENCV_VIDEO_CAPTURE_HPP
#define ALLOTHETIC_ARGUS_OPENCV_VIDEO_CAPTURE_HPP

#include <Argus/Argus.h>
#include <opencv2/opencv.hpp>

class ArgusVideoCapture
{
    private:
        cv::Mat cvFrameBuffer;

    public:
        static void displayAttachedCameraInfo();

        ArgusVideoCapture();
        cv::Mat grab();

    private:
        static void displayUUID(const Argus::UUID& uuid);
        static void displaySensorModeInfo(Argus::SensorMode* sensorMode, const char* indent);
};

#endif
