
// VisionApp.h: VisionApp 애플리케이션의 기본 헤더 파일
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"       // 주 기호입니다.


// CVisionAppApp:
// 이 클래스의 구현에 대해서는 VisionApp.cpp을(를) 참조하세요.
//

class CVisionAppApp : public CWinAppEx	//NOTE: 확장된 기능을 사용하기 위해 CWinAppEx에서 상속
{
public:
	CVisionAppApp() noexcept;	//NOTE: noexcept: 예외가 발생하지 않을 것을 컴파일러와 런타임에 약속하는 키워드, 생성자에서 예외가 발생하지 않도록 보장하는 경우 사용


// 재정의입니다.
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 구현입니다.
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	//NOTE: virtual: 가상 함수, 상속받은 클래스에서 재정의될 수 있는 함수
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CVisionAppApp theApp;
