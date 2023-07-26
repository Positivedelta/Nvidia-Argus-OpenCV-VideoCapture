//
// (c) Bit Parallel Ltd, July 2023
//

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <Argus/Argus.h>
#include <Argus/Ext/DolWdrSensorMode.h>
#include <Argus/Ext/PwlWdrSensorMode.h>
#include <EGLStream/NV/ImageNativeBuffer.h>

#include "argus_opencv_video_capture.hpp"

ArgusVideoCapture::ArgusVideoCapture(const int32_t cameraDeviceIndex, const int32_t sensorModeIndex):
    cameraDeviceIndex(cameraDeviceIndex), sensorModeIndex(sensorModeIndex), dmaBufferFd(0), cvImageBuffer(nullptr), timestamp(uint64_t(0)) {

    // set up the Argus API framework
    //
    cameraProvider = Argus::UniqueObj<Argus::CameraProvider>(Argus::CameraProvider::create());
    iCameraProvider = Argus::interface_cast<Argus::ICameraProvider>(cameraProvider);
    if (!iCameraProvider) throw std::string("Unable to get the core Argus::ICameraProvider interface");

    auto status = iCameraProvider->getCameraDevices(&cameraDevices);
    if ((status != Argus::STATUS_OK) || (cameraDevices.size() == 0)) throw std::string("Failed to find any camera devices");
    if (cameraDeviceIndex >= cameraDevices.size())
    {
        auto sb = std::stringstream();
        sb << "Camera device " << std::to_string(cameraDeviceIndex) << " was requested but the maximum device index is " << std::to_string(cameraDevices.size() - 1);

        throw sb.str();
    }

    auto* iCameraProperties = Argus::interface_cast<Argus::ICameraProperties>(cameraDevices[cameraDeviceIndex]);
    if (!iCameraProperties) throw std::string("Failed to get the Argus::ICameraProperties interface");

    status = iCameraProperties->getAllSensorModes(&sensorModes);
    if (status != Argus::STATUS_OK) throw std::string("Failed to get any sensor modes from the camera device");
    if (sensorModes.size() == 0) throw std::string("No sensor modes are available");
    if (sensorModeIndex >= sensorModes.size())
    {
        auto sb = std::stringstream();
        sb << "Sensor mode " << std::to_string(sensorModeIndex) << " was requested, but the maximum mode is " << std::to_string(sensorModes.size() - 1);

        throw sb.str();
    }

    auto* iSensorMode = Argus::interface_cast<Argus::ISensorMode>(sensorModes[sensorModeIndex]);
    if (!iSensorMode) throw std::string("Failed to get the Argus::ISensorMode interface");
    resolution = iSensorMode->getResolution();

    captureSession = Argus::UniqueObj<Argus::CaptureSession>(iCameraProvider->createCaptureSession(cameraDevices[cameraDeviceIndex], &status));
    if (status != Argus::STATUS_OK) throw std::string("Failed to create a Argus::CaptureSession instance");

    iSession = Argus::interface_cast<Argus::ICaptureSession>(captureSession);
    if (!iSession) throw std::string("Cannot get the Argus::ICaptureSession interface");

    // instantiate a stream between the Argus camera image capturing sub-system (the producer) and the image acquisition code (the consumer)
    // a consumer object is then created from the stream and is used to aquire image frames, see the ArgusVideoCapture::grab() method
    // note, using the default mailbox mode
    //
    auto streamSettings = Argus::UniqueObj<Argus::OutputStreamSettings>(iSession->createOutputStreamSettings(Argus::STREAM_TYPE_EGL));
    auto* iEGLStreamSettings = Argus::interface_cast<Argus::IEGLOutputStreamSettings>(streamSettings);
    if (!iEGLStreamSettings) throw std::string("Cannot get the Argus::IEGLOutputStreamSettings interface");
    iEGLStreamSettings->setPixelFormat(Argus::PIXEL_FMT_YCbCr_420_888);
    iEGLStreamSettings->setResolution(resolution);
    iEGLStreamSettings->setMetadataEnable(true);

    stream = Argus::UniqueObj<Argus::OutputStream>(iSession->createOutputStream(streamSettings.get()));
    if (!stream) throw std::string("Failed to create an Argus::OutputStream instance");

    consumer = Argus::UniqueObj<EGLStream::FrameConsumer>(EGLStream::FrameConsumer::create(stream.get()));
    iFrameConsumer = Argus::interface_cast<EGLStream::IFrameConsumer>(consumer);
    if (!iFrameConsumer) throw std::string("Failed to initialize the EGLStream::IFrameConsumer instance");

    request = Argus::UniqueObj<Argus::Request>(iSession->createRequest(Argus::CAPTURE_INTENT_STILL_CAPTURE));
    auto* iRequest = Argus::interface_cast<Argus::IRequest>(request);
    if (!iRequest) throw std::string("Failed to get the capture Argus::IRequest interface");

    status = iRequest->enableOutputStream(stream.get());
    if (status != Argus::STATUS_OK) throw std::string("Failed to enable the capture request stream");

    iSourceSettings = Argus::interface_cast<Argus::ISourceSettings>(request);
    if (!iSourceSettings) throw std::string("Failed to get the Argus::ISourceSettings interface");
    iSourceSettings->setSensorMode(sensorModes[sensorModeIndex]);

    // FIXME! add set / get methods for any appropriate source and control settings
    //
//  auto* iAutoControlSettings = Argus::interface_cast<Argus::IAutoControlSettings>(iRequest->getAutoControlSettings());
//  iAutoControlSettings->setAwbMode(Argus::AWB_MODE_OFF);
//  iAutoControlSettings->setAwbLock(true);
//  iAutoControlSettings->setAeAntibandingMode(Argus::AE_ANTIBANDING_MODE_OFF);
//  iAutoControlSettings->setAeLock(true);

    // FIXME! should there be a default frame rate?
    //        1, set here to 30 FPS
    //        2, although if you print the value beforehand it's set to 30 FPS, is the driver doing this?
    //
    iSourceSettings->setFrameDurationRange(Argus::Range<uint64_t>(33333334L));

    // begin capturing camera frames
    //
    status = iSession->repeat(request.get());
    if (status != Argus::STATUS_OK) throw std::string("Failed to trigger repeating capture requests");

    // FIXME! this seems like the correct thing to do, but the code works without the call to waitUntilConnected()
    //
    const auto* iEglOutputStream = Argus::interface_cast<Argus::IEGLOutputStream>(stream);
    status = iEglOutputStream->waitUntilConnected();
    if (status != Argus::STATUS_OK) throw std::string("The Argus::OutputStream has failed to connect");
}

