# VisionApp — MFC Vision Inspection UI

## Purpose
MDI(Multiple Document Interface) 기반 MFC 데스크탑 애플리케이션.
GigE 카메라 제어 백엔드(`myOmronC++`)와 연동하여 실시간 이미지를 표시하고
카메라 파라미터를 조작하기 위한 프론트엔드 UI 프레임워크.
현재는 UI 스캐폴딩 단계로, 백엔드와의 통합 작업이 진행 중.

## Tech Stack
- **MFC (Microsoft Foundation Classes)** — Static link, CMDIFrameWndEx 기반 MDI
- **C++20**, MSVC v145 (Visual Studio 2022)
- **Unicode** 빌드, x64 플랫폼
- 리소스 언어: Korean (0x0412)

## Key Classes

| Class | 역할 |
|---|---|
| `CVisionAppApp` | 앱 진입점 (`CWinAppEx`). `CMultiDocTemplate` 등록, 메인 프레임 생성 |
| `CMainFrame` | MDI 메인 프레임 (`CMDIFrameWndEx`). MenuBar, ToolBar, StatusBar, 4개 도킹 패널 관리 |
| `CChildFrame` | MDI 자식 프레임 (`CMDIChildWndEx`). `CSplitterWndEx` 포함 |
| `CVisionAppDoc` | 문서 클래스 (`CDocument`). 데이터 모델 및 직렬화 |
| `CVisionAppView` | 뷰 클래스 (`CView`). 문서 내용 렌더링 (`OnDraw`) |
| `CFileView` | 파일 탐색 도킹 패널 (`CDockablePane`). `CViewTree` + 커스텀 툴바 |
| `CClassView` | 클래스/구조 브라우저 도킹 패널 (`CDockablePane`). `CViewTree` + 정렬 기능 |
| `COutputWnd` | 출력 로그 도킹 패널 (`CDockablePane`). 빌드/디버그/검색 3탭 (`CMFCTabCtrl`) |
| `CPropertiesWnd` | 속성 그리드 도킹 패널 (`CDockablePane`). `CMFCPropertyGridCtrl` |
| `CViewTree` | 공용 트리 컨트롤 (`CTreeCtrl`). FileView·ClassView에서 재사용 |

## Project Structure

```
VisionApp/
├── VisionApp.h / .cpp          # 앱 클래스 (CWinAppEx), 진입점
├── MainFrm.h / .cpp            # MDI 메인 프레임, 도킹 패널 생성
├── ChildFrm.h / .cpp           # MDI 자식 프레임 (splitter 포함)
├── VisionAppDoc.h / .cpp       # 문서 클래스 (데이터 모델)
├── VisionAppView.h / .cpp      # 뷰 클래스 (렌더링)
│
├── FileView.h / .cpp           # 파일 탐색 도킹 패널
├── ClassView.h / .cpp          # 클래스 구조 도킹 패널
├── OutputWnd.h / .cpp          # 멀티탭 출력 도킹 패널
├── PropertiesWnd.h / .cpp      # 속성 그리드 도킹 패널
├── ViewTree.h / .cpp           # 공용 트리 컨트롤
│
├── framework.h                 # MFC 프레임워크 인클루드
├── pch.h / pch.cpp             # 미리 컴파일된 헤더
├── targetver.h                 # Windows 대상 버전
├── Resource.h                  # 리소스 ID 정의
├── VisionApp.rc                # UI 리소스 (메뉴, 다이얼로그, 문자열, 아이콘)
├── res/                        # 아이콘·비트맵 에셋
│
├── VisionApp.vcxproj           # Visual Studio 프로젝트 파일
└── x64/Debug|Release/          # 빌드 아티팩트 (git-ignored)
```

## MFC 아키텍처 패턴

| 패턴 | 구현 |
|---|---|
| **MDI** | `CMultiDocTemplate` → `CChildFrame` → `CVisionAppView` |
| **Document/View** | `CVisionAppDoc` (데이터) ↔ `CVisionAppView` (렌더링) |
| **Docking Panes** | `CDockablePane` 파생 4개 패널, auto-hide 지원 |
| **MDI Tabs** | `EnableMDITabbedGroups(TRUE)` — 자식 창을 탭으로 표시 |
| **Message Maps** | `DECLARE_MESSAGE_MAP` / `BEGIN_MESSAGE_MAP` |
| **Visual Styles** | Office 2007, VS 2005, Windows 7 테마 동적 전환 |

## Backend Integration 계획

현재 `VisionApp`은 `myOmronC++` 백엔드와 직접 연결되어 있지 않음.
통합 시 다음 작업이 필요:

- `GigEManager` 헤더 인클루드 및 프로젝트 참조 추가
- `CVisionAppView::OnDraw()`에 카메라 프레임 렌더링 연결
- `CPropertiesWnd` — 카메라 파라미터 표시 (노출, 게인 등)
- `COutputWnd` — 캡처 상태 및 로그 출력
- `CFileView` — 저장된 캡처 이미지 탐색

## Build

```bash
msbuild ..\CameraArchitecture.sln /p:Configuration=Debug /p:Platform=x64
```

솔루션 레벨에서 빌드하면 `myOmronC++`와 함께 빌드됨.
`VisionApp.vcxproj`만 단독 빌드도 가능 (백엔드 미연결 상태에서).
