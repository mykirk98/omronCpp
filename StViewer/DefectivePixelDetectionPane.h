#pragma once

#include "IStImageCallback.h"
#include "DefectivePixelListCtrl.h"
#include "DefectivePixelManager.h"

class CDefectivePixelDetectionToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

// CDefectivePixelDetectionPane

class CDefectivePixelDetectionPane : public CDockablePane, public IStImageCallback
{
	DECLARE_DYNAMIC(CDefectivePixelDetectionPane)

public:
	CDefectivePixelDetectionPane(GenApi::INodeMap *pINodeMap, IStreamingCtrl *pIStreamingCtrl, StApi::IStImageDisplayWnd *pIStImageDisplayWnd);
	virtual ~CDefectivePixelDetectionPane();
	void AdjustLayout();

	GenApi::INodeMap *GetINodeMapForDefectivePixelDetectionFilter()
	{
		return(m_objCDefectivePixelManager.GetINodeMapForDefectivePixelDetectionFilter());
	}
	void ShowPane(BOOL bShow, BOOL bDelay, BOOL bActivate/* = TRUE*/);
public:
	//IStImageCallback
	void OnIStImage(StApi::IStImage *pIStImage);

protected:
	IStreamingCtrl * const m_pIStreamingCtrl;
	StApi::CIStImageAveragingFilterPtr m_pIStImageAveragingFilter;
	StApi::CIStImageBufferPtr m_pIStImageBuffer;
	CEvent m_objRcvImageDoneEvent;
	size_t m_nRcvedFrameCount;
	size_t m_nFrameCount;
	bool m_bSaveAveragedImage;
	bool m_bFirstTime;
	void SaveImage(StApi::IStImage *pIStImage, GenICam::gcstring &strFileName);
protected:
	CDefectivePixelManager m_objCDefectivePixelManager;
	CDefectivePixelListCtrl m_objListCtrl;
	CDefectivePixelDetectionToolBar m_wndToolBar;
	CStatusBar     m_wndStatusBar;
	const GenApi::CBooleanPtr m_pIBoolean_PixelCorrectionAllEnabled;
	const GenApi::CFloatPtr m_pIFloat_AcquisitionFrameRate;
	const GenApi::CFloatPtr m_pIFloat_ExposureTime;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateDetect(CCmdUI *pCmdUI);
	afx_msg void OnBnClickedDetect();
	afx_msg void OnUpdateClearDetected(CCmdUI *pCmdUI);
	afx_msg void OnBnClickedClearDetected();
	afx_msg void OnUpdateRegister(CCmdUI *pCmdUI);
	afx_msg void OnBnClickedRegister();
	afx_msg void OnUpdateDeregister(CCmdUI *pCmdUI);
	afx_msg void OnBnClickedDeregister();
	afx_msg void OnUpdateGetRegisteredPixelInfo(CCmdUI *pCmdUI);
	afx_msg void OnBnClickedGetRegisteredPixelInfo();
	afx_msg void OnUpdateSaveAveragedImage(CCmdUI *pCmdUI);
	afx_msg void OnBnClickedSaveAveragedImage();
	afx_msg void OnUpdateHighlightDefectivePixels(CCmdUI *pCmdUI);
	afx_msg void OnBnClickedHighlightDefectivePixels();
	afx_msg void OnUpdatePixelCorrectionEnabled(CCmdUI *pCmdUI);
	afx_msg void OnBnClickedPixelCorrectionEnabled();
};


