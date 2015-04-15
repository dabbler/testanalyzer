
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
	QT += printsupport
	LIBS += -lQt5Concurrent
}

CONFIG += qt \
        thread

LIBS += \
                -lwfdb

DEFINES += NEW_ANALYZER_STUFF

QMAKE_LIBDIR += .

INCLUDEPATH += .

CONFIG(debug, debug|release) {
        TARGET = photon.debug
        OBJECTS_DIR = tmp/debug
        RCC_DIR = tmp/debug/rcc
}
CONFIG(release, debug|release) {
        TARGET = photon
        OBJECTS_DIR = tmp/release
        RCC_DIR = tmp/release/rcc
        QT -= testlib
}

unix {
#    LIBS += -lcurl
}

# Define how to create version.h
win32 {
	version.target = version.h
	version.commands = C:\\Program Files\\TortoiseSVN\\bin\\SubWCRev.exe ./.. "version.tmpl" "version.h"
	version.depends = FORCE
}

!win32 {
    version.target = version.h
    version.commands = ./subwcrev.sh "version.tmpl" "version.h"
    version.depends = FORCE
}

QMAKE_EXTRA_TARGETS += version

DESTDIR = out
MOC_DIR = tmp/moc
UI_DIR = tmp/ui

INCLUDEPATH += tmp/ui

include(photon.prl)

