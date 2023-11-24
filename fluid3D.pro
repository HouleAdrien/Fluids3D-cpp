MOC_DIR = ./moc
OBJECTS_DIR = ./obj

INCLUDEPATH += $$PWD

HEADERS = $$files(headers/*.h)
SOURCES = main.cpp \
          $$files(sources/*.cpp)

RESOURCES += \
    shaders.qrc

# Conditional inclusion for different Qt versions
greaterThan(QT_MAJOR_VERSION, 5) {
    QT += core gui opengl openglwidgets
} else {
    QT += widgets core gui opengl
}
