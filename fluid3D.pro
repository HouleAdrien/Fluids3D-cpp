MOC_DIR = ./moc
OBJECTS_DIR = ./obj

INCLUDEPATH += $$PWD

HEADERS       = $$files(headers/*.h)

SOURCES       = main.cpp \
                $$files(sources/*.cpp)

RESOURCES += \
    shaders.qrc
QT += widgets
QT += core gui opengl openglwidgets

DISTFILES += \
    skybox.frag


