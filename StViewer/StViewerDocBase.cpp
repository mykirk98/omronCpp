#pragma once


// StViewerDocBase.cpp : implementation of the CStViewerDocBase class
//

#include "stdafx.h"
#include "StViewer.h"
#include "StViewerDocBase.h"
#include "resource.h"
#include <propkey.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT CStViewerDocBase::m_nUnusedID = IDW_UNUSED_ID_FIRST;
CStViewerDocBase::CStViewerDocBase() try: m_pCGraphView(NULL), m_pCNodeMapView(NULL)
{
}
catch (...)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocBase::SaveStillImage()
{
	try
	{
		StApi::CIStImageBufferPtr pIStImageBuffer(StApi::CreateIStImageBuffer(NULL));
		StApi::CIStStillImageFilerPtr pIStStillImageFiler(StApi::CreateIStFiler(StApi::StFilerType_StillImage));

		GetImageToSave(pIStStillImageFiler, pIStImageBuffer);
		ShowFileDlgAndSaveImage(pIStStillImageFiler, pIStImageBuffer->GetIStImage());
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocBase::ShowFileDlgAndSaveImage(StApi::IStStillImageFiler *pIStStillImageFiler, StApi::IStImage *pIStImage)
{

	typedef struct
	{
		StApi::EStStillImageFileFormat_t eFileType;
		LPCTSTR szExt;
		LPCTSTR szFilter;
	} SFILE_TYPE;

	const SFILE_TYPE psFileTypeList[] = {
		{ StApi::StStillImageFileFormat_Bitmap, TEXT(".bmp"), TEXT("Bitmap File(*.bmp)|*.bmp") },
		{ StApi::StStillImageFileFormat_JPEG, TEXT(".jpg"), TEXT("Jpeg File(*.jpg)|*.jpg") },
		{ StApi::StStillImageFileFormat_TIFF, TEXT(".tif"), TEXT("Tiff File(*.tif)|*.tif") },
		{ StApi::StStillImageFileFormat_PNG, TEXT(".png"), TEXT("PNG File(*.png)|*.png") },
		{ StApi::StStillImageFileFormat_CSV, TEXT(".csv"), TEXT("CSV File(*.csv)|*.csv") },
		{ StApi::StStillImageFileFormat_StApiRaw, TEXT(".straw"), TEXT("StRaw File(*.straw)|*.straw") },
	};

	{
		const StApi::EStPixelFormatNamingConvention_t ePixelFormat = pIStImage->GetImagePixelFormat();


		CString strDefExt;
		CString strFilter;
		std::vector <StApi::EStStillImageFileFormat_t> vecFormatList;
		for (size_t i = 0; i < _countof(psFileTypeList); ++i)
		{
			const SFILE_TYPE *pFileType = &psFileTypeList[i];
			if (pIStStillImageFiler->IsSaveSupported(ePixelFormat, pFileType->eFileType))
			{
				if (strDefExt.IsEmpty())
				{
					strDefExt = pFileType->szExt;
				}
				strFilter.Append(pFileType->szFilter);
				strFilter.AppendChar(TEXT('|'));
				vecFormatList.push_back(pFileType->eFileType);
			}
		}
		strFilter.AppendChar(TEXT('|'));

		CFileDialog dlg(FALSE, strDefExt, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
		dlg.m_ofn.lpstrTitle = TEXT("Save Still Image File");
		if (dlg.DoModal() == IDOK)
		{
			CString strFileName(dlg.m_ofn.lpstrFile);
			CString strExt(PathFindExtension(dlg.m_ofn.lpstrFile));
			StApi::EStStillImageFileFormat_t nFileType = vecFormatList[dlg.m_ofn.nFilterIndex - 1];
			for (size_t i = 0; i < _countof(psFileTypeList); ++i)
			{
				const SFILE_TYPE *pFileType = &psFileTypeList[i];
				if (strExt.CompareNoCase(pFileType->szExt) == 0)
				{
					nFileType = pFileType->eFileType;
					break;
				}
			}

			GenICam::gcstring strgcFileName(strFileName);
			pIStStillImageFiler->Save(pIStImage, nFileType, strgcFileName);

		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocBase::AddDeviceLog(UINT nResID)
{

	CStViewerApp *pCStViewerApp = dynamic_cast<CStViewerApp*>(AfxGetApp());
	if (pCStViewerApp)
	{
		CMDIFrameWndEx *pCMDIFrameWndEx = dynamic_cast<CMDIFrameWndEx*>(pCStViewerApp->GetMainWnd());
		if (pCMDIFrameWndEx)
		{
			CString strText;
			strText.LoadString(nResID);
			pCMDIFrameWndEx->SendMessage(WM_ADD_LOG, (WPARAM)(LPCTSTR)GetTitle(), (LPARAM)(LPCTSTR)strText);
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocBase::OnUpdateDrawingCommandHandler(CCmdUI *pCmdUI)
{
	const UINT nID = pCmdUI->m_nID;
	if (ID_DRAWING_NONE <= nID)
	{
		const UINT nTargetID = (UINT)(GetDrawingItemType() + ID_DRAWING_NONE);
		pCmdUI->SetRadio(nID == nTargetID);
	}
	else
	{
		if (ID_H_GRID_COUNT <= nID)
		{
			pCmdUI->Enable(TRUE);
			CObList listButtons;
			const int nCurIndex = GetGridCount(nID == ID_H_GRID_COUNT);
			if (0 < CMFCToolBar::GetCommandButtons(nID, listButtons))
			{
				for (POSITION posCombo = listButtons.GetHeadPosition(); posCombo != NULL;)
				{
					CMFCToolBarComboBoxButton* pCombo = DYNAMIC_DOWNCAST(CMFCToolBarComboBoxButton, listButtons.GetNext(posCombo));
					if (pCombo != NULL/* && CMFCToolBar::IsLastCommandFromButton(pCombo)*/)
					{
						if (pCombo->GetCurSel() != nCurIndex)
						{
							pCombo->SelectItem(nCurIndex);
						}
						break;
					}
				}
			}
		}

	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocBase::OnDrawingCommandHandler(UINT nID)
{
	if (ID_DRAWING_NONE <= nID)
	{
		SetDrawingItemType(nID - ID_DRAWING_NONE);
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocBase::SetGridCount(UINT nID)
{
	CObList listButtons;
	if (0 < CMFCToolBar::GetCommandButtons(nID, listButtons))
	{
		for (POSITION posCombo = listButtons.GetHeadPosition(); posCombo != NULL;)
		{
			CMFCToolBarComboBoxButton* pCombo = DYNAMIC_DOWNCAST(CMFCToolBarComboBoxButton, listButtons.GetNext(posCombo));
			if (pCombo != NULL && CMFCToolBar::IsLastCommandFromButton(pCombo))
			{
				SetGirdCount(nID == ID_H_GRID_COUNT, pCombo->GetCurSel());
				break;
			}
		}
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocBase::OnSelectHGridCount()
{
	SetGridCount(ID_H_GRID_COUNT);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocBase::OnSelectVGridCount()
{
	SetGridCount(ID_V_GRID_COUNT);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
UINT CStViewerDocBase::GetUnusedID(CWnd *pWnd)
{
	bool bBlankIDFound = false;
	UINT nID = CStViewerDocBase::m_nUnusedID;
	for (size_t i = IDW_UNUSED_ID_FIRST; i <= IDW_UNUSED_ID_LAST; ++i)
	{
		if (pWnd->GetDlgItem(nID) == NULL)
		{
			bBlankIDFound = true;
			break;
		}
		++nID;
		if (IDW_UNUSED_ID_LAST < nID)
		{
			nID = IDW_UNUSED_ID_FIRST;
		}
	}

	if (bBlankIDFound)
	{
		CStViewerDocBase::m_nUnusedID = nID + 1;
		if (IDW_UNUSED_ID_LAST < CStViewerDocBase::m_nUnusedID)
		{
			CStViewerDocBase::m_nUnusedID = IDW_UNUSED_ID_FIRST;
		}
		return(nID);
	}
	return(0);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocBase::GetPixelInfoText(StApi::IStImageDisplayWnd *pIStImageDisplayWnd, CString &strText)
{
	if (pIStImageDisplayWnd != NULL)
	{
		//Get current mouse cursor position.
		CPoint point;
		::GetCursorPos(&point);

		//Convert the mouse cursor position from the coordinate on the window to the coordinate on the image.
		int32_t nImagePos[] = { point.x, point.y };
		pIStImageDisplayWnd->ScreenToImage(&nImagePos[0], &nImagePos[1]);
		strText.AppendFormat(TEXT("(%d, %d) - "), nImagePos[0], nImagePos[1]);

		StApi::EStPixelFormatNamingConvention_t ePrevPixelFormat = StApi::StPFNC_Unknown;
		for (size_t i = 0; i < 2; i++)
		{
			//
			StApi::IStImage *pIStImage = NULL;
			if (pIStImageDisplayWnd->HasImage())
			{
				try
				{
					pIStImage = (i == 0) ? pIStImageDisplayWnd->GetRegisteredIStImage() : pIStImageDisplayWnd->GetConvertedIStImage();
					StApi::EStPixelFormatNamingConvention_t ePixelFormat = pIStImage->GetImagePixelFormat();

					//Skip second processing if the displayed image is same as the stored image.
					if (ePixelFormat != ePrevPixelFormat)
					{
						ePrevPixelFormat = ePixelFormat;
						if (0 < i)
						{
							strText.Append(TEXT(" -> "));
						}


						//Get pixel format name.
						const StApi::IStPixelFormatInfo *pIStPixelFormatInfo = StApi::GetIStPixelFormatInfo(ePixelFormat);
						strText.AppendFormat(GCSTRING_2_LPCTSTR(pIStPixelFormatInfo->GetName()));
						strText.AppendFormat(TEXT(" : "));

						//Get pixel information.
						StApi::IStPixelComponentValue *pIStPixelComponentValue = pIStImage->GetIStPixelComponentValue(nImagePos[0], nImagePos[1]);
						if (pIStPixelComponentValue)
						{
							size_t nCount = pIStPixelComponentValue->GetCount();
							for (size_t j = 0; j < nCount; j++)
							{
								StApi::EStPixelComponent_t nPixelComponent = pIStPixelComponentValue->GetPixelComponent(j);
								const StApi::IStPixelComponentInfo *pIStPixelComponentInfo = StApi::GetIStPixelComponentInfo(nPixelComponent);
								if (0 < j)
								{
									strText.AppendChar(TEXT(','));
								}

								switch (nPixelComponent)
								{
								case(StApi::StPixelComponent_Data32f):
								case(StApi::StPixelComponent_Data64f):
									strText.AppendFormat(TEXT("%f"), pIStPixelComponentValue->GetValueF(j));
									break;
								case(StApi::StPixelComponent_Data64):
										strText.AppendFormat(TEXT("%I64u"), (uint64_t)pIStPixelComponentValue->GetValue(j));
										break;
								default:
									{
										const int64_t nValue = pIStPixelComponentValue->GetValue(j);
										if (pIStPixelFormatInfo->GetName().compare(pIStPixelComponentInfo->GetName()) == 0)
											//if (pIStPixelFormatInfo->IsMono())
										{
											strText.AppendFormat(TEXT("%I64d"), nValue);
										}
										else
										{
											strText.AppendFormat(TEXT("%s=%I64d"), GCSTRING_2_LPCTSTR(pIStPixelComponentInfo->GetName()), nValue);
										}
									}
									break;
								}
							}
						}
					}
				}
				catch (...)
				{

				}
			}

		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CStViewerDocSingleDocBase::CStViewerDocSingleDocBase() try : m_pIStImageDisplayWnd(NULL)
{

	//Create object and get IStImageDisplayWnd pointer.
	m_pIStImageDisplayWnd.Reset(StApi::CreateIStWnd(StApi::StWindowType_ImageDisplay));
	m_pIInteger_HorizontalGridLineCount = m_pIStImageDisplayWnd->GetINodeMap()->GetNode("HorizontalGridLineCount");
	m_pIInteger_VerticalGridLineCount = m_pIStImageDisplayWnd->GetINodeMap()->GetNode("VerticalGridLineCount");
	m_pIEnumeration_DrawingItemType = m_pIStImageDisplayWnd->GetINodeMap()->GetNode("DrawingItemType");
}
catch (...)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocSingleDocBase::OnInitDisplayImageWnd(HWND hWnd)
{

	if (m_pIStImageDisplayWnd.IsValid())
	{
		m_pIStImageDisplayWnd->Show(hWnd, StApi::StWindowMode_Child);
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocSingleDocBase::OnDisplayImageWndResize(HWND /*hWnd*/, CRect &rect)
{
	if (m_pIStImageDisplayWnd.IsValid())
	{
		m_pIStImageDisplayWnd->SetPosition(rect.left, rect.top, rect.Width(), rect.Height());
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocSingleDocBase::GetImageToSave(StApi::IStStillImageFiler *pIStStillImageFiler, StApi::IStImageBuffer *pIStImageBuffer)
{
#if 0
	pIStImageBuffer->CopyImage(m_pIStImageDisplayWnd->GetConvertedIStImage());
#else
	StApi::CIStImageBufferPtr pIStImageBufferRegistered;
	pIStImageBufferRegistered.Reset(StApi::CreateIStImageBuffer(NULL));
	{
		//Get registered image.
		StApi::IStImage *pIStImageRegisteredTmp = m_pIStImageDisplayWnd->GetRegisteredIStImage();
		if (pIStImageRegisteredTmp == NULL) return;

		pIStImageBufferRegistered->CopyImage(pIStImageRegisteredTmp);
	}
	StApi::IStImage *pIStImageRegistered = pIStImageBufferRegistered->GetIStImage();
	const StApi::EStPixelFormatNamingConvention_t eSrcPixelFormat = pIStImageRegistered->GetImagePixelFormat();

	//Get curreng configuration.
	const StApi::IStPixelFormatConverter *pIStPixelFormatConverterForPreview = m_pIStImageDisplayWnd->GetIStPixelFormatConverter();

	const StApi::EStPixelFormatNamingConvention_t eDestPixelFormat = pIStPixelFormatConverterForPreview->GetDestinationPixelFormat();


	if (eSrcPixelFormat == eDestPixelFormat)
	{
		pIStImageBuffer->CopyImage(pIStImageRegistered);
	}
	else
	{
		//Convert from a registered image to a preview image.
		StApi::CIStPixelFormatConverterPtr pIStPixelFormatConverter(StApi::CreateIStConverter(StApi::StConverterType_PixelFormat));
		pIStPixelFormatConverter->CopySettings(pIStPixelFormatConverterForPreview);

		GenApi::CNodeMapPtr pINodeMap_DisplayImage(m_pIStImageDisplayWnd->GetINodeMap());
		GenApi::CEnumerationPtr pIEnumeration_BayerInterpolationMethodForStillImageFile(pINodeMap_DisplayImage->GetNode("BayerInterpolationMethodForStillImageFile"));
		if (pIEnumeration_BayerInterpolationMethodForStillImageFile->GetCurrentEntry()->GetSymbolic().compare("Auto") == 0)
		{
			pIStPixelFormatConverter->SetBayerInterpolationMethod(StApi::StBayerInterpolationMethod_BiLinear3);
		}

		pIStPixelFormatConverter->Convert(pIStImageRegistered, pIStImageBuffer);
	}
#endif
	if (m_pIStImageDisplayWnd->GetEnableDrawingOnSavingImage())
	{
		m_pIStImageDisplayWnd->GetIStDrawingTool()->DrawOnIStImageBuffer(pIStImageBuffer);
	}


	if (eDestPixelFormat == StApi::StPFNC_Mono8)
	{
		GenApi::CNodeMapPtr pINodeMap_PFConv_Preview(m_pIStImageDisplayWnd->GetINodeMap());
		GenApi::CNodeMapPtr pINodeMap_PFConv_Save(pIStStillImageFiler->GetINodeMap());

		{
			GenApi::CEnumerationPtr pIEnumeration_Preview(pINodeMap_PFConv_Preview->GetNode("ColorMapType"));
			GenApi::CEnumerationPtr pIEnumeration_Save(pINodeMap_PFConv_Save->GetNode("ColorMapType"));
			pIEnumeration_Save->SetIntValue(pIEnumeration_Preview->GetIntValue());
		}
		{
			GenApi::CBooleanPtr pIBoolean_Preview(pINodeMap_PFConv_Preview->GetNode("ColorMapInversion"));
			GenApi::CBooleanPtr pIBoolean_Save(pINodeMap_PFConv_Save->GetNode("ColorMapInversion"));
			pIBoolean_Save->SetValue(pIBoolean_Preview->GetValue());
		}
		{
			GenApi::CIntegerPtr pIInteger_Preview(pINodeMap_PFConv_Preview->GetNode("ColorMapPhase"));
			GenApi::CIntegerPtr pIInteger_Save(pINodeMap_PFConv_Save->GetNode("ColorMapPhase"));
			pIInteger_Save->SetValue(pIInteger_Preview->GetValue());
		}
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int CStViewerDocSingleDocBase::GetGridCount(bool isHoriz)
{
	return((int)(isHoriz ? m_pIInteger_HorizontalGridLineCount->GetValue() : m_pIInteger_VerticalGridLineCount->GetValue()));
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocSingleDocBase::SetGirdCount(bool isHoriz, int nCount)
{
	if (isHoriz)
	{
		m_pIInteger_HorizontalGridLineCount->SetValue(nCount);
	}
	else
	{
		m_pIInteger_VerticalGridLineCount->SetValue(nCount);
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
size_t CStViewerDocSingleDocBase::GetDrawingItemType()
{
	return((size_t)m_pIEnumeration_DrawingItemType->GetIntValue());
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocSingleDocBase::SetDrawingItemType(size_t nType)
{
	m_pIEnumeration_DrawingItemType->SetIntValue(nType);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDocSingleDocBase::RegisterNodesToView()
{
	//Get the NodeMap of the ImageDisplayWnd.
	if (m_pIStImageDisplayWnd)
	{
		{
			GenApi::CNodeMapPtr pINodeMap(m_pIStImageDisplayWnd->GetINodeMap());
			GenICam::gcstring strTitle("ImageDisplayWnd");
			m_vecRegisteredINodeHandles.push_back((m_pCNodeMapView->RegisterINode(pINodeMap->GetNode("Root"), strTitle)));
		}

		{
			GenApi::CNodeMapPtr pINodeMap(m_pIStImageDisplayWnd->GetIStPixelFormatConverter()->GetINodeMap());
			GenICam::gcstring strTitle("PixelFormatConverter");
			m_vecRegisteredINodeHandles.push_back(m_pCNodeMapView->RegisterINode(pINodeMap->GetNode("Root"), strTitle));
		}
	}

	if (m_pCGraphView)
	{
		GenApi::CNodeMapPtr pINodeMapGraphDataFilter(m_pCGraphView->GetINodeMapForGraphDataFilter());
		m_pCNodeMapView->RegisterINode(pINodeMapGraphDataFilter->GetNode("Root"), "Graph Data Filter");
		m_pCNodeMapView->RegisterINode(m_pCGraphView->GetINodeMapForGraphDisplayWnd()->GetNode("Root"), "Graph Display Wnd");
	}

	//Get the NodeMap of the NodeMapDisplayWnd.
	{
		GenApi::CNodeMapPtr pINodeMap(m_pCNodeMapView->GetINodeMap());
		GenICam::gcstring strTitle("NodeMapWnd");
		m_vecRegisteredINodeHandles.push_back(m_pCNodeMapView->RegisterINode(pINodeMap->GetNode("Root"), strTitle));
	}
}