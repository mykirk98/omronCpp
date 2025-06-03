#pragma once

#include "IStImageAveragingFilter.h"
#include "IStreamingCtrl.h"
#include "DefectivePixelManager.h"
#include <algorithm>
#include "afxcmn.h"
#include <map>

// CDefectivePixelListCtrl


class CDefectivePixelListCtrl : public CMFCListCtrl, public IDefectivePixelListCtrl
{
	DECLARE_DYNAMIC(CDefectivePixelListCtrl)

public:
	CDefectivePixelListCtrl(CDefectivePixelManager *pCDefectivePixelManager);
	virtual ~CDefectivePixelListCtrl();

	virtual int OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn);
	virtual void Sort(int iColumn, BOOL bAscending = TRUE, BOOL bAdd = FALSE);

	void OnInitialUpdate();
	size_t GetSelectedRegisteredItemCount();
	size_t GetSelectedNotRegisteredItemCount();
	void RegisterSelectedPixel();
	void DeregisterSelectedPixel();
	void UpdateDefectivePixelList();
protected:
	CDefectivePixelManager *m_pCDefectivePixelManager;

	//Last sorting information
	int m_iLastSortColumn;
	BOOL m_bLastSortAscending;
	BOOL m_bLastSortAdd;
	void AddDefectivePixel(uint8_t nColor, LPCTSTR szColor, StApi::PSStDefectivePixelInformation_t pInfo, int32_t nRegistered = -1);
	void AddDefectivePixel(uint8_t nColor, LPCTSTR szColor, uint16_t x, uint16_t y, int32_t nRegistered = -1);
	void GetSelectedPixelList(std::vector<EachDefectPixelPosition_t> &vecPxelList);
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
};


