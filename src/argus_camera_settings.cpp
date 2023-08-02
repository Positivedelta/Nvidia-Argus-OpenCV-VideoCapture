//
// (c) Bit Parallel Ltd, July 2023
//

#include <iostream>
#include <iomanip>

#include <Argus/Argus.h>
#include <Argus/Ext/DolWdrSensorMode.h>
#include <Argus/Ext/PwlWdrSensorMode.h>

#include "argus_camera_settings.hpp"

ArgusCameraSettings::ArgusCameraSettings(Argus::ISourceSettings*& iSourceSettings, Argus::IAutoControlSettings*& iAutoControlSettings):
    iSourceSettings(iSourceSettings), iAutoControlSettings(iAutoControlSettings) {
}

std::tuple<uint64_t, uint64_t> ArgusCameraSettings::getFrameDurationRange() const
{
    const auto range = iSourceSettings->getFrameDurationRange();
    return {range.min(), range.max()};
}

bool ArgusCameraSettings::getFrameDurationRange(const std::tuple<uint64_t, uint64_t> range)
{
    const auto status = iSourceSettings->setFrameDurationRange(Argus::Range(std::get<MIN_VALUE>(range), std::get<MAX_VALUE>(range)));
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: The call to setFrameDurationRange({" << std::get<MIN_VALUE>(range) << ", " << std::get<MAX_VALUE>(range) << "}) has failed\n";
        return false;
    }

    return true;
}

double ArgusCameraSettings::getFrameRate() const
{
    const auto range = iSourceSettings->getFrameDurationRange();
    return 1000000000.0 / range.min();
}

bool ArgusCameraSettings::setFrameRate(const double rate)
{
    const auto status = iSourceSettings->setFrameDurationRange(Argus::Range<uint64_t>(1000000000 / rate));
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: The call to setFrameRate(" << rate << ") has failed\n";
        return false;
    }

    return true;
}

bool ArgusCameraSettings::getAutoExposureLock() const
{
    return iAutoControlSettings->getAeLock();
}

bool ArgusCameraSettings::setAutoExposureLock(const bool lock)
{
    const auto status = iAutoControlSettings->setAeLock(lock);
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: The call to setAutoExposureLock(" << lock << ") has failed\n";
        return false;
    }

    return true;
}

std::tuple<uint64_t, uint64_t> ArgusCameraSettings::getExposureTimeRange() const
{
    const auto range = iSourceSettings->getExposureTimeRange();
    return {range.min(), range.max()};
}

bool ArgusCameraSettings::setExposureTime(uint64_t time)
{
    return setExposureTimeRange({time, time});
}

bool ArgusCameraSettings::setExposureTimeRange(const std::tuple<uint64_t, uint64_t>& range)
{
    const auto status = iSourceSettings->setExposureTimeRange(Argus::Range(std::get<MIN_VALUE>(range), std::get<MAX_VALUE>(range)));
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: The call to setExposureTimeRange({" << std::get<MIN_VALUE>(range) << ", " << std::get<MAX_VALUE>(range) << "}) has failed\n";
        return false;
    }

    return true;
}

std::tuple<float, float> ArgusCameraSettings::getGainRange() const
{
    const auto range = iSourceSettings->getGainRange();
    return {range.min(), range.max()};
}

bool ArgusCameraSettings::setGain(const float gain)
{
    return setGainRange({gain, gain});
}

bool ArgusCameraSettings::setGainRange(const std::tuple<float, float>& range)
{
    const auto status = iSourceSettings->setGainRange(Argus::Range(std::get<MIN_VALUE>(range), std::get<MAX_VALUE>(range)));
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: The call to setGainRange({" << std::get<MIN_VALUE>(range) << ", " << std::get<MAX_VALUE>(range) << "}) has failed\n";
        return false;
    }

    return true;
}

bool ArgusCameraSettings::getAutoWhiteBalanceLock() const
{
    return iAutoControlSettings->getAwbLock();
}

bool ArgusCameraSettings::setAutoWhiteBalanceLock(const bool lock)
{
    const auto status = iAutoControlSettings->setAwbLock(lock);
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: The call to setAutoWhiteBalanceLock(" << lock << ") has failed\n";
        return false;
    }

    return true;
}

int32_t ArgusCameraSettings::getAutoWhiteBalanceMode() const
{
    return awbIndexLookup[std::string(iAutoControlSettings->getAwbMode().getName())];
}

std::string ArgusCameraSettings::getAutoWhiteBalanceModeName() const
{
    return std::string(iAutoControlSettings->getAwbMode().getName());
}

bool ArgusCameraSettings::setAutoWhiteBalanceMode(const int32_t mode)
{
    const auto& awbMode = awbModeLookup[mode];
    const auto status = iAutoControlSettings->setAwbMode(awbMode);
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: The call to setAutoWhiteBalanceMode(" << mode << ") has failed\n";
        return false;
    }

    return true;
}

//
// static methods
//

void ArgusCameraSettings::displayAttachedCameraInfo()
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

void ArgusCameraSettings::displayUUID(const Argus::UUID& uuid)
{
    auto iosFlags = std::ios_base::fmtflags(std::cout.flags());
    std::cout << std::setfill('0') << std::setw(8) << std::hex << uuid.time_low << ",";
    std::cout << std::setw(4) << std::hex << uuid.time_mid << "," << std::setw(4) << uuid.time_hi_and_version << "," << std::setw(4) << uuid.clock_seq << ",";
    std::cout << std::setw(2) << uint32_t(uuid.node[0]) << "," << std::setw(2) << uint32_t(uuid.node[1]) << "," << std::setw(2) << uint32_t(uuid.node[2]) << ",";
    std::cout << std::setw(2) << uint32_t(uuid.node[3]) << "," << std::setw(2) << uint32_t(uuid.node[4]) << "," << std::setw(2) << uint32_t(uuid.node[5]) << "\n";
    std::cout.flags(iosFlags);
}

void ArgusCameraSettings::displaySensorModeInfo(Argus::SensorMode* sensorMode, const char* indent)
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
