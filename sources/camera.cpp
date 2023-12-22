#include "../headers/camera.h"

Camera::Camera(QVector3D position, QVector3D up, QVector3D front)
    : Position(position), Front(front), WorldUp(up), Yaw(-90.0f), Pitch(0.0f),
    MovementSpeed(2.5f), MouseSensitivity(0.1f) {
    updateCameraVectors();
}

QMatrix4x4 Camera::GetViewMatrix() const {
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(Position, Position + Front, Up);
    return viewMatrix;
}


void Camera::ProcessKeyboard(int key, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (key == Qt::Key_Up)
        Position += Front * velocity;
    if (key == Qt::Key_Down)
        Position -= Front * velocity;
    if (key == Qt::Key_Left)
        Position -= Right * velocity;
    if (key == Qt::Key_Right)
        Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    QVector3D front;
    front.setX(cos(qDegreesToRadians(Yaw)) * cos(qDegreesToRadians(Pitch)));
    front.setY(sin(qDegreesToRadians(Pitch)));
    front.setZ(sin(qDegreesToRadians(Yaw)) * cos(qDegreesToRadians(Pitch)));
    Front = front.normalized();
    Right = QVector3D::crossProduct(Front, WorldUp).normalized();
    Up = QVector3D::crossProduct(Right, Front).normalized();
}

void Camera::InvertPitch() {
    Pitch = -Pitch;
    updateCameraVectors();
}
