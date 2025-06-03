// DefectivePixelListCtrl.cpp : 
//

#include "stdafx.h"
#include "StViewer.h"
#include "DefectivePixelListCtrl.h"

using namespace StApi;
using namespace GenApi;

// CDefectivePixelListCtrl

IMPLEMENT_DYNAMIC(CDefectivePixelListCtrl, CMFCListCtrl)

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CDefectivePixelListCtrl::CDefectivePixelListCtrl(CDefectivePixelManager *pCDefectivePixelManager) :
	m_pCDefectivePixelManager(pCDefectivePixelManager),
	m_iLastSortColumn(0),
	m_bLastSortAscending(TRUE),
	m_bLastSortAdd(FALSE)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CDefectivePixelListCtrl::~CDefectivePixelListCtrl()
{
}

BEGIN_MESSAGE_MAP(CDefectivePixelListCtrl, CMFCListCtrl)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CDefectivePixelListCtrl::OnLvnItemchanged)
END_MESSAGE_MAP()

// CDefectivePixelListCtrl
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelListCtrl::OnInitialUpdate()
{
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	InsertColumn(LIST_COL_STATUS, TEXT("Status"), LVCFMT_CENTER, 150);
	InsertColumn(LIST_COL_COLOR, TEXT("Color"), LVCFMT_CENTER, 150);
	InsertColumn(LIST_COL_X, TEXT("x"), LVCFMT_CENTER, 75);
	InsertColumn(LIST_COL_Y, TEXT("y"), LVCFMT_CENTER, 75);
	InsertColumn(LIST_COL_EVALUATION, TEXT("Evaluation"), LVCFMT_CENTER, 100);
	InsertColumn(LIST_COL_REFERENCE, TEXT("Reference"), LVCFMT_CENTER, 100);
	InsertColumn(LIST_COL_DIFFERENCE, TEXT("Difference(%)"), LVCFMT_CENTER, 100);
	UpdateDefectivePixelList();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelListCtrl::UpdateDefectivePixelList()
{
	LockWindowUpdate();
	while (GetItemCount()) DeleteItem(0);

	m_pCDefectivePixelManager->UpdateDefectivePixelList(this);
	Sort(m_iLastSortColumn, m_bLastSortAscending, m_bLastSortAdd);

	UnlockWindowUpdate();

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelListCtrl::AddDefectivePixel(uint8_t nColor, LPCTSTR szColor, StApi::PSStDefectivePixelInformation_t pInfo, int32_t nRegistered)
{
	CString strValue;
	if (0 <= nRegistered)
	{
		strValue.Format(TEXT("Registered[%d]"), nRegistered);
	}
	else
	{
		strValue.Format(TEXT(""));
	}

	const int nItemIndex = (int)GetItemCount();
	InsertItem(nItemIndex, strValue);

	SetItemText(nItemIndex, LIST_COL_COLOR, szColor);
#if _MSC_VER < 1900
#if _WIN64
	strValue.Format(TEXT("%I64u"), pInfo->x);
#else
	strValue.Format(TEXT("%u"), pInfo->x);
#endif
#else
	strValue.Format(TEXT("%zu"), pInfo->x);
#endif
	SetItemText(nItemIndex, LIST_COL_X, strValue);
#if _MSC_VER < 1900
#if _WIN64
	strValue.Format(TEXT("%I64u"), pInfo->y);
#else
	strValue.Format(TEXT("%u"), pInfo->y);
#endif
#else
	strValue.Format(TEXT("%zu"), pInfo->y);
#endif
	SetItemText(nItemIndex, LIST_COL_Y, strValue);
	strValue.Format(TEXT("%.2f"), pInfo->dblEvaluationValue);
	SetItemText(nItemIndex, LIST_COL_EVALUATION, strValue);
	strValue.Format(TEXT("%.2f"), pInfo->dblReferenceValue);
	SetItemText(nItemIndex, LIST_COL_REFERENCE, strValue);
	strValue.Format(TEXT("%.2f"), pInfo->dblDeltaRatio * 100);
	SetItemText(nItemIndex, LIST_COL_DIFFERENCE, strValue);
	SetItemData(nItemIndex, (DWORD_PTR)((nColor << 30) | ((pInfo->x & 0x7FFF) << 15) | (pInfo->y & 0x7FFF)));
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelListCtrl::AddDefectivePixel(uint8_t nColor, LPCTSTR szColor, uint16_t x, uint16_t y, int32_t nRegistered)
{

	const int nItemIndex = (int)GetItemCount();
	CString strValue;
	strValue.Format(TEXT("Registered[%d]"), nRegistered);
	InsertItem(nItemIndex, strValue);

	SetItemText(nItemIndex, LIST_COL_COLOR, szColor);

	strValue.Format(TEXT("%u"), x);
	SetItemText(nItemIndex, LIST_COL_X, strValue);

	strValue.Format(TEXT("%u"), y);
	SetItemText(nItemIndex, LIST_COL_Y, strValue);

	SetItemData(nItemIndex, (DWORD_PTR)((nColor << 30) | ((x & 0x7FFF) << 15) | (y & 0x7FFF)));
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int CDefectivePixelListCtrl::OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn)
{
	EachDefectPixelPosition_t pPosition[] =
	{
#ifdef ENABLED_TUPLE
		std::make_tuple((uint32_t)lParam1 >> 30, (lParam1 >> 15) & 0x7FFF, lParam1 & 0x7FFF),
		std::make_tuple((uint32_t)lParam2 >> 30, (lParam2 >> 15) & 0x7FFF, lParam2 & 0x7FFF)
#else
		std::make_pair((uint32_t)lParam1 >> 30, std::make_pair((lParam1 >> 15) & 0x7FFF, lParam1 & 0x7FFF)),
		std::make_pair((uint32_t)lParam2 >> 30, std::make_pair((lParam2 >> 15) & 0x7FFF, lParam2 & 0x7FFF))
#endif //ENABLED_TUPLE
	};
	return(m_pCDefectivePixelManager->Compare(iColumn, pPosition));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelListCtrl::GetSelectedPixelList(std::vector<EachDefectPixelPosition_t> &vecPxelList)
{
	int nIndex = -1;

	for (;;)
	{
		nIndex = GetNextItem(nIndex, LVNI_SELECTED);
		if (nIndex < 0) break;

		const DWORD_PTR dwData = GetItemData(nIndex);
#ifdef ENABLED_TUPLE
		vecPxelList.push_back(std::make_tuple(((uint32_t)dwData >> 30), (dwData >> 15) & 0x7FFF, dwData & 0x7FFF));
#else
		vecPxelList.push_back(std::make_pair(((uint32_t)dwData >> 30), std::make_pair((dwData >> 15) & 0x7FFF, dwData & 0x7FFF)));
#endif //ENABLED_TUPLE
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelListCtrl::RegisterSelectedPixel()
{
	// Gets the selected pixel list.
	std::vector<EachDefectPixelPosition_t> vecPxelList;
	GetSelectedPixelList(vecPxelList);

	m_pCDefectivePixelManager->RegisterSelectedPixel(vecPxelList);

	UpdateDefectivePixelList();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelListCtrl::DeregisterSelectedPixel()
{
	// Gets the selected pixel list.
	std::vector<EachDefectPixelPosition_t> vecPxelList;
	GetSelectedPixelList(vecPxelList);

	m_pCDefectivePixelManager->DeregisterSelectedPixel(vecPxelList);

	UpdateDefectivePixelList();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelListCtrl::Sort(int iColumn, BOOL bAscending, BOOL bAdd)
{

	m_iLastSortColumn = iColumn;
	m_bLastSortAscending = bAscending;
	m_bLastSortAdd = bAdd;
	CMFCListCtrl::Sort(iColumn, bAscending, bAdd);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
size_t CDefectivePixelListCtrl::GetSelectedRegisteredItemCount()
{
	size_t nCount = 0;
	int nItemNo = -1;
	do
	{
		nItemNo = GetNextItem(nItemNo, LVNI_SELECTED);
		if (nItemNo < 0) break;

		const DWORD_PTR dwData = GetItemData(nItemNo);
		const std::pair<size_t, size_t> sPos = std::make_pair(dwData >> 16, dwData & 0xFFFF);


		if(GetItemText(nItemNo, LIST_COL_STATUS).Left(8).CompareNoCase(TEXT("Register")) == 0)
		{
			++nCount;
		}

	} while (true);
	return(nCount);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
size_t CDefectivePixelListCtrl::GetSelectedNotRegisteredItemCount()
{
	size_t nCount = 0;
	int nItemNo = -1;
	do
	{
		nItemNo = GetNextItem(nItemNo, LVNI_SELECTED);
		if (nItemNo < 0) break;

		const DWORD_PTR dwData = GetItemData(nItemNo);
		const std::pair<size_t, size_t> sPos = std::make_pair(dwData >> 16, dwData & 0xFFFF);

		if (GetItemText(nItemNo, LIST_COL_STATUS).Left(8).CompareNoCase(TEXT("Register")) != 0)
		{
			++nCount;
		}

	} while (true);
	return(nCount);
}

void CDefectivePixelListCtrl::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (
		(
			(pNMLV->uNewState & (~pNMLV->uOldState)) | 
			((~pNMLV->uNewState) & pNMLV->uOldState)
		)
		& LVIS_SELECTED
			)
	{
		// Gets the selected pixel list.
		std::vector<EachDefectPixelPosition_t> vecPxelList;
		GetSelectedPixelList(vecPxelList);

		m_pCDefectivePixelManager->SetSelectedPixelInformation(vecPxelList);

	}
	*pResult = 0;
}
