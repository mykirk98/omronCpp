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
| `BasicCamera` | GigE 카메라 초기화 및 순차 이미지 캡처 관리 |
| `ThreadSafeQueue<T>` | Mutex + condition-variable queue used between capture threads and the saver pool |
| `ImageSaverThreadPool` | Fixed thread pool that dequeues `FrameData` and saves to disk |
| `Logger` | Dedicated logging thread to avoid blocking hot paths |
| `CSVWriter` | 프레임 타이밍 메트릭용 스레드 안전 CSV 파일 작성기 |
| `ImageProcess` | 프레임 정보 출력, 포맷 변환, 다중 포맷 저장 등 이미지 처리 유틸리티 |
| `NodeMapUtil` | 카메라 노드맵 조회·저장·로드 및 트리거 모드 설정 유틸리티 |
| `GigEUtil` | GigE Vision IP 주소 업데이트, 하트비트 타임아웃, 디바이스 생성 유틸리티 |
| `PathToGuiAdapter` | 경로 메시지를 큐에서 GUI 동기화 매니저로 전달하는 어댑터 스레드 |
| `LightManager` + `ILightController` | Factory-pattern light controller abstraction (currently disabled on branch `noLight`) |
| `LightFactory` | 런타임 등록 기반 조명 컨트롤러 싱글톤 팩토리 |
| `LCP24100SS` / `LCP100DC` | Concrete light controller implementations |
| `LCP24100SSAdapter` / `LCP100DCAdapter` | `ILightController` 인터페이스로 감싸는 어댑터 |
| `config.h` | Camera serial→IP/name mapping and GigE Vision protocol constants |

## Project Structure

```
myOmronC++/
├── config.h                    # 카메라 시리얼→IP/이름 매핑, GigE Vision 프로토콜 상수
├── main.cpp                    # 콘솔 테스트 진입점 (git-ignored)
│
├── GigEManager.h / .cpp        # 최상위 카메라 매니저
├── GigECamera.h / .cpp         # 단일 카메라 래퍼 (트리거, 프레임 캡처)
├── BasicCamera.h / .cpp        # 순차 이미지 캡처 관리
├── GigEUtil.h / .cpp           # GigE Vision IP·하트비트·디바이스 생성 유틸리티
├── NodeMapUtil.h / .cpp        # 노드맵 조회·저장·로드, 트리거 설정 유틸리티
│
├── ThreadSafeQueue.h           # Mutex + condvar 기반 제네릭 큐 (헤더 전용)
├── ImageSaverThreadPool.h / .cpp  # 프레임 비동기 디스크 저장 스레드 풀
├── ImageProcess.h / .cpp       # 이미지 처리 유틸리티 (포맷 변환, 저장)
├── Logger.h / .cpp             # 전용 로깅 스레드
├── CSVWriter.h                 # 스레드 안전 CSV 파일 작성기 (헤더 전용)
├── PathToGuiAdapter.h / .cpp   # 경로 메시지 → GUI 동기화 어댑터 스레드
│
├── ILightController.h          # 조명 컨트롤러 인터페이스
├── LightManager.h / .cpp       # 조명 컨트롤러 팩토리 패턴 추상화
├── LightFactory.h / .cpp       # 런타임 등록 기반 싱글톤 팩토리
├── LCP24100SS.h / .cpp         # 조명 컨트롤러 구현체
├── LCP100DC.h / .cpp           # 조명 컨트롤러 구현체
├── LCP24100SSAdapter.h         # ILightController 어댑터 (헤더 전용)
├── LCP100DCAdapter.h           # ILightController 어댑터 (헤더 전용)
│
├── Experiments.cpp             # 실험용 코드
├── myOmronC++.vcxproj          # Visual Studio 프로젝트 파일
└── x64/Debug/                  # 빌드 아티팩트 (git-ignored)
```

## Architecture Notes

**Threading / producer-consumer pattern:**
Each `GigECamera` runs a capture thread that pushes `FrameData` into `ThreadSafeQueue`.
`ImageSaverThreadPool` workers pop from the queue and write files asynchronously.
`Logger` has its own dedicated thread so logging never blocks the capture path.

**`main.cpp` is git-ignored** (listed in `.gitignore`). It is the entry point for the
C++ console test harness and is not committed to the repository.
