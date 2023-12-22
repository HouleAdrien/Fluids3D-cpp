#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QtMath>

class Camera {
public:
    Camera(QVector3D position = QVector3D(0.0f, 0.0f, 0.0f),
           QVector3D up = QVector3D(0.0f, 1.0f, 0.0f),
           QVector3D front = QVector3D(0.0f, 0.0f,1.0f));

    QMatrix4x4 GetViewMatrix() const;
    QVector3D Position;
    void ProcessKeyboard(int key, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    void InvertPitch();
    void updateCameraVectors();
    QVector3D Front;
    QVector3D Up;
    QVector3D Right;
    QVector3D WorldUp;
private:

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;

};

#endif // CAMERA_H
