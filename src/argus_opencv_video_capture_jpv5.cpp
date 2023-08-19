//
// (c) Bit Parallel Ltd, July 2023
//

#include <iostream>
#include <sstream>

#include "nvbuf_utils.h"
#include "argus_opencv_video_capture.hpp"

bpl::ArgusVideoCapture::ArgusVideoCapture(const int32_t cameraDeviceIndex, const int32_t sensorModeIndex):
    cameraDeviceIndex(cameraDeviceIndex), sensorModeIndex(sensorModeIndex), argusCameraSettings(ArgusCameraSettings(iSourceSettings, iAutoControlSettings)),
    dmaBufferFd(0), cvImageBuffer(nullptr), image(nullptr), timestamp(uint64_t(0)) {

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

    // notes 1, the following settings instances are used by the ArgusCameraSettings class
    //       2, set the user selected sensor mode, passed in as a constructor argument
    //
    iSourceSettings = Argus::interface_cast<Argus::ISourceSettings>(request);
    if (!iSourceSettings) throw std::string("Failed to get the Argus::ISourceSettings interface");
    iSourceSettings->setSensorMode(sensorModes[sensorModeIndex]);

    iAutoControlSettings = Argus::interface_cast<Argus::IAutoControlSettings>(iRequest->getAutoControlSettings());
    if (!iAutoControlSettings) throw std::string("Failed to get the Argus::IAutoControlSettings interface");

//  // FIXME! should there be a default frame rate?
//  //        1, set here to 30 FPS
//  //        2, although if you print the value beforehand it's set to 30 FPS, is the driver doing this?
//  //
//  iSourceSettings->setFrameDurationRange(Argus::Range<uint64_t>(33333334L));

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

bpl::ArgusVideoCapture::~ArgusVideoCapture()
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

    cameraProvider.reset();
}

cv::Mat bpl::ArgusVideoCapture::grab()
{
    // notes 1, acquire a captured camera frame, using mailbox mode
    //       2, frame and iFrame are retained as instance properties as they are required by image as used in saveAsJPEG()
    //
    auto status = Argus::STATUS_OK;
    frame = Argus::UniqueObj<EGLStream::Frame>(iFrameConsumer->acquireFrame(FIVE_SECONDS_IN_NANOSECONDS, &status));
    if (status != Argus::STATUS_OK) throw std::string("Failed to aquire a camera frame from the EGLStream::IFrameConsumer instance");

    iFrame = Argus::interface_cast<EGLStream::IFrame>(frame);
    if (!iFrame) throw std::string("Failed to get the EGLStream::IFrame interface");
    timestamp = iFrame->getTime();
    captureId = iFrame->getNumber();

    image = iFrame->getImage();
    if (!image) throw std::string("Failed to get an image from the EGLStream::IFrame instance");

    auto* iNativeBuffer = Argus::interface_cast<EGLStream::NV::IImageNativeBuffer>(image);
    if (!iNativeBuffer) throw std::string("IImageNativeBuffer not supported for image type");

    if (dmaBufferFd)
    {
        NvBufferMemUnMap(dmaBufferFd, 0, &cvImageBuffer);
        NvBufferDestroy(dmaBufferFd);
    }

    dmaBufferFd = iNativeBuffer->createNvBuffer(resolution, NVBUF_COLOR_FORMAT_ARGB, NVBUF_LAYOUT_PITCH);
//  auto dmaBufferParams = NvBufferParams();
//  NvBufferGetParams(dmaBufferFd, &dmaBufferParams);
//  std::cout << "**** Pitch: " << dmaBufferParams.pitch[0] << ", Offset: " << dmaBufferParams.offset[0] << "\n";

    NvBufferMemMap(dmaBufferFd, 0, NvBufferMem_Read_Write, &cvImageBuffer);
    NvBufferMemSyncForCpu(dmaBufferFd, 0, &cvImageBuffer);
    return cv::Mat(resolution.height(), resolution.width(), CV_8UC4, cvImageBuffer);
}

cv::Size2i bpl::ArgusVideoCapture::getResolution() const
{
    return cv::Size2i(resolution.width(), resolution.height());
}

uint64_t bpl::ArgusVideoCapture::getTimestamp() const
{
    return timestamp;
}

uint32_t bpl::ArgusVideoCapture::getCaptureId() const
{
    return captureId;
}

// notes 1, this method uses a fixed JPEG quality value set to 95, the IImageJPEG instance doesn't allow this to be changed
//       2, could implement this using a DIY version of this class, see the 09_camera_jpeg_capture sample, be aware that this
//          sample relies on the included NvJPEGEncoder class and its dependencies that must separately compiled and linked
//
bool bpl::ArgusVideoCapture::saveAsJPEG(const std::string& fileName) const
{
    if (!image) throw std::string("Unable to save the captured frame as a JPEG, grab() has not been called");

    auto* iImageJPEG = Argus::interface_cast<EGLStream::IImageJPEG>(image);
    if (!iImageJPEG)
    {
        std::cout << "Unable to save the captured frame as a JPEG, failed to aquire the EGLStream::IImageJPEG interface\n";
        return false;
    }

    const auto status = iImageJPEG->writeJPEG(fileName.c_str());
    if (status != Argus::STATUS_OK) throw std::string("Failed to write the captured frame as a JPEG to: " + fileName);

    return true;
}

bpl::ArgusCameraSettings& bpl::ArgusVideoCapture::getCameraSettings()
{
    return argusCameraSettings;
}

bool bpl::ArgusVideoCapture::restart()
{
    bool success = true;

    iSession->stopRepeat();
    auto status = iSession->waitForIdle(ONE_SECOND_IN_NANOSECONDS);
    if (status != Argus::STATUS_OK)
    {
        std::cout << "Error: Timeout whilst waiting for the repeating capture requests to stop\n";
        std::cout << "Warning: Any prior settings updates may not have taken effect\n";
        success = false;
    }

    status = iSession->repeat(request.get());
    if (status != Argus::STATUS_OK) throw std::string("Failed to trigger repeating capture requests");

    return success;
}
