//
//  main.cpp
//  ColorMixing
//
//
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <assert.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

int width = 500;
int height = 500;
#define PI 3.1415926

void DrawFilledCircle(GLfloat x,GLfloat y,GLfloat r){
    int i;
    int triangleAmount = 20;
    
    GLfloat twicePi = 2.0f * PI;
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // center of circle
    for(i = 0; i <= triangleAmount;i++) {
        glVertex2f(
                   x + (r * cos(i *  twicePi / triangleAmount)),
                   y + (r * sin(i * twicePi / triangleAmount))
                   );
    }
    glEnd();
}

void DrawCircle(GLfloat x, GLfloat y, GLfloat r){
    int i;
    int lineAmount = 100;
    
    GLfloat twicePi = 2.0f * PI;
    
    glBegin(GL_LINE_LOOP);
    for(i = 0; i <= lineAmount;i++) {
        glVertex2f(
                   x + (r * cos(i *  twicePi / lineAmount)),
                   y + (r* sin(i * twicePi / lineAmount))
                   );
    }
    glEnd();
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    // rendering
    //DrawCircle(250,250, 50);
    DrawFilledCircle(250,250, 50);
    
    glFlush();
    glutPostRedisplay();
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, height, 0.0, -10.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 27:
            exit(0);
            break;
    }
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (width, height);
    glutInitWindowPosition (100, 100);
    glutCreateWindow (argv[0]);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc (keyboard);
    glutMainLoop();
    return 0;
}

