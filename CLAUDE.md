# Vision Inspection Automation - Project Guide


## Project Overview
A WPF application that manages multiple GigE cameras for real-time control and image capture.
Planned: automated vision-based defect inspection.

## Tech Stack
- **C#**: .NET 10.0-windows, WPF, CommunityToolkit.Mvvm 8.x
- **C++**: C++20, MSVC v145 (VS 2022), OpenCV 4.12.0, Omron StApi SDK (via `STAPI_ROOT_PATH`)

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
Two independent projects in one solution:

```
CameraArchitecture.sln
├── myOmronC++/          # C++20 backend — GigE camera control (see myOmronC++/CLAUDE.md)
└── VisionApp/           # C# WPF frontend — .NET 10.0-windows (see VisionApp/CLAUDE.md)
```
