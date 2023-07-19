//
// (c) Bit Parallel Ltd, July 2023
//

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <Argus/Argus.h>
#include <Argus/Ext/DolWdrSensorMode.h>
#include <Argus/Ext/PwlWdrSensorMode.h>

#include "argus_opencv_video_capture.hpp"

ArgusVideoCapture::ArgusVideoCapture()
{
}

cv::Mat ArgusVideoCapture::grab()
{
    return cvFrameBuffer;
}

void ArgusVideoCapture::displayAttachedCameraInfo()
{
    const char* indent = "    ";

    auto cameraProvider = Argus::UniqueObj<Argus::CameraProvider>(Argus::CameraProvider::create());
    auto* iCameraProvider = Argus::interface_cast<Argus::ICameraProvider>(cameraProvider);
    if (!iCameraProvider) throw std::string("Unable to get the core Argus::ICameraProvider interface");

    std::cout << "Argus Version: " << iCameraProvider->getVersion() << "\n";

    auto cameraDevices = std::vector<Argus::CameraDevice*>();
    auto status = iCameraProvider->getCameraDevices(&cameraDevices);
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Failed to get any camera devices from the provider\n";
        return;
    }

    if (cameraDevices.size() == 0)
    {
        std::cout << "No camera devices are available\n";
        return;
    }

    std::cout << "Available Camera Devices:\n";
    for (auto i = 0; i < cameraDevices.size(); i++)
    {
        std::cout << "  ==== CameraDevice " << i << ": =========================================\n";

        auto* cameraDevice = cameraDevices[i];
        auto* iCameraProperties = Argus::interface_cast<Argus::ICameraProperties>(cameraDevice);
        if (iCameraProperties)
        {
            auto sensorModes = std::vector<Argus::SensorMode*>();
            iCameraProperties->getAllSensorModes(&sensorModes);
            std::cout << indent << "UUID:                      ";
            displayUUID(iCameraProperties->getUUID());

            std::cout << indent << "MaxAeRegions:              " << iCameraProperties->getMaxAeRegions() << "\n";
            std::cout << indent << "MaxAwbRegions:             " << iCameraProperties->getMaxAwbRegions() << "\n";

            auto i32Range = iCameraProperties->getFocusPositionRange();
            std::cout << indent << "FocusPositionRange:        [" << i32Range.min() << ", " <<  i32Range.max() << "]\n";

            std::cout << indent << "LensApertureRange:    ";
            auto availableFnums = std::vector<float>();
            iCameraProperties->getAvailableApertureFNumbers(&availableFnums);
            for (auto fnum : availableFnums) std::cout << indent << " " << fnum;
            std::cout << "\n";

            auto fRange = iCameraProperties->getIspDigitalGainRange();
            std::cout << indent << "IspDigitalGainRange:       [" << fRange.min() << ", " << fRange.max() << "]\n";

            fRange = iCameraProperties->getExposureCompensationRange();
            std::cout << indent << "ExposureCompensationRange: [" << fRange.min() << ", " << fRange.max() << "]\n";
            std::cout << indent << "NumSensorModes:            " << sensorModes.size() << "\n";

            char modeIndent[32];
            snprintf(modeIndent, sizeof(modeIndent), "%s    ", indent);
            for (auto i = 0; i < sensorModes.size(); i++)
            {
                std::cout << indent << "SensorMode " << i << ":\n";
                displaySensorModeInfo(sensorModes[i], modeIndent);
            }
        }
    }
}

//
// private methods
//

void ArgusVideoCapture::displayUUID(const Argus::UUID& uuid)
{
    auto iosFlags = std::ios_base::fmtflags(std::cout.flags());
    std::cout << std::setfill('0') << std::setw(8) << std::hex << uuid.time_low << ",";
    std::cout << std::setw(4) << std::hex << uuid.time_mid << "," << std::setw(4) << uuid.time_hi_and_version << "," << std::setw(4) << uuid.clock_seq << ",";
    std::cout << std::setw(2) << uint32_t(uuid.node[0]) << "," << std::setw(2) << uint32_t(uuid.node[1]) << "," << std::setw(2) << uint32_t(uuid.node[2]) << ",";
    std::cout << std::setw(2) << uint32_t(uuid.node[3]) << "," << std::setw(2) << uint32_t(uuid.node[4]) << "," << std::setw(2) << uint32_t(uuid.node[5]) << "\n";
    std::cout.flags(iosFlags);
}

