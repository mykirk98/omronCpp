QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    devicewidget.cpp \
    main.cpp \
    mainwindow.cpp \
    previewimagewidget.cpp \
    settingdevicewidget.cpp

HEADERS += \
    devicewidget.h \
    mainwindow.h \
    previewimagewidget.h \
    settingdevicewidget.h

FORMS += \
    devicewidget.ui \
    mainwindow.ui \
    previewimagewidget.ui \
    settingdevicewidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# SentechSDK
win32 {
    INCLUDEPATH += "$$(STAPI_ROOT_PATH)/include"
    contains(QMAKE_TARGET.arch, x86_64) {
        LIBS += -L"$$(STAPI_ROOT_PATH)/lib/x64"
    } else {
        LIBS += -L"$$(STAPI_ROOT_PATH)/lib/Win32"
    }
}
unix:!macx:{
    INCLUDEPATH += $(STAPI_ROOT_PATH)/include/GenICam \
        $(STAPI_ROOT_PATH)/include/StApi
    LIBS += -L$(STAPI_ROOT_PATH)/lib/GenICam \
        -lGCBase \
        -lGenApi \
        -llog4cpp \
        -L$(STAPI_ROOT_PATH)/lib \
        -lStApi_TL -lStApi_IP -lStApi_GUI_qt -lturbojpeg -lpng16
}
unix:macx:{
    FRAMEWORKPATH = /Library/Frameworks/SentechSDK.framework
    INCLUDEPATH += $${FRAMEWORKPATH}/Headers \
        $${FRAMEWORKPATH}/Headers/GenICam \
        $${FRAMEWORKPATH}/Headers/StApi
    LIBS += -L$${FRAMEWORKPATH}/Libraries/GenICam -lGCBase -lGenApi \
          -L$${FRAMEWORKPATH}/Libraries -lStApi_TL -lStApi_IP -lStApi_GUI_qt
}