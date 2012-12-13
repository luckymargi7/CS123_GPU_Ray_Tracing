/*!
   @file   CamtransCamera.cpp
   @author Ben Herila (ben@herila.net)
   @author Evan Wallace (evan.exe@gmail.com)
   @date   Fall 2010

   @brief  This is the perspective camera class you will need to fill in for the
   Camtrans assignment.  See the assignment handout for more details.

   For extra credit, you can also create an Orthographic camera. Create another
   class if you want to do that.

*/

#include "CamtransCamera.h"
//#include <Settings.h>
#include <qgl.h>

CamtransCamera::CamtransCamera()
{
    // @TODO: [CAMTRANS] Fill this in...
    m_aspectRatio = 1; // width/height
    m_nearPlane = 1;
    m_farPlane = 30;
    m_heightAngle = 60;
    m_position = Vector4(2.0, 2.0, 2.0,1);
    m_look = Vector4(-1.0, -1.0, -1.0, 0);
    m_up = Vector4(0.0, 1.0, 0.0, 0);

    updateUVW();
    updateModelviewMatrix();
    updateProjectionMatrix();
    updateViewingTransformation();
}

CamtransCamera::~CamtransCamera() {
    //as all the member variables are on a stack
    //there is nothing to delete.
}

void CamtransCamera::setAspectRatio(double a)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_aspectRatio = a;

    updateProjectionMatrix();
    updateViewingTransformation();
}

Matrix4x4 CamtransCamera::getProjectionMatrix() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_projectionMatrix;
}

Matrix4x4 CamtransCamera::getModelviewMatrix() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_modelviewMatrix;
}

Matrix4x4 CamtransCamera::getViewingTransformation() const
{
    return m_viewingTransformation;
}

Vector4 CamtransCamera::getPosition() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_position;
}

Vector4 CamtransCamera::getLook() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_look;
}

Vector4 CamtransCamera::getUp() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_up;
}

double CamtransCamera::getAspectRatio() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_aspectRatio;
}

double CamtransCamera::getHeightAngle() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_heightAngle;
}

void CamtransCamera::orientLook(const Vector4 &eye, const Vector4 &look, const Vector4 &up)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_position = eye;

    //make sure that m_position is always homogenized
    if (m_position.w == 0.0)
        m_position.w = 1.0;
    else
        m_position = m_position/m_position.w;

    m_look = look;
    m_up = up;

    m_look.normalize();
    m_up.normalize();

    updateUVW();
    updateModelviewMatrix();
    updateViewingTransformation();
}

void CamtransCamera::setHeightAngle(double h)
{
    // @TODO: [CAMTRANS] Fill this in...
    if (h < 180 && h > 0)
        m_heightAngle = h;
    else if (h >= 180)
        m_heightAngle = 179;
    else if (h <= 0)
        m_heightAngle = 1;

    updateProjectionMatrix();
    updateViewingTransformation();
}

void CamtransCamera::translate(const Vector4 &v)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_position = getTransMat(v)*m_position;

    updateModelviewMatrix();
    updateViewingTransformation();
}

void CamtransCamera::rotateU(double degrees)
{
    // @TODO: [CAMTRANS] Fill this in...
    double radians = toRadians(degrees);
    Matrix4x4 rotMat = getRotMat(m_position, m_camU, -1.0*radians);

    m_look = rotMat*m_look;
    m_up = rotMat*m_up;

    updateUVW();
    updateModelviewMatrix();
    updateViewingTransformation();
}

void CamtransCamera::rotateV(double degrees)
{
    // @TODO: [CAMTRANS] Fill this in...
    double radians = toRadians(degrees);
    Matrix4x4 rotMat = getRotMat(m_position, m_camV, radians);

    m_look = rotMat*m_look;
    m_up = rotMat*m_up;

    updateUVW();
    updateModelviewMatrix();
    updateViewingTransformation();
}

void CamtransCamera::rotateN(double degrees)
{
    // @TODO: [CAMTRANS] Fill this in...
    double radians = toRadians(degrees);
    Matrix4x4 rotMat = getRotMat(m_position, m_camW, radians);

    m_look = rotMat*m_look;
    m_up = rotMat*m_up;

    updateUVW();
    updateModelviewMatrix();
    updateViewingTransformation();
}

void CamtransCamera::setClip(double nearPlane, double farPlane)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;

    updateProjectionMatrix();
    updateViewingTransformation();
}

double CamtransCamera::toRadians(double degrees)
{
    return degrees*M_PI/180.0f;
}

void CamtransCamera::updateModelviewMatrix() {
    REAL dataM3 [16] = { m_camU.x, m_camU.y, m_camU.z, 0,
                         m_camV.x, m_camV.y, m_camV.z, 0,
                         m_camW.x, m_camW.y, m_camW.z, 0,
                         0, 0, 0, 1};

    Matrix4x4 M3 = Matrix4x4(dataM3);
    Matrix4x4 M4 = getInvTransMat(m_position);

    m_modelviewMatrix = M3*M4;
}

void CamtransCamera::updateProjectionMatrix() {
    REAL c = -1.0*m_nearPlane/m_farPlane;
    REAL dataM1 [16] = { 1, 0, 0, 0,
                         0, 1, 0, 0,
                         0, 0, -1.0/(c+1), c/(c+1),
                         0, 0, -1, 0};
    Matrix4x4 M1 = Matrix4x4(dataM1);

    REAL tanHeightAngle = tan(toRadians(m_heightAngle)/2);
    REAL tanWidthAngle = m_aspectRatio * tanHeightAngle;
    Vector4 scale = Vector4(1.0/(tanWidthAngle*m_farPlane), 1.0/(tanHeightAngle*m_farPlane), 1/m_farPlane, 1);
    Matrix4x4 M2 = getScaleMat(scale);

    m_projectionMatrix = M1*M2;
}

void CamtransCamera::updateViewingTransformation() {
    REAL tanHeightAngle = tan(toRadians(m_heightAngle)/2);
    REAL tanWidthAngle = m_aspectRatio * tanHeightAngle;
    Vector4 scale = Vector4(1.0/(tanWidthAngle*m_farPlane), 1.0/(tanHeightAngle*m_farPlane), 1/m_farPlane, 1);
    Matrix4x4 M2 = getScaleMat(scale);

    REAL dataM3 [16] = { m_camU.x, m_camU.y, m_camU.z, 0,
                         m_camV.x, m_camV.y, m_camV.z, 0,
                         m_camW.x, m_camW.y, m_camW.z, 0,
                         0, 0, 0, 1};

    Matrix4x4 M3 = Matrix4x4(dataM3);
    Matrix4x4 M4 = getInvTransMat(m_position);

    m_viewingTransformation = M4.getInverse()*M3.getInverse()*M2.getInverse();
}

void CamtransCamera::updateUVW() {
    m_camW = (m_look*-1.0);
    m_camW.normalize();
    m_camV = m_up - (m_up.dot(m_camW))*m_camW;
    m_camV.normalize();
    m_camU = m_camV.cross(m_camW);
    m_camU.normalize();

    m_up = m_camV; //set m_up as the v-axis
}

