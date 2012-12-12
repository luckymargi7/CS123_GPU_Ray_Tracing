#include "RayScene.h"
#include "Settings.h"
#include "CS123SceneData.h"
#include "shape/MathCone.h"
#include "shape/MathCube.h"
#include "shape/MathSphere.h"
#include "shape/MathCylinder.h"
#include "Settings.h"
#include "CamtransCamera.h"
#include <assert.h>
#include <math.h>
#include <QPainter>

using namespace std;

RayScene::RayScene(QList<SimplePrimitive> primList, QList<CS123SceneLightData> lightList, CS123SceneGlobalData gdata){

    m_shapeList = &primList;
    m_lightList = &lightList;

    m_ka = gdata.ka;
    m_kd = gdata.kd;
    m_kt = gdata.kt;
    m_ks = gdata.ks;

    m_cube = new MathCube();
    m_cone = new MathCone();
    m_sphere = new MathSphere();
    m_cylinder = new MathCylinder();

    //insert pointers to QImages of the textures into the hashmap
    for(int i=0; i<m_shapeList->length(); i++){
        SimplePrimitive cprim = m_shapeList->at(i);
        QString qfilename = QString::fromStdString(cprim.filemap.filename);
        if(cprim.filemap.isUsed && !m_imagehash.contains(qfilename)){
            m_imagehash.insert(qfilename,loadImage(qfilename));
        }
    }

}

RayScene::~RayScene(){

    delete m_cube;
    delete m_cone;
    delete m_sphere;
    delete m_cylinder;

    foreach (QImage* value, m_imagehash)
         delete value;

}

void RayScene::renderGeometry(Camera *camera, Canvas2D *canvas, int width, int height){

    CamtransCamera* cam = (CamtransCamera*)camera;
    canvas->changeSize(width, height);
    BGRA* pix = canvas->data();
    double canvas_width = (double)width;
    double canvas_height = (double)height;

    m_inv = cam->getSansUnhinging();

    Vector4 peye = m_inv*Vector4(0, 0, 0, 1);

    for(int col = 0; col< canvas->width(); col++){
        for(int row = 0; row< canvas->height(); row++){
            double x = col+.5;
            double y = row+.5;

            double pfilmx = ((2*x)/canvas_width)-1;
            double pfilmy = 1-(2*y/canvas_height);
            Vector4 pfilm = Vector4( pfilmx, pfilmy, -1, 1);
            Vector4 pworld = m_inv*pfilm;

            DividedPoint vector;
            vector.D = (pworld-peye).getNormalized();
            vector.P = peye;

            vector = getT(vector);//very time intensive step

            Vector4 finalc = Vector4(0,0,0,0);
            if(vector.t != -1){
                finalc = getColor(vector);
            }

            pix[row*canvas->width()+col].r = capAndScale(finalc.x);
            pix[row*canvas->width()+col].b = capAndScale(finalc.y);
            pix[row*canvas->width()+col].g = capAndScale(finalc.z);

        }
    }

}


DividedPoint RayScene::getT(DividedPoint vector){//fixed

    SimplePrimitive fprim;
    double currt, finalt;
    finalt = -1;

    //check for closest intersection
    for(int pnum = 0; pnum < m_shapeList->length(); pnum++){
        SimplePrimitive cprim = m_shapeList->at(pnum);

        switch(cprim.type){
            case PRIMITIVE_CUBE:
                currt = m_cube->findIntersect(vector,cprim.transMat);
                break;
            case PRIMITIVE_CONE:
                currt = m_cone->findIntersect(vector,cprim.transMat);
                break;
            case PRIMITIVE_CYLINDER:
                currt = m_cylinder->findIntersect(vector,cprim.transMat);
                break;
            case PRIMITIVE_SPHERE:
                currt = m_sphere->findIntersect(vector,cprim.transMat);
                break;
            default:
                currt = m_cube->findIntersect(vector,cprim.transMat);
                break;
        }

        if(currt > 0 && (finalt < 0 || finalt > currt)){
            finalt = currt;
            fprim = cprim;
        }

    }

    vector.t = finalt;
    if(finalt != -1){
        vector.prim = fprim;
    }

    return vector;
}

DividedPoint RayScene::getNormal(DividedPoint point){

    switch(point.prim.type){
        case PRIMITIVE_CUBE:
            point = m_cube->findNormal(point);
            break;
        case PRIMITIVE_CONE:
            point = m_cone->findNormal(point);
            break;
        case PRIMITIVE_CYLINDER:
            point = m_cylinder->findNormal(point);
            break;
        case PRIMITIVE_SPHERE:
            point = m_sphere->findNormal(point);
            break;
        default:
            point = m_cube->findNormal(point);
            break;
    }

    return point;
}

