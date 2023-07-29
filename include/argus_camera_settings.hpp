//
// (c) Bit Parallel Ltd, July 2023
//

#ifndef ALLOTHETIC_ARGUS_CAMERA_SETTINGS_HPP
#define ALLOTHETIC_ARGUS_CAMERA_SETTINGS_HPP

#include <cstdint>
#include <tuple>

#include <Argus/Argus.h>

//
// FIXME! remove the API dependency on Argus::AwbMode
//

class ArgusCameraSettings
{
    public:
        const static inline uint32_t MIN_VALUE = 0;
        const static inline uint32_t MAX_VALUE = 1;

    private:
        Argus::ISourceSettings*& iSourceSettings;
        Argus::IAutoControlSettings*& iAutoControlSettings;

        //
        // a subset of the argus camera settings, see the Nvidia documenation for the full details
        //
        // *frame duration time range
        // *frame rate
        // *exposure time range
        // *gain range
        // *auto exposure lock
        // *auto white balance lock
        // *auto white balance mode
        //
        // ISP digital gain range
        // AC region horizontal
        // AC region vertical
        // YUV format
        // focus position
        // aperture position
        // aperture f-num
        // aperture motor speed
        // de-noise mode
        // de-noise strength
        // edge enhance mode
        // edge enjance strength
        // auto exposure anitbanding mode
        //

    public:
        ArgusCameraSettings(Argus::ISourceSettings*& iSourceSettings, Argus::IAutoControlSettings*& iAutoControlSettings);

        std::tuple<uint64_t, uint64_t> getFrameDurationRange() const;
        bool getFrameDurationRange(const std::tuple<uint64_t, uint64_t> range);

        // note, these methods are implemented using the above get/setFrameDurationRange()
        //
        double getFrameRate() const;
        bool setFrameRate(const double rate);

        bool getAutoExposureLock() const;
        bool setAutoExposureLock(const bool lock);
        std::tuple<uint64_t, uint64_t> getExposureTimeRange() const;
        bool setExposureTime(const uint64_t time);
        bool setExposureTimeRange(const std::tuple<uint64_t, uint64_t>& range);

        std::tuple<float, float> getGainRange() const;
        bool setGain(const float gain);
        bool setGainRange(const std::tuple<float, float>& range);

        bool getAutoWhiteBalanceLock() const;
        bool setAutoWhiteBalanceLock(const bool lock);
        Argus::AwbMode getAutoWhiteBalanceMode() const;
        bool setAutoWhiteBalanceMode(const Argus::AwbMode& mode);
};

#endif
