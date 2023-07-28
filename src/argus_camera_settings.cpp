//
// (c) Bit Parallel Ltd, July 2023
//

#include <iostream>
#include <string>

#include "argus_camera_settings.hpp"

ArgusCameraSettings::ArgusCameraSettings(Argus::ISourceSettings*& iSourceSettings, Argus::IAutoControlSettings*& iAutoControlSettings):
    iSourceSettings(iSourceSettings), iAutoControlSettings(iAutoControlSettings) {
}

Argus::Range<uint64_t> ArgusCameraSettings::getFrameDurationRange() const
{
    return iSourceSettings->getFrameDurationRange();
}

bool ArgusCameraSettings::getFrameDurationRange(const Argus::Range<uint64_t> range)
{
    const auto status = iSourceSettings->setFrameDurationRange(range);
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: The call to setFrameDurationRange(Argus::Range<uint64_t>(" << range.min() << ", " << range.max() << ")) has failed\n";
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

Argus::Range<uint64_t> ArgusCameraSettings::getExposureTimeRange() const
{
    return iSourceSettings->getExposureTimeRange();
}

bool ArgusCameraSettings::setExposureTime(uint64_t time)
{
    return setExposureTimeRange(Argus::Range<uint64_t>(time));
}

bool ArgusCameraSettings::setExposureTimeRange(const Argus::Range<uint64_t>& range)
{
    const auto status = iSourceSettings->setExposureTimeRange(range);
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: The call to setExposureTimeRange(Argus::Range<uint64_t>(" << range.min() << ", " << range.max() << ")) has failed\n";
        return false;
    }

    return true;
}

Argus::Range<float> ArgusCameraSettings::getGainRange() const
{
    return iSourceSettings->getGainRange();
}

bool ArgusCameraSettings::setGain(const float gain)
{
    return setGainRange(Argus::Range<float>(gain));
}

bool ArgusCameraSettings::setGainRange(const Argus::Range<float>& range)
{
    const auto status = iSourceSettings->setGainRange(range);
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: The call to setGainRange(Argus::Range<uint64_t>(" << range.min() << ", " << range.max() << ")) has failed\n";
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

Argus::AwbMode ArgusCameraSettings::getAutoWhiteBalanceMode() const
{
    return iAutoControlSettings->getAwbMode();
}

bool ArgusCameraSettings::setAutoWhiteBalanceMode(const Argus::AwbMode& mode)
{
    const auto status = iAutoControlSettings->setAwbMode(mode);
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: The call to setAutoWhiteBalanceMode(" << mode.getName() << ") has failed\n";
        return false;
    }

    return true;
}
