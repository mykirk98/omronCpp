#pragma once


// CStillImageFilesDlg

class CStillImageFilesDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStillImageFilesDlg)

public:
	CStillImageFilesDlg(StApi::IStStillImageFiler *pIStStillImageFiler, StApi::IStPixelFormatConverter *pIStPixelFormatConverter, CWnd* pParent = nullptr);
	virtual ~CStillImageFilesDlg();

//
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_STILL_IMAGE_FILES };
#endif

protected:
	StApi::IStStillImageFiler *m_pIStStillImageFiler;
	StApi::IStPixelFormatConverter *m_pIStPixelFormatConverter;
	StApi::CIStNodeMapDisplayWndPtr m_pIStNodeMapDisplayWnd;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	CString m_strStillImageFilesPath;
	CString m_strStillImageFilesPattern;
	afx_msg void OnBnClickedButtonStillImageFilesPath();
	int m_nMaximumImageFileCount;
	int m_nSaveAImagePer;
	int m_nFileType;
	virtual BOOL OnInitDialog();
};
