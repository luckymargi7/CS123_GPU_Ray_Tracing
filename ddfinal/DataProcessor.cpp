#include "DataProcessor.h"
#include "lib/CS123SceneData.h"
#include "lib/CS123ISceneParser.h"
#include <QtGlobal>


DataProcessor::DataProcessor()
{}

DataProcessor::~DataProcessor()
{}

CS123SceneGlobalData DataProcessor::getGlobalData(){
    return m_globalData;
}

std::vector<SimplePrimitive>* DataProcessor::getPrimLists(){
    return &m_primLists;
}

std::vector<CS123SceneLightData>* DataProcessor::getLightLists(){
    return &m_lightLists;;
}

void DataProcessor::parse(CS123ISceneParser *parser)
{
    CS123SceneGlobalData gdata;
    parser->getGlobalData(gdata);
    setGlobal(gdata);

    for(int i = 0; i <parser->getNumLights(); i++){
        CS123SceneLightData ldata;
        parser->getLightData(i,ldata);
        addLight(ldata);
    }

    finishParsing(*parser->getRootNode(), Matrix4x4::identity());
}

void DataProcessor::addPrimitive(const CS123ScenePrimitive &scenePrimitive, const Matrix4x4 &matrix){

    SimplePrimitive cprim;
    cprim.material = scenePrimitive.material;
    cprim.filemap = *scenePrimitive.material.textureMap;
    cprim.type = scenePrimitive.type;
    cprim.transMat = matrix;

    m_primLists.push_back(cprim);
}

void DataProcessor::addLight(const CS123SceneLightData &sceneLight){
    CS123SceneLightData cLight = sceneLight;
    m_lightLists.push_back(cLight);
}

void DataProcessor::setGlobal(const CS123SceneGlobalData &global){
    m_globalData = global;
}

void DataProcessor::finishParsing(CS123SceneNode cNode, Matrix4x4 oldTM){
    Matrix4x4 transmat = oldTM*calcTransMat(cNode.transformations);
    std::vector<CS123ScenePrimitive*> primVect = cNode.primitives;
    std::vector<CS123ScenePrimitive*>::const_iterator j;

    for(j = primVect.begin(); j!=primVect.end(); j++){
        CS123ScenePrimitive cprim = **j;
        addPrimitive(cprim, transmat);
    }

    //do the same to all the children
    std::vector<CS123SceneNode*> childVect = cNode.children;

    for(int k = 0; k<childVect.size(); k++){
        finishParsing(*childVect.at(k), transmat);
    }

}

Matrix4x4 DataProcessor::calcTransMat(std::vector<CS123SceneTransformation*> tranVect){

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

