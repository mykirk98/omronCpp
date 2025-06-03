#pragma once

#include <QWidget>
#include <qtimer.h>
#include "ui_previewimagewidget.h"

class PreviewImageWidget : public QWidget
{
	Q_OBJECT

public:
	PreviewImageWidget(QWidget *parent = nullptr);
	~PreviewImageWidget();
	QImage m_objQImage;

protected:
	virtual void paintEvent(QPaintEvent *) override;


private:
	Ui::PreviewImageWidget ui;

};
