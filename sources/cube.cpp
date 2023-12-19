#include "../headers/cube.h"
#include <random>
Cube::Cube(GridGeometry* _terrain)
    {
    initializeOpenGLFunctions();
    terrain =_terrain;
    arrayBuf.create();
}

Cube::~Cube() {
    arrayBuf.destroy();
}

void Cube::createPointGeometry(QVector3D InstantiatedPosition ) {

    vertices.push_back( {InstantiatedPosition,
        {0, 0 ,0},
        {0, 0 ,0},
    });


    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), static_cast<int>(vertices.size()) * sizeof(cube_VertexData));
    arrayBuf.release();
}


void Cube::UpdateParticles(float dt) {
    for (int i = 0; i < vertices.size(); i++) {
        QVector3D& position = vertices[i].position;
        QVector3D& velocity = vertices[i].velocity; 
        QVector3D& accumulatedForce = vertices[i].accumulatedForce; 

        float groundOffset = 0.5f;

        // Calculate gravitational force
        QVector3D gravityForce = QVector3D(0.0f, gravity, 0.0f);

        // Update the velocity and accumulated force based on the gravitational force
        accumulatedForce += gravityForce * dt;
        velocity += accumulatedForce * dt;

        // Calculate the new position
        QVector3D newPosition = position + velocity * dt;

        // Check if the new position is below the ground
        float groundHeight = terrain->getHeightAtPosition(newPosition.x(), newPosition.z());
        if ((newPosition.y() - groundOffset) < groundHeight) {
            // Calculate the rebound force (you can adjust this value as needed)
            QVector3D reboundForce = QVector3D(0.0f, -velocity.y() * restitutionCoefficient, 0.0f);

            // Apply the rebound force and reduce accumulated force
            velocity += reboundForce;
            accumulatedForce -= reboundForce;

            // Update the position to be just above the ground
            newPosition.setY(groundHeight + groundOffset); // Add a small offset to prevent particles from getting stuck in the ground

            QVector3D horizontalVelocity(velocity.x(), 0.0f, velocity.z());
            QVector3D frictionForce = -horizontalVelocity.normalized() * frictionCoefficient;
            velocity += frictionForce * dt;

            // Calculate terrain gradient and apply sliding force
            QVector2D terrainGradient2D = terrain->getGradientAtPosition(newPosition.x(), newPosition.z());
            QVector3D terrainGradient(terrainGradient2D.x(), 0.0f, terrainGradient2D.y());

            QVector3D slidingForce = QVector3D(terrainGradient.x(), 0.0f, terrainGradient.z()) * slidingCoefficient;
            velocity -= slidingForce * dt;
        }

        // Update the particle's position
        position = newPosition;
    }

    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), static_cast<int>(vertices.size()) * sizeof(cube_VertexData));
    arrayBuf.release();
}




void Cube::render(QOpenGLShaderProgram* program) {
    arrayBuf.bind();

    int vertexLocation = program->attributeLocation("vertex");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(QVector3D));

    glDrawArrays(GL_POINTS, 0, vertices.size()); // Render all points

    arrayBuf.release();
}
