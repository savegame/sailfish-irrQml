TEMPLATE = app
CONFIG += c++11 qt 
debug: CONFIG += qml_debug
QT += quick core


INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/irrlicht/source/qt
INCLUDEPATH += $$PWD/irrlicht/include

include(Irrlicht_include.pri)
#include(irrlicht/irrlicht_static.pri)

TARGET = IrrQuickTest 

DESTDIR = $$OUT_PWD
LIBS += -L$$DESTDIR -L$$DESTDIR/lib
LIBS += -Wl,-rpath,$$DESTDIR/
LIBS += -lirrlicht  -Llib


DEFINES += SRCDIR="\\\"$$PWD/data\\\""
#DEFINES += USE_WIDGET_STYLE
DEFINES += Q_ENABLE_OPENGL_FUNCTIONS_DEBUG
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += _MEDIA_PATH=\\\"$$PWD/irrlicht/media/\\\"
DEFINES += _IRR_COMPILE_WITH_QGLFUNCTIONS_

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    SGIrrNode.h \
    IrrQuickWindow.h \
    ScreenNode.h \
    ScreenShaderCB.h 

SOURCES += \
    main.cpp \
    SGIrrNode.cpp \
    IrrQuickWindow.cpp \
    ScreenNode.cpp \
    ScreenShaderCB.cpp 

android: {
DISTFILES += \
    android/AndroidManifest.xml \
    android/res/values/styles.xml
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}

