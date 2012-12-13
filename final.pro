# 
# CS123 Final Project Starter Code
# Adapted from starter code graciously provided by CS195-U: 3D Game Engines
#

QT += opengl xml
#QT += core gui opengl xml

TARGET = final
TEMPLATE = app

# If you add your own folders, add them to INCLUDEPATH and DEPENDPATH, e.g.
# INCLUDEPATH += folder1 folder2
# DEPENDPATH += folder1 folder2

SOURCES += main.cpp \
    scenegraph/Point.cpp \
    scenegraph/Scene.cpp \
    math/CS123Matrix.cpp \
    math/CS123Matrix.inl \
    math/CS123Vector.inl \
    lib/CS123XmlSceneParser.cpp \
    ui/mainwindow.cpp \
    ui/view.cpp \
    ui/Canvas2D.cpp \
    ui/SupportCanvas2D.cpp \
    camera/CamtransCamera.cpp

HEADERS += \
    scenegraph/Point.h \
    scenegraph/Scene.h \
    math/CS123Algebra.h \
    math/TA_Tester.h \
    lib/CS123Common.h \
    lib/CS123ISceneParser.h \
    lib/CS123SceneData.h \
    lib/CS123XmlSceneParser.h \
    ui/mainwindow.h \
    ui/view.h \
    ui/Canvas2D.h \
    ui/SupportCanvas2D.h \
    camera/CamtransCamera.h \
    camera/Camera.h

FORMS += mainwindow.ui
INCLUDEPATH += brush camera lib math scenegraph ui
DEPENDPATH += brush camera lib math scenegraph ui
DEFINES += TIXML_USE_STL
OTHER_FILES +=

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS_WARN_ON -= -Wall
QMAKE_CXXFLAGS_WARN_ON += -Waddress -Warray-bounds -Wc++0x-compat -Wchar-subscripts -Wformat\
                          -Wmain -Wmissing-braces -Wparentheses -Wreorder -Wreturn-type \
                          -Wsequence-point -Wsign-compare -Wstrict-aliasing -Wstrict-overflow=1 -Wswitch \
                          -Wtrigraphs -Wuninitialized -Wunused-label -Wunused-variable \
                          -Wvolatile-register-var -Wno-extra
# QMAKE_CXX_FLAGS_WARN_ON += -Wunknown-pragmas -Wunused-function -Wmain

macx {
    QMAKE_CXXFLAGS_WARN_ON -= -Warray-bounds -Wc++0x-compat
}
