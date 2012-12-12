/*!
   @file   CS123Matrix.cpp
   @author Travis Fischer (fisch0920@gmail.com)
   @date   Fall 2008
   
   @brief
      Provides basic functionality for a templated, arbitrarily-sized matrix.
      You will need to fill this file in for the Camtrans assignment.

**/

#include "CS123Algebra.h"
#include <iostream>

//@name Routines which generate specific-purpose transformation matrices
//@{---------------------------------------------------------------------
// @returns the scale matrix described by the vector
Matrix4x4 getScaleMat(const Vector4 &v) {
    // [CAMTRANS] Fill this in...

    Matrix4x4 xrm = Matrix4x4::identity();

    xrm.a = v.x; xrm.b = 0.0; xrm.c = 0.0; xrm.d = 0.0;
    xrm.e = 0.0; xrm.f = v.y; xrm.g = 0.0; xrm.h = 0.0;
    xrm.i = 0.0; xrm.j = 0.0; xrm.k = v.z; xrm.l = 0.0;
    xrm.m = 0.0; xrm.n = 0.0; xrm.o = 0.0; xrm.p = 1.0;

    return xrm;

}

// @returns the translation matrix described by the vector
Matrix4x4 getTransMat(const Vector4 &v) {
    // [CAMTRANS] Fill this in...

    Matrix4x4 xrm = Matrix4x4::identity();

    xrm.a = 1.0; xrm.b = 0.0; xrm.c = 0.0; xrm.d = v.x;
    xrm.e = 0.0; xrm.f = 1.0; xrm.g = 0.0; xrm.h = v.y;
    xrm.i = 0.0; xrm.j = 0.0; xrm.k = 1.0; xrm.l = v.z;
    xrm.m = 0.0; xrm.n = 0.0; xrm.o = 0.0; xrm.p = 1.0;

    return xrm;

}

// @returns the rotation matrix about the x axis by the specified angle
Matrix4x4 getRotXMat (const REAL radians) {
    // [CAMTRANS] Fill this in...
    REAL cosval = cos(radians);
    REAL sinval = sin(radians);

    Matrix4x4 xrm = Matrix4x4::identity();

    xrm.a = 1.0; xrm.b = 0.0; xrm.c = 0.0; xrm.d = 0.0;
    xrm.e = 0.0; xrm.f = cosval; xrm.g = -sinval; xrm.h = 0.0;
    xrm.i = 0.0; xrm.j = sinval; xrm.k = cosval; xrm.l = 0.0;
    xrm.m = 0.0; xrm.n = 0.0; xrm.o = 0.0; xrm.p = 1.0;

    return xrm;

}

// @returns the rotation matrix about the y axis by the specified angle
Matrix4x4 getRotYMat (const REAL radians) {
    // [CAMTRANS] Fill this in...
    REAL cosval = cos(radians);
    REAL sinval = sin(radians);

    Matrix4x4 xrm = Matrix4x4::identity();

    xrm.a = cosval; xrm.b = 0.0; xrm.c = sinval; xrm.d = 0.0;
    xrm.e = 0.0; xrm.f = 1.0; xrm.g = 0.0; xrm.h = 0.0;
    xrm.i = -sinval; xrm.j = 0.0; xrm.k = cosval; xrm.l = 0.0;
    xrm.m = 0.0; xrm.n = 0.0; xrm.o = 0.0; xrm.p = 1.0;

    return xrm;

}

// @returns the rotation matrix about the z axis by the specified angle
Matrix4x4 getRotZMat (const REAL radians) {
    // [CAMTRANS] Fill this in...
    REAL cosval = cos(radians);
    REAL sinval = sin(radians);

    Matrix4x4 xrm = Matrix4x4::identity();

    xrm.a = cosval; xrm.b = -sinval; xrm.c = 0.0; xrm.d = 0.0;
    xrm.e = sinval; xrm.f = cosval; xrm.g = 0.0; xrm.h = 0.0;
    xrm.i = 0.0; xrm.j = 0.0; xrm.k = 1.0; xrm.l = 0.0;
    xrm.m = 0.0; xrm.n = 0.0; xrm.o = 0.0; xrm.p = 1.0;

    return xrm;

}

// @returns the rotation matrix around the vector and point by the specified angle
Matrix4x4 getRotMat  (const Vector4 &p, const Vector4 &v, const REAL a) {

    // [CAMTRANS] Fill this in...
    REAL theta = atan2(v.z,v.x);
    REAL lambda = a;
    REAL phi = -atan2(v.y, sqrt(v.x*v.x+v.z*v.z));

    Matrix4x4 To = getInvTransMat(p);// translate to origin

    Matrix4x4 M1 = getRotYMat(theta);
    Matrix4x4 M2 = getRotZMat(phi);
    Matrix4x4 M3 = getRotXMat(lambda);

    Matrix4x4 M1inv = getInvRotYMat(theta);
    Matrix4x4 M2inv = getInvRotZMat(phi);

    Matrix4x4 M = M1inv*M2inv*M3*M2*M1;// full rotation matrix

    Matrix4x4 ToInv = getTransMat(p);//translate back to point

    Matrix4x4 finalmat =  ToInv*M*To;//translate to origin, rotate, and then translate back to point

    return finalmat;

}


// @returns the inverse scale matrix described by the vector
Matrix4x4 getInvScaleMat(const Vector4 &v) {
    // [CAMTRANS] Fill this in...

    Matrix4x4 xrm = Matrix4x4::identity();

    xrm.a = 1.0/v.x; xrm.b = 0.0; xrm.c = 0.0; xrm.d = 0.0;
    xrm.e = 0.0; xrm.f = 1.0/v.y; xrm.g = 0.0; xrm.h = 0.0;
    xrm.i = 0.0; xrm.j = 0.0; xrm.k = 1.0/v.z; xrm.l = 0.0;
    xrm.m = 0.0; xrm.n = 0.0; xrm.o = 0.0; xrm.p = 1.0;

    return xrm;

}

// @returns the inverse translation matrix described by the vector
Matrix4x4 getInvTransMat(const Vector4 &v) {
    // [CAMTRANS] Fill this in...

    Matrix4x4 xrm = Matrix4x4::identity();

    xrm.a = 1.0; xrm.b = 0.0; xrm.c = 0.0; xrm.d = -v.x;
    xrm.e = 0.0; xrm.f = 1.0; xrm.g = 0.0; xrm.h = -v.y;
    xrm.i = 0.0; xrm.j = 0.0; xrm.k = 1.0; xrm.l = -v.z;
    xrm.m = 0.0; xrm.n = 0.0; xrm.o = 0.0; xrm.p = 1.0;

    return xrm;

}

// @returns the inverse rotation matrix about the x axis by the specified angle
Matrix4x4 getInvRotXMat (const REAL radians) {
    // [CAMTRANS] Fill this in...
    REAL cosval = cos(radians);
    REAL sinval = sin(radians);

    Matrix4x4 xrm = Matrix4x4::identity();

    xrm.a = 1.0; xrm.b = 0.0; xrm.c = 0.0; xrm.d = 0.0;
    xrm.e = 0.0; xrm.f = cosval; xrm.g = sinval; xrm.h = 0.0;
    xrm.i = 0.0; xrm.j = -sinval; xrm.k = cosval; xrm.l = 0.0;
    xrm.m = 0.0; xrm.n = 0.0; xrm.o = 0.0; xrm.p = 1.0;

    return xrm;

}

// @returns the inverse rotation matrix about the y axis by the specified angle
Matrix4x4 getInvRotYMat (const REAL radians) {
    // [CAMTRANS] Fill this in...
    REAL cosval = cos(radians);
    REAL sinval = sin(radians);

    Matrix4x4 xrm = Matrix4x4::identity();

    xrm.a = cosval; xrm.b = 0.0; xrm.c = -sinval; xrm.d = 0.0;
    xrm.e = 0.0; xrm.f = 1.0; xrm.g = 0.0; xrm.h = 0.0;
    xrm.i = sinval; xrm.j = 0.0; xrm.k = cosval; xrm.l = 0.0;
    xrm.m = 0.0; xrm.n = 0.0; xrm.o = 0.0; xrm.p = 1.0;

    return xrm;

}

// @returns the inverse rotation matrix about the z axis by the specified angle
Matrix4x4 getInvRotZMat (const REAL radians) {
    // [CAMTRANS] Fill this in...
    REAL cosval = cos(radians);
    REAL sinval = sin(radians);

    Matrix4x4 xrm = Matrix4x4::identity();

    xrm.a = cosval; xrm.b = sinval; xrm.c = 0.0; xrm.d = 0.0;
    xrm.e = -sinval; xrm.f = cosval; xrm.g = 0.0; xrm.h = 0.0;
    xrm.i = 0.0; xrm.j = 0.0; xrm.k = 1.0; xrm.l = 0.0;
    xrm.m = 0.0; xrm.n = 0.0; xrm.o = 0.0; xrm.p = 1.0;

    return xrm;

}

// @returns the inverse rotation matrix around the vector and point by the specified angle
Matrix4x4 getInvRotMat  (const Vector4 &p, const Vector4 &v, const REAL a) {
    // [CAMTRANS] Fill this in...

    return getRotMat(p, v, -a);

}


//@}---------------------------------------------------------------------

