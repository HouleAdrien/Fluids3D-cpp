#include "../headers/sphere.h"
#include <random>
Sphere::Sphere(GridGeometry* _terrain,QVector3D CenterPosition,float radius ,int numSegments) : indexBuf(QOpenGLBuffer::IndexBuffer){
    initializeOpenGLFunctions();
    terrain = _terrain;
    arrayBuf.create();
    indexBuf.create(); // Create an index buffer
    createSphereGeometry( CenterPosition,  radius,  numSegments);
}


Sphere::~Sphere() {
    arrayBuf.destroy();
    indexBuf.destroy();
}

void Sphere::createSphereGeometry(QVector3D CenterPosition, float radius, int numSegments) {
    vertices.clear(); // Clear existing vertices
    QVector<GLuint> indices; // Add this line to store the indices

    // Create vertices for the sphere
    for (int i = 0; i <= numSegments; ++i) {
        float phi = 2 * M_PI * i / numSegments; // Azimuthal angle
        for (int j = 0; j <= numSegments; ++j) {
            float theta = M_PI * j / numSegments; // Polar angle

            // Calculate the position of the vertex in spherical coordinates
            float x = radius * sin(theta) * cos(phi);
            float y = radius * sin(theta) * sin(phi);
            float z = radius * cos(theta);

            // Translate the spherical coordinates to the center position
            QVector3D position = CenterPosition + QVector3D(x, y, z);

            // Add the vertex to the list
            vertices.push_back({position, {0, 0, 0}, {0, 0, 0}, {x, y, z}});
        }
    }

    // Generate triangles based on the vertices

    for (int i = 0; i < numSegments; ++i) {
        for (int j = 0; j < numSegments; ++j) {
            int p0 = i * (numSegments + 1) + j;
            int p1 = p0 + 1;
            int p2 = (i + 1) * (numSegments + 1) + j;
            int p3 = p2 + 1;

            // Add two triangles for each quad
            indices.push_back(p0);
            indices.push_back(p1);
            indices.push_back(p2);

            indices.push_back(p2);
            indices.push_back(p1);
            indices.push_back(p3);
        }
    }

    // Allocate the vertex and index data to the buffer
    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), static_cast<int>(vertices.size()) * sizeof(sphere_VertexData));
    indexBuf.bind();
    indexBuf.allocate(indices.data(), static_cast<int>(indices.size()) * sizeof(GLuint));
    arrayBuf.release();
    indexBuf.release();
}



void Sphere::UpdateParticles(float dt) {
    if(vertices.empty()){
        return;
    }
    QVector3D& position = vertices[0].position;
    QVector3D& velocity = vertices[0].velocity;
    QVector3D& accumulatedForce = vertices[0].accumulatedForce;

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


    for(int i = 1; i < vertices.size() ; i++){
        vertices[i].position = newPosition + vertices[i].offset;
    }


    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), static_cast<int>(vertices.size()) * sizeof(sphere_VertexData));
    arrayBuf.release();
    timevalue += dt;
}

void Sphere::render(QOpenGLShaderProgram* program) {
    arrayBuf.bind();
    indexBuf.bind(); // Bind the index buffer

    int vertexLocation = program->attributeLocation("vertex");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offsetof(sphere_VertexData, position), 3, sizeof(sphere_VertexData));

    int velocityLocation = program->attributeLocation("velocity");
    program->enableAttributeArray(velocityLocation);
    program->setAttributeBuffer(velocityLocation, GL_FLOAT, offsetof(sphere_VertexData, velocity), 3, sizeof(sphere_VertexData));

    int accumulatedForceLocation = program->attributeLocation("accumulatedForce");
    program->enableAttributeArray(accumulatedForceLocation);
    program->setAttributeBuffer(accumulatedForceLocation, GL_FLOAT, offsetof(sphere_VertexData, accumulatedForce), 3, sizeof(sphere_VertexData));

    int offsetLocation = program->attributeLocation("offset");
    program->enableAttributeArray(offsetLocation);
    program->setAttributeBuffer(offsetLocation, GL_FLOAT, offsetof(sphere_VertexData, offset), 3, sizeof(sphere_VertexData));

    int timeLocation = program->uniformLocation("time");

    program->setUniformValue(timeLocation, timevalue);


    glDrawElements(GL_TRIANGLES, static_cast<int>(indexBuf.size() / sizeof(GLuint)), GL_UNSIGNED_INT, nullptr);

    indexBuf.release();
    arrayBuf.release();
}
