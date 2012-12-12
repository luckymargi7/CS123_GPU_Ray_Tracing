# 
# CS123 Final Project Starter Code
# Adapted from starter code graciously provided by CS195-U: 3D Game Engines
#

QT += core gui opengl

TARGET = final
TEMPLATE = app

# If you add your own folders, add them to INCLUDEPATH and DEPENDPATH, e.g.
# INCLUDEPATH += folder1 folder2
# DEPENDPATH += folder1 folder2

SOURCES += main.cpp \
    mainwindow.cpp \
    view.cpp \
    scenegraph/OpenGLScene.cpp \
    scenegraph/Point.cpp \
    scenegraph/RayScene.cpp \
    scenegraph/Scene.cpp \
    scenegraph/SceneviewScene.cpp \
    scenegraph/SelectionRecorder.cpp \
    scenegraph/ShapesScene.cpp \
    math/CS123Matrix.cpp \
    math/CS123Matrix.inl \
    math/CS123Vector.inl \
    lib/CS123XmlSceneParser.cpp \
    ui/mainwindow.cpp \
    ui/view.cpp

HEADERS += mainwindow.h \
    view.h \
    scenegraph/OpenGLScene.h \
    scenegraph/Point.h \
    scenegraph/RayScene.h \
    scenegraph/Scene.h \
    scenegraph/SceneviewScene.h \
    scenegraph/SelectionRecorder.h \
    scenegraph/ShapesScene.h \
    math/CS123Algebra.h \
    math/TA_Tester.h \
    lib/CS123Common.h \
    lib/CS123ISceneParser.h \
    lib/CS123SceneData.h \
    lib/CS123XmlSceneParser.h \
    ui/mainwindow.h \
    ui/view.h

FORMS += mainwindow.ui
