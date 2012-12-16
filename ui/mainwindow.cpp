#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "camera/CamtransCamera.h"
#include "CS123XmlSceneParser.h"
#include "Scene.h"
#include <QSettings>
#include <math.h>
#include <QFileDialog>
#include <QMessageBox>
#include <assert.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Resize the window because the window is huge since all docks were visible
    resize(630, 630);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e); // allow the superclass to handle this for the most part...

    switch (e->type()) {

    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;

    default:
        break;

    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Stop any raytracing, otherwise the thread will hang around until done
    ui->canvas2D->cancelRender();

    QMainWindow::closeEvent(event);
}

void MainWindow::fileOpen()
{
    QStringList fileNames;
    QFileDialog dialog(this, "", "/course/cs123/data/");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Scene files (*.xml)"));
    if (dialog.exec())
        fileNames = dialog.selectedFiles();

    QString file = (fileNames.size() > 0) ? fileNames[0] : NULL;

    if (!file.isNull())
    {
        if (file.endsWith(".xml"))
        {
            CS123XmlSceneParser parser(file.toAscii().data());
            if (parser.parse())
            {
                Scene *scene = new Scene;
                Scene::parse(scene, &parser);

                // Set the camera for the new scene
                CS123SceneCameraData camera;
                if (parser.getCameraData(camera))
                {
                    camera.pos.data[3] = 1;
                    camera.look.data[3] = 0;
                    camera.up.data[3] = 0;

                    CamtransCamera *cam = new CamtransCamera();
                    cam->orientLook(camera.pos, camera.look, camera.up);
                    cam->setHeightAngle(camera.heightAngle);
                }

            }
            else
            {
                QMessageBox::critical(this, "Error", "Could not load scene \"" + file + "\"");
            }
        }
        else
        {
            if (!ui->canvas2D->loadImage(file))
            {
                QMessageBox::critical(this, "Error", "Could not load image \"" + file + "\"");
            }

        }
    }
}
