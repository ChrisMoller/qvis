QT += core gui widgets charts

CONFIG += c++11

DEL_DIR = echo

#LIBS += -L/usr/local/lib/apl -lapl
LIBS += -L`apl --show_lib_dir` -lapl

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    xml.cpp

HEADERS += \
    mainwindow.h \
    xml.h \
    XMLtags.def

FORMS +=

#else: unix:!android: target.path = /opt/$${TARGET}/bin
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /usr/local/bin

!isEmpty(target.path): INSTALLS += target

