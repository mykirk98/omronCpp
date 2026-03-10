# Vision Inspection Automation - Project Guide


## Project Overview
A WPF application that manages multiple GigE cameras for real-time control and image capture.
Planned: automated vision-based defect inspection.

## Tech Stack
TODO: explain which version of framework or library i use.

## Development Principles

Follow @docs/tdd_principles.md for TDD and Tidy First workflow.
When I say "go", follow the plan.md workflow defined in that document.

## Build Commands

**Full solution (Visual Studio / MSBuild):**
```bash
msbuild CameraArchitecture.sln /p:Configuration=Debug /p:Platform=x64
msbuild CameraArchitecture.sln /p:Configuration=Release /p:Platform=x64
```

**C# WPF app only:**
```bash
dotnet build VisionApp/VisionApp.csproj
dotnet run --project VisionApp/VisionApp.csproj
```

**C++ project** requires Visual Studio 2022 with MSVC v145. External dependencies must be installed and environment variables set:
- `STAPI_ROOT_PATH` — Omron StApi SDK root (headers + libs)
- OpenCV 4.12.0 installed at `C:\opencv\`

## Project Structure
TODO: remove key classes, key classes. It will explain detail of each project under directory's CLAUDE.md. Instead, show the hierarchy.

Two independent projects in one solution:

### `myOmronC++/` — C++20 backend (GigE camera control)
Manages multiple Omron GigE cameras via StApi (Omron camera SDK) and GenICam. Real-time image capture runs in per-camera threads; frames flow through `ThreadSafeQueue<FrameData>` into `ImageSaverThreadPool` for async disk writes.

Key classes:
- `GigEManager` — top-level coordinator: initializes StApi, discovers cameras, manages `GigECamera` instances and their threads
- `GigECamera` — wraps a single camera: configures trigger mode, grabs frames
- `ThreadSafeQueue<T>` — mutex + condition-variable queue used between capture threads and the saver pool
- `ImageSaverThreadPool` — fixed thread pool that dequeues `FrameData` and saves to disk
- `Logger` — dedicated logging thread to avoid blocking hot paths
- `LightManager` + `ILightController` / `LCP24100SS` / `LCP100DC` — factory-pattern light controller abstraction (currently disabled on branch `noLight`)
- `config.h` — camera serial→IP/name mapping and GigE Vision protocol constants

### `VisionApp/` — C# WPF frontend (.NET 10.0-windows)
MVVM architecture using **CommunityToolkit.Mvvm**. Currently uses `MockCameraService` (no real C++ interop yet); `Interop/` is a placeholder for future P/Invoke or C++/CLI bridge.

Key relationships:
- `MainViewModel` owns the state machine (`AppState`: Idle → Connected → Running → Stopped) and exposes `RelayCommand`s (Connect / Run / Stop) whose `CanExecute` depends on state
- `ICameraService` abstracts camera operations; `MockCameraService` simulates 3 cameras (TOP/BOTTOM/SIDE) at ~10 FPS using a `DispatcherTimer` — allows UI development without hardware
- `LogService` is an event bus (`LogReceived` event); `MainViewModel` subscribes and appends to `LogEntries` (capped at 500)
- Frame delivery: `ICameraService.FrameReceived` → `MainViewModel.OnFrameReceived` → dispatched to UI thread → `CameraTileViewModel.CurrentImage`
- `CameraTileViewModel` holds the per-camera `BitmapSource` and FPS shown in the camera grid

## Architecture Notes
#TODO: remove this section. It will handle plan.md later.
**No C++ ↔ C# integration yet.** The WPF app is developed against `MockCameraService`. When real integration is added it will live in `VisionApp/Interop/` and implement `ICameraService`.

**Threading contract:** All `ICameraService` events fire on a background thread. `MainViewModel` always marshals back via `Application.Current.Dispatcher.Invoke`.

**State machine is the source of truth for toolbar buttons.** Changing `CurrentState` in `MainViewModel` triggers `OnCurrentStateChanged`, which calls `OnPropertyChanged` for `CanConnect/CanRun/CanStop` and `NotifyCanExecuteChanged` on each command.

**`main.cpp` in `myOmronC++/` is git-ignored** (listed in `.gitignore`). It is the entry point for the C++ console test harness and is not committed.
