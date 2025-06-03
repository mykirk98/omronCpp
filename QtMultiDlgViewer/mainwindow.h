#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <StApi_TL.h>
#include <StApi_IP.h>
#include <StApi_GUI.h>

#include "devicewidget.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

void OnException(QWidget *pParent, const GenICam::GenericException &e);
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
	void closeEvent(QCloseEvent *);
protected:
	StApi::CIStSystemPtrArray m_objIStSystemPtrList;
	StApi::IStDeviceReleasable *CreateIStDevice();

	void UpdateLayout();
	//Qt slots.
private slots:
	void on_btnOpen_clicked();
public slots:
	void on_widgetDevice_closed(DeviceWidget *);
	void on_widgetDevice_lost(DeviceWidget *);
private:
    Ui::MainWindow *ui;

	std::vector<DeviceWidget*> m_vecDeviceWidget;
};
#endif // MAINWINDOW_H
