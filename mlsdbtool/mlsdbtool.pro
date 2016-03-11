TEMPLATE=app
TARGET=geoclue-mlsdb-tool
QT-=gui
QT+=core # concurrent
CONFIG+=c++11
include(../common/common.pri)
SOURCES += main.cpp
target.path=/usr/bin
INSTALLS=target
