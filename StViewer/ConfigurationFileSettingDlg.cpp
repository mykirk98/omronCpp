// ConfigurationFileSettingDlg.cpp : 
//

#include "stdafx.h"
#include "StViewer.h"
#include "ConfigurationFileSettingDlg.h"
#include "afxdialogex.h"


// CConfigurationFileSettingDlg

IMPLEMENT_DYNAMIC(CConfigurationFileSettingDlg, CDialogEx)


const int CConfigurationFileSettingDlg::m_pnComboBoxID[StConfigurationFileType_Count] =
{
	IDC_COMBO_CONFIG_FILE_DEVICE_NODE_MAP_WINDOW,
	IDC_COMBO_CONFIG_FILE_DISPLAY_IMAGE_WINDOW,
	IDC_COMBO_CONFIG_FILE_DEFECTIVE_PIXEL_DETECTION,
	IDC_COMBO_CONFIG_FILE_PIXEL_FORMAT_CONVERTER,
	IDC_COMBO_CONFIG_FILE_STVIEWER
};

CConfigurationFileSettingDlg::CConfigurationFileSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CConfigurationFileSettingDlg::IDD, pParent)
{

}

CConfigurationFileSettingDlg::~CConfigurationFileSettingDlg()
{
}

void CConfigurationFileSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CConfigurationFileSettingDlg, CDialogEx)
END_MESSAGE_MAP()


// CConfigurationFileSettingDlg


BOOL CConfigurationFileSettingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	
	CStViewerApp *pCStViewerApp = dynamic_cast<CStViewerApp*>(AfxGetApp());
	for (EStConfigurationFileType_t eType = StConfigurationFileType_First; eType < StConfigurationFileType_Count; eType = (EStConfigurationFileType_t)(eType + 1))
	{
		InitComboBox(m_pnComboBoxID[eType], pCStViewerApp->GetConfigurationFileTarget(eType));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}


void CConfigurationFileSettingDlg::OnOK()
{
	CStViewerApp *pCStViewerApp = dynamic_cast<CStViewerApp*>(AfxGetApp());
	for (EStConfigurationFileType_t eType = StConfigurationFileType_First; eType < StConfigurationFileType_Count; eType = (EStConfigurationFileType_t)(eType + 1))
	{
		pCStViewerApp->SetConfigurationFileTarget(eType, GetCurTarget(m_pnComboBoxID[eType]));
	}
	CDialogEx::OnOK();
}

void CConfigurationFileSettingDlg::InitComboBox(int nCtrlID, EStConfigurationFileTarget_t eIniValue)
{
	CComboBox objComboBox;
	objComboBox.Attach(GetDlgItem(nCtrlID)->GetSafeHwnd());

	LPCTSTR pszNames[StConfigurationFileTarget_Count] = {
		TEXT("None"),
		TEXT("All cameras and all files"),
		TEXT("Each model of camera and each type of file"),
		TEXT("Each camera and each file"),
	};

	for (size_t i = 0; i < _countof(pszNames); ++i)
	{
		const int nIndex = objComboBox.AddString(pszNames[i]);
		objComboBox.SetItemData(nIndex, i);
		if (i == (size_t)eIniValue)
		{
			objComboBox.SetCurSel(nIndex);
		}
	}
	objComboBox.Detach();
}
EStConfigurationFileTarget_t CConfigurationFileSettingDlg::GetCurTarget(int nCtrlID)
{
	CComboBox objComboBox;
	objComboBox.Attach(GetDlgItem(nCtrlID)->GetSafeHwnd());
	const EStConfigurationFileTarget_t eTarget = (EStConfigurationFileTarget_t)objComboBox.GetItemData(objComboBox.GetCurSel());
	objComboBox.Detach();
	return(eTarget);
}