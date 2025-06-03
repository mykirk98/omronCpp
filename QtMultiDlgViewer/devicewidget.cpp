#include "devicewidget.h"
#include "mainwindow.h"
#include "previewimagewidget.h"
#include "settingdevicewidget.h"
using namespace GenTL;
using namespace StApi;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
DeviceWidget::DeviceWidget(StApi::IStDeviceReleasable *pIStDeviceRelesable, QWidget *parent)
	: QWidget(parent),
	m_labelFPS(NULL), m_labelFrameCount(NULL), m_labelDropCount(NULL), m_qTimer(NULL),
	m_pIStPixelFormatConverter(CreateIStConverter(StConverterType_PixelFormat)),
	m_isStreamingStarted(false), m_isDeviceLostDetected(false), m_isNewImage(false), m_nDroppedFrameCount(0)
{
	ui.setupUi(this);
	m_labelFPS = new QLabel();
	m_labelFrameCount = new QLabel();
	m_labelDropCount = new QLabel();
	m_qTimer = new QTimer(this);
	m_labelFPS->setAlignment(Qt::Alignment::enum_type::AlignVCenter | Qt::Alignment::enum_type::AlignHCenter);
	m_labelFrameCount->setAlignment(Qt::Alignment::enum_type::AlignVCenter | Qt::Alignment::enum_type::AlignHCenter);
	m_labelDropCount->setAlignment(Qt::Alignment::enum_type::AlignVCenter | Qt::Alignment::enum_type::AlignHCenter);
	ui.statusBar->addWidget(m_labelFPS, 1);
	ui.statusBar->addWidget(m_labelFrameCount, 1);
	ui.statusBar->addWidget(m_labelDropCount, 1);

	connect(this, &DeviceWidget::closed, dynamic_cast<MainWindow*>(parent), &MainWindow::on_widgetDevice_closed);
	connect(this, &DeviceWidget::lost, dynamic_cast<MainWindow*>(parent), &MainWindow::on_widgetDevice_lost);

	OpenDevice(pIStDeviceRelesable);
	InitNodeMap();

	m_qTimer->setInterval(33);
	connect(m_qTimer, &QTimer::timeout, this, &DeviceWidget::on_timeout);
	m_qTimer->start();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
DeviceWidget::~DeviceWidget()
{
	m_qTimer->stop();
	delete m_qTimer;
	delete m_labelDropCount;
	delete m_labelFrameCount;
	delete m_labelFPS;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::InitNodeMap()
{
	//m_nTimerID = SetTimer(m_nTimerID, 33, NULL);
	ui.textTitle->setText(m_strTitle);

	UpdateButtonState();

	GenApi::CNodeMapPtr pINodeMapRemoteDevice(m_pIStDevice->GetRemoteIStPort()->GetINodeMap());

	try
	{
		const char *pszNames[] =
		{
			"TestImageType",
			"PixelFormat",
			"Width",
			"Height",
			"AcquisitionFrameRate",
			"ExposureMode",
			"ExposureTime",
			"GainSelector",
			"Gain",
			"BalanceRatioSelector",
			"BalanceRatio",
			"BalanceWhiteAuto",
			"TriggerSelector",
			"TriggerMode",
			"TriggerSource",
			"TriggerSoftware",
		};
		for (size_t i = 0; i < sizeof(pszNames) / sizeof(pszNames[0]); ++i)
		{
			GenApi::CNodePtr pINode(pINodeMapRemoteDevice->GetNode(pszNames[i]));
			if (pINode.IsValid())
			{
				ui.widgetSetting->RegisterINode(pINode, "Remote Device");
			}
		}
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(this, e);
	}

	UpdateStatusBar();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::setVisible(bool visible)
{
	if(visible)
	{
		QWidget::setVisible(visible);
		ui.widgetPreview->repaint();
	}

}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::CloseDevice()
{
	m_qTimer->stop();
	if (m_pIStDevice.IsValid())
	{
		ui.widgetSetting->terminate();
		if (m_pIStDataStream.IsValid())
		{
			StopStreaming();
			m_pIStDataStream.Reset(NULL);
		}
		m_pIStRegisteredCallbackDeviceLost.Reset(NULL);
		m_pIStDevice.Reset(NULL);
		m_isStreamingStarted = false;
		m_isDeviceLostDetected = false;
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::OpenDevice(StApi::IStDeviceReleasable *pIStDeviceReleasable)
{
	m_pIStDevice = pIStDeviceReleasable;
	if (!m_pIStDevice.IsValid())
	{
		return;
	}

	try
	{
		GenApi::CNodeMapPtr pINodeMapRemoteDevice(m_pIStDevice->GetRemoteIStPort()->GetINodeMap());

		//Register device lost event.
		GenApi::CNodeMapPtr pINodeMapLocalDevice(m_pIStDevice->GetLocalIStPort()->GetINodeMap());
		if (pINodeMapLocalDevice)
		{
			GenApi::CNodePtr pINodeEventDeviceLost(pINodeMapLocalDevice->GetNode("EventDeviceLost"));
			if (pINodeEventDeviceLost.IsValid())
			{
				m_pIStRegisteredCallbackDeviceLost.Reset(StApi::RegisterCallback(pINodeEventDeviceLost, *this, &DeviceWidget::OnDeviceLost, (void*)NULL, GenApi::cbPostOutsideLock));
			}
		}

		//Start Event Acquisition Thread
		m_pIStDevice->StartEventAcquisitionThread();

		//Set title.
		m_strTitle = m_pIStDevice->GetIStDeviceInfo()->GetDisplayName();
		GenICam::gcstring strUserName;
		try
		{
			strUserName = m_pIStDevice->GetIStDeviceInfo()->GetUserDefinedName();
		}
		catch (...)
		{
			strUserName = "";
		}
		if (0 < strUserName.length())
		{
			m_strTitle.append("[");
			m_strTitle.append(strUserName);
			m_strTitle.append("]");
		}

		//Create object and get IStDataStream pointer.
		m_pIStDataStream.Reset(m_pIStDevice->CreateIStDataStream(0));
		if (m_pIStDataStream.IsValid())
		{
			//Register callback function to receive images.
			RegisterCallback(m_pIStDataStream, *this, &DeviceWidget::OnStCallback, (void*)NULL);
		}
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(this, e);
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::OnStCallback(StApi::IStCallbackParamBase *pIStCallbackParamBase, void * /*pvContext*/)
{
	try
	{
		StApi::EStCallbackType_t eStCallbackType = pIStCallbackParamBase->GetCallbackType();
		if (eStCallbackType == StApi::StCallbackType_GenTLEvent_DataStreamNewBuffer)
		{

			StApi::IStCallbackParamGenTLEventNewBuffer *pIStCallbackParamGenTLEventNewBuffer = dynamic_cast<StApi::IStCallbackParamGenTLEventNewBuffer*>(pIStCallbackParamBase);
			StApi::IStDataStream *pIStDataStream = pIStCallbackParamGenTLEventNewBuffer->GetIStDataStream();

			// Wait until the data is acquired.
			// If the data has been received, IStStreamBufferReleasable interface pointer is retrieved.
			IStStreamBufferReleasable *pIStStreamBufferReleasable = pIStDataStream->RetrieveBuffer(0);
			if (pIStStreamBufferReleasable == NULL) return;
			if (pIStStreamBufferReleasable->GetIStStreamBufferInfo()->IsIncomplete())
			{
				pIStStreamBufferReleasable->Release();
				return;
			}

			GenApi::AutoLock objAutoLock(m_objLockForStreamingBuffer);

			if (m_pIStStreamBuffer.IsValid())
			{
				m_nDroppedFrameCount += pIStStreamBufferReleasable->GetIStStreamBufferInfo()->GetFrameID() - m_pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID() - 1;
			}
			m_pIStStreamBuffer.Reset(pIStStreamBufferReleasable);

			m_isNewImage = true;
		}
		else if (eStCallbackType == StApi::StCallbackType_GenTLEvent_DataStreamError)
		{
#ifdef Q_OS_WIN32
			StApi::IStCallbackParamGenTLEventErrorDS *pIStCallbackParamGenTLEventErrorDS = dynamic_cast<StApi::IStCallbackParamGenTLEventErrorDS*>(pIStCallbackParamBase);
			OutputDebugString(GCSTRING_2_LPCTSTR(pIStCallbackParamGenTLEventErrorDS->GetDescription()));
#else
#endif
		}
	}
	catch (...)
	{
		//TODO:
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::OnDeviceLost(GenApi::INode *pINode, void*)
{

	if (GenApi::IsAvailable(pINode))
	{
		if (m_pIStDevice->IsDeviceLost())
		{
			m_isDeviceLostDetected = true;
			emit lost(this);
		}
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::StartStreaming(uint64_t nFrameCount)
{
	if (!m_pIStDevice.IsValid()) return;
	if (!m_pIStDataStream.IsValid()) return;
	try
	{
		m_nDroppedFrameCount = 0;

		// Start the image acquisition of the host side.
		m_pIStDataStream->StartAcquisition(nFrameCount);

		// Start the image acquisition of the camera side.
		m_pIStDevice->AcquisitionStart();

		m_isStreamingStarted = true;
		UpdateButtonState();
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(this, e);
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::StopStreaming()
{
	if (!m_pIStDevice.IsValid()) return;
	if (!m_pIStDataStream.IsValid()) return;
	try
	{
		// Stop the image acquisition of the camera side.
		m_pIStDevice->AcquisitionStop();

		// Stop the image acquisition of the host side.
		m_pIStDataStream->StopAcquisition();

		m_isStreamingStarted = false;
		UpdateButtonState();

		ConvertImageToVisibleFormat();
	}
	catch (const GenICam::GenericException &e)
	{
		if (!m_isDeviceLostDetected)
		{
			OnException(this, e);
		}
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::ConvertImageToVisibleFormat()
{
	GenApi::AutoLock objAutoLock(m_objLockForStreamingBuffer);
	if (m_pIStStreamBuffer.IsValid())
	{
		if (!m_pIStImageBuffer.IsValid())
		{
			m_pIStImageBuffer = CreateIStImageBuffer();
		}

		IStImage *pIStImage = m_pIStStreamBuffer->GetIStImage();
		const EStPixelFormatNamingConvention_t ePFNCSrc = pIStImage->GetImagePixelFormat();
		const IStPixelFormatInfo *pIStPixelFormatInfo = GetIStPixelFormatInfo(ePFNCSrc);

		const EStPixelFormatNamingConvention_t ePFNCDest = pIStPixelFormatInfo->IsMono() ? StPFNC_Mono8 : StPFNC_BGRa8;

		m_pIStPixelFormatConverter->SetDestinationPixelFormat(ePFNCDest);
		m_pIStPixelFormatConverter->Convert(pIStImage, m_pIStImageBuffer);

		m_pIStStreamBuffer.Reset(NULL);
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool DeviceWidget::GetBitmapImage(QImage &objQImage)
{
	ConvertImageToVisibleFormat();
	if (!m_pIStImageBuffer.IsValid()) return(false);
	{
		IStImage *pIStImage = m_pIStImageBuffer->GetIStImage();
		const int nImageWidth = (int)pIStImage->GetImageWidth();
		const int nImageHeight = (int)pIStImage->GetImageHeight();
		const int nImageLinePitchSrc = (int)pIStImage->GetImageLinePitch();
		const EStPixelFormatNamingConvention_t ePFNC = pIStImage->GetImagePixelFormat();
		const QImage::Format qFormat = (ePFNC == StPFNC_Mono8) ? QImage::Format_Grayscale8 : QImage::Format_RGB32;
		if ((objQImage.width() != nImageWidth) || (objQImage.height() != nImageHeight) || (objQImage.format() != qFormat))
		{
			objQImage = QImage(nImageWidth, nImageHeight, qFormat);
		}
		uint8_t *pbyteBufferSrc = (uint8_t*)pIStImage->GetImageBuffer();

		// Copy the image.
		uint8_t *pbyteBufferDest = objQImage.bits();
		const int nImageLinePitchDest = objQImage.bytesPerLine();
		const size_t nEachLineDataSize = (nImageLinePitchSrc < nImageLinePitchDest) ? nImageLinePitchSrc : nImageLinePitchDest;
		for (int y = 0; y < nImageHeight; ++y)
		{
			memcpy(pbyteBufferDest, pbyteBufferSrc, nEachLineDataSize);
			pbyteBufferDest += nImageLinePitchDest;
			pbyteBufferSrc += nImageLinePitchSrc;
		}
	}
	return(true);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool DeviceWidget::GetLatestImage(StApi::IStImageBuffer *pIStImageBuffer)
{
	ConvertImageToVisibleFormat();
	if (!m_pIStImageBuffer.IsValid()) return(false);
	pIStImageBuffer->CopyImage(m_pIStImageBuffer->GetIStImage());
	return(true);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::on_btnAcqStart_clicked()
{
	StopStreaming();
	StartStreaming();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::on_btnSnap_clicked()
{
	StopStreaming();
	StartStreaming(1);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::on_btnAcqStop_clicked()
{
	StopStreaming();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::on_btnClose_clicked()
{
	close();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::closeEvent(QCloseEvent *)
{
	CloseDevice();
	emit closed(this);
	deleteLater();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::UpdateButtonState()
{
	ui.btnAcqStart->setEnabled(!m_isStreamingStarted);
	ui.btnAcqStop->setEnabled(m_isStreamingStarted);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::on_timeout()
{
	if (m_isNewImage)
	{
		if (GetBitmapImage(ui.widgetPreview->m_objQImage))
		{
			m_isNewImage = false;
			ui.widgetPreview->repaint();
		}
	}
	UpdateStatusBar();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DeviceWidget::UpdateStatusBar()
{
	try
	{
		if (m_pIStDataStream.IsValid())
		{
			const double dblFPS = m_pIStDataStream->GetCurrentFPS();
			const uint64_t nFrameCount = m_pIStDataStream->GetIStDataStreamInfo()->GetNumDelivered();

			QString strText;
			strText.setNum(dblFPS, 'f', 2);
			strText.append(" FPS");
			m_labelFPS->setText(strText);

			QString strNum;
			strText = "Rcv:";
			strNum.setNum(nFrameCount);
			strText.append(strNum);
			m_labelFrameCount->setText(strText);

			strText = "Drop:";
			strNum.setNum(m_nDroppedFrameCount);
			strText.append(strNum);
			m_labelDropCount->setText(strText);
		}
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(this, e);
	}

}