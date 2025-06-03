#pragma once
#include <afxwin.h>
class CMyDocManager :
	public CDocManager
{
public:
	CMyDocManager() : m_nTypeIndex(0)
	{
	}
	void OnFileNew() override;
	void OnFileOpen() override;
	size_t m_nTypeIndex;
};

