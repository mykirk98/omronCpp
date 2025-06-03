#pragma once

#include <QWidget>
#include "ui_settingdevicewidget.h"
#include <StApi_TL.h>
#include <StApi_IP.h>
#include <StApi_GUI.h>

class SettingDeviceWidget : public QWidget
{
	Q_OBJECT

public:
	SettingDeviceWidget(QWidget *parent = nullptr);
	~SettingDeviceWidget();

    // Add pINode to the display window.
    void RegisterINode(GenApi::INode *pINode, const GenICam::gcstring strTitle)
    {
        m_pIStNodeMapDisplayWnd->RegisterINode(pINode, strTitle);
    }

    // Refresh display.
    void RefreshDisplay()
    {
        m_pIStNodeMapDisplayWnd->Refresh();
    }

    // Close and Release IStNodeMapDisplayWnd.
    void terminate();
private:
	void resizeEvent(QResizeEvent *event) override;
    StApi::CIStNodeMapDisplayWndPtr m_pIStNodeMapDisplayWnd;
	Ui::SettingDeviceWidget ui;

};
