#pragma once

#include "NodeMapView.h"
#include "GraphView.h"

class CStViewerDocBase : public CDocument
{
public:
	CStViewerDocBase();
	virtual ~CStViewerDocBase() {};

	virtual void GetStatusBarText(size_t nIndex, CString &strText) = 0;
	static void GetPixelInfoText(StApi::IStImageDisplayWnd *pIStImageDisplayWnd, CString &strText);

	void AddDeviceLog(UINT nResID);

	virtual void OnInitDisplayImageWnd(HWND hWnd) = 0;
	virtual void OnDisplayImageWndResize(HWND hWnd, CRect &rect) = 0;

	virtual void RegisterNodesToView() = 0;
	void DeregisterNodesFormView()
	{
		if (m_pCNodeMapView)
		{
			m_pCNodeMapView->DeregisterINode(m_vecRegisteredINodeHandles);
		}
	}
protected:
	CNodeMapView	*m_pCNodeMapView;
	CGraphView	*m_pCGraphView;
	std::vector<StApi::RegisteredINodeHandle_t> m_vecRegisteredINodeHandles;
protected:
	/*!
	Save a still image.
	*/
	void SaveStillImage();
	void ShowFileDlgAndSaveImage(StApi::IStStillImageFiler *, StApi::IStImage *pIStImage);
	void SetGridCount(UINT nID);

	UINT GetUnusedID(CWnd *);
	static UINT m_nUnusedID;

protected:
	virtual void GetImageToSave(StApi::IStStillImageFiler *, StApi::IStImageBuffer *) = 0;
	virtual int GetGridCount(bool isHoriz) = 0;
	virtual void SetGirdCount(bool isHoriz, int nCount) = 0;
	virtual size_t GetDrawingItemType() = 0;
	virtual void SetDrawingItemType(size_t) = 0;
protected:
	afx_msg void OnUpdateDrawingCommandHandler(CCmdUI *pCmdUI);
	afx_msg void OnDrawingCommandHandler(UINT nID);
	afx_msg void OnSelectHGridCount();
	afx_msg void OnSelectVGridCount();
};


class CStViewerDocSingleDocBase : public CStViewerDocBase
{
public:
	CStViewerDocSingleDocBase();
	virtual ~CStViewerDocSingleDocBase() {};

	void OnInitDisplayImageWnd(HWND hWnd) override;
	void OnDisplayImageWndResize(HWND hWnd, CRect &rect) override;
	virtual void RegisterNodesToView() override;
protected:
	StApi::CIStImageDisplayWndPtr m_pIStImageDisplayWnd;
	GenApi::CEnumerationPtr m_pIEnumeration_DrawingItemType;
	GenApi::CIntegerPtr m_pIInteger_HorizontalGridLineCount;
	GenApi::CIntegerPtr m_pIInteger_VerticalGridLineCount;

protected:
	virtual int GetGridCount(bool isHoriz) override;
	virtual void SetGirdCount(bool isHoriz, int nCount) override;
	virtual size_t GetDrawingItemType() override;
	virtual void SetDrawingItemType(size_t) override;
protected:
	void GetImageToSave(StApi::IStStillImageFiler *, StApi::IStImageBuffer *);

};