Vector4 RayScene::getColor(DividedPoint pointDiv){
    Vector4 finalc = Vector4(0,0,0,0);

    pointDiv = getNormal(pointDiv);//put the normal vector into the point
    pointDiv.normal = pointDiv.normal.getNormalized();//normalize it
    Vector4 point = pointDiv.P+pointDiv.D*pointDiv.t;

    for(int lnum = 0; lnum<m_lightList->length(); lnum++){
        CS123SceneLightData currlight =  m_lightList->at(lnum);
        Vector4 lightV = currlight.pos-point;
        double lightDist = lightV.getMagnitude();
        bool useLight = true;

        //figure out whether to use this light
        if(settings.useShadows){

            if(currlight.type == LIGHT_POINT){
                DividedPoint blockerPdiv;
                blockerPdiv.P = point;
                blockerPdiv.D = lightV.getNormalized();
                blockerPdiv = getT(blockerPdiv);//now a point not just a vector

                if(blockerPdiv.t != -1){
                    Vector4 blockerP = blockerPdiv.P+blockerPdiv.D*blockerPdiv.t;
                    double blockDist = (blockerP-point).getMagnitude();
                    if(blockDist<=lightDist){
                        useLight = false;
                    }
                }
            } else if(currlight.type==LIGHT_DIRECTIONAL){
                DividedPoint blockerPdiv;
                blockerPdiv.P = point;
                blockerPdiv.D = (Vector4(-1,-1,-1,0)*currlight.dir).getNormalized();
                blockerPdiv = getT(blockerPdiv);//now a point not just a vector

                if(blockerPdiv.t != -1){
                    useLight = false;
                }
            } else{
                useLight = false;
            }
        }

        //calculate the lights contribution
        if(useLight){
            CS123SceneMaterial material = pointDiv.prim.material;

            Vector4 lp;
            double fatt;
            bool updateColor = true;

            if(currlight.type == LIGHT_DIRECTIONAL && settings.useDirectionalLights){//if you need to use a directional light, l=direction
                lp = Vector4(-1,-1,-1,0)*currlight.dir;
                fatt = 1;
            } else if(currlight.type == LIGHT_DIRECTIONAL){//if you don't use directional l=|0>
                updateColor = false;
            } else if(currlight.type == LIGHT_POINT && settings.usePointLights){//if it is a point (or other) use lightV
                lp = lightV;
                fatt = min(1.0,1/(currlight.function.x+currlight.function.y*lightDist+currlight.function.z*pow(lightDist,2)));
            } else{
                updateColor = false;
            }

            Vector4 dContrib;
            if(settings.useTextureMapping){

                Point ab;
                switch(pointDiv.prim.type){
                    case PRIMITIVE_CUBE:
                        ab = m_cube->getAB(pointDiv);
                        break;
                    case PRIMITIVE_CONE:
                        ab = m_cone->getAB(pointDiv);
                        break;
                    case PRIMITIVE_CYLINDER:
                        ab = m_cylinder->getAB(pointDiv);
                        break;
                    case PRIMITIVE_SPHERE:
                        ab = m_sphere->getAB(pointDiv);
                        break;
                    default:
                        ab = m_cube->getAB(pointDiv);
                        break;
                }

                double a = ab.x;
                double b = ab.y;

                if(a<0){
                    //cout<<a<<"\n"<<flush;
                }

                const QString filename = QString::fromStdString(pointDiv.prim.filemap.filename);
                BGRA texColor = getTexturePixel(filename,a,b,pointDiv.prim.filemap.repeatU,pointDiv.prim.filemap.repeatV);
                double blend = pointDiv.prim.material.blend;


                dContrib.x = blend*(((int)texColor.r)/255.0)+(1-blend)*material.cDiffuse.r;
                dContrib.y = blend*(((int)texColor.b)/255.0)+(1-blend)*material.cDiffuse.b;
                dContrib.z = blend*(((int)texColor.g)/255.0)+(1-blend)*material.cDiffuse.g;

            } else{
                dContrib.x = material.cDiffuse.r;
                dContrib.y = material.cDiffuse.b;
                dContrib.z = material.cDiffuse.g;
            }


            if(updateColor){
                Vector4 peye = m_inv*Vector4(0,0,0,1);
                Vector4 v = (peye-point).getNormalized();
                Vector4 n = pointDiv.normal;
                Vector4 r = (Vector4(2,2,2,0)*n*(n.dot(lp.getNormalized()))-lp.getNormalized());

                double rdotv = (r.dot(v)<0)?0:r.dot(v);
                double ndotl = (n.dot(lp.getNormalized())<0)?0:n.dot(lp.getNormalized());

                finalc.x = finalc.x+fatt*currlight.color.r*(dContrib.x*ndotl+m_ks*material.cSpecular.r*pow(rdotv,material.shininess));
                finalc.y = finalc.y+fatt*currlight.color.b*(dContrib.y*ndotl+m_ks*material.cSpecular.b*pow(rdotv,material.shininess));
                finalc.z = finalc.z+fatt*currlight.color.g*(dContrib.z*ndotl+m_ks*material.cSpecular.g*pow(rdotv,material.shininess));//include light
            }
        }

    }

    finalc.x = finalc.x + pointDiv.prim.material.cAmbient.r;
    finalc.y = finalc.y + pointDiv.prim.material.cAmbient.b;
    finalc.z = finalc.z + pointDiv.prim.material.cAmbient.g;

    return finalc;
}

int RayScene::capAndScale(double finalc){
    double finalv = (finalc<0)?0:finalc;
    finalv = floor(255*((finalv>1)?1:finalv));
    return (int)finalv;
}

QImage* RayScene::loadImage(const QString &file){
    QImage* cImage = new QImage();
    if (cImage->load(file)){
        if (cImage->format() != QImage::Format_RGB32){
            QImage *old = cImage;
            cImage = new QImage(old->convertToFormat(QImage::Format_RGB32));
            delete old;
        }
    }
    return cImage;
}

BGRA RayScene::getTexturePixel(const QString filename, double a, double b, int u, int v){


    QImage* image = m_imagehash.value(filename);
    BGRA* data = (BGRA*)image->bits();


    int w = image->width();
    int h = image->height();


    int s = ((int)(a*u*w))%w;
    int t = ((int)(b*v*h))%h;
    BGRA pixel = data[t*w+s];

    return pixel;
}

