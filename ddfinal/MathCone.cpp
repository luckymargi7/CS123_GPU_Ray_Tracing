
#include "MathCone.h"
#include <iostream>
#include <CS123Common.h>

using namespace std;

MathCone::MathCone() : MathShape(){

}

MathCone::~MathCone(){
    //clean up
}

REAL MathCone::findIntersect(DividedPoint vector, Matrix4x4 transMat){

    Matrix4x4 invTransMat = transMat.getInverse();
    vec4<REAL> preal = invTransMat*vector.P;
    vec4<REAL> dreal = invTransMat*vector.D;

    REAL px = preal.x; REAL py = preal.y; REAL pz = preal.z;
    REAL dx = dreal.x; REAL dy = dreal.y; REAL dz = dreal.z;

    REAL finalt, currt;
    finalt = -1;

    REAL A = dx*dx+dz*dz-(1.0/4)*dy*dy;
    REAL B = 2*px*dx+2*pz*dz-(1.0/2)*py*dy+(1.0/4)*dy;
    REAL C = px*px+pz*pz-(1.0/4)*py*py+(1.0/4)*py-(1.0/16);
    REAL dis = B*B-4*A*C;

    //body
    if(dis>=0){
        currt = (-B+sqrt(dis))/(2*A);
        if(currt > .001 && (py+dy*currt)>=-.5 && (py+dy*currt)<=.5){
            if(finalt <0 || currt < finalt){
                finalt = currt;
            }
        }

        currt = (-B-sqrt(dis))/(2*A);
        if(currt > .001 && (py+dy*currt)>=-.5 && (py+dy*currt)<=.5){
            if(finalt <0 || currt < finalt){
                finalt = currt;
            }
        }
    }

    //bottom (N={0,-1,0} p={0,-.5,0}
    currt = -(.5+py)/dy;
    if(((px+dx*currt)*(px+dx*currt)+(pz+dz*currt)*(pz+dz*currt)) <= (.5*.5)&& currt >.001 ){
        if(finalt > currt || finalt == -1){
            finalt = currt;
        }
    }

    return finalt;

}

DividedPoint MathCone::findNormal(DividedPoint point){

    Matrix4x4 transMat = point.prim.transMat;
    Matrix4x4 invTransMat = transMat.getInverse();
    vec4<REAL> preal = invTransMat*point.P;
    vec4<REAL> dreal = invTransMat*point.D;
    double t = point.t;

    REAL px = preal.x; REAL py = preal.y; REAL pz = preal.z;
    REAL dx = dreal.x; REAL dy = dreal.y; REAL dz = dreal.z;

    transMat.d = 0;
    transMat.h = 0;
    transMat.l = 0;
    transMat.m = 0;
    transMat.n = 0;
    transMat.o = 0;
    transMat.p = 1;

    if(py+dy*t < -.49999){
        point.normal = transMat.getInverse().getTranspose()*Vector4(0,-1,0,0);
        return point;
    } else{
        point.normal = transMat.getInverse().getTranspose()*Vector4(2*(px+dx*t), (1.0/4.0)-(1.0/2.0)*(py+dy*t), 2*(pz+dz*t), 0);
        return point;
    }

}

Point MathCone::getAB(DividedPoint point){
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

    if(py+dy*t < -.49999){
        return Point(x+.5,1-(z+.5),0,1);
    } else{
        double pi = 3.14159;
        if(x==0){
            if(z>0){//90
                return Point(.25,1-(y+.5),0,1);
            } else{//270
                return Point(.75,1-(y+.5),0,1);
            }
        }

        if(z>0){
            if(x>0){
                return Point((atan(-z/x)+2*pi)/(2*pi),1-(y+.5),0,1);
            } else{
                return Point((atan(-z/x)+pi)/(2*pi),1-(y+.5),0,1);//.5->.75
            }
        } else if(z<0){
            if(x<0){
                return Point((atan(-z/x)+pi)/(2*pi),1-(y+.5),0,1);
            } else{
                return Point((atan(-z/x))/(2*pi),1-(y+.5),0,1);
            }
        } else{
            if(x<0){//180
                return Point(.5,1-(y+.5),0,1);
            } else{//0
                return Point(0,1-(y+.5),0,1);
            }
        }
    }
}
