#include <algorithm>
#include <iostream>
#include <fstream>
#include <GL/glut.h>
#include <cmath>
#include <random>

#include <vector>

#include "parallel_for.h"

// inspired by C++2's Trevors Lab

class Vector2 {
public:
    float x;
    float y;

    Vector2(float x1, float y1): x(x1),y(y1) {}
    Vector2():x(0), y(0) {}

    Vector2 operator+(const Vector2 &other) {
        return Vector2(other.x + x, other.y + y);
    }

    Vector2 operator-(const Vector2 &other) {
        return Vector2(other.x - x, other.y - y);
    }
    Vector2 operator-() const {
        return Vector2(-x, -y);
    }

    Vector2 operator*(float factor) const {
        return Vector2(x * factor, y * factor);
    }
    Vector2 operator/(float factor) const {
        return Vector2(x / factor, y / factor);
    }

    bool operator==(const Vector2 &other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Vector2 &other) const {
        return !(x == other.x && y == other.y);
    }


    float operator*(const Vector2 &other) const {
        return other.x * x + other.y * y;
    }


    float magnitude() const { return std::sqrt(x * x + y * y); }


    Vector2 normalized() const {
        float m = magnitude();
        if (m == 0) {
            return Vector2(0.0f, 0.0f);
        } else {
            return Vector2(x / m, y / m);
        }
    }

};

// VARS //

int screenWidth, screenHeight;
float boundX, boundY;

float deltaTime = 16 * pow(10, -3);

// init

int numParticles = 500;
float particleSpacing = 5.0f;

// particle properties

std::vector<Vector2> positions;
std::vector<Vector2> predictedPositions;
std::vector<Vector2> velocities;
std::vector<float> densities;
float radius = 5.0f;
float mass = 1.0f;

// physics vals

float gravity = -0.0f;
float smoothingRadius = 1.7f*radius;
float targetDensity = 2.0f;
float pressureMultiplier = 8.0f;


void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble) w, 0.0, (GLdouble) h);
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(deltaTime * pow(10, 3), timer, 0);
}

void printVector2(const Vector2& v, const std::string& name) {
    std::cout << name << ": (" << v.x << ", " << v.y << ")" << std::endl;
}

void drawCircle(float cx, float cy, float r) {
    int numSegments = 100;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= numSegments; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(numSegments);
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}


float convertDensityToPressure(float density) {
    float densityError = density - targetDensity;
    float pressure = densityError * pressureMultiplier;
    return pressure;
}

float smoothingKernel(float rad, float dst) {
    if(0 <= dst && dst <= rad) {
        float vol = 15/(M_PI * pow(rad, 6));
        // std::cout << "r,d: " << rad << "," << dst <<  "\n";
        // std::cout << "r-d: " << rad - dst << "\n";
        float val = std::max(0.0f, rad-dst);
        return pow(val, 3) * vol;
    }
    return 0;
}

float smoothingKernelDerivative(float rad, float dst) {
    if (dst > rad) return 0;

    float v = (rad - dst);
    float scale = -45.0f / (M_PI * pow(rad, 6));

    return pow(v, 2) * scale;
}

float calculateDensity(Vector2 samplepoint) {
    float density = 0;

    for (int i = 0; i < positions.size(); i++) {
        float dst = (positions[i]-samplepoint).magnitude();
        float influence = smoothingKernel(smoothingRadius, dst);
        density+= mass*influence;
    }
    return density;
}

void updateDensities() {
    for(int i = 0; i < numParticles; i++) {
        densities[i] = calculateDensity(positions[i]);
    }
}

Vector2 GetRandomDir() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib(-1, 1);

    return Vector2(distrib(gen), distrib(gen));
}

float calcSharedPressure(float densA, float densB) {
    float pressureA = convertDensityToPressure(densA);
    float pressureB = convertDensityToPressure(densB);

    return (pressureA + pressureB)/2;
}

