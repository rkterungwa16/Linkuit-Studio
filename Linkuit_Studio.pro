QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

SOURCES += \
   $${PWD}/main.cpp

RESOURCES += \
    Gui/resources.qrc

RC_ICONS = images/linkuit_icon6.ico \
           images/icons/linkuit-studio-file.ico

include(Common.pri)
include(QtAwesome/QtAwesome.pri)
