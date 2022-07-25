# SD Engine
A mini game engine

### How to Build:
- Use Visual Studio 2019

### Engine Structure:
- Renderer
DirectX 11 API pipeline
Support multi-pass, multi-render targets (for deferred pipeline)
Customizable constant buffers
Support instanced rendering, customizable vertex formats
Hlsl shaders
Perspective & Orthographic cameras
2D textures, sprites, sprite animations, bitmap fonts
- Animation
Skeletal mesh & animation
FBX file loading (Assimp)
FABRIK Solver
- Audio
3D audio (fmod)
- Core
Time, Clock & Stopwatch
File IO, XML, Image, String, ByteBuffer Utilities
Development Console
Event System
Job System (Multi-threading)
- Input
Mouse movement handling (clipped, relative), delta reporting
Keyboard & Mouse key handling
Support controllers (XInput 9.1.0)
- Math
Simple 2D shapes
3D vector & matrix (also for render pipeline)
Easing functions
Curves & Spline (Bezier, Hermite, Catmull-Rom)
- Window
Win32API for window handing


