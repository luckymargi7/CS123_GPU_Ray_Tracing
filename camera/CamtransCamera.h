/*!
   @file   CamtransCamera.h
   @author Ben Herila (ben@herila.net)
   @author Evan Wallace (evan.exe@gmail.com)
   @date   Fall 2010

   @brief  Contains definitions for an abstract virtual camera class that students
           will implement in the Camtrans assignment.
*/

#ifndef CAMTRANSCAMERA_H
#define CAMTRANSCAMERA_H

#include "math/CS123Algebra.h"
#include "Camera.h"
#include <math.h>

/**

 @class CamtransCamera

 @brief The perspective camera class students will implement in the Camtrans assignment.

 Part of the CS123 support code.

 @author  Evan Wallace (edwallac)
 @author  Ben Herila (bherila)
 @date    9/1/2010

*/
class CamtransCamera : public Camera
{
public:
    //! Initialize your camera.
    CamtransCamera();
    ~CamtransCamera();

    //! Sets the aspect ratio of this camera. Automatically called by the GUI when the window is resized.
    virtual void setAspectRatio(double aspectRatio);

    //! Returns the projection matrix given the current camera settings.
    virtual Matrix4x4 getProjectionMatrix() const;

    //! Returns the modelview matrix given the current camera settings.
    virtual Matrix4x4 getModelviewMatrix() const;

    //! Returns the viewingTransformation matrix given the current camera settings.
    virtual Matrix4x4 getViewingTransformation() const;

    //! Returns the current position of the camera.
    Vector4 getPosition() const;

    //! Returns the current 'look' vector for this camera.
    Vector4 getLook() const;

    //! Returns the current 'up' vector for this camera.
    Vector4 getUp() const;

    //! Returns the currently set aspect ratio.
    double getAspectRatio() const;

    //! Returns the currently set height angle.
    double getHeightAngle() const;

    //! Orients this camera's look vector with a current eye position, given look and up vectors.
    void orientLook(const Vector4 &eye, const Vector4 &look, const Vector4 &up);

    //! Sets the height angle of this camera.
    void setHeightAngle(double h);

    //! Translates the camera along a given vector.
    void translate(const Vector4 &v);

    //! Rotates the camera about the U axis by a specified number of degrees.
    void rotateU(double degrees);

    //! Rotates the camera about the V axis by a specified number of degrees.
    void rotateV(double degrees);

    //! Rotates the camera about the N axis by a specified number of degrees.
    void rotateN(double degrees);

    //! Sets the near and far clip planes for this camera.
    void setClip(double nearPlane, double farPlane);

private:
    void updateProjectionMatrix();
    void updateModelviewMatrix();
    void updateViewingTransformation();
    void updateUVW();
    double toRadians(double degrees);

    Matrix4x4 m_modelviewMatrix;
    Matrix4x4 m_projectionMatrix;
    Matrix4x4 m_viewingTransformation;
    double m_aspectRatio, m_heightAngle, m_widthAngle, m_nearPlane, m_farPlane;
    Vector4 m_position, m_look, m_up, m_camU, m_camV, m_camW;

};

#endif // CAMTRANSCAMERA_H
