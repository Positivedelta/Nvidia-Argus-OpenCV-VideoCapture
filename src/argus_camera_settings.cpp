//
// (c) Bit Parallel Ltd, July 2023
//

#include <iostream>

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
