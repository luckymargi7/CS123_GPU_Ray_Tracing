/**
  CS123 2-dimens. Contains support code necessary for Brush,
  Filter, Intersect, and Ray.
  
  YOU WILL NEED TO FILL THIS IN!
  
  Please contact the authors if there are any bugs.
  
  @date   2010-08-24
  @author Evan Wallace (edwallac)
  @author Ben Herila (ben@herila.net)
  
**/

// For your convenience, a few headers are included for you.
#include <math.h>
#include <assert.h>
#include "Canvas2D.h"

#include <iostream>
#include <string>
#include <QCoreApplication>
#include <QImage>
#include <QString>
#include <QPainter>

using namespace std;

#define MAX_DEPTH 4

Canvas2D::Canvas2D()
{
    // @TODO: Initialize any pointers in this class here.
    m_scene = NULL;

}

Canvas2D::~Canvas2D()
{
    // @TODO: Be sure to release all memory that you allocate.
    delete m_scene;
}

// This is called when the canvas size is changed. You can change the canvas size
// by calling resize(...). You probably won't need to fill this in, but you can
// if you want to.
void Canvas2D::notifySizeChanged(int w, int h) {
    
}

void Canvas2D::setScene(Scene *scene)
{
    if (scene) {
        delete m_scene;
        m_scene = scene;
    }
}

void Canvas2D::paintEvent(QPaintEvent *e) {
    // You probably won't need to fill this in, but you can if you want to override
    // any painting events for the 2D canvas. For now, we simply call the superclass.
    this->resize(600, 600);
    SupportCanvas2D::paintEvent(e);
    
}

// *****************************************************************************************
// ** BRUSH
// *****************************************************************************************

void Canvas2D::mouseDown(int x, int y)
{
    // @TODO: [BRUSH] Mouse interaction for brush. You will probably want to create a
    //        separate class for each of your brushes. Remember that you can use the
    //        static Settings object to get the currently selected brush and its parameters.
    
}

void Canvas2D::mouseDragged(int x, int y)
{
}

void Canvas2D::mouseUp(int x, int y)
{
}


void Canvas2D::renderImage(Camera *camera, int width, int height)
{
    if (m_scene)
    {
        // @TODO: raytrace the scene based on settings
        //        YOU MUST FILL THIS IN FOR INTERSECT/RAY
        
        // If you want the interface to stay responsive, make sure to call
        // QCoreApplication::processEvents() periodically during the rendering
        this->resize(width, height);
        //this->renderImage(camera, 0, height-1, 0, width-1, width, height);
    }

}

