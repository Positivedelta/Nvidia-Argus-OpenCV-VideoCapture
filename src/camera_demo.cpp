//
// (c) Bit Parallel Ltd - 2023
//

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <Argus/Argus.h>
#include <opencv2/opencv.hpp>

#include "argus_camera_settings.hpp"
#include "argus_opencv_video_capture.hpp"

int32_t main(int32_t argc, char** argv)
{
    std::cout << "Argus CSI Camera Demo\n";

    try
    {
        ArgusVideoCapture::displayAttachedCameraInfo();

        const auto camera = 0;
        const auto sensorMode = 5;
        auto capture = ArgusVideoCapture(camera, sensorMode);
        auto& settings = capture.getCameraSettings();
        settings.setFrameRate(30.0);
        settings.setAutoWhiteBalanceMode(Argus::AWB_MODE_AUTO);
        capture.restart();

        std::cout << "\nSelected Camera Settings:\n";
        std::cout << "\tMinimum Frame Rate: " << settings.getFrameRate() << " FPS (" << settings.getFrameDurationRange().min() << "ns)\n";
        std::cout << "\tAuto Exposure lock: " << (settings.getAutoExposureLock() ? "On" : "Off") << "\n";
        std::cout << "\tExposure Time Range, Min: " << settings.getExposureTimeRange().min() << "ns, Max: " << settings.getExposureTimeRange().max() << "ns\n";
        std::cout << "\tGain Range, Min: " << settings.getGainRange().min() << ", Max: " << settings.getGainRange().max() << "\n";
        std::cout << "\tAuto White Balance lock: " << (settings.getAutoWhiteBalanceLock() ? "On" : "Off") << "\n";
        std::cout << "\tAuto White Balance Mode: " << settings.getAutoWhiteBalanceMode().getName() << "\n";

        const auto fpsLocation = cv::Point(6, 30);
        const auto fpsColour = cv::Scalar(0xff, 0xff, 0xff);
        auto previousTimestamp = uint64_t(0);
        while (true)
        {
            auto cvFrame = capture.grab();

            auto fps = std::stringstream();
            fps << "FPS: " << std::fixed << std::setprecision(2) << (1000000000.0 / int32_t(capture.getTimestamp() - previousTimestamp));
            cv::putText(cvFrame, fps.str(), fpsLocation, cv::FONT_HERSHEY_COMPLEX, 1, fpsColour, 2); 
            cv::imshow("CSI Live Camera Video", cvFrame);

            previousTimestamp = capture.getTimestamp();

//          const auto cvFrameSize = capture.getResolution();
//          std::cout << "Resolution: (" << cvFrameSize.width << ", " << cvFrameSize.height << ")\n";
//          std::cout << "Timestamp: " << capture.getTimestamp() << ", Capture ID: " << capture.getCaptureId() << "\n";

            if (cv::waitKey(1) == 27) break;
        }

        cv::destroyAllWindows();
    }
    catch (const std::string& message)
    {
        std::cout << "Error: " << message << "\n";
    }
}
