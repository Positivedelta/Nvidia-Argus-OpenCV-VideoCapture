//
// (c) Bit Parallel Ltd - 2023
//

#include <cstdint>
#include <iostream>
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

        auto previousTimestamp = uint64_t(0);
        while (true)
        {
            auto cvFrame = capture.grab();
            cv::imshow("CSI Camera Image Grab", cvFrame);

//          const auto cvFrameSize = capture.getResolution();
//          std::cout << "Resolution: (" << cvFrameSize.width << ", " << cvFrameSize.height << ")\n";
//          std::cout << "Timestamp: " << capture.getTimestamp() << ", Capture ID: " << capture.getCaptureId() << "\n";

            auto fps = 1000000000.0 / int32_t(capture.getTimestamp() - previousTimestamp);
            previousTimestamp = capture.getTimestamp();
            std::cout << fps << "\n";

            cv::waitKey(1);
        }
    }
    catch (const std::string& message)
    {
        std::cout << "Error: " << message << "\n";
    }
}
