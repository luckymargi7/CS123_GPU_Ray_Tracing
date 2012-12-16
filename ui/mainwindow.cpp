#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lib/CS123XmlSceneParser.h"
#include <QSettings>
#include <math.h>
#include <QFileDialog>
#include <QMessageBox>
#include <assert.h>
#include <QRunnable>
#include <time.h>
#include "camera/CamtransCamera.h"
#include "scenegraph/Scene.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_camera = new CamtransCamera();

    resize(1000,630);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_camera;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Stop any raytracing, otherwise the thread will hang around until done
    ui->canvas2D->cancelRender();

    QMainWindow::closeEvent(event);
}

void MainWindow::fileNew()
{
    ui->canvas2D->newImage();
}

void MainWindow::fileOpen()
{
    QString file = QFileDialog::getOpenFileName(this, QString(), "/course/cs123/data/", "All files (*.*)");
    if (!file.isNull())
    {
        if (file.endsWith(".xml"))
        {
            CS123XmlSceneParser parser(file.toAscii().data());
            if (parser.parse())
            {
                if (m_scene) {
                    delete m_scene;
                    m_scene = NULL;
                }

                m_scene = new Scene();
                Scene::parse(m_scene, &parser);

                // Set the camera for the new scene
                CS123SceneCameraData camera;
                if (parser.getCameraData(camera))
                {
                    camera.pos.data[3] = 1;
                    camera.look.data[3] = 0;
                    camera.up.data[3] = 0;

                    m_camera->orientLook(camera.pos, camera.look, camera.up);
                    m_camera->setHeightAngle(camera.heightAngle);
                }

                //render image
                QApplication::processEvents();

                ui->canvas2D->setScene(m_scene);

                // Render the image
                QSize activeTabSize = ui->canvas2D->size();

                ui->canvas2D->renderImage(m_camera, activeTabSize.width(), activeTabSize.height());
                ui->canvas2D->repaint();
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

void MainWindow::fileSave()
{
    ui->canvas2D->saveImage();
}
