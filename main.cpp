#include <algorithm>
#include <iostream>
#include <fstream>
#include <GL/glut.h>
#include <cmath>

#include <vector>

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble) w, 0.0, (GLdouble) h);
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

struct Vector2
{
    float x;
    float y;

    Vector2(float x1, float y1): x(x1),y(y1) {

    }

    Vector2():x(0), y(0) {

    }
};

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

// VARS //

int screenWidth, screenHeight;
float boundX, boundY;

// init

int circleCount = 800;
float particleSpacing = 10.0f;

// particle properties

std::vector<Vector2> positions;
std::vector<Vector2> velocities;
float radius = 10.0f;
float mass = 1.0f;

// physics vals

float gravity = -0.5f;
float smoothingRadius = 1.0f;

float smoothingKernel(float rad, float dst) {
    float vol = M_PI * pow(rad, 5) / 10;
    float val = std::max(0.0f, rad-dst);
    return pow(val, 3) / vol;
}

float calculateDensity(Vector2 samplepoint) {
    float density = 0;

    for (auto position: positions) {
        float dst = sqrt(pow(position.x - samplepoint.x,2) - (position.y - samplepoint.y,2));
        float influence = smoothingKernel(smoothingRadius, dst);
        density+= mass*influence;
    }
    return density;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0f, 0.0f, 1.0f);

    // initialize

    for(int i = 0; i < positions.size(); i++) {
        drawCircle(positions[i].x, positions[i].y, radius);
    }

    // update

    for(int i = 0; i < positions.size(); i++) {
        velocities[i].y += gravity;

        positions[i].x += velocities[i].x;
        positions[i].y += velocities[i].y;

        // collisions
        if(positions[i].y - radius < 0 || positions[i].y + radius > screenHeight) {
            positions[i].y = radius;
            velocities[i].y = -velocities[i].y * 0.8f;
        }
        if(positions[i].x - radius < 0 || positions[i].x + radius > screenWidth) {
            positions[i].x = radius;
            velocities[i].x = -velocities[i].x * 0.8f;
        }

    }

    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1024, 576);

    glutCreateWindow("Hello world!");

    // initialize
    int particlesPerRow = (int)sqrt(circleCount);
    int particlesPerCol = (circleCount - 1) / particlesPerRow + 1;
    float spacing = radius*2 + particleSpacing;

    boundX = glutGet(GLUT_WINDOW_WIDTH) - radius;
    boundY = glutGet(GLUT_WINDOW_HEIGHT) - radius;

    screenWidth = glutGet(GLUT_WINDOW_WIDTH);
    screenHeight = glutGet(GLUT_WINDOW_HEIGHT);

    for(int i = 0; i < circleCount; i++) {
        int row = i / particlesPerRow;
        int col = i % particlesPerRow;

        // Center the entire grid of circles within the window
        float offsetX = (screenWidth - (particlesPerRow - 1) * spacing) / 2.0f;
        float offsetY = (screenHeight - (particlesPerCol - 1) * spacing) / 2.0f;

        float x = col * spacing + offsetX;
        float y = row * spacing + offsetY;

        positions.push_back(Vector2(x, y));
        velocities.push_back(Vector2());
    }

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}
