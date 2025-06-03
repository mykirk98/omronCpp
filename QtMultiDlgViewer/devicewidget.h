#pragma once

#include <QMainWindow>
#include <StApi_TL.h>
#include <StApi_IP.h>
#include <StApi_GUI.h>
#include "ui_devicewidget.h"

class DeviceWidget : public QWidget
{
	Q_OBJECT

public:
	DeviceWidget(StApi::IStDeviceReleasable *, QWidget *parent = nullptr);
	~DeviceWidget();

signals:
	void closed(DeviceWidget *);
	void lost(DeviceWidget *);

protected:
	void closeEvent(QCloseEvent *);

public slots:
	void setVisible(bool visible) override;

private slots:
	//Qt slots.
	//void on_btnOpen_close();
	void on_btnAcqStart_clicked();
	void on_btnSnap_clicked();
	void on_btnAcqStop_clicked();
	void on_btnClose_clicked();
	void on_timeout();
private:
	Ui::DeviceWidget ui;
	QLabel *m_labelFPS;
	QLabel *m_labelFrameCount;
	QLabel *m_labelDropCount;
	QTimer *m_qTimer;
private:
	StApi::CIStImageBufferPtr m_pIStImageBuffer;
	StApi::CIStPixelFormatConverterPtr m_pIStPixelFormatConverter;
	StApi::CIStDevicePtr m_pIStDevice;
	StApi::CIStRegisteredCallbackPtr m_pIStRegisteredCallbackDeviceLost;
	StApi::CIStDataStreamPtr m_pIStDataStream;
	StApi::CIStStreamBufferPtr m_pIStStreamBuffer;
	
	GenApi::CLock m_objLockForStreamingBuffer;
	QString m_strTitle;

	bool m_isStreamingStarted;
	bool m_isDeviceLostDetected;
	bool m_isNewImage;
	uint64_t m_nDroppedFrameCount;
	void CloseDevice();
	void OpenDevice(StApi::IStDeviceReleasable *);
	void InitNodeMap();
	void StartStreaming(uint64_t nFrameCount = GENTL_INFINITE);
	void StopStreaming();
	void OnDeviceLost(GenApi::INode *pINode, void*);
	void OnStCallback(StApi::IStCallbackParamBase *pIStCallbackParamBase, void * /*pvContext*/);
	void ConvertImageToVisibleFormat();
	bool GetBitmapImage(QImage &objQImage);
	bool GetLatestImage(StApi::IStImageBuffer *);
	void UpdateButtonState();
	void UpdateStatusBar();
};
