#-------------------------------------------------
#
# Project created by QtCreator 2018-01-31T18:19:18
#
#-------------------------------------------------

QT       += widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 1bitpaint
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    staticdata.cpp \
    canvas.cpp

HEADERS  += mainwindow.h \
    staticdata.h \
    canvas.h

FORMS    += mainwindow.ui

message($$QMAKESPEC)
linux-oe-g++ {
    message("Is arm")
    LIBS += -lqsgepaper
}

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
