#pragma once


// CConfigurationFileSettingDlg 

class CConfigurationFileSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CConfigurationFileSettingDlg)

public:
	CConfigurationFileSettingDlg(CWnd* pParent = NULL);
	virtual ~CConfigurationFileSettingDlg();

	enum { IDD = IDD_DIALOG_CONFIGURATION_FILE };


protected:
	static const int m_pnComboBoxID[StConfigurationFileType_Count];
	void InitComboBox(int nCtrlID, EStConfigurationFileTarget_t eIniValue);
	EStConfigurationFileTarget_t GetCurTarget(int nCtrlID);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
