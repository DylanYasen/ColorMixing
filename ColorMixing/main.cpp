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

int width = 800;
int height = 800;
#define PI 3.1415926

/*
 HUE-PRESERVING BLENDING
 
 (αAFA)CA +(αBFB)CB
 1. αA and αB are the alpha values associated with the two colors
 2. FA and FB are respective fractional components. The scalar values
 (αAFA) and (αBFB) can be interpreted as combined weights for the
 two input colors. The original version of those compositing operators
 assumes colors in RGB color space.
*/

void DrawFilledCircle(GLfloat x,GLfloat y,GLfloat r){
    int i;
    int triangleAmount = 36;
    
    
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

void RGBtoHSL(GLfloat r,GLfloat g,GLfloat b,GLfloat &h,GLfloat &s,GLfloat &l){
    
    // Convert the RGB values to the range 0-1
    r /= 255;
    g /= 255;
    b /= 255;
    
    // Find the minimum and maximum values of R, G and B.
    GLfloat tempMin = fminf(r,g);
    GLfloat tempMin2 = fminf(g, b);
    GLfloat min = fminf(tempMin, tempMin2);
    
    GLfloat tempMax = fmaxf(r,g);
    GLfloat tempMax2 = fmaxf(g, b);
    GLfloat max = fmaxf(tempMax, tempMax2);
    
    // 3 calculate the Luminace value by adding the max and min values and
    GLfloat lum = (min + max) / 2;
    
    // 4 find the Saturation
    GLfloat sat = 0;
    if(min == max){
        sat = 0;
    }else{
        //If Luminance is smaller then 0.5, then Saturation = (max-min)/(max+min)
        //If Luminance is bigger then 0.5. then Saturation = ( max-min)/(2.0-max-min)
        
        if(lum < 0.5){
            sat = (max - min) / (max + min);
        }else{
            sat = (max - min) / (2.0 - max - min);
        }
    }
    
    // 5 find Hue
    /*
    If Red is max, then Hue = (G-B)/(max-min)
    If Green is max, then Hue = 2.0 + (B-R)/(max-min)
    If Blue is max, then Hue = 4.0 + (R-G)/(max-min)
    */
    GLfloat hue = 0;
    if(max == r){
        hue = (g-b) / (max - min);
    }
    else if(max == g){
        hue = 2.0 + (b-r) / (max-min);
    }
    else{
        hue = 4.0 + (r-g) / (max-min);
    }
    
    /*
     The Hue value you get needs to be multiplied by 60 to 
     convert it to degrees on the color circle
     If Hue becomes negative you need to add 360 to, because a circle has 360 degrees
     */
    hue *= 60;
    if(hue < 0)
        hue += 360;
    
    // result
    h = hue;
    s = sat;
    l = lum;
}

GLfloat HSLtoRGBtestCond(float temp1,float temp2,GLfloat tempColorComponent){
    
    GLfloat colorChannelVal = 0;
    // test 1
    // If 6 x tempC is smaller then 1,
    // colorComp = temp2 + (temp1 – temp2) x 6 x tempC
    if (6 * tempColorComponent < 1) {
        colorChannelVal = temp2 + (temp1 - temp2) * 6 * tempColorComponent;
    }
    // test 2
    // If 2 x temporary_R is smaller then 1, Red = temporary_1
    else if (2 * tempColorComponent < 1){
        colorChannelVal = temp1;
    }
    // test3
    // If 3 x temporary_R is smaller then 2, Red = temporary_2 + (temporary_1 – temporary_2) x (0.666 – temporary_R) x 6
    else if(3 * tempColorComponent < 2){
        colorChannelVal = temp2 + (temp1 - temp2) * (2/3 - tempColorComponent)*6;
    }
    
    else if(3 * tempColorComponent > 2){
        colorChannelVal = temp2;
    }
    
    return colorChannelVal;
}

void HSLtoRGB(GLfloat h,GLfloat s,GLfloat l,GLfloat &r,GLfloat &g,GLfloat &b){
    /*
     1.
     if there is no Saturation it means that it’s a shade of grey. So in that case we just need to convert the Luminance and set R,G and B to that level.
     */
    if(s == 0){
        r = l * 255;
        g = r;
        b = r;
    }
    // if there is saturation
    else{
        
        // 2.
        GLfloat temp1 = 0;
        // less than 0.5 = Luminance x (1.0+Saturation)
        if (l < 0.5) {
            temp1 = l * (1 + s);
        }
        // temporary_1 = Luminance + Saturation – Luminance x Saturation
        else{
            temp1 = l + s - l * s;
        }
        
        // 3.
        GLfloat temp2 = 0;
        // temporary_2 = 2 x Luminance – temporary_1
        temp2 = 2 * l - temp1;
        
        // 4.
        // convert the 360 degrees in a circle to 1 by dividing the angle by 360.
        h /= 360;
        
        // 5.
        // temporary variable for each color channel
        // has to be 0 - 1
        GLfloat tempR = h + (1/3);
        GLfloat tempG = h;
        GLfloat tempB = h - (1/3);
        
        // TODO: use clamp instead
        if(tempR > 1)
            tempR -= 1;
        else if(tempR < 1)
            tempR += 1;
        if(tempG > 1)
            tempG -= 1;
        else if(tempG < 1)
            tempG += 1;
        if(tempB > 1)
            tempB -= 1;
        else if(tempB < 1)
            tempB += 1;
       
        // 6.
        /*
         Now we need to do up to 3 tests to select the correct formula for each color channel. Let’s start with Red.
         */
        r = HSLtoRGBtestCond(temp1, temp2, tempR);
        g = HSLtoRGBtestCond(temp1, temp2, tempG);
        b = HSLtoRGBtestCond(temp1, temp2, tempB);
        
        // map values in between 0 - 255
        r *= 255;
        g *= 255;
        b *= 255;
    }
}

bool EqualHue(GLfloat r1,GLfloat g1,GLfloat b1,GLfloat r2,GLfloat g2,GLfloat b2){
    GLfloat h1,s1,l1;
    RGBtoHSL(r1, g1, b1, h1, s1, l1);
    
    GLfloat h2,s2,l2;
    RGBtoHSL(r2, g2, b2, h2, s2, l2);
    
    if(h1 == h2)
        return true;
    
    return false;
}


/*
 the hue angle can be rotated by 180 degrees to obtain
 the opposite color.
 */
void OppositeColor(GLfloat &r,GLfloat &g,GLfloat &b){
    GLfloat h,s,l;
    RGBtoHSL(r, g, b, h, s, l);
    
    h += 180;
    if(h > 360){
        h -= 360;
    }
    
    HSLtoRGB(h, s, l, r, g, b);
}

void HuePreservingBlend(GLfloat r1,GLfloat g1,GLfloat b1,GLfloat r2,GLfloat g2,GLfloat b2,GLfloat &r3,GLfloat &g3,GLfloat &b3){
    
    if (EqualHue(r1, g1, b1, r2, g2, b2)) {
        r3 = r1 + r2;
        g3 = g1 + g2;
        b3 = b1 + b2;
    }else{
        OppositeColor(r2, g2, b2);
        r3 = r1 + r2;
        g3 = g1 + g2;
        b3 = b1 + b2;
     
        if (!EqualHue(r1, b1, g1, r3, g3, b3)) {
            OppositeColor(r1, g1, b1);
            r3 = r1 + r2;
            g3 = g1 + g2;
            b3 = b1 + b2;
        }
    }
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    glEnable     (GL_BLEND);
    glBlendFunc  (GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
  
    glColor4f(0.1,0.1,0.4,0.5);
    DrawFilledCircle(200,250,100);
    
    glColor4f(0.4,0.2,0.6,0.5);
    DrawFilledCircle(340,250,100);
    
    GLfloat r,g,b;
    HuePreservingBlend(0.1, 0.1, 0.4, 0.4, 0.2, 0.6, r, g, b);
    
    glColor4f(r,g,b,0.2);
    DrawFilledCircle(270,350,100);
    
    glFlush();
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w, h, 0.0, -10.0, 10.0);
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
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize (width, height);
    glutInitWindowPosition (100, 100);
    glutCreateWindow (argv[0]);
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc (keyboard);
    glutMainLoop();
    
    
    return 0;
}

