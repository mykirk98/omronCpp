
// MFCSingleDocViewerDoc.h : interface of the CMFCSingleDocViewerDoc class
//


#pragma once
#include "PropertiesWnd.h"


class CMFCSingleDocViewerDoc : public CDocument
{
protected: // create from serialization only
	CMFCSingleDocViewerDoc();
	DECLARE_DYNCREATE(CMFCSingleDocViewerDoc)

// Attributes
public:
	bool IsDeviceOpened() const { return(m_pIStDevice.IsValid()); }
	bool IsStreamingStarted() const { return(m_isStreamingStarted); }
// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument() override;
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CMFCSingleDocViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	void CloseDevice();
	void OpenDevice(StApi::IStDeviceReleasable *);

protected:
	StApi::CIStImageBufferPtr m_pIStImageBuffer;
	StApi::CIStPixelFormatConverterPtr m_pIStPixelFormatConverter;
	StApi::CIStDevicePtr m_pIStDevice;
	StApi::CIStRegisteredCallbackPtr m_pIStRegisteredCallbackDeviceLost;
	StApi::CIStDataStreamPtr m_pIStDataStream;
	StApi::CIStStreamBufferPtr m_pIStStreamBuffer;
	GenApi::CLock m_objLockForStreamingBuffer;
	std::vector<uint8_t> m_vecBitmapInfo;
	CPropertiesWnd    m_wndProperties;
	HWND m_hWnd;
	bool m_isStreamingStarted;
	bool m_isDeviceLostDetected;
	HWND GetWndHandle();
	void StartStreaming(uint64_t nFrameCount = GENTL_INFINITE);
	void StopStreaming();
public:
	void OnDeviceLost(GenApi::INode *pINode, void*);
	void OnStCallback(StApi::IStCallbackParamBase *pIStCallbackParamBase, void * /*pvContext*/);
	void ConvertImageToVisibleFormat();
	BOOL GetBitmapImage(CDC *pDC, CBitmap &objCBitmap);
	BOOL GetLatestImage(StApi::IStImageBuffer *);
	StApi::IStDevice *GetIStDevice() { return(m_pIStDevice); }
	StApi::IStDataStream *GetIStDataStream() { return(m_pIStDataStream); }
// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	afx_msg void OnCameraAcqStart();
	afx_msg void OnCameraAcqStop();
	afx_msg void OnCameraSnap();
	afx_msg void OnUpdateCameraAcqStop(CCmdUI *pCmdUI);
	afx_msg void OnUpdateCameraAcqStart(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI *pCmdUI);
	afx_msg void OnUpdateCameraSnap(CCmdUI *pCmdUI);
	afx_msg void OnPropertiesWnd();
	afx_msg void OnUpdatePropertiesWnd(CCmdUI *pCmdUI);
};
