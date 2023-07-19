# An Argus OpenCV VideoCapture class for the Jetson Orin, Xavier & Nano
#### For use with supported CSI cameras, written in C++17

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

#### Execute
```
./camera-demo
```

#### Tested Using
```
JetPack 4.6.4, 5.0.2 and 5.1.1
OpenCV 4.1.1, 4.6.0 and 4.8.0
```
