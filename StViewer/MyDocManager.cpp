#include "stdafx.h"
#include "MyDocManager.h"
#include "sal.h"

void CMyDocManager::OnFileNew()
{
	if (m_templateList.IsEmpty())
	{
		TRACE(traceAppMsg, 0, "Error: no document templates registered with CWinApp.\n");
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		return;
	}

	CDocTemplate* pTemplate = reinterpret_cast<CDocTemplate*>(m_templateList.GetHead());
	if (m_templateList.GetCount() > 1)
	{
		size_t i = 0;
		POSITION pos = m_templateList.GetHeadPosition();
		// add all the CDocTemplates in the list by name
		while (pos != NULL)
		{
			CDocTemplate* pTemp = reinterpret_cast<CDocTemplate*>(m_templateList.GetNext(pos));
			if (i == m_nTypeIndex)
			{
				pTemplate = pTemp;
				break;
			}
			++i;
		}
	}

	ASSERT(pTemplate != NULL);
	ASSERT_KINDOF(CDocTemplate, pTemplate);

	pTemplate->OpenDocumentFile(NULL);
	// if returns NULL, the user has already been alerted
}
void CMyDocManager::OnFileOpen()
{
	CFileDialog dlgFile(TRUE, 
		TEXT("straw"),
		NULL, 
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, 
		TEXT("StRaw file(*.straw)|*.straw|Bitmap file(*.bmp)|*.bmp|PNG file(*.png)|*.png|TIFF file(*.tif, *.tiff)|*.tif;*.tiff|JPEG file(*.jpg, *.jpeg)|*.jpg;*.jpeg|All files(*.*)|*.*||")
		);
	CString title;
	title.LoadString(AFX_IDS_OPENFILE);

	dlgFile.m_ofn.lpstrTitle = title;
	CString strFileName;
	dlgFile.m_ofn.lpstrFile = strFileName.GetBuffer(_MAX_PATH);
	if (IDOK == dlgFile.DoModal())
	{
		OpenDocumentFile(strFileName);
	}
}