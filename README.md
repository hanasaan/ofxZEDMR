# ofxZEDMR
OF Addon for building mixed-reality experience using Stereolab's [ZED Mini](https://www.stereolabs.com/zed-mini/) camera and Valve Software's [OpenVR](https://github.com/ValveSoftware/openvr) HMD (Vive or Oculus).

## Dependencies
- Please use my([@hanasaan](https://github.com/hanasaan/)) fork.
- [ofxOpenVR](https://github.com/hanasaan/ofxOpenVR)
- [ofxZED](https://github.com/hanasaan/ofxZED)

## Requirement
  - openFrameworks 0.11.0 64bit
  - Visual Studio 2017
  - CUDA 11.0
  - ZED SDK 3.2.2

## Reference Implementation
- I heavily referred Stereolab's [ZED SDK Unity plugin](https://github.com/stereolabs/zed-unity) and [Unreal plugin](https://github.com/stereolabs/zed-unreal-plugin) implementation.
- dlls are imported from [ZED Unreal plugin](https://github.com/stereolabs/zed-unreal-plugin/tree/UE4.21_ZedSdk3.1).

## Note
- Tested with ZED Mini + HTC Vive only. Oculus rift is not tested. 
- Depth occulusion mask, environmental lighting is not supported. (Todo)
