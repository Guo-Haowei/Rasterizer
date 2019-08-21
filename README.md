# Software Rasterizer

The purpose of this project is to learn how graphics API such as OpenGL or Direct3D works.
The rendering pipeline is written in C++, and then it uses opengl to render the generated texture buffer as a full screen quad.

## Features

- [x] Texture mapping (projection correction)
- [x] Face culling
- [x] Depth test
- [x] Model loading
- [x] Animation
- [x] Shadow Mapping
- [ ] Anti Aliasing (FXAA)
- [ ] Optimization (multithreading, simd operations)

## Screenshot

| Triangle  | Texture |
| --------- | ------------- |
| <img src="https://github.com/Guo-Haowei/SoftwareRenderer/blob/master/screenshots/triangle.png" width="300"> | <img src="https://github.com/Guo-Haowei/SoftwareRenderer/blob/master/screenshots/texture.png" width="300"> |

<!-- | Model | Animation |
| ------------ | ----- |
| <img src="https://github.com/Guo-Haowei/SoftwareRenderer/blob/master/screenshots/model.gif" width="300"> | <img src="https://github.com/Guo-Haowei/SoftwareRenderer/blob/master/screenshots/animation.gif" width="300"> | -->

| Animation | Shadow Mapping |
| ------------ | ----- |
| <img src="https://github.com/Guo-Haowei/SoftwareRenderer/blob/master/screenshots/animation.gif" width="300"> | <img src="https://github.com/Guo-Haowei/SoftwareRenderer/blob/master/screenshots/shadow.gif" width="300"> |

## Build

### Windows
```
./build-assimp-win.sh
mkdir build
cd build/
cmake ..
cmake --build . --config Release
```

### MacOS
```
./build-assimp-macos.sh
mkdir build
cd build/
cmake -G Xcode ..
cmake --build . --config Release
```
