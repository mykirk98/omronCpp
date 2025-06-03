#pragma once

#include "CButtonImage.h"

// CEachDeviceDlg dialog

class CEachDeviceDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEachDeviceDlg)

public:
	CEachDeviceDlg(StApi::IStDeviceReleasable *pIStDevice, CWnd* pParent = nullptr);	// standard constructor
	virtual ~CEachDeviceDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DEVICE };
#endif

protected:
	StApi::CIStImageBufferPtr m_pIStImageBuffer;
	StApi::CIStPixelFormatConverterPtr m_pIStPixelFormatConverter;
	StApi::CIStDevicePtr m_pIStDevice;
	StApi::CIStRegisteredCallbackPtr m_pIStRegisteredCallbackDeviceLost;
	StApi::CIStDataStreamPtr m_pIStDataStream;
	StApi::CIStStreamBufferPtr m_pIStStreamBuffer;
	StApi::CIStNodeMapDisplayWndPtr m_pIStNodeMapDisplayWnd;
	CStatusBar     m_wndStatusBar;

	GenApi::CLock m_objLockForStreamingBuffer;
	std::vector<uint8_t> m_vecBitmapInfo;
	CString m_strTitle;
	CBitmap m_objCBitmap;

	bool m_isStreamingStarted;
	bool m_isDeviceLostDetected;
	bool m_isNewImage;
	uint64_t m_nDroppedFrameCount;
	UINT_PTR m_nTimerID;
	CBrush m_objBackgroundBrush;
	void CloseDevice();
	void OpenDevice(StApi::IStDeviceReleasable *);
	void StartStreaming(uint64_t nFrameCount = GENTL_INFINITE);
	void StopStreaming();

	void OnDeviceLost(GenApi::INode *pINode, void*);
	void OnStCallback(StApi::IStCallbackParamBase *pIStCallbackParamBase, void * /*pvContext*/);
	void ConvertImageToVisibleFormat();
	BOOL GetBitmapImage(CDC *pDC, CBitmap &objCBitmap);
	BOOL GetLatestImage(StApi::IStImageBuffer *);
	void UpdateButtonState();
	void UpdateStatusBar();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL DestroyWindow();
	afx_msg void OnBnClickedButtonAcqStart();
	afx_msg void OnBnClickedButtonAcqStop();
	afx_msg void OnBnClickedButtonClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
private:
	CButtonImage m_btnImage;
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedButtonSnap();
};
