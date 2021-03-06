QT += gui gui-private widgets
CONFIG += c++11 link_pkgconfig
PKGCONFIG += qtermwidget5 x11

TARGET = qt

DEFINES += STR_VERSION=\\\"1.0\\\"

# QxtGlobalShortcut
HEADERS += $$files(src/3rdparty/*.h)
SOURCES += $$files(src/3rdparty/*.cpp)

HEADERS += $$files(src/*.h)
SOURCES += $$files(src/*.cpp)

INCLUDEPATH += src src/3rdparty

RESOURCES += src/icons.qrc
FORMS += $$files(src/forms/*.ui)

OTHER_FILES += $$files(desktop/*) AUTHORS COPYING README

unix {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }

    target.path = $$PREFIX/bin

    shortcut.path = $$PREFIX/share/applications
    shortcut.files = desktop/quickterminal.desktop

    INSTALLS += target shortcut
}

