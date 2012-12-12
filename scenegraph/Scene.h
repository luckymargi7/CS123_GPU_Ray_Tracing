/*!
   @file   Scene.h
   @author Ben Herila (ben@herila.net)
   @author Evan Wallace (evan.exe@gmail.com)
   @date   Fall 2010

   @brief  This is a great place to put common data structures that need to be used by
   all your scenes.
*/

#ifndef SCENE_H
#define SCENE_H

#include <CS123Algebra.h>
#include "CS123SceneData.h"

class Camera;
class CS123ISceneParser;


/**

 @class Scene

 @brief This is the base class for all scenes. Modify this class if you want to provide
 common functionality to all your scenes.

 @author  Evan Wallace (edwallac)
 @author  Ben Herila (bherila)
 @date    9/1/2010

**/

class Scene
{
public:
    Scene();
    virtual ~Scene();

    static void parse(Scene *sceneToFill, CS123ISceneParser *parser);

    Matrix4x4 calcTransMat(std::vector<CS123SceneTransformation*> tranVect);
    QList<SimplePrimitive> getPrimList();
    QList<CS123SceneLightData> getLightList();
    CS123SceneGlobalData getGlobalData();

    void printMat(Matrix4x4 mat);

    ///*! @brief Returns this scene's local camera (a pointer to one of the cameras in SupportCanvas3D). */
    //Camera *getCamera() { return m_camera; }

    ///*! @brief Sets this scene's local camera(a pointer to one of the cameras in SupportCanvas3D). */
    //void setCamera(Camera *newCamera);


protected:

    /*! Adds a primitive to the scene. */
    virtual void addPrimitive(const CS123ScenePrimitive &scenePrimitive, const Matrix4x4 &matrix);

    /*! Adds a light to the scene. */
    virtual void addLight(const CS123SceneLightData &sceneLight);

    /*! Sets the global data for the scene. */
    virtual void setGlobal(const CS123SceneGlobalData &global);

    void finishParsing(CS123SceneNode cNode, Scene *sceneToFill, Matrix4x4 oldTM);

    CS123ISceneParser* m_parser;

    CS123SceneGlobalData m_globalData;

    QList<CS123SceneLightData>* m_lightList;
    QList<SimplePrimitive>* m_primList;


private:
    //Camera *m_camera;
    float m_kt;
    float m_kd;
    float m_ka;
    float m_ks;

};

#endif // SCENE_H
