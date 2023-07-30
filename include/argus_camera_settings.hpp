//
// (c) Bit Parallel Ltd, July 2023
//

#ifndef ALLOTHETIC_ARGUS_CAMERA_SETTINGS_HPP
#define ALLOTHETIC_ARGUS_CAMERA_SETTINGS_HPP

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <tuple>

#include <Argus/Argus.h>

//
// note, the Argus::Range and Argus::AwbMode classes have been abstracted away in the API
//       not sure if this was sensible or not... they can always be restored!
//

class ArgusCameraSettings
{
    public:
        const static inline uint32_t MIN_VALUE = 0;
        const static inline uint32_t MAX_VALUE = 1;

        // note, this ordering must be maintained with the awbModeLookup array below
        //
        const static inline int32_t AWB_MODE_OFF = 0;
        const static inline int32_t AWB_MODE_AUTO = 1;
        const static inline int32_t AWB_MODE_INCANDESCENT = 2;
        const static inline int32_t AWB_MODE_FLUORESCENT = 3;
        const static inline int32_t AWB_MODE_WARM_FLUORESCENT = 4;
        const static inline int32_t AWB_MODE_DAYLIGHT = 5;
        const static inline int32_t AWB_MODE_CLOUDY_DAYLIGHT = 6;
        const static inline int32_t AWB_MODE_TWILIGHT = 7;
        const static inline int32_t AWB_MODE_SHADE = 8;
        const static inline int32_t AWB_MODE_MANUAL = 9;

    private:
        static inline std::array<std::reference_wrapper<const Argus::AwbMode>, 10> awbModeLookup {
            Argus::AWB_MODE_OFF, Argus::AWB_MODE_AUTO, Argus::AWB_MODE_INCANDESCENT,
            Argus::AWB_MODE_FLUORESCENT, Argus::AWB_MODE_WARM_FLUORESCENT,
            Argus::AWB_MODE_DAYLIGHT, Argus::AWB_MODE_CLOUDY_DAYLIGHT,
            Argus::AWB_MODE_TWILIGHT, Argus::AWB_MODE_SHADE, Argus::AWB_MODE_MANUAL
        };

        static inline std::map<std::string, int32_t> awbIndexLookup {
            {std::string(Argus::AWB_MODE_OFF.getName()), AWB_MODE_OFF}, {std::string(Argus::AWB_MODE_AUTO.getName()), AWB_MODE_AUTO},
            {std::string(Argus::AWB_MODE_INCANDESCENT.getName()), AWB_MODE_INCANDESCENT}, {std::string(Argus::AWB_MODE_FLUORESCENT.getName()), AWB_MODE_FLUORESCENT},
            {std::string(Argus::AWB_MODE_WARM_FLUORESCENT.getName()), AWB_MODE_WARM_FLUORESCENT}, {std::string(Argus::AWB_MODE_DAYLIGHT.getName()), AWB_MODE_DAYLIGHT},
            {std::string(Argus::AWB_MODE_CLOUDY_DAYLIGHT.getName()), AWB_MODE_CLOUDY_DAYLIGHT}, {std::string(Argus::AWB_MODE_TWILIGHT.getName()), AWB_MODE_TWILIGHT},
            {std::string(Argus::AWB_MODE_SHADE.getName()), AWB_MODE_SHADE}, {std::string(Argus::AWB_MODE_MANUAL.getName()), AWB_MODE_MANUAL}
        };

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
        int32_t getAutoWhiteBalanceMode() const;
        std::string getAutoWhiteBalanceModeName() const;
        bool setAutoWhiteBalanceMode(const int32_t mode);
};

#endif
