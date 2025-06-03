#include "mainwindow.h"

#include <QApplication>
#include <QDesktopWidget>

using namespace GenTL;
using namespace StApi;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	StApi::CStApiAutoInit	m_objStApiAutoInit;

	QDesktopWidget * deskWidget = QApplication::desktop();
	QRect deskRect = deskWidget->screenGeometry();

    MainWindow w;
	w.move(deskRect.width() / 4, deskRect.height() / 4);
    w.show();
    return a.exec();
}
