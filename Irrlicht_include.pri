# Это файл включения  в проект путей до библиотеки Irrlicht
# сделан для более быстрой смены этих самых путей на разных машинах
INCLUDEPATH += $$PWD/irrlicht/include
INCLUDEPATH += $$PWD/irrlicht/source/Irrlicht
LIBS += -lirrlicht

macx {
    QMAKE_CXXFLAGS += -framework OpenGL -framework Cocoa -framework Carbon -framework IOKit -arch x86_64
    LIBS += -framework OpenGL -framework Cocoa -framework Carbon -framework IOKit -arch x86_64
}

