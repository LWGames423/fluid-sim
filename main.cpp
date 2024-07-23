#include <iostream>
#include <GL/glut.h>
#include <cmath>

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

int screenWidth, screenHeight;
float boundX, boundY;
float radius = 10.0f;
float circleX, circleY;
float velocityY = 0.0f, velocityX = 0.0f;
float gravity = -0.5f;


void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0f, 0.0f, 1.0f);


    drawCircle(circleX, circleY, radius);

    velocityY += gravity;
    circleY += velocityY;

    if(circleY - radius < 0) {
        circleY = radius;
        velocityY = -velocityY * 0.8f;
    }

    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(400, 300);

    glutCreateWindow("Hello world!");

    // initialize initial circle position
    circleX = glutGet(GLUT_WINDOW_WIDTH) / 2.0f;
    circleY = glutGet(GLUT_WINDOW_HEIGHT) / 2.0f;


    boundX = glutGet(GLUT_WINDOW_WIDTH) - radius;
    boundY = glutGet(GLUT_WINDOW_HEIGHT) - radius;

    screenWidth = glutGet(GLUT_WINDOW_WIDTH);
    screenHeight = glutGet(GLUT_WINDOW_HEIGHT);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}
