#include <QtGui/QApplication>
#include "ui/mainwindow.h"
#include "lib/CS123XmlSceneParser.h"
#include "scenegraph/Scene.h"

int main(int argc, char *argv[])
{

    //This is parsing the file
    QString file = "/course/cs123/data/scenes/intersect/ctc_isect.xml";
    CS123XmlSceneParser parser(file.toAscii().data());
    if(parser.parse()){
        Scene *scene = new Scene;
        Scene::parse(scene, &parser);
        //then we want to do something like "ui->canvas3D->setScene(scene);" to actually set/display the scene
        //at this point "scene" has a prim list, a light list, and the global data.  The definition of
        //a simplePrimitive is found in CS123SceneData.h.
    }


    //What they gave us is below here
    QApplication a(argc, argv);
    MainWindow w;

    bool startFullscreen = false;

    w.show();

    if (startFullscreen) {
        // We cannot use w.showFullscreen() here because on Linux that creates the
        // window behind all other windows, so we have to set it to fullscreen after
        // it has been shown.
        w.setWindowState(w.windowState() | Qt::WindowFullScreen);
    }

    return a.exec();
}

