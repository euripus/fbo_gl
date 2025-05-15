TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG(release, debug|release) {
    #This is a release build
    DEFINES += NDEBUG
    QMAKE_CXXFLAGS += -s
} else {
    #This is a debug build
    DEFINES += DEBUG
    TARGET = $$join(TARGET,,,_d)
}

DESTDIR = $$PWD/bin

QMAKE_CXXFLAGS += -std=c++17 -Wno-unused-parameter -Wconversion -Wold-style-cast

INCLUDEPATH += $$PWD/include

LIBS += -L$$PWD/lib

unix:{
    LIBS += -lglfw -lGL -lGLEW
}
win32:{
    LIBS += -lglfw3dll -lopengl32 -lglew32dll
    LIBS += -static-libgcc -static-libstdc++
    LIBS += -static -lpthread
}

SOURCES += \
    src/input/input.cpp \
    src/input/inputglfw.cpp \
    src/main.cpp \
    src/render/renderer.cpp \
    src/render/texture.cpp \
    src/render/vertex_buffer.cpp \
    src/res/imagedata.cpp \
    src/window.cpp

HEADERS += \
    src/input/input.h \
    src/input/inputglfw.h \
    src/input/key_codes.h \
    src/render/AABB.h \
    src/render/render_states.h \
    src/render/renderer.h \
    src/render/texture.h \
    src/render/vertex_buffer.h \
    src/res/imagedata.h \
    src/scene_data.h \
    src/window.h

DISTFILES +=