/*

void Canvas2D::renderImage(Camera *camera, int rowstart, int rowend, int colstart, int colend, int width, int height){
    if (m_scene)
    {
        // @TODO: raytrace the scene based on settings
        //        YOU MUST FILL THIS IN FOR INTERSECT/RAY

        // If you want the interface to stay responsive, make sure to call
        // QCoreApplication::processEvents() periodically during the rendering

        BGRA* pix = this->data();
        int i;

        QCoreApplication::processEvents();

        for (int rowcounter = rowstart; rowcounter <= rowend; ++rowcounter) {
            for (int colcounter = colstart; colcounter <= colend; ++colcounter) {
                Vector4 pEye = camera->getPosition();
                i = rowcounter*width + colcounter;

                CS123SceneColor color;
                color.r = color.g = color.b = 0;
                color.a = 255;

                if (settings.useAntiAliasing) {
                    int ss = (settings.useSuperSampling && settings.numSuperSamples >= 2) ? settings.numSuperSamples : 3;

                    for (int dy = -ss/2; dy < (ss+1)/2; dy++) {
                        for (int dx = -ss/2; dx < (ss+1)/2; dx++) {
                            int x = max(0, min(colcounter + dx/(ss-1), width));
                            int y = max(0, min(rowcounter + dy/(ss-1), height));

                            Vector4 direction = calcDirection(camera, x, y, width, height);
                            color += trace(pEye, direction, 0);
                        }
                    }

                    color /= (ss*ss);
                } else {
                    //Calculate direction
                    Vector4 direction = calcDirection(camera, colcounter, rowcounter, width, height);
                    color = trace(pEye, direction, 0);
                }

                color = color*255;
                capColor(color);

                pix[i].r = color.r;
                pix[i].g = color.g;
                pix[i].b = color.b;
                pix[i].a = color.a;

                QCoreApplication::processEvents();
            }

            if(!settings.useMultiThreading)
                this->repaint();

            QCoreApplication::processEvents();
        }
    }

    QCoreApplication::processEvents();
}

Vector4 Canvas2D::calcDirection(Camera* camera, int colcounter, int rowcounter, int width, int height){
    //Calculate direction
    Vector4 pWorld = calcPworld(colcounter, rowcounter, width, height, camera);
    Vector4 pEye = camera->getPosition();
    Vector4 direction = pWorld - pEye;
    return direction.getNormalized();
}

CS123SceneColor Canvas2D::trace(Vector4& pEye, Vector4& direction, int depth) {
    CS123SceneColor bg, color;
    bg.r = bg.g = bg.b = 0;
    bg.a = 1;
    color.r = color.g = color.b = color.a = 0;
    //If depth is over max
    if (depth > MAX_DEPTH) 
        return bg;
    
    REAL minT = -1;
    Shape* shape = NULL;
    //Make ray scene go through each object and intersect
    minT = get_first_intersection(pEye, direction, &shape);
    
    //If the ray intersects with something
    if (minT > 0 && shape != NULL) {
        CS123SceneColor point_color, reflect_color;
        point_color.r = point_color.g = point_color.b = point_color.a = 0;
        reflect_color.r = reflect_color.g = reflect_color.b = reflect_color.a = 0;
        
        //get intersecting point
        Vector4 intersectPt = pEye + direction*minT;
        
        //Get normal_world, and material
        Vector4 normal_world = getNormalWorld(shape, intersectPt);
        CS123SceneMaterial material = shape->getMaterial();

        calcIllumination(shape, material, normal_world, pEye, intersectPt, point_color);
        
        color += point_color;
        
        //Reflective Rays
        if (m_scene->get_ks() > 0 && depth < MAX_DEPTH && settings.useReflection) {
            Vector4 reflDir = reflectVector(direction, normal_world);
            CS123SceneColor Ir = trace(intersectPt, reflDir, depth+1);
            CS123SceneColor IrOr = Ir * material.cReflective;
            
            //reflect_color = ks*Ir*Or
            reflect_color = IrOr * m_scene->get_ks();

            color += reflect_color;
        }

        return color;
        
    } else
        return bg;
    
}

void Canvas2D::capColor(CS123SceneColor &color) {
    //Make sure that the any number less than 0 is changed to 0.
    color.r = ((color.r < 0) ? 0 : color.r);
    color.g = ((color.g < 0) ? 0 : color.g);
    color.b = ((color.b < 0) ? 0 : color.b);
    color.a = ((color.a < 0) ? 0 : color.a);
    
    //Make sure that the any number greater than 255 is changed to 255.
    color.r = ((color.r > 255) ? 255 : color.r);
    color.g = ((color.g > 255) ? 255 : color.g);
    color.b = ((color.b > 255) ? 255 : color.b);
    color.a = ((color.a > 255) ? 255 : color.a);
}

REAL Canvas2D::get_first_intersection(Vector4& pEye, Vector4& direction, Shape **shape) {
    REAL minT = -1;
    
    list<Shape*> shapes = m_scene->getShapes();
    
    //Make ray scene go through each object and intersect
    for (list<Shape*>::iterator it = shapes.begin(); it != shapes.end(); it++) {
        Shape* curshape = *it;
        double objT = curshape->intersect(pEye, direction);
        
        if ((objT > 0 + EPSILON) && (minT < 0 || minT > objT)){
            minT = objT;
            *shape = curshape;
        }
    }
    
    return minT;
}

REAL Canvas2D::is_blocked_by_obj(Vector4& pEye, Vector4& direction, Vector4& lightPos, Shape *curr_shape) {
    Vector4 newpEye = pEye;
    Vector4 newDir = direction;
    Shape *other = NULL;

    REAL minT = -1;

    do{
        minT = get_first_intersection(newpEye, newDir, &other);
        newpEye = newpEye + newDir*minT;
        if (EQ(minT, 0))
            break;
    } while(other == curr_shape && minT != -1);

    //Special case where the light is closer to the object than the
    //obj that could possibly block the light
    REAL dist1, dist2;
    dist1 = 0;
    dist2 = 0;
    if(minT >= 0) {
        Vector4 intersectPt = newpEye + newDir*minT;
        dist1 = (intersectPt).getDistance2(newpEye);
        dist2 = (lightPos).getDistance2(newpEye);
    }

    return (minT > 0 && other != curr_shape && dist1 <= dist2) ;
}

Vector4 Canvas2D::getNormalWorld(Shape *shape, Vector4 intersectPt) {
    //Calculate object-to-world space transformation matrix
    Matrix4x4 M = shape->getTransformationMat();
    REAL data3x3 [16] = { M.data[0], M.data[1], M.data[2], 0,
                          M.data[4], M.data[5], M.data[6], 0,
                          M.data[8], M.data[9], M.data[10], 0,
                          0, 0, 0, 1};
    
    Matrix4x4 M_3 = Matrix4x4(data3x3);
    M_3 = Matrix4x4::transpose(M_3);
    M_3 = M_3.getInverse();
    
    //Get and transform normal
    Vector3 normal = shape->getNormalAtPoint(intersectPt);
    Vector4 normal4 = Vector4(normal.x, normal.y, normal.z, 0);
    
    Vector4 normal_world = M_3*normal4;

    return normal_world.getNormalized();
}

Vector4 Canvas2D::reflectVector(Vector4 ray, Vector4 normal) {
    Vector3 ray3 = Vector3(ray.x, ray.y, ray.z);
    Vector3 normal3 = Vector3(normal.x, normal.y, normal.z);
    
    Vector3 reflRay = ray3.reflectVector(normal3);
    
    return Vector4(reflRay.x, reflRay.y, reflRay.z, 0);
}

Vector4 Canvas2D::calcPfilm(double x, double y, double x_max, double y_max) {
    double newX = 2*x/x_max - 1;
    double newY = 1 - 2*y/y_max;
    
    return Vector4(newX, newY, -1, 1);
}

Vector4 Canvas2D::calcPworld(double x, double y, double x_max, double y_max, Camera* camera) {
    Vector4 pFilm = calcPfilm(x, y, x_max, y_max);
    Matrix4x4 viewingTransformation = camera->getViewingTransformation();
    
    return viewingTransformation*pFilm;
}

//Color values range from 0 to 1
void Canvas2D::calcIllumination(Shape *shape, CS123SceneMaterial &material, Vector4& normal, Vector4& pEye, Vector4& intersectPt, CS123SceneColor& color){
    
    Vector4 lightDir;
    QCoreApplication::processEvents();
    
    list<CS123SceneLightData*> m_lights = m_scene->getLights();
    list<CS123SceneLightData*>::iterator it;
    double kd = m_scene->get_kd();
    double ks = m_scene->get_ks();
    
    //Ambient
    double ka = m_scene->get_ka();
    color += (material.cAmbient * ka);

    //Diffuse + Specular + possibly texture-mapping
    for (it = m_lights.begin(); it != m_lights.end(); it++) {
        CS123SceneLightData *light = *it;

        //lightDir from intersectPt to light source
        if (light->type == LIGHT_POINT)
            lightDir = light->pos - intersectPt;
        else if (light->type == LIGHT_DIRECTIONAL)
            lightDir = light->dir*-1;
        
        lightDir.normalize();
        
        REAL dLight = light->pos.getDistance(intersectPt);
        REAL fatt = (light->type == LIGHT_DIRECTIONAL) ? 1 : min(1.0, 1/(light->function.x + light->function.y*dLight + light->function.z*dLight*dLight));

        // The if statement makes sure to account for shadows!
        // If there's no object that blocks the way from the intersecting point to the light
        if (!(is_blocked_by_obj(intersectPt, lightDir, light->pos, shape) && (settings.useShadows))){
            double normalDotLight = normal.dot(lightDir);

            //normalized reflected light from light i
            Vector4 refl = reflectVector(lightDir*-1, normal).getNormalized();

            //normalized line of sight - from intersectPt to camera
            Vector4 view = (pEye - intersectPt).getNormalized();
            double reflDotView = refl.dot(view);

            normalDotLight = (normalDotLight < 0) ? 0 : normalDotLight;
            reflDotView = (reflDotView < 0) ? 0 : reflDotView;

            REAL const_a = kd * normalDotLight;
            REAL const_b = ks * pow(reflDotView, material.shininess);

            QImage *texture = shape->getTexture();
            //Texture mapping or no?
            if (settings.useTextureMapping && texture != NULL) {
                Vector2 textureCoord = getTextureCoord(shape, material, (int) texture->width(), (int) texture->height(), intersectPt);

                QColor textureColor = texture->pixel(textureCoord.x, textureCoord.y);
                CS123SceneColor tColor;
                tColor.r = textureColor.red()/255.0;
                tColor.g = textureColor.green()/255.0;
                tColor.b = textureColor.blue()/255.0;
                tColor.a = 1;

                color += (light->color * ((material.cDiffuse * (1 - material.blend) * const_a + (tColor * material.blend * normalDotLight)) + (material.cSpecular * const_b))) * fatt;
            } else {
                color += (light->color * ((material.cDiffuse * const_a)  + (material.cSpecular * const_b))) * fatt;
            }
        }
        
        QCoreApplication::processEvents();
        
    }
    

}

Vector2 Canvas2D::getTextureCoord(Shape *shape, CS123SceneMaterial material, int textureWidth, int textureHeight, Vector4 intersectPt) {
    //Step one, map intersecting point to unit square
    Vector2 unitCoord = shape->getUnitCoord(shape->getPtObj(intersectPt));

    //Step two, use the unit coordinate and  map to texture
    return Vector2((int)(unitCoord.x * material.textureMap->repeatU * textureWidth) % textureWidth,
                   (int)(unitCoord.y * material.textureMap->repeatV * textureHeight) % textureHeight);
}
*/

void Canvas2D::cancelRender()
{
    // TODO: cancel the raytracer (optional)

}


void Canvas2D::settingsChanged() {

}
