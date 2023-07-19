//
// (c) Bit Parallel Ltd - 2023
//

#include <cstdint>
#include <iostream>
#include <string>

#include "argus_opencv_video_capture.hpp"

int32_t main(int32_t argc, char** argv)
{
    std::cout << "Argus CSI Camera Demo\n";

    try
    {
        ArgusVideoCapture::displayAttachedCameraInfo();
    }
    catch (const std::string& message)
    {
        std::cout << "Error: " << message << "\n";
    }
}
