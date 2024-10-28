#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class OrbitCamera
{
public:
    OrbitCamera(const glm::vec3& center, const glm::vec3& upVector, float radius, float minRadius, float azimuthAngle, float polarAngle)
        : center_(center)
        , upVector_(upVector)
        , radius_(radius)
        , _minRadius(minRadius)
        , azimuthAngle_(azimuthAngle)
        , polarAngle_(polarAngle)
    {
    }

    void rotateAzimuth(const float radians);
    void rotatePolar(const float radians);
    void zoom(const float by);

    void moveHorizontal(const float distance);
    void moveVertical(const float distance);
    void moveForBack(const float distance);


    glm::mat4 getViewMatrix() const;
    glm::vec3 getEye() const;
    glm::vec3 getCenter() const;
    glm::vec3 getViewPoint() const;
    glm::vec3 getUpVector() const;
    glm::vec3 getNormalizedViewVector() const;
    float getAzimuthAngle() const;
    float getPolarAngle() const;
    float getRadius() const;
    glm::vec3 getPosition();

private:
    glm::vec3 center_; // Center of the orbit camera sphere (the point upon which the camera looks)
    glm::vec3 upVector_; // Up vector of the camera
    float radius_; // Radius of the orbit camera sphere
    float _minRadius; // Minimal radius of the orbit camera sphere (cannot fall below this value)
    float azimuthAngle_; // Azimuth angle on the orbit camera sphere
    float polarAngle_; // Polar angle on the orbit camera sphere
};

void OrbitCamera::rotateAzimuth(const float radians)
{
    azimuthAngle_ += radians;

    // Keep azimuth angle within range <0..2PI) - it's not necessary, just to have it nicely output
    const auto fullCircle = 2.0f * 3.14159f;
    azimuthAngle_ = fmodf(azimuthAngle_, fullCircle);
    if (azimuthAngle_ < 0.0f) {
        azimuthAngle_ = fullCircle + azimuthAngle_;
    }
}

void OrbitCamera::rotatePolar(const float radians)
{
    polarAngle_ += radians;

    // Check if the angle hasn't exceeded quarter of a circle to prevent flip, add a bit of epsilon like 0.001 radians
    const auto polarCap = 3.14159f / 2.0f - 0.001f;
    if (polarAngle_ > polarCap) {
        polarAngle_ = polarCap;
    }

    if (polarAngle_ < -polarCap) {
        polarAngle_ = -polarCap;
    }
}

void OrbitCamera::zoom(const float by)
{
    radius_ -= by;
    if (radius_ < _minRadius) {
        radius_ = _minRadius;
    }
}

glm::vec3 OrbitCamera::getUpVector() const
{
    return upVector_;
}

glm::mat4 OrbitCamera::getViewMatrix() const
{
    const auto eye = getEye();
    return glm::lookAt(eye, eye + getNormalizedViewVector(), upVector_);
}

glm::vec3 OrbitCamera::getEye() const
{
    // Calculate sines / cosines of angles
    const auto sineAzimuth = sin(azimuthAngle_);
    const auto cosineAzimuth = cos(azimuthAngle_);
    const auto sinePolar = sin(polarAngle_);
    const auto cosinePolar = cos(polarAngle_);

    // Calculate eye position out of them
    const auto x = center_.x + radius_ * cosinePolar * cosineAzimuth;
    const auto y = center_.y + radius_ * sinePolar;
    const auto z = center_.z + radius_ * cosinePolar * sineAzimuth;

    return glm::vec3(x, y, z);
}

glm::vec3 OrbitCamera::getCenter() const
{

    return center_;
}

glm::vec3 OrbitCamera::getViewPoint() const
{
    return center_;
}

float OrbitCamera::getAzimuthAngle() const {
    return azimuthAngle_;
}
    
float OrbitCamera::getPolarAngle() const {
    return polarAngle_;
}

void OrbitCamera::moveHorizontal(const float distance)
{
        const glm::vec3 viewVector = getNormalizedViewVector();
    const glm::vec3 strafeVector = glm::normalize(glm::cross(viewVector, upVector_));
    center_ += strafeVector * distance;
}

void OrbitCamera::moveForBack(const float distance)
{
    glm::vec3 viewVector = getNormalizedViewVector();

    viewVector.y = 0.0f;

    glm::vec3 horizontalDirection = glm::normalize(viewVector);

    center_ += horizontalDirection * distance;
}

void OrbitCamera::moveVertical(const float distance)
{
    center_ += upVector_ * distance;
}

glm::vec3 OrbitCamera::getNormalizedViewVector() const
{
    return glm::normalize(getViewPoint() - getEye());
}

glm::vec3 OrbitCamera::getPosition() 
{
    glm::mat4 viewMatrix = glm::lookAt(getEye(), center_, upVector_);

    
    glm::vec4 eyeHomogeneous(getEye(), 1.0f); 
    glm::vec4 relativePosition = glm::inverse(viewMatrix) * eyeHomogeneous;
    glm::vec3 relativePosition3D = glm::vec3(relativePosition);

    return relativePosition3D;
    
    }
