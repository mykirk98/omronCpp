#include "StdAfx.h"
#include "CButtonImage.h"
BEGIN_MESSAGE_MAP(CButtonImage, CButton)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CButtonImage::OnEraseBkgnd(CDC* pDC)
{
	return(TRUE);
}
