
// StViewerDoc.h : interface of the CStViewerDoc class
//


#pragma once


#include "NodeMapView.h"
#include "GraphView.h"
#include "DefectivePixelDetectionPane.h"
#include "AVIFile.h"
#include "IStImageCallback.h"
#include "IStreamingCtrl.h"
#include "StViewerDocBase.h"

#define USE_DISPLAY_RELEASABLE_IMAGE_BUFFER

class CStViewerDoc : public CStViewerDocSingleDocBase, public IStreamingCtrl
{
protected: // create from serialization only
	CStViewerDoc();
	DECLARE_DYNCREATE(CStViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();

// Implementation
public:
	virtual ~CStViewerDoc();

public:

	/*!
	Get the current FPS value.
	*/
	void GetFPSString(CString &str);

	/*!
	Get the received image count.
	*/
	uint64_t GetReceivedImageCount(){ return(m_nReceivedImageCount); };

	/*!
	Get the dropped frame count.
	*/
	uint64_t GetDroppedIDFrameCount(){ return(m_nDroppedIDCount); };


	void GetStatusBarText(size_t nIndex, CString &strText);


// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()


protected:
	StApi::CIStRegisteredCallbackPtr m_pIStRegisteredCallbackDeviceLost;
	StApi::CIStDataStreamPtr m_pIStDataStream;
	StApi::CIStDevicePtr m_pIStDevice;
	bool m_IsAcquisitionRunning;
	uint64_t m_nReceivedImageCount;
	uint64_t m_nDroppedIDCount;
	uint64_t m_nLastID;
	bool	m_bFirstFrame;
	CString m_strDeviceName;

	CDefectivePixelDetectionPane *m_pCDefectivePixelDetectionPane;

	bool	m_IsDeviceLost;

	CIStImageCallbackList m_objCIStImageCallbackList;

	GenApi::CLock m_CLockForAVI;
	CSaveMultipleImagesFileBase *m_pCSaveMultipleImagesFileBase;
	GenApi::CNodeMapRef	m_pStViewerNodeMapRef;
	GenApi::CNodeMapPtr m_pStViewerNodeMap;
	/*!
	Return true if recording is running.
	*/
	bool IsRecording(){ return(m_pCSaveMultipleImagesFileBase != NULL); };

	/*!
	Callback function for EventDeviceLost.
	*/
	void OnDeviceLost(GenApi::INode *pINode, void*);

	/*!
	Callback function for EventNewBuffer.
	*/
	void OnStCallback(StApi::IStCallbackParamBase *, void *pvContext);

	/*!
	Return true if image acquisition is running.
	*/
	bool IsAcquisitionRunning() const {return(m_IsAcquisitionRunning);};

	/*!
	Start image acquisition.
	*/
	void StartImageAcquisition();

	/*!
	Stop image acquisition.
	*/
	void StopImageAcquisition();

	/*!
	Start recording.
	*/
	void StartRecording();

	/*!
	Stop recording.
	*/
	void StopRecording();

	/*!
	Save or load a camera config file.
	*/
	void CameraConfigFile(BOOL isOpenMode);

	/*!
	Save a camera description file.
	*/
	void SaveCameraDescriptionFile();


protected:
	void RegisterNodesToView() override;
	void CreateStViewerNodeMap();
protected:
	afx_msg void OnUpdateCommandHandler(CCmdUI *pCmdUI);
	afx_msg void OnCommandHandler(UINT nID);
	virtual void OnCloseDocument();

	afx_msg void OnFileStartFfc();
	afx_msg void OnUpdateFileStartFfc(CCmdUI *pCmdUI);
	afx_msg void OnGraphDataSourceCommandHandler(CCmdUI *pCmdUI);
	afx_msg void OnGraphDataSource(UINT nID);
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
};
