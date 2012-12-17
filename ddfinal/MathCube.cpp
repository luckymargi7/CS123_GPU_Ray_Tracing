
#include "MathCube.h"
#include <iostream>
#include <CS123Common.h>
#include <assert.h>

using namespace std;

MathCube::MathCube() : MathShape(){
}

MathCube::~MathCube(){
    //clean up
}

double MathCube::findIntersect(DividedPoint vector, Matrix4x4 transMat){

    Matrix4x4 invTransMat = transMat.getInverse();
    vec4<REAL> preal = invTransMat*vector.P;
    vec4<REAL> dreal = invTransMat*vector.D;

    double px = preal.x; double py = preal.y; double pz = preal.z;
    double dx = dreal.x; double dy = dreal.y; double dz = dreal.z;

    double finalt, currt;
    finalt = -1;

    //top face ( N = {0,1,0} p = {0, .5, 0} )
    currt = (.5 - py)/dy;
    if((px+dx*currt <= .5 && px+dx*currt >= -.5)&&(pz+dz*currt < .5 && pz+dz*currt >-.5)){
        if(currt > .001 && (finalt > currt || finalt == -1)){
            finalt = currt;
        }
    }

    //bottom face ( N = {0,-1,0} p = {0,-.5, 0} )
    currt = -(.5+py)/dy;
    if((px+dx*currt <= .5 && px+dx*currt >= -.5)&&(pz+dz*currt <= .5 && pz+dz*currt >=-.5)){
        if(currt > .001 && (finalt > currt || finalt == -1)){
            finalt = currt;
        }
    }

    //right face ( N = {1,0,0} p = {.5,0,0} )
    currt = (.5-px)/dx;
    if((py+dy*currt <= .5 && py+dy*currt >= -.5)&&(pz+dz*currt <= .5 && pz+dz*currt >=-.5)){
        if(currt > .001 && (finalt > currt || finalt == -1)){
            finalt = currt;
        }
    }

    //left face ( N = {-1,0,0} p = {-.5,0,0} )
    currt = -(.5+px)/dx;
    if((py+dy*currt <= .5 && py+dy*currt >= -.5)&&(pz+dz*currt <= .5 && pz+dz*currt >=-.5)){
        if(currt > .001 && (finalt > currt || finalt == -1)){
            finalt = currt;
        }
    }

    //front face ( N = {0,0,1} p = {0,0,.5} )
    currt = (.5-pz)/dz;
    if((py+dy*currt <= .5 && py+dy*currt >= -.5)&&(px+dx*currt <= .5 && px+dx*currt >=-.5)){
        if(currt > .001 && (finalt > currt || finalt == -1)){
            finalt = currt;
        }
    }

    //back face ( N = {0,0,-1} p = {0,0,-.5} )
    currt = -(.5+pz)/dz;
    if((py+dy*currt <= .5 && py+dy*currt >= -.5)&&(px+dx*currt <= .5 && px+dx*currt >=-.5)){
        if(currt > .001 && (finalt > currt || finalt == -1)){
            finalt = currt;
        }
    }

    return finalt;

}

DividedPoint MathCube::findNormal(DividedPoint point){

    Matrix4x4 transMat = point.prim.transMat;
    Matrix4x4 invTransMat = transMat.getInverse();
    vec4<REAL> preal = invTransMat*point.P;
    vec4<REAL> dreal = invTransMat*point.D;
    double t = point.t;

    REAL px = preal.x; REAL py = preal.y; REAL pz = preal.z;
    REAL dx = dreal.x; REAL dy = dreal.y; REAL dz = dreal.z;

    REAL pdtx = px + dx*t;
    REAL pdty = py + dy*t;
    REAL pdtz = pz + dz*t;

    transMat.d = 0;
    transMat.h = 0;
    transMat.l = 0;
    transMat.m = 0;
    transMat.n = 0;
    transMat.o = 0;
    transMat.p = 1;

    if(pdtx > .49999){
        point.normal = transMat.getInverse().getTranspose()*Vector4(1,0,0,0);
        return point;
    } else if(pdtx<-.49999){
        point.normal = transMat.getInverse().getTranspose()*Vector4(-1,0,0,0);
        return point;
    } else if(pdty>.49999){
        point.normal = transMat.getInverse().getTranspose()*Vector4(0,1,0,0);
        return point;
    } else if(pdty<-.49999){
        point.normal = transMat.getInverse().getTranspose()*Vector4(0,-1,0,0);
        return point;
    } else if(pdtz <0){
        point.normal = transMat.getInverse().getTranspose()*Vector4(0,0,-1,0);
        return point;
    } else{
        point.normal = transMat.getInverse().getTranspose()*Vector4(0,0,1,0);
        return point;
    }

}

Point MathCube::getAB(DividedPoint point){
    Matrix4x4 transMat = point.prim.transMat;
    Matrix4x4 invTransMat = transMat.getInverse();
    vec4<REAL> preal = invTransMat*point.P;
    vec4<REAL> dreal = invTransMat*point.D;
    double t = point.t;

    REAL px = preal.x; REAL py = preal.y; REAL pz = preal.z;
    REAL dx = dreal.x; REAL dy = dreal.y; REAL dz = dreal.z;

    REAL pdtx = px + dx*t;//intersection point in object space!
    REAL pdty = py + dy*t;
    REAL pdtz = pz + dz*t;

    if(pdtx > .49999){//y^ z<
        return Point(1-(pdtz+.5),1-(pdty+.5),0,1);
    } else if(pdtx<-.49999){//y^ z>
        return Point(pdtz+.5,1-(pdty+.5),0,1);
    } else if(pdty>.49999){//x> z\/
        return Point(pdtx+.5,pdtz+.5,0,1);
    } else if(pdty<-.49999){//x> z^
        return Point(pdtx+.5,1-(pdtz+.5),0,1);
    } else if(pdtz <0){//<x y^
        return Point(1-(pdtx+.5),1-(pdty+.5),0,1);
    } else{//x> y^
        return Point(pdtx+.5,1-(pdty+.5),0,1);
    }
}


