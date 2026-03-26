# VisionInspectionApp — MFC 비전 검사 데스크톱 애플리케이션

## Purpose
MFC 기반 SDI(Single Document Interface) 데스크톱 애플리케이션.
Visual Studio 스타일의 도킹 패널 레이아웃으로 비전 검사 UI를 제공한다.
현재 UI 프레임워크만 구축된 상태이며, `myOmronC++` 카메라 백엔드와 미연결.

## Frameworks & Dependencies
- **MFC** (Microsoft Foundation Classes) — 정적 링크, Document/View 아키텍처
- **MSVC v145** (Visual Studio 2022)
- **Platform**: Win32 / x64
- **Windows SDK** 10.0

## Key Classes

| Class | Responsibility |
|---|---|
| `CVisionInspectionAppApp` | 앱 진입점, OLE 초기화, 문서 템플릿 등록 |
| `CMainFrame` | 메인 프레임 윈도우, 도킹 패널 레이아웃 관리 |
| `CVisionInspectionAppDoc` | 문서 클래스 (데이터 모델) |
| `CVisionInspectionAppView` | 뷰 클래스 (이미지 렌더링 영역, 현재 빈 OnDraw) |
| `CClassView` | 클래스 트리 도킹 패널 |
| `CFileView` | 파일 트리 도킹 패널 |
| `COutputWnd` | 출력 창 (Build / Debug / Find 탭) |
| `CPropertiesWnd` | 속성 그리드 도킹 패널 |
| `CViewTree` | 트리 컨트롤 기본 클래스 |

## Architecture Notes

**Document/View 패턴:**
`CVisionInspectionAppDoc`이 데이터를 소유하고, `CVisionInspectionAppView`가 렌더링을 담당한다.

**도킹 UI 레이아웃:**
`CMainFrame`이 좌측(FileView, ClassView), 하단(OutputWnd), 우측(PropertiesWnd) 도킹 패널을 관리한다.
MFC의 `CDockablePane`, `CMFCToolBar`, `CMFCMenuBar` 등을 사용한다.

**리소스:**
한국어(0x0412) 리소스. `res/` 디렉터리에 아이콘, 비트맵, 툴바 이미지 포함.

## Build

```bash
msbuild CameraArchitecture.sln /p:Configuration=Debug /p:Platform=x64
msbuild CameraArchitecture.sln /p:Configuration=Release /p:Platform=x64
```
