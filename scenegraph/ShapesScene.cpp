#include "ShapesScene.h"
#include "Settings.h"
#include "Camera.h"

#include "shape/Shape.h"
#include "shape/Cube.h"
#include "shape/Cylinder.h"
#include "shape/Cone.h"
#include "shape/Sphere.h"
#include "shape/Toroid.h"

#include <qgl.h>
#include <SupportCanvas3D.h>
#include <iostream>


Vector4 lightDirection = Vector4(1, -1, -1, 0).getNormalized();

ShapesScene::ShapesScene()
{
    // Use a shiny orange material
    memset(&m_material, 0, sizeof(m_material));
    m_material.cAmbient.r = 0.2f;
    m_material.cAmbient.g = 0.1f;
    m_material.cDiffuse.r = 1;
    m_material.cDiffuse.g = 0.5f;
    m_material.cSpecular.r = m_material.cSpecular.g = m_material.cSpecular.b = 1;
    m_material.shininess = 64;

    // Use a white directional light from the upper left corner
    memset(&m_light, 0, sizeof(m_light));
    m_light.type = LIGHT_DIRECTIONAL;
    m_light.dir = lightDirection;
    m_light.color.r = m_light.color.g = m_light.color.b = 1;
    m_light.id = 0;

    //TODO: [SHAPES] Allocate any additional memory you need...
    m_shape = new Cube(settings.shapeParameter1,settings.shapeParameter2);//

}

ShapesScene::~ShapesScene()
{
    //TODO: [SHAPES] Don't leak memory!
   delete m_shape;//

}

void ShapesScene::setLights(const Camera *follow)
{
    // YOU DON'T NEED TO TOUCH THIS METHOD,
    // unless you want to do fancy lighting...

    // Disable all OpenGL lights so the lights from the previous scene
    // don't appear in the next one.  There are 8 lights in OpenGL,
    // GL_LIGHT0 to GL_LIGHT7, and each GL_LIGHT* is one greater than
    // the previous one.  This means the nth light is GL_LIGHT0 + n.
    for (int id = 0; id < NUM_OPENGL_LIGHTS; ++id)
        glDisable(GL_LIGHT0 + id);

    m_light.dir = lightDirection * follow->getModelviewMatrix().getInverse().getTranspose();

    // Load one light in the upper left (subclasses will have to load lights
    // for sceneview anyway, so they will overwrite this light).
    setLight(m_light);
}

void ShapesScene::renderGeometry(bool useMaterials)
{
    //Decide if you need a new shape (different tesselation values or different shape)
    int oldShape = m_shape->getShapeType();
    int oldVert = m_shape->getVertNum();
    int oldHoriz = m_shape->getHorizNum();
    int newParam1 = settings.shapeParameter1;
    int newParam2 = settings.shapeParameter2;

    if(oldShape != settings.shapeType || oldVert != newParam1 || oldHoriz != newParam2){
        //note that I don't put delete m_shape here because sometimes you want to keep it.  I thinkt that the repetition
        // of delete m_shape; in the code is a reasonable tradeoff for not remaking the cube any more times then I need to.

        switch(settings.shapeType){
            case 0:
                if(oldVert == newParam1 && settings.shapeType == oldShape){//if only oldHoriz is off, that doesn't matter
                    break;
                }
                delete m_shape;
                m_shape = new Cube(newParam1,newParam2);
                break;
            case 1:
                if(newParam2 <= 3 && oldHoriz <= 3 && settings.shapeType == oldShape && newParam1 == oldVert){
                    break;
                }
                delete m_shape;
                m_shape = new Cone(newParam1, newParam2);
                break;
            case 2:
                if(settings.shapeType == oldShape && ((newParam1 <=3 && oldVert <=3)||(newParam2 <= 2 && oldHoriz <=2))){
                    break;
                }
                delete m_shape;
                m_shape = new Sphere(newParam1, newParam2);
                break;
            case 3:
                if(newParam2 <= 3 && oldHoriz <= 3 && settings.shapeType == oldShape && newParam1 == oldVert){
                    break;
                }
                delete m_shape;
                m_shape = new Cylinder(newParam1, newParam2);
                break;

            case 4:
                delete m_shape;
                m_shape = new Toroid(newParam1, newParam2);
                break;

            default:
                if(oldShape == 0 && oldVert == newParam1){
                    break;
                }
                delete m_shape;
                m_shape = new Cube(settings.shapeParameter1,settings.shapeParameter2);
                break;
        }
    }

    // Draw the Shape
    if (useMaterials)
        applyMaterial(m_material);

    m_shape->shapeRenderGeometry();

}

void ShapesScene::renderNormals()
{
    //TODO: [SHAPES] Rendering normals is OPTIONAL in shapes, and it's not
    //      extra credit. However, we believe that rendering normals will
    //      greatly assist you with visual debugging - both in this and
    //      future assignments. Therefore, we highly recommend you fill
    //      this method in.
    //

    /*
    Vector4 normal = Vector4(0, 0, 1, 0);
    renderNormal(Vector4(-1, -1, 0, 1), normal);
    renderNormal(Vector4(+1, -1, 0, 1), normal);
    renderNormal(Vector4(0, +1, 0, 1), normal);
    */
}

