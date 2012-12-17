#include "DataProcessor.h"
#include "lib/CS123SceneData.h"
//#include "lib/CS123ISceneParser.h"

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
    /*CS123SceneGlobalData gdata;
    parser->getGlobalData(gdata);
    setGlobal(gdata);

    for(int i = 0; i <parser->getNumLights(); i++){
        CS123SceneLightData ldata;
        parser->getLightData(i,ldata);
        addLight(ldata);
    }

    finishParsing(*parser->getRootNode(), Matrix4x4::identity());*/

    m_globalData.ka = 1;
    m_globalData.ks = 0;
    m_globalData.kd = 0;
    m_globalData.kt = 0;

    SimplePrimitive cprim;
    CS123SceneMaterial cmat;
    CS123SceneColor acolor;
    acolor.r = 100; acolor.b = 200; acolor.g = 100;
    cmat.cAmbient = acolor;
    cmat.cDiffuse = acolor;
    cprim.material = cmat;
    cprim.type = PRIMITIVE_CUBE;
    Matrix4x4 cMat;
    cMat.a = 2; cMat.b = 0; cMat.c = 0; cMat.d = 0;
    cMat.e = 0; cMat.f = 1; cMat.g = 0; cMat.h = 0;
    cMat.i = 0; cMat.j = 0; cMat.k = 1; cMat.l = 0;
    cMat.m = 0; cMat.n = 0; cMat.o = 0; cMat.p = 1;
    cprim.transMat = cMat;
    m_primLists.push_back(cprim);

    CS123SceneLightData cldata;
    cldata.id = 0;
    cldata.type = 
    CS123SceneColor bcolor;
    bcolor.r = 255; bcolor.g = 255; bcolor.b = 255;
    cldata.color = bcolor;
    cldata.pos = Vector4(0,2,0,1);

    m_lightLists.push_back(cldata);

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

