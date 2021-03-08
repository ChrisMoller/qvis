QT += core gui widgets charts
#QT += core gui widgets charts 3dcore 3drender 3dinput

CONFIG += c++11
CONFIG += debug_and_release

LIBS += -L`apl --show_lib_dir` -lapl -lreadline  \
        `pkg-config --libs gsl` -lQt5DataVisualization

#LIBS += -L/home/moller/Downloads/apl-1.8.1402/src/.libs \
#       -lQt5DataVisualization -lapl -lreadline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aplexec.cpp \
    chartdata.cpp \
    chartcontrols.cpp \
    chartwindow.cpp \
    complexspinbox.cpp \
    curves.cpp \
    extents.cpp \
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
    complexspinbox.h \
    curves.h \
    extents.h \
    history.h \
    mainwindow.h \
    xml.h \
    XMLtags.def

FORMS +=

PREFIX = /usr/local

include(./Qt-Color-Widgets/color_widgets.pri)

unix {
   target.path = $$PREFIX/bin
   extra.path = $$PREFIX/share/qvis/styles
   extra.files = styles/*.qss
   DEFINES += STYLES="$$extra.path"
}

!isEmpty(target.path): INSTALLS += target extra

DEFINES += PREFIX=$$PREFIX

#CONFIG(debug) {
#  DEFINES += DEBUG
#}

message("prefix = " + $$PREFIX)
