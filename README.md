# Rasterizer

The purpose of this project is to learn how graphics API such as OpenGL works.

## Features

- [x] Texture mapping (projection correction)
- [x] Face culling
- [x] Depth test
- [x] Model loading
- [x] Animation
- [x] Optimization (parallelization, glm simd operation)
- [ ] Anti Aliasing (FXAA)

## Screenshot

| Triangle                                                                                                    | Texture                                                                                                    |
| ----------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------- |
| <img src="https://github.com/Guo-Haowei/Rasterizer/blob/master/screenshots/triangle.png" width="300"> | <img src="https://github.com/Guo-Haowei/Rasterizer/blob/master/screenshots/texture.png" width="300"> |

| Animation                                                                                                    | Shadow Mapping                                                                                            |
| ------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------- |
| <img src="https://github.com/Guo-Haowei/Rasterizer/blob/master/screenshots/animation.gif" width="300"> | <img src="https://github.com/Guo-Haowei/Rasterizer/blob/master/screenshots/shadow.gif" width="300"> |

Sponza
<img src="https://github.com/Guo-Haowei/Rasterizer/blob/master/screenshots/sponza.png">

## Build

### Windows
```
./build-assimp-win.sh
mkdir build
cd build/
cmake ..
cmake --build . --config Release
```
