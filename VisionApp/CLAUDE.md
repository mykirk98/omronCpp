# VisionApp — C# WPF Frontend

## Purpose
WPF desktop application for camera monitoring and control.
Currently developed against `MockCameraService` (no real C++ interop yet).

## Stack
- **C#**, .NET 10.0-windows, WPF
- **CommunityToolkit.Mvvm 8.x** — MVVM source generators, `RelayCommand`, `ObservableObject`

## Directory Structure
```
VisionApp/
├── Models/         AppState.cs, CameraInfo.cs, LogEntry.cs
├── ViewModels/     MainViewModel.cs, CameraTileViewModel.cs
├── Services/       ICameraService.cs, LogService.cs, MockCameraService.cs
├── Converters/     BoolToVisibilityConverter.cs, BoolToColorConverter.cs
├── Resources/      Styles.xaml
├── Views/          (XAML views)
└── Interop/        (placeholder for future P/Invoke or C++/CLI bridge)
```

## Key Relationships

**State machine (`MainViewModel`):**
- `AppState`: `Idle → Connected → Running → Stopped`
- `CurrentState` change triggers `OnCurrentStateChanged` → updates `CanConnect/CanRun/CanStop`
  and calls `NotifyCanExecuteChanged` on Connect / Run / Stop commands
- State machine is the single source of truth for toolbar button enabled states

**Camera service:**
- `ICameraService` abstracts all camera operations
- `MockCameraService` simulates 3 cameras (TOP/BOTTOM/SIDE) at ~10 FPS using `DispatcherTimer`
  — enables UI development without hardware

**Log service:**
- `LogService` is an event bus (`LogReceived` event)
- `MainViewModel` subscribes and appends to `LogEntries` (capped at 500 entries)

**Frame delivery pipeline:**
```
ICameraService.FrameReceived
  → MainViewModel.OnFrameReceived   (background thread)
  → Dispatcher.Invoke               (marshal to UI thread)
  → CameraTileViewModel.CurrentImage (BitmapSource + FPS)
```

## Architecture Notes

**Threading contract:** All `ICameraService` events fire on a background thread.
`MainViewModel` always marshals back via `Application.Current.Dispatcher.Invoke`.

**No C++ ↔ C# integration yet.** When real integration is added it will live in
`Interop/` and implement `ICameraService`.
