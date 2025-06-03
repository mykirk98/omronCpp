#pragma once
#include <afxwin.h>
class CButtonImage :
	public CButton
{
public:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

