QT += core gui widgets

# Qt 6 用 openglwidgets，Qt 5 用 opengl
greaterThan(QT_MAJOR_VERSION, 5): QT += openglwidgets
else: QT += opengl


CONFIG += c++17 console

SOURCES += \
    glwidget.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    glwidget.h \
    mainwindow.h


LIBS += -lopengl32
