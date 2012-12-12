/*!
   @file   RayScene.h
   @author Ben Herila (ben@herila.net)
   @author Evan Wallace (evan.exe@gmail.com)
   @date   Fall 2010
*/

#ifndef RAYSCENE_H
#define RAYSCENE_H

#include "Scene.h"
#include "shape/MathShape.h"
#include "shape/MathCone.h"
#include "shape/MathCube.h"
#include "shape/MathCylinder.h"
#include "shape/MathSphere.h"
#include "SceneviewScene.h"
#include "Canvas2D.h"
#include <QHash>


/*!
  @class RayScene
  @brief Students will implement this class as necessary in the Ray project.
**/

#define RECURN 2

class RayScene : public Scene
{
public:
    RayScene(QList<SimplePrimitive> primList, QList<CS123SceneLightData> lightList, CS123SceneGlobalData gdata);
    virtual ~RayScene();
    void renderGeometry(Camera *camera, Canvas2D* canvas, int width, int height);

private:
    QImage* loadImage(const QString &file);
    QHash<QString,QImage*> m_imagehash;
    BGRA getTexturePixel(const QString filename, double a, double b, int u, int v);

    QList<CS123SceneLightData>* m_lightList;
    QList<SimplePrimitive>* m_shapeList;
    DividedPoint getT(DividedPoint vector);
    DividedPoint getNormal(DividedPoint point);
    Vector4 getColor(DividedPoint point);
    int capAndScale(double finalc);


    REAL m_ka;
    REAL m_kd;
    REAL m_ks;
    REAL m_kt;

    MathCube* m_cube;
    MathCone* m_cone;
    MathSphere* m_sphere;
    MathCylinder* m_cylinder;

    Matrix4x4 m_inv;

};

#endif // RAYSCENE_H
