QT += core gui widgets charts

CONFIG += c++11

LIBS += -L`apl --show_lib_dir` -lapl -lreadline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aplexec.cpp \
    chartdata.cpp \
    chartcontrols.cpp \
    chartwindow.cpp \
    curves.cpp \
    history.cpp \
    main.cpp \
    mainwindow.cpp \
    main_curves.cpp \
    main_parameters.cpp \
    xml.cpp

HEADERS += \
    aplexec.h \
    chartdata.h \
    chartcontrols.h \
    chartwindow.h \
    curves.h \
    history.h \
    mainwindow.h \
    xml.h \
    XMLtags.def

FORMS +=

PREFIX = /usr/local

include(./Qt-Color-Widgets/color_widgets.pri)

unix {
   target.path = $$PREFIX/bin
   extra.path = $$PREFIX/share/qvis
   extra.files = styles/*.qss
}

!isEmpty(target.path): INSTALLS += target extra

DEFINES += PREFIX=$$PREFIX

message("prefix = " + $$PREFIX)
