
// StStillImageDoc.h : interface of the CStStillImageDoc class
//


#pragma once


#include "DefectivePixelDetectionPane.h"
#include "AVIFile.h"
#include "IStImageCallback.h"
#include "IStreamingCtrl.h"
#include "StViewerDocBase.h"

class CStStillImageDoc : public CStViewerDocSingleDocBase
{
protected: // create from serialization only
	CStStillImageDoc();
	DECLARE_DYNCREATE(CStStillImageDoc)

// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual ~CStStillImageDoc();

public:


	void GetStatusBarText(size_t nIndex, CString &strText);


// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()


protected:
	CString m_strFileName;
	CTime m_UpdateTime;

protected:
	afx_msg void OnUpdateCommandHandler(CCmdUI *pCmdUI);
	afx_msg void OnCommandHandler(UINT nID);
	virtual void OnCloseDocument();

	afx_msg void OnGraphDataSourceCommandHandler(CCmdUI *pCmdUI);
	afx_msg void OnGraphDataSource(UINT nID);
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
protected:
	void RegisterNodesToView() override;
};
