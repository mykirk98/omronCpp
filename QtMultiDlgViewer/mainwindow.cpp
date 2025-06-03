#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qmessagebox.h>
#define MAXIMUM_DEVICE_COUNT 4

using namespace GenTL;
using namespace StApi;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OnException(QWidget *pParent, const GenICam::GenericException &e)
{
	//Get the exception contents.
	GenICam::gcstring strSourceFileName(e.GetSourceFileName());
	const unsigned int iSourceLine = e.GetSourceLine();
	GenICam::gcstring strDescription(e.GetDescription());

	//Make message string.
	QString strSourceLine;
	strSourceLine.setNum(iSourceLine);

	QString strMessage;
	strMessage = strSourceFileName;
	strMessage.append(strSourceLine);
	strMessage.append(strDescription);

	//Show message box.
	QMessageBox msgBox(pParent);
	msgBox.setWindowTitle("Exception");

	msgBox.setText(strMessage);
	msgBox.exec();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	//Create object and initialize "IStSystemPtr" list.
	const uint32_t nCount = StSystemVendor_Count;
	//const uint32_t nCount = 1;	//Default only
	//const uint32_t nCount = 2;	//Default + Euresys
	for (uint32_t i = StSystemVendor_Default; i < nCount; ++i)
	{
		EStSystemVendor_t eStSystemVendor = (EStSystemVendor_t)i;
		try
		{
			// Create a system object, to get the IStSystemReleasable interface pointer.
			// After the system object is no longer needed, call the IStSystemReleasable::Release(), please discard the system object.
			IStSystemReleasable *pIStSystemReleasable = CreateIStSystem(eStSystemVendor, StInterfaceType_All);
			m_objIStSystemPtrList.Register(pIStSystemReleasable);
		}
		catch (const GenICam::GenericException &e)
		{
			if (eStSystemVendor == StSystemVendor_Default)
			{
				OnException(this, e);
			}
		}
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}
//-----------------------------------------------------------------------------
//Select device and get IStDeviceReleasable pointer.
//-----------------------------------------------------------------------------
IStDeviceReleasable *MainWindow::CreateIStDevice()
{
	IStDeviceReleasable *pIStDeviceReleasable = NULL;

	//Create "DeviceSelectionWnd".
	CIStDeviceSelectionWndPtr pIStDeviceSelectionWnd(CreateIStWnd(StWindowType_DeviceSelection));

	//Move the "DeviceSelectionWnd" to the center of the main window.
	int nWidth = 1280;
	int nHeight = 720;
	int nOffsetX = x() + (width() - nWidth) / 2;
	if (nOffsetX < 0) nOffsetX = 0;
	int nOffsetY = y() + (height() - nHeight) / 2;
	if (nOffsetY < 0) nOffsetY = 0;
	pIStDeviceSelectionWnd->SetPosition(nOffsetX, nOffsetY, nWidth, nHeight);

	//Specify the "IStSystem" to use.
	pIStDeviceSelectionWnd->RegisterTargetIStSystemList(m_objIStSystemPtrList);

	//Show the "DeviceSelectionWnd".
#ifdef Q_OS_WIN32
	pIStDeviceSelectionWnd->Show((HWND)ui->centralwidget->winId(), StApi::StWindowMode_Modal);
#else
	pIStDeviceSelectionWnd->Show(ui->centralwidget, StApi::StWindowMode_Modal);
#endif

	//Get selected device information.
	StApi::IStInterface *pIStInterface = NULL;
	const StApi::IStDeviceInfo *pIStDeviceInfo = NULL;
	pIStDeviceSelectionWnd->GetSelectedDeviceInfo(&pIStInterface, &pIStDeviceInfo);

	if (pIStDeviceInfo != NULL)
	{
		//Get available DeviceAccessFlag.
		DEVICE_ACCESS_FLAGS eDeviceAccessFlags = DEVICE_ACCESS_CONTROL;
		switch (pIStDeviceInfo->GetAccessStatus())
		{
		case(DEVICE_ACCESS_STATUS_READONLY):
			eDeviceAccessFlags = DEVICE_ACCESS_READONLY;
			break;
		case(DEVICE_ACCESS_STATUS_READWRITE):
			eDeviceAccessFlags = DEVICE_ACCESS_CONTROL;
			break;
		}

		//For GigE Vision switchover function
		if (eDeviceAccessFlags == DEVICE_ACCESS_CONTROL)
		{
			const uint32_t nDevCount = pIStInterface->GetDeviceCount();
			for (uint32_t nDevIndex = 0; nDevIndex < nDevCount; ++nDevIndex)
			{
				if (pIStInterface->GetIStDeviceInfo(nDevIndex) == pIStDeviceInfo)
				{
					GenApi::CIntegerPtr pInteger_DeviceSelector(pIStInterface->GetIStPort()->GetINodeMap()->GetNode("DeviceSelector"));
					if (GenApi::IsWritable(pInteger_DeviceSelector))
					{
						pInteger_DeviceSelector->SetValue(nDevIndex);

						GenApi::CIntegerPtr pInteger_SwitchoverKey(pIStInterface->GetIStPort()->GetINodeMap()->GetNode("GevApplicationSwitchoverKey"));
						if (GenApi::IsWritable(pInteger_SwitchoverKey))
						{
							//If you use switchover function for GigE Vision, please set a switchover key here.
							const uint16_t nSwitchoverKey = 0;
							pInteger_SwitchoverKey->SetValue(nSwitchoverKey);
						}
					}
					break;
				}
			}
		}

		//Create object and get IStDeviceReleasable pointer.
		GenICam::gcstring strDeviceID = pIStDeviceInfo->GetID();
		pIStDeviceReleasable = pIStInterface->CreateIStDevice(strDeviceID, eDeviceAccessFlags);
	}

	return(pIStDeviceReleasable);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MainWindow::on_btnOpen_clicked()
{
	CIStDevicePtr pIStDevice(CreateIStDevice());
	if (pIStDevice.IsValid())
	{
		DeviceWidget *pDeviceWidget = new DeviceWidget(pIStDevice.Move(), this);
		m_vecDeviceWidget.push_back(pDeviceWidget);
		UpdateLayout();
		pDeviceWidget->show();
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MainWindow::UpdateLayout()
{
	QRect rect;
	const int nMarginX = 16;
	const int nMarginY = 16;
	const size_t nCount = m_vecDeviceWidget.size();
	
	ui->btnOpen->setEnabled(nCount < MAXIMUM_DEVICE_COUNT);
	
	if (nCount == 0)
	{
		rect.setRect(0, 0, ui->btnOpen->width() + nMarginX * 2, ui->btnOpen->height() + nMarginY * 2);
	}
	else
	{
		DeviceWidget *pDeviceWidget = m_vecDeviceWidget[0];
		QRect rectEach;
		rectEach.setRect(0, 0, pDeviceWidget->width(), pDeviceWidget->height());

		rect = rectEach;
		rect.moveTop(ui->btnOpen->y() + ui->btnOpen->height() + nMarginY);
		for (size_t i = 0; i < nCount; ++i)
		{
			pDeviceWidget = m_vecDeviceWidget[i];
			rect.moveLeft((rectEach.width() + nMarginX) * i + nMarginX);
			pDeviceWidget->move(rect.left(), rect.top());
		}
		rect.setRect(0, 0, (rectEach.width() + nMarginX) * nCount + nMarginX * 2, ui->btnOpen->y() + ui->btnOpen->height() + rectEach.height() + nMarginY * 2);
	}

	setFixedSize(rect.width(), rect.height());
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent *pEvent)
{
	while(0 < m_vecDeviceWidget.size())
	{
		DeviceWidget *pDeviceWidget = m_vecDeviceWidget[0];
		pDeviceWidget->close();
	}
	pEvent->accept();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MainWindow::on_widgetDevice_closed(DeviceWidget *pDeviceWidget)
{
	for (std::vector<DeviceWidget*>::iterator itr = m_vecDeviceWidget.begin(); itr != m_vecDeviceWidget.end(); ++itr)
	{
		if (*itr == pDeviceWidget)
		{
			m_vecDeviceWidget.erase(itr);
			break;
		}
	}
	UpdateLayout();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MainWindow::on_widgetDevice_lost(DeviceWidget *pDeviceWidget)
{
	//Show message box.
	QMessageBox msgBox(this);
	msgBox.setWindowTitle("Exception");

	msgBox.setText("Devict lost.");
	msgBox.exec();
	pDeviceWidget->close();
}