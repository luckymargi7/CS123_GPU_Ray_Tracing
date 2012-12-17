
#include "MathShape.h"
#include <iostream>
#include <CS123Common.h>

using namespace std;

MathShape::MathShape(){
}

MathShape::~MathShape(){
    //clean here
}

void MathShape::printMat(Matrix4x4 mat){

    cout << mat.a << " "<<mat.b<< " "<<mat.c << " " << mat.d<<"\n"
            << mat.e << " " << mat.f << " "<< mat.g << " " <<mat.h<<"\n"
            <<mat.i<< " " << mat.j<< " " << mat.k<<" "<< mat.l<<"\n"
            <<mat.m << " " <<mat.n<< " "<<mat.o<<" " << mat.p<<"\n\n"<<flush;
}
