
// MFCMultiDlgViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFCMultiDlgViewer.h"
#include "MFCMultiDlgViewerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAXIMUM_DEVICE_COUNT 4

//-----------------------------------------------------------------------------
// CMFCMultiDlgViewerDlg dialog
//-----------------------------------------------------------------------------
CMFCMultiDlgViewerDlg::CMFCMultiDlgViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMFCMultiDlgViewerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCMultiDlgViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CMFCMultiDlgViewerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CMFCMultiDlgViewerDlg::OnBnClickedButtonOpen)
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_DEVICE_WND_CLOSED, OnDeviceWndClosed)
	ON_MESSAGE(WM_DEVICE_LOST, OnDeviceLost)
END_MESSAGE_MAP()

// CMFCMultiDlgViewerDlg message handlers

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CMFCMultiDlgViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCMultiDlgViewerDlg::OnPaint()
{
	// If you add a minimize button to your dialog, you will need the code below
	//  to draw the icon. 
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//-----------------------------------------------------------------------------
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
//-----------------------------------------------------------------------------
HCURSOR CMFCMultiDlgViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCMultiDlgViewerDlg::OnBnClickedButtonOpen()
{
	StApi::IStDeviceReleasable *pIStDeviceReleasable = dynamic_cast<CMFCMultiDlgViewerApp*>(AfxGetApp())->CreateIStDevice();
	if (pIStDeviceReleasable == NULL) return;

	CEachDeviceDlg *pCEachDeviceDlg = new CEachDeviceDlg(pIStDeviceReleasable, this);
	pCEachDeviceDlg->Create(IDD_DIALOG_DEVICE, this);
	m_vecCEachDeviceDlg.push_back(pCEachDeviceDlg);
	UpdateLayout();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCMultiDlgViewerDlg::UpdateLayout()
{
	CRect rect;
	const int nMarginX = 16;
	const int nMarginY = 16;
	const size_t nCount = m_vecCEachDeviceDlg.size();
	CRect rectOpenButton;
	CWnd *pOpenBtnWnd = GetDlgItem(IDC_BUTTON_OPEN);
	pOpenBtnWnd->EnableWindow(nCount < MAXIMUM_DEVICE_COUNT);
	pOpenBtnWnd->GetWindowRect(&rectOpenButton);
	ScreenToClient(&rectOpenButton);
	if (nCount == 0)
	{
		rect.SetRect(0, 0, rectOpenButton.Width() + nMarginX * 2, rectOpenButton.Height() + nMarginY * 2);
	}
	else
	{
		CRect rectEach;
		CEachDeviceDlg *pCEachDeviceDlg = m_vecCEachDeviceDlg[0];
		pCEachDeviceDlg->GetWindowRect(&rectEach);
		ScreenToClient(&rectEach);
		rect.SetRect(0, 0, rectEach.Width(), rectEach.Height());
		rect.MoveToY(rectOpenButton.bottom + nMarginY);
		for(size_t i = 0; i < nCount; ++i)
		{
			pCEachDeviceDlg = m_vecCEachDeviceDlg[i];
			rect.MoveToX((rectEach.Width() + nMarginX) * i + nMarginX);
			pCEachDeviceDlg->MoveWindow(&rect);
			pCEachDeviceDlg->ShowWindow(SW_SHOW);
		}
		rect.SetRect(0, 0, (rectEach.Width() + nMarginX) * nCount + nMarginX * 2, rectOpenButton.bottom + rectEach.Height() + nMarginY * 2);
	}

	CRect rectWin;
	CRect rectClient;
	GetWindowRect(&rectWin);
	GetClientRect(&rectClient);

	rect.bottom += rectWin.Height() - rectClient.Height();
	SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOMOVE | SWP_NOZORDER);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CMFCMultiDlgViewerDlg::DestroyWindow()
{
	for (size_t i = 0; i < m_vecCEachDeviceDlg.size(); ++i)
	{
		CEachDeviceDlg *pCEachDeviceDlg = m_vecCEachDeviceDlg[i];
		pCEachDeviceDlg->DestroyWindow();
	}
	return CDialogEx::DestroyWindow();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
LRESULT CMFCMultiDlgViewerDlg::OnDeviceWndClosed(WPARAM wParam, LPARAM lParam)
{
	for (std::vector<CEachDeviceDlg*>::iterator itr = m_vecCEachDeviceDlg.begin(); itr != m_vecCEachDeviceDlg.end(); ++itr)
	{
		if (*itr == (CEachDeviceDlg*)wParam)
		{
			m_vecCEachDeviceDlg.erase(itr);
			break;
		}
	}
	UpdateLayout();
	return(0);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
LRESULT CMFCMultiDlgViewerDlg::OnDeviceLost(WPARAM wParam, LPARAM lParam)
{
	CEachDeviceDlg *pCEachDeviceDlg = (CEachDeviceDlg *)wParam;
	AfxMessageBox(TEXT("Devict lost."), MB_ICONSTOP);
	pCEachDeviceDlg->DestroyWindow();
	return(0);
}
