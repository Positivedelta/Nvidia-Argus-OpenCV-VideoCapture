# An Argus OpenCV VideoCapture class for the Jetson Orin, Xavier & Nano
#### For use with supported L4T CSI cameras, written in C++17

#### Notes
- It may be neccessary to edit `CMakeLists.txt` and/or the `cmake/` helper scripts in order to find the Argus and NVMMAPI dependencies
- These dependencies are likely to be found in `/usr/src/jetson_multimedia_api`
- This project also requires OpenCV, this should be found automatically by CMake

#### Build
```
mkdir build
cd build
cmake ..
```

#### Install Instructions
To install the core shared library, the include files and test applications, use:

```
make install
```

The following default paths are used, edit `CMakeLists.txt` if you want to update these
- The shared library is installed to `bit-parallel/lib`
- The include files are installed to `bit-parallel/include/camera`
- The test appliaction is installed to `bit-parallel/bin/camera`

#### Execute
```
./argus-csi-camera-demo --query-camera-devices
./argus-csi-camera-demo -q
./argus-csi-camera-demo -d 0 -m 5
```

#### Tested Using
- JetPack `v4.6.4`, `v5.0.2` and `v5.1.1`
- OpenCV `v4.1.1`, `v4.6.0` and `v4.8.0`
