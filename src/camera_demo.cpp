//
// (c) Bit Parallel Ltd - 2023
//

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <opencv2/opencv.hpp>

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
