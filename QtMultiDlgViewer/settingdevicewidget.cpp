#include "settingdevicewidget.h"
#include <QPainter>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
SettingDeviceWidget::SettingDeviceWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

    m_pIStNodeMapDisplayWnd.Reset(StApi::CreateIStWnd(StApi::StWindowType_NodeMapDisplay));
	m_pIStNodeMapDisplayWnd->SetVisibleAlphabeticMode(false);
	m_pIStNodeMapDisplayWnd->SetVisibleCollapse(false);
	m_pIStNodeMapDisplayWnd->SetVisibleDescription(false);
	m_pIStNodeMapDisplayWnd->SetVisibleExpand(false);
	m_pIStNodeMapDisplayWnd->SetVisibleFilter(false);
	m_pIStNodeMapDisplayWnd->SetVisibleMenu(false);
	m_pIStNodeMapDisplayWnd->SetVisiblePolling(false);
	m_pIStNodeMapDisplayWnd->SetVisibleRefresh(false);
	m_pIStNodeMapDisplayWnd->SetVisibleStatusBar(false);
	m_pIStNodeMapDisplayWnd->SetVisibleVisibility(false);
	//m_pIStNodeMapDisplayWnd->SetPosition(0, 0, ui.width(), ui.height());
	
#ifdef Q_OS_WIN32
    m_pIStNodeMapDisplayWnd->Show((HWND)this->winId(), StApi::StWindowMode_Child);
#else
    m_pIStNodeMapDisplayWnd->Show(ui.verticalLayoutWidget, StApi::StWindowMode_Child);
#endif
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
SettingDeviceWidget::~SettingDeviceWidget()
{
    terminate();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SettingDeviceWidget::terminate()
{
    if (m_pIStNodeMapDisplayWnd.IsValid())
    {
        m_pIStNodeMapDisplayWnd->Close();
        m_pIStNodeMapDisplayWnd.Reset(NULL);
    }
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SettingDeviceWidget::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	const QSize qsize(size());
	ui.verticalLayoutWidget->resize(qsize);
	m_pIStNodeMapDisplayWnd->SetPosition(0, 0, qsize.width(), qsize.height());
}
