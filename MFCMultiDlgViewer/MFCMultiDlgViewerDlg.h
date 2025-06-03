
// MFCMultiDlgViewerDlg.h : header file
//

#pragma once

#include <vector>
#include "CEachDeviceDlg.h"


// CMFCMultiDlgViewerDlg dialog
class CMFCMultiDlgViewerDlg : public CDialogEx
{
// Construction
public:
	CMFCMultiDlgViewerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MFCMULTIDLGVIEWER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	std::vector<CEachDeviceDlg*> m_vecCEachDeviceDlg;

	void UpdateLayout();
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonOpen();
	virtual BOOL DestroyWindow(); 
	afx_msg LRESULT OnDeviceWndClosed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDeviceLost(WPARAM wParam, LPARAM lParam);
};
