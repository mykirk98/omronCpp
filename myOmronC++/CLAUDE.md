# myOmronC++ — C++20 GigE Camera Control Backend

## Purpose
Controls multiple Omron GigE cameras via StApi (Omron camera SDK) and GenICam.
Real-time image capture runs in per-camera threads; frames flow through
`ThreadSafeQueue<FrameData>` into `ImageSaverThreadPool` for async disk writes.

## SDKs & Dependencies
- **Omron StApi SDK** — camera discovery, configuration, frame acquisition (via `STAPI_ROOT_PATH`)
- **GenICam** — generic camera parameter access
- **OpenCV 4.12.0** — image processing (installed at `C:\opencv\`)
- **C++20**, MSVC v145 (Visual Studio 2022)

## Key Classes

| Class | Responsibility |
|---|---|
| `GigEManager` | Top-level coordinator: initializes StApi, discovers cameras, manages `GigECamera` instances and their threads |
| `GigECamera` | Wraps a single camera: configures trigger mode, grabs frames |
| `ThreadSafeQueue<T>` | Mutex + condition-variable queue used between capture threads and the saver pool |
| `ImageSaverThreadPool` | Fixed thread pool that dequeues `FrameData` and saves to disk |
| `Logger` | Dedicated logging thread to avoid blocking hot paths |
| `LightManager` + `ILightController` | Factory-pattern light controller abstraction (currently disabled on branch `noLight`) |
| `LCP24100SS` / `LCP100DC` | Concrete light controller implementations |
| `config.h` | Camera serial→IP/name mapping and GigE Vision protocol constants |

## Architecture Notes

**Threading / producer-consumer pattern:**
Each `GigECamera` runs a capture thread that pushes `FrameData` into `ThreadSafeQueue`.
`ImageSaverThreadPool` workers pop from the queue and write files asynchronously.
`Logger` has its own dedicated thread so logging never blocks the capture path.

**`main.cpp` is git-ignored** (listed in `.gitignore`). It is the entry point for the
C++ console test harness and is not committed to the repository.
