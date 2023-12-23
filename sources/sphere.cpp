#include "../headers/sphere.h"
#include <random>
Sphere::Sphere(GridGeometry* _terrain,QVector3D CenterPosition,float radius ,int numSegments,SWEFluid* _fluid) : indexBuf(QOpenGLBuffer::IndexBuffer){
    initializeOpenGLFunctions();
    terrain = _terrain;
    fluid = _fluid;
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
    if (vertices.empty()) {
        return;
    }

    QVector3D& position = vertices[0].position;
    QVector3D& velocity = vertices[0].velocity;
    QVector3D& accumulatedForce = vertices[0].accumulatedForce;

    // Calculate gravitational force
    QVector3D gravityForce = QVector3D(0.0f, gravity, 0.0f);

    // Update velocity and accumulated force based on gravity
    accumulatedForce += gravityForce * dt;
    velocity += accumulatedForce * dt;

    // Calculate the new position
    QVector3D newPosition = position + velocity * dt;

    // Get ground and water heights
    float groundHeight = terrain->getHeightAtPosition(newPosition.x(), newPosition.z());
    float waterHeight = fluid->getWaterHeight(newPosition.x(), newPosition.z());

    // Check for water and ground interactions
    if (newPosition.y() < waterHeight && waterHeight > groundHeight) {
        handleBuoyancy(newPosition, velocity, accumulatedForce, waterHeight, dt);
    } else if (newPosition.y() < groundHeight) {
        handleGroundInteraction(newPosition, velocity, accumulatedForce, groundHeight, dt);
    }


    // Boundary checks for the terrain
    float maxX = terrain->gridWidth - 0.5f;
    float maxZ = terrain->gridDepth - 0.5f;
    float minX = 0 + 0.5f;
    float minZ = 0 + 0.5f;

    // Check and adjust for X boundary
    if (newPosition.x() > maxX) {
        newPosition.setX(maxX);
        velocity.setX(-velocity.x());
    } else if (newPosition.x() < minX) {
        newPosition.setX(minX);
        velocity.setX(-velocity.x());
    }

    // Check and adjust for Z boundary
    if (newPosition.z() > maxZ) {
        newPosition.setZ(maxZ);
        velocity.setZ(-velocity.z());
    } else if (newPosition.z() < minZ) {
        newPosition.setZ(minZ);
        velocity.setZ(-velocity.z());
    }

    // Update the particle's position
    position = newPosition;

    // Update position and vertices
    position = newPosition;
    for (int i = 1; i < vertices.size(); i++) {
        vertices[i].position = newPosition + vertices[i].offset;
    }

    // Buffer update
    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), static_cast<int>(vertices.size()) * sizeof(sphere_VertexData));
    arrayBuf.release();
    timevalue += dt;
}

void Sphere::handleBuoyancy(QVector3D& newPosition, QVector3D& velocity, QVector3D& accumulatedForce, float waterHeight, float dt) {

    float buoyancyCoefficient = 1.0f; // Adjust based on fluid density and sphere volume
    float waterDensity = 1000.0f; // Water density in kg/m^3

    // Calculate submerged volume (simplified example)
    float submergedVolume = 0.5f; //
    QVector3D buoyancyForce = QVector3D(0.0f, buoyancyCoefficient * submergedVolume * waterDensity * gravity, 0.0f);

    // Apply buoyancy force
    velocity += buoyancyForce * dt;
    accumulatedForce += buoyancyForce * dt;

    // Adjust position if below water surface
    if (newPosition.y() < waterHeight) {
        newPosition.setY(waterHeight );
    }
}

void Sphere::handleGroundInteraction(QVector3D& newPosition, QVector3D& velocity, QVector3D& accumulatedForce, float groundHeight, float dt) {
    float sphereRadius = 0.5f; // Sphere radius

    // Rebound force
    QVector3D reboundForce = QVector3D(0.0f, -velocity.y() * restitutionCoefficient, 0.0f);
    velocity += reboundForce;
    accumulatedForce -= reboundForce;

    // Update position to be just above the ground
    newPosition.setY(groundHeight + sphereRadius);


    QVector3D horizontalVelocity(velocity.x(), 0.0f, velocity.z());
    QVector3D frictionForce = -horizontalVelocity.normalized() * frictionCoefficient;
    velocity += frictionForce * dt;

    // Calculate terrain gradient and apply sliding force
    QVector2D terrainGradient2D = terrain->getGradientAtPosition(newPosition.x(), newPosition.z());
    QVector3D terrainGradient(terrainGradient2D.x(), 0.0f, terrainGradient2D.y());

    QVector3D slidingForce = QVector3D(terrainGradient.x(), 0.0f, terrainGradient.z()) * slidingCoefficient;
    velocity -= slidingForce * dt;
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
