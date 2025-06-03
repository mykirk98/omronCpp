#pragma once

#include <vector>

// CAVIFileDlg

class CAVIFileDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAVIFileDlg)

public:
	CAVIFileDlg(StApi::IStVideoFiler *pIStVideoFiler, CWnd* pParent = NULL);
	virtual ~CAVIFileDlg();

	enum { IDD = IDD_DIALOG_VIDEO_FILE };


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	/*!
	Add file name to the avi file name list.
	*/
	afx_msg void OnBnClickedButtonAddFileName();

protected:
	StApi::IStVideoFiler *m_pIStVideoFiler;
	StApi::CIStNodeMapDisplayWndPtr m_pIStNodeMapDisplayWnd;
};
