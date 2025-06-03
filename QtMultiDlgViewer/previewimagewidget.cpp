#include "previewimagewidget.h"
#include <QPainter>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PreviewImageWidget::PreviewImageWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PreviewImageWidget::~PreviewImageWidget()
{
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void PreviewImageWidget::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	const QSize sizeTarget(width(), height());
	QRect rectBack;
	if (m_objQImage.isNull())
	{
		rectBack = QRect(0, 0, sizeTarget.width(), sizeTarget.height());
	}
	else
	{
		const QRect rectImage = QRect(0, 0, m_objQImage.width(), m_objQImage.height());
		const double dblAspectRatioOfImage = rectImage.width() / (double)rectImage.height();
		const double dblAspectRatioOfTarget = sizeTarget.width() / (double)sizeTarget.height();
		
		
		if (dblAspectRatioOfImage < dblAspectRatioOfTarget)
		{
			const int nTargetX = (int)(sizeTarget.height() * dblAspectRatioOfImage + 0.5);
			QRect rectTarget(0, 0, nTargetX, sizeTarget.height());
			painter.drawImage(rectTarget, m_objQImage, rectImage);

			rectBack = QRect(nTargetX, 0, sizeTarget.width(), sizeTarget.height());
		}
		else
		{
			const int nTargetY = (int)(sizeTarget.width() / dblAspectRatioOfImage + 0.5);
			QRect rectTarget(0, 0, sizeTarget.width(), nTargetY);
			painter.drawImage(rectTarget, m_objQImage, rectImage);

			rectBack = QRect(0, nTargetY, sizeTarget.width(), sizeTarget.height());
		}
	}
	painter.fillRect(rectBack, Qt::BrushStyle::DiagCrossPattern);

}
