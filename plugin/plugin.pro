TARGET = geoclue-mlsdb
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app

target.path = /usr/libexec

QT = core dbus network

CONFIG += link_pkgconfig
PKGCONFIG += qofono-qt5 qofonoext connman-qt5 libsailfishkeyprovider

LIBS += -lrt

packagesExist(qt5-boostable) {
    DEFINES += HAS_BOOSTER
    PKGCONFIG += qt5-boostable
} else {
    warning("qt5-boostable not available; startup times will be slower")
}

# not installed
dbus_geoclue.files = \
    org.freedesktop.Geoclue.xml \
    org.freedesktop.Geoclue.Position.xml
dbus_geoclue.header_flags = "-l MlsdbProvider -i mlsdbprovider.h"
dbus_geoclue.source_flags = "-l MlsdbProvider"

DBUS_ADAPTORS = \
    dbus_geoclue

# installed
session_dbus_service.files = org.freedesktop.Geoclue.Providers.Mlsdb.service
session_dbus_service.path = /usr/share/dbus-1/services
geoclue_provider.files = geoclue-mlsdb.provider
geoclue_provider.path = /usr/share/geoclue-providers

include (../common/common.pri)
HEADERS += \
    mlsdbprovider.h \
    mlsdbonlinelocator.h \
    locationtypes.h

SOURCES += \
    main.cpp \
    mlsdbprovider.cpp \
    mlsdbonlinelocator.cpp

OTHER_FILES = \
    $${dbus_geoclue.files} \
    $${session_dbus_service.files} \
    $${geoclue_provider.files}

INSTALLS += target session_dbus_service geoclue_provider
