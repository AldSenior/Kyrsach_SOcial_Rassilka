QT += core gui widgets network sql
CONFIG += c++17

TARGET = social_media_service
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    database.cpp \
    scheduler.cpp \
    socialmedia.cpp \
    logger.cpp \
    settingsdialog.cpp \
    targetdialog.cpp

HEADERS += \
    mainwindow.h \
    database.h \
    scheduler.h \
    socialmedia.h \
    logger.h \
    settingsdialog.h \
    targetdialog.h \
    posttarget.h

RESOURCES +=

# Указываем путь для выходных файлов
DESTDIR = $$PWD/build
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui
