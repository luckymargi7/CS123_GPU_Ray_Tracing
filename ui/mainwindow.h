#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class Scene;
class CamtransCamera;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    //! Overridden from QWidget. Handles the window close event.
    virtual void closeEvent(QCloseEvent *e);

private:
    Ui::MainWindow *ui;
    Scene *m_scene;
    CamtransCamera *m_camera;

public slots:
    //! Creates a new image
    void fileNew();

    //! Displays a dialog box to open a 2D image or 3D scene file.
    void fileOpen();

    //! Displays a dialog box to save the current 2D image. Can be extended (for extra credit) to save the current 3D scene.
    void fileSave();

};

#endif // MAINWINDOW_H

