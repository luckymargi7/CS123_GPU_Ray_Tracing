
#include "MathSphere.h"
#include <iostream>
#include <CS123Common.h>

using namespace std;

MathSphere::MathSphere() : MathShape(){

}

MathSphere::~MathSphere(){
    //clean up
}

REAL MathSphere::findIntersect(DividedPoint vector, Matrix4x4 transMat){

    Matrix4x4 invTransMat = transMat.getInverse();
    vec4<REAL> preal = invTransMat*vector.P;
    vec4<REAL> dreal = invTransMat*vector.D;

    REAL px = preal.x; REAL py = preal.y; REAL pz = preal.z;
    REAL dx = dreal.x; REAL dy = dreal.y; REAL dz = dreal.z;

    REAL finalt, currt;
    finalt = -1;

    //body
    REAL A = dx*dx+dz*dz+dy*dy;
    REAL B = 2*px*dx+2*pz*dz+2*py*dy;
    REAL C = px*px+pz*pz+py*py-.5*.5;
    REAL dis = B*B-4*A*C;

    if(dis>=0){
        currt = (-B+sqrt(dis))/(2*A);
        if(currt > .001){
            if(finalt <0 || currt < finalt){
                finalt = currt;
            }
        }

        currt = (-B-sqrt(dis))/(2*A);
        if(currt > .001){
            if(finalt <0 || currt < finalt){
                finalt = currt;
            }
        }
    }

    return finalt;
}

DividedPoint MathSphere::findNormal(DividedPoint point){
    Matrix4x4 transMat = point.prim.transMat;
    Matrix4x4 invTransMat = transMat.getInverse();
    vec4<REAL> preal = invTransMat*point.P;
    vec4<REAL> dreal = invTransMat*point.D;
    double t = point.t;

    REAL px = preal.x; REAL py = preal.y; REAL pz = preal.z;
    REAL dx = dreal.x; REAL dy = dreal.y; REAL dz = dreal.z;

    REAL x = px + dx*t;
    REAL y = py + dy*t;
    REAL z = pz + dz*t;

    transMat.d = 0;
    transMat.h = 0;
    transMat.l = 0;
    transMat.m = 0;
    transMat.n = 0;
    transMat.o = 0;
    transMat.p = 1;

    point.normal = transMat.getInverse().getTranspose()*Vector4(2*x,2*y,2*z,0);
    return point;
}

Point MathSphere::getAB(DividedPoint point){
    Matrix4x4 transMat = point.prim.transMat;
    Matrix4x4 invTransMat = transMat.getInverse();
    vec4<REAL> preal = invTransMat*point.P;
    vec4<REAL> dreal = invTransMat*point.D;
    double t = point.t;

    REAL px = preal.x; REAL py = preal.y; REAL pz = preal.z;
    REAL dx = dreal.x; REAL dy = dreal.y; REAL dz = dreal.z;

    REAL x = px + dx*t;
    REAL y = py + dy*t;
    REAL z = pz + dz*t;

    double pi = 3.14159;
    double r = sqrt(x*x+z*z+y*y);
    double phi = asin(y/r);
    double v = phi/pi+.5;

    if(x==0){
        if(z>0){//90
            return Point(.25,v,0,1);
        } else{//270
            return Point(.75,v,0,1);
        }
    }

    if(z>0){
        if(x>0){
            return Point((atan(-z/x)+2*pi)/(2*pi),v,0,1);
        } else{
            return Point((atan(-z/x)+pi)/(2*pi),v,0,1);//.5->.75
        }
    } else if(z<0){
        if(x<0){
            return Point((atan(-z/x)+pi)/(2*pi),v,0,1);
        } else{
            return Point((atan(-z/x))/(2*pi),v,0,1);
        }
    } else{
        if(x<0){//180
            return Point(.5,v,0,1);
        } else{//0
            return Point(0,v,0,1);
        }
    }
}
