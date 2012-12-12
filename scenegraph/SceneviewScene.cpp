#include <qgl.h>

#include "SceneviewScene.h"
#include "CS123SceneData.h"
#include "Camera.h"


SceneviewScene::SceneviewScene()
{
    // TODO: [SCENEVIEW] Set up anything you need for your Sceneview scene here...
    m_lightList = new QList<CS123SceneLightData>();
    m_primList = new QList<SimplePrimitive>();
    int num = 20;
    m_cube = new Cube(num,num);
    m_cone = new Cone(num,num);
    m_sphere = new Sphere(num,num);
    m_cylinder = new Cylinder(num,num);
    m_toroid = new Toroid(num,num);


}

SceneviewScene::~SceneviewScene()
{
    // TODO: [SCENEVIEW] Don't leak memory!
    delete m_lightList;
    delete m_primList;
    delete m_cube;
    delete m_cone;
    delete m_sphere;
    delete m_cylinder;
    delete m_toroid;

}

QList<SimplePrimitive> SceneviewScene::getPrimList(){
    QList<SimplePrimitive>* list = new QList<SimplePrimitive>();
    list->append(*m_primList);
    return *list;
}

CS123SceneGlobalData SceneviewScene::getGlobalData(){
    return m_globalData;
}

QList<CS123SceneLightData> SceneviewScene::getLightList(){
    QList<CS123SceneLightData>* list = new QList<CS123SceneLightData>();
    list->append(*m_lightList);
    return *list;
}

void SceneviewScene::addPrimitive(const CS123ScenePrimitive &scenePrimitive, const Matrix4x4 &matrix){

    SimplePrimitive cprim;
    cprim.material = scenePrimitive.material;
    cprim.filemap = *scenePrimitive.material.textureMap;
    cprim.type = scenePrimitive.type;
    cprim.transMat = matrix;

    m_primList->append(cprim);

}

void SceneviewScene::addLight(const CS123SceneLightData &sceneLight){
    CS123SceneLightData cLight = sceneLight;
    m_lightList->append(cLight);
}

void SceneviewScene::setLights(const Camera *follow)
{
    // TODO: [SCENEVIEW] Fill this in...
    //
    // Use OpenGL to set up the lighting for your scene. The lighting information
    // will most likely be stored in CS123SceneLightData structures.
    //
    for(int i = 0; i<8; i++){
        glDisable(GL_LIGHT0+i);
    }

    for(int i = 0; i<m_lightList->length(); i++){
        setLight(m_lightList->at(i));
    }
}

void SceneviewScene::renderGeometry(bool useMaterials)
{

    // TODO: [SCENEVIEW] Fill this in...
    //
    // This is where you should render the geometry of the scene. Use what you
    // know about OpenGL and leverage your Shapes classes to get the job done.
    //

    glEnable(GL_NORMALIZE);
    glMatrixMode(GL_MODELVIEW);
    for(int i=0; i< m_primList->length(); i++){
        SimplePrimitive cprim = m_primList->at(i);

        glPushMatrix();

        Matrix4x4 updated = cprim.transMat.getTranspose();
        GLdouble* matBegin = &updated.a;
        glMultMatrixd(matBegin);

        if(useMaterials){
            applyMaterial(cprim.material);
        }
        switch(cprim.type){
        case PRIMITIVE_CUBE://actual draw it here
            m_cube->shapeRenderGeometry();
            break;
        case PRIMITIVE_CONE:
            m_cone->shapeRenderGeometry();
            break;
        case PRIMITIVE_CYLINDER:
            m_cylinder->shapeRenderGeometry();
            break;
        case PRIMITIVE_TORUS:
            m_toroid->shapeRenderGeometry();
            break;
        case PRIMITIVE_SPHERE:
            m_sphere->shapeRenderGeometry();
            break;
        case PRIMITIVE_MESH:
            m_cube->shapeRenderGeometry();
            break;
        }
        glPopMatrix();
    }



}

void SceneviewScene::renderNormals()
{
    // TODO: [SCENEVIEW] Fill this in...
    //
    // Rendering normals is an **optional** part of Sceneview (sorry, not extra credit)
    // You can put all of your renderNormal() calls here if you want to render normals.
    //
    // HINT: Rendering normals can be a helpful debugging tool if things don't look
    // quite right. Your Shapes might be incorrect, and if that's the case, you'll
    // need to go back and fix your buggy shapes code before turning in Sceneview.
    //
}


void SceneviewScene::setSelection(int x, int y)
{
    // TODO: [MODELER LAB] Fill this in...
    //
    // Using m_selectionRecorder, set m_selectionIndex to the index in your
    // flattened parse tree of the object under the mouse pointer.  The
    // selection recorder will pick the object under the mouse pointer with
    // some help from you, all you have to do is:

    // 1) Set this to the number of objects you will be drawing.
    int numObjects = 0;

    // 2) Start the selection process
    m_selectionRecorder.enterSelectionMode(x, y, numObjects);

    // 3) Draw your objects, calling m_selectionRecorder.setObjectIndex() before drawing each one.

    // 4) Find out which object you selected, if any (-1 means no selection).
    m_selectionIndex = m_selectionRecorder.exitSelectionMode();
}
