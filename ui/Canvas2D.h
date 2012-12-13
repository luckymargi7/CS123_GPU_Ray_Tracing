/*!
   @file   Canvas2D.h
   @author Ben Herila (ben@herila.net)
   @author Evan Wallace (evan.exe@gmail.com)
   @date   Fall 2010
*/

#ifndef CANVAS2D_H
#define CANVAS2D_H

#include "ui/SupportCanvas2D.h"
#include "math/CS123Algebra.h"
#include "lib/CS123SceneData.h"

class Scene;

class Camera;

/*!
   @class Canvas2D

   @brief 2D canvas that students will implement in the Brush and Filter assignments.
   The same canvas will be used to display raytraced images in the Intersect and Ray
   assignments.

**/
class Canvas2D : public SupportCanvas2D
{
    Q_OBJECT
public:
    Canvas2D();
    virtual ~Canvas2D();

    void setScene(Scene *scene);

    //! UI will call this from the button on the "Ray" dock
    void renderImage(Camera *camera, int width, int height);
    // For multithreading
    //void renderImage(Camera *camera, int rowstart, int rowend, int colstart, int colend, int width, int height);

    //! This will be called when the settings have changed
    virtual void settingsChanged();

public slots:
    //! UI will call this from the button on the "Ray" dock
    void cancelRender();


protected:
    virtual void paintEvent(QPaintEvent *);  /// overridden from SupportCanvas2D
    virtual void mouseDown(int x, int y);    /// called when the left mouse button is pressed down on the canvas
    virtual void mouseDragged(int x, int y); /// called when the left mouse button is dragged on the canvas
    virtual void mouseUp(int x, int y);      /// called when the left mouse button is released

    //! This will be called when the size of the canvas has been changed
    virtual void notifySizeChanged(int w, int h);



private:
    Scene *m_scene;

    //For intersect
    /*
    Vector4 calcPfilm(double x, double y, double x_max, double y_max);
    Vector4 calcDirection(Camera* camera, int colcounter, int rowcounter, int width, int height);
    Vector4 calcPworld (double x, double y, double x_max, double y_max, Camera* camera);
    void calcIllumination(Shape *shape, CS123SceneMaterial& material, Vector4& normal, Vector4& pEye, Vector4& intersectPt, CS123SceneColor& color);
    CS123SceneColor trace(Vector4& pEye, Vector4& direction, int depth);
    REAL get_first_intersection(Vector4& pEye, Vector4& direction, Shape **shape);
    REAL is_blocked_by_obj(Vector4& pEye, Vector4& direction, Vector4& lightPos, Shape *curr_shape);
    Vector4 getNormalWorld(Shape *shape, Vector4 intersectPt);
    void capColor(CS123SceneColor& color);
    Vector4 reflectVector(Vector4 ray, Vector4 normal);
    Vector2 getTextureCoord(Shape *shape, CS123SceneMaterial material, int textureWidth, int textureHeight, Vector4 intersectPt);
    */
};

#endif // CANVAS2D_H