void ArgusVideoCapture::displaySensorModeInfo(Argus::SensorMode* sensorMode, const char* indent)
{
    auto iosFlags = std::ios_base::fmtflags(std::cout.flags());

    auto* iSensorMode = Argus::interface_cast<Argus::ISensorMode>(sensorMode);
    if (iSensorMode)
    {
        auto resolution = iSensorMode->getResolution();
        std::cout << indent << "Resolution:         " << resolution.width() << "x" << resolution.height() << "\n";

        auto hdrRatioRange = iSensorMode->getHdrRatioRange();
        std::cout << indent << "HdrRatioRange:      [" << std::setprecision(6) << std::fixed << hdrRatioRange.min() << ", " << hdrRatioRange.max() << "]\n";
        std::cout.flags(iosFlags);

        auto u64TimeRange = iSensorMode->getExposureTimeRange();
        if (hdrRatioRange.max() > 1.0f)
        {
            std::cout << indent << "ExposureTimeRange for long exposure:  [" << u64TimeRange.min() << ", " << u64TimeRange.max() << "]\n";

            const auto shortMin = uint64_t(u64TimeRange.min() / hdrRatioRange.max());
            const auto shortMax = uint64_t(u64TimeRange.max() / hdrRatioRange.min());
            std::cout << indent << "ExposureTimeRange for short exposure: [" << shortMin << ", " << shortMax << "]\n";
        }

        auto u64FrameRange = iSensorMode->getFrameDurationRange();
        std::cout << indent << "FrameDurationRange: [" << u64FrameRange.min() << ", " << u64FrameRange.max() << "]\n";

        auto fpsMin = 1000000000.0 / u64FrameRange.max();
        auto fpsMax = 1000000000.0 / u64FrameRange.min();
        std::cout << indent << "                    (" << std::setprecision(2) << std::fixed << fpsMin << " to " << fpsMax << " fps)\n";

        auto aGainRange = iSensorMode->getAnalogGainRange();
        std::cout << indent << "AnalogGainRange:    [" << std::setprecision(6) << std::fixed << aGainRange.min() << ", " << aGainRange.max() << "]\n";
        std::cout << indent << "InputBitDepth:      " << iSensorMode->getInputBitDepth() << "\n";
        std::cout << indent << "OutputBitDepth:     " << iSensorMode->getOutputBitDepth() << "\n";
        std::cout << indent << "SensorModeType:     " << iSensorMode->getSensorModeType().getName() << "\n";

        // note, move this appropriately if further formatting is used
        //
        std::cout.flags(iosFlags);

        auto* dolMode = Argus::interface_cast<Argus::Ext::IDolWdrSensorMode>(sensorMode);
        auto* pwlMode = Argus::interface_cast<Argus::Ext::IPwlWdrSensorMode>(sensorMode);
        if (dolMode)
        {
            std::cout << indent << "DOL WDR Mode Properties:\n";
            std::cout << indent << "  ExposureCount:        " << dolMode->getExposureCount() << "\n";
            std::cout << indent << "  OpticalBlackRowCount: " << dolMode->getOpticalBlackRowCount() << "\n";

            auto vbpRowCounts = std::vector<uint32_t>();
            dolMode->getVerticalBlankPeriodRowCount(&vbpRowCounts);
            std::cout << indent << "  VBPRowCounts:         [" << vbpRowCounts[0];
            for (auto i = 1; i < vbpRowCounts.size(); i++) std::cout << ", " << vbpRowCounts[i];
            std::cout << indent << "]\n";

            std::cout << indent << "  LineInfoMarkerWidth:  " << dolMode->getLineInfoMarkerWidth() << "\n";
            std::cout << indent << "  LeftMarginWidth:      " << dolMode->getLeftMarginWidth() << "\n";
            std::cout << indent << "  RightMarginWidth:     " << dolMode->getRightMarginWidth() << "\n";

            resolution = dolMode->getPhysicalResolution();
            std::cout << indent << "  PhysicalResolution:   " << resolution.width() << "x" << resolution.height() << "\n";
        }
        else if (pwlMode)
        {
            std::cout << indent << "PWL WDR Mode Properties:\n";
            std::cout << indent << "  ControlPointCount:    " << pwlMode->getControlPointCount() << "\n";

            auto controlPoints = std::vector<Argus::Point2D<float>>();
            pwlMode->getControlPoints(&controlPoints);
            std::cout << indent << "  ControlPoints:        {(" << controlPoints[0].x() << ", " << controlPoints[0].y() << ")";
            for (auto i = 1; i < controlPoints.size(); i++) std::cout << ", (" << controlPoints[i].x() << ", " << controlPoints[i].y() << ")";
            std::cout << "}\n";
        }
        else
        {
            std::cout << indent << "WDR Mode: No\n";
        }
    }
}