ArgusVideoCapture::~ArgusVideoCapture()
{
    // note, no error checking as there is not much that can be done anyway...
    //
    iSession->stopRepeat();
    iSession->waitForIdle();

    if (dmaBufferFd)
    {
        NvBufferMemUnMap(dmaBufferFd, 0, &cvImageBuffer);
        NvBufferDestroy(dmaBufferFd);
    }
}

bool ArgusVideoCapture::setFrameRate(const double frameRate)
{
    bool success = true;

    iSession->stopRepeat();
    auto status = iSession->waitForIdle(ONE_SECOND_IN_NANOSECONDS);
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: Timeout whilst waiting for the repeating capture requests to stop\n";
        std::cout << "Warning: The call to setFrameRate(" << frameRate << ") may not have taken effect\n";
        success = false;
    }

    status = iSourceSettings->setFrameDurationRange(Argus::Range<uint64_t>(1000000000 / frameRate));
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: The call to setFrameRate(" << frameRate << ") has failed\n";
        success = false;
    }

    status = iSession->repeat(request.get());
    if (status != Argus::STATUS_OK) throw std::string("Failed to trigger repeating capture requests");

    return success;
}

cv::Mat ArgusVideoCapture::grab()
{
    // acquire a captured camera frame, using mailbox mode
    //
    auto status = Argus::STATUS_OK;
    const auto frame = Argus::UniqueObj<EGLStream::Frame>(iFrameConsumer->acquireFrame(FIVE_SECONDS_IN_NANOSECONDS, &status));
    if (status != Argus::STATUS_OK) throw std::string("Failed to aquire a camera frame from the EGLStream::IFrameConsumer instance");

    auto* iFrame = Argus::interface_cast<EGLStream::IFrame>(frame);
    if (!iFrame) throw std::string("Failed to get the EGLStream::IFrame interface");
    timestamp = iFrame->getTime();
    captureId = iFrame->getNumber();

    auto* image = iFrame->getImage();
    if (!image) throw std::string("Failed to get an image from the EGLStream::IFrame instance");

    auto* iNativeBuffer = Argus::interface_cast<EGLStream::NV::IImageNativeBuffer>(image);
    if (!iNativeBuffer) throw std::string("IImageNativeBuffer not supported for image type");

    if (dmaBufferFd)
    {
        NvBufferMemUnMap(dmaBufferFd, 0, &cvImageBuffer);
        NvBufferDestroy(dmaBufferFd);
    }

    dmaBufferFd = iNativeBuffer->createNvBuffer(resolution, NvBufferColorFormat_ARGB32, NvBufferLayout_Pitch);
/*  auto dmaBufferParams = NvBufferParams();
    NvBufferGetParams(dmaBufferFd, &dmaBufferParams);
    std::cout << "**** Pitch: " << dmaBufferParams.pitch[0] << ", Offset: " << dmaBufferParams.offset[0] << "\n"; */

    NvBufferMemMap(dmaBufferFd, 0, NvBufferMem_Read_Write, &cvImageBuffer);
    NvBufferMemSyncForCpu(dmaBufferFd, 0, &cvImageBuffer);
    return cv::Mat(resolution.height(), resolution.width(), CV_8UC4, cvImageBuffer);
}

cv::Size2i ArgusVideoCapture::getResolution() const
{
    return cv::Size2i(resolution.width(), resolution.height());
}

uint64_t ArgusVideoCapture::getTimestamp() const
{
    return timestamp;
}

uint32_t ArgusVideoCapture::getCaptureId() const
{
    return captureId;
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
