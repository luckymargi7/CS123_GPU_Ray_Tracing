#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <math/CS123Algebra.h>
#include "lib/CS123SceneData.h"

class CS123ISceneParser;

class DataProcessor
{
public:
    DataProcessor();
    virtual ~DataProcessor();

    static void parse(CS123ISceneParser *parser);

    Matrix4x4 calcTransMat(std::vector<CS123SceneTransformation*> tranVect);
    std::vector<CS123SceneLightData>* getLightLists();
    std::vector<SimplePrimitive>* getPrimLists();
    CS123SceneGlobalData getGlobalData();

protected:

    /*! Adds a primitive to the scene. */
    virtual void addPrimitive(const CS123ScenePrimitive &scenePrimitive, const Matrix4x4 &matrix);

    /*! Adds a light to the scene. */
    virtual void addLight(const CS123SceneLightData &sceneLight);

    /*! Sets the global data for the scene. */
    virtual void setGlobal(const CS123SceneGlobalData &global);

    void finishParsing(CS123SceneNode cNode, Matrix4x4 oldTM);

    CS123ISceneParser* m_parser;

    CS123SceneGlobalData m_globalData;

    std::vector<CS123SceneLightData> m_lightLists;
    std::vector<SimplePrimitive> m_primLists;
};

#endif // DATAPROCESSOR_H