Vector2 calculatePressureForce(int particleIndex) {
    Vector2 pressureForce;

    for(int otherParticleIndex = 0; otherParticleIndex < numParticles; otherParticleIndex++) {
        if(particleIndex == otherParticleIndex) continue;
        Vector2 offset = positions[otherParticleIndex] - positions[particleIndex];

        float dst = offset.magnitude();
        Vector2 dir;

        if(dst == 0) {
            dir = GetRandomDir();
        }
        else {
            dir = offset/dst;
        }
        float slope = smoothingKernelDerivative(dst, smoothingRadius);

        float density = densities[otherParticleIndex];
        float sharedPressure = calcSharedPressure(density, densities[particleIndex]);

        pressureForce = pressureForce + ((dir * sharedPressure *  slope * mass) / density);
    }
    return pressureForce;
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0f, 0.0f, 1.0f);

    screenWidth = glutGet(GLUT_WINDOW_WIDTH);
    screenHeight = glutGet(GLUT_WINDOW_HEIGHT);

    // draw

    for(int i = 0; i < positions.size(); i++) {
        drawCircle(positions[i].x, positions[i].y, radius);
    }

    // calculations

    // apply gravity + predict pos + densities
    PARALLEL_FOR_BEGIN(positions.size()) {
        velocities[i].y += gravity * deltaTime;
        predictedPositions[i] = positions[i]+velocities[i] * deltaTime;
        densities[i] = calculateDensity(predictedPositions[i]);
    } PARALLEL_FOR_END();

    // calc and apply presssure force
    PARALLEL_FOR_BEGIN(positions.size()) {
        Vector2 pressureForce = calculatePressureForce(i);
        // f=ma; a = f/m
        Vector2 pressureAccel = pressureForce/densities[i];
        velocities[i] = velocities[i] + pressureAccel * deltaTime;

    } PARALLEL_FOR_END();

    // calculate positions
    PARALLEL_FOR_BEGIN(positions.size()) {
        positions[i] = positions[i] + velocities[i] * deltaTime;
    } PARALLEL_FOR_END();

    // collisions

    PARALLEL_FOR_BEGIN(positions.size()) {
        if(positions[i].y - radius < 0 ) {
            positions[i].y = radius;
            velocities[i].y = -velocities[i].y * 1.0f;
        }
        else if(positions[i].y + radius > screenHeight) {
            positions[i].y = screenHeight - radius;
            velocities[i].y = -velocities[i].y * 1.0f;
        }

        if(positions[i].x - radius < 0 ) {
            positions[i].x = radius;
            velocities[i].x = -velocities[i].x * 1.0f;
        }
        else if(positions[i].x + radius > screenWidth) {
            positions[i].x = screenWidth - radius;
            velocities[i].x = -velocities[i].x * 1.0f;
        }
     }PARALLEL_FOR_END();


    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1920*0.4, 1200*0.4);

    glutCreateWindow("Hello world!");

    // initialize
    int particlesPerRow = (int)sqrt(numParticles);
    int particlesPerCol = (numParticles - 1) / particlesPerRow + 1;
    float spacing = radius*2 + particleSpacing;

    boundX = glutGet(GLUT_WINDOW_WIDTH) - radius;
    boundY = glutGet(GLUT_WINDOW_HEIGHT) - radius;

    screenWidth = glutGet(GLUT_WINDOW_WIDTH);
    screenHeight = glutGet(GLUT_WINDOW_HEIGHT);

    for(int i = 0; i < numParticles; i++) {
        int row = i / particlesPerRow;
        int col = i % particlesPerRow;

        // Center the entire grid of circles within the window
        float offsetX = (screenWidth - (particlesPerRow - 1) * spacing) / 2.0f;
        float offsetY = (screenHeight - (particlesPerCol - 1) * spacing) / 2.0f;

        float x = col * spacing + offsetX;
        float y = row * spacing + offsetY;

        positions.push_back(Vector2(x, y));
        velocities.push_back(Vector2());
        densities.push_back(calculateDensity(positions[i]));
        predictedPositions.push_back(Vector2());

    }


    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}
