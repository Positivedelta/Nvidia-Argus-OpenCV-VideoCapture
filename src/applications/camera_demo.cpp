//
// (c) Bit Parallel Ltd, August 2023
//

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <tuple>

#include <opencv2/opencv.hpp>

#include "argus_camera_settings.hpp"
#include "argus_opencv_video_capture.hpp"

int32_t main(int32_t argc, char** argv)
{
    std::cout << "Argus CSI Camera Demo\n";

    try
    {
        if ((argc == 2) && ((std::string(argv[1]) == "-q") || (std::string(argv[1]) == "-query-camera-devices")))
        {
            bpl::ArgusCameraSettings::displayAttachedCameraInfo();
            return 0;
        }
        else if ((argc != 5) || (std::string(argv[1]) != "-d") || (std::string(argv[3]) != "-m"))
        {
            std::cout << "Invalid arguments, please use:\n";
            std::cout << argv[0] << " -q, --query-camera-devices\n";
            std::cout << argv[0] << " -d [#camera device] -m [#sensor mode]\n";
            return 0;
        }

        auto camera = 0;
        auto sensorMode = 0;
        try
        {
            camera = std::stoi(argv[2]);
            sensorMode = std::stoi(argv[4]);
        }
        catch (const std::exception& ex)
        {
            std::cout << "Invalid numeric parameter\n";
            return 0;
        }

        auto capture = bpl::ArgusVideoCapture(camera, sensorMode);
        auto& settings = capture.getCameraSettings();
        settings.setFrameRate(30.0);
        settings.setAutoWhiteBalanceMode(bpl::ArgusCameraSettings::AWB_MODE_AUTO);
        capture.restart();

        std::cout << "Selected Camera Settings:\n";
        std::cout << "\tMinimum Frame Rate: " << settings.getFrameRate() << " FPS\n";
        std::cout << "\tAuto White Balance Mode: " << settings.getAutoWhiteBalanceModeName() << "\n";
        std::cout << "\tAuto White Balance lock: " << (settings.getAutoWhiteBalanceLock() ? "On" : "Off") << "\n";

        const auto [minExposureTime, maxExposureTime] = settings.getExposureTimeRange();
        std::cout << "\tExposure Time Range, Min: " << minExposureTime << "ns, Max: " << maxExposureTime << "ns\n";
        std::cout << "\tAuto Exposure lock: " << (settings.getAutoExposureLock() ? "On" : "Off") << "\n";

        const auto [minGain, maxGain] = settings.getGainRange();
        std::cout << "\tGain Range, Min: " << minGain << ", Max: " << maxGain << "\n";
        std::cout << std::endl;

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

            if (cv::waitKey(1) == 27)
            {
//              // as a test, save the final frame as a JPEG
//              //
//              capture.saveAsJPEG("capture.jpg");
                break;
            }
        }

        cv::destroyAllWindows();
    }
    catch (const std::string& message)
    {
        std::cout << "Error: " << message << "\n";
    }
}
