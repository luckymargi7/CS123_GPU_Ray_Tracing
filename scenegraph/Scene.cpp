#include "Scene.h"
#include "Camera.h"
#include "CS123SceneData.h"
#include "CS123ISceneParser.h"
#include <QtGlobal>


Scene::Scene() //: m_camera(NULL)//
{

}

Scene::~Scene()
{
    // Do not delete m_camera, it is owned by SupportCanvas3D

}

void Scene::parse(Scene *sceneToFill, CS123ISceneParser *parser)
{

    CS123SceneGlobalData gdata;
    parser->getGlobalData(gdata);
    sceneToFill->setGlobal(gdata);

    for(int i = 0; i <parser->getNumLights(); i++){
        CS123SceneLightData ldata;
        parser->getLightData(i,ldata);
        sceneToFill->addLight(ldata);
    }

    sceneToFill->finishParsing(*parser->getRootNode(), sceneToFill, Matrix4x4::identity());

    // TODO: load scene into sceneToFill using setGlobal(), addLight(), addPrimitive(), and finishParsing()
}

//void Scene::setCamera(Camera *newCamera)//
//{
//    // Do not delete the old m_camera, it is owned by SupportCanvas3D
//    m_camera = newCamera;//
//}//

void Scene::addPrimitive(const CS123ScenePrimitive &scenePrimitive, const Matrix4x4 &matrix){}

void Scene::addLight(const CS123SceneLightData &sceneLight){}

void Scene::setGlobal(const CS123SceneGlobalData &global)
{

    m_globalData = global;
    m_kd = global.kd;
    m_ks = global.ks;
    m_ka = global.ka;
    m_kt = global.kt;

}

void Scene::finishParsing(CS123SceneNode cNode, Scene *sceneToFill, Matrix4x4 oldTM){
    Matrix4x4 transmat = oldTM*calcTransMat(cNode.transformations);
    std::vector<CS123ScenePrimitive*> primVect = cNode.primitives;
    std::vector<CS123ScenePrimitive*>::const_iterator j;

    for(j = primVect.begin(); j!=primVect.end(); j++){
        CS123ScenePrimitive cprim = **j;

        cprim.material.cDiffuse.r = cprim.material.cDiffuse.r*m_kd;
        cprim.material.cDiffuse.g = cprim.material.cDiffuse.g*m_kd;
        cprim.material.cDiffuse.b = cprim.material.cDiffuse.b*m_kd;

        cprim.material.cAmbient.r = cprim.material.cAmbient.r*m_ka;
        cprim.material.cAmbient.g = cprim.material.cAmbient.g*m_ka;
        cprim.material.cAmbient.b = cprim.material.cAmbient.b*m_ka;

        sceneToFill->addPrimitive(cprim, transmat);
    }

    //do the same to all the children
    std::vector<CS123SceneNode*> childVect = cNode.children;

    for(int k = 0; k<childVect.size(); k++){
        finishParsing(*childVect.at(k), sceneToFill, transmat);
    }


}

Matrix4x4 Scene::calcTransMat(std::vector<CS123SceneTransformation*> tranVect){

    Matrix4x4 totalMat = Matrix4x4::identity();
    //how to handle one transformation
    std::vector<CS123SceneTransformation*>::const_iterator i;
    for(i = tranVect.begin(); i!=tranVect.end(); i++){
        Matrix4x4 tmat;
        CS123SceneTransformation cTran = **i;
        switch(cTran.type){

        case TRANSFORMATION_TRANSLATE:
            tmat = getTransMat(cTran.translate);
            break;
        case TRANSFORMATION_SCALE:
            tmat = getScaleMat(cTran.scale);
            break;
        case TRANSFORMATION_ROTATE:
            tmat = getRotMat(Vector4(0,0,0,0),cTran.rotate,cTran.angle);
            break;
        case TRANSFORMATION_MATRIX:
            tmat = cTran.matrix;
            break;
        }

        totalMat = totalMat*tmat;

    }

    return totalMat;

}

void Scene::printMat(Matrix4x4 mat){

    cout << mat.a << " "<<mat.b<< " "<<mat.c << " " << mat.d<<"\n"
            << mat.e << " " << mat.f << " "<< mat.g << " " <<mat.h<<"\n"
            <<mat.i<< " " << mat.j<< " " << mat.k<<" "<< mat.l<<"\n"
            <<mat.m << " " <<mat.n<< " "<<mat.o<<" " << mat.p<<"\n\n"<<flush;
}




