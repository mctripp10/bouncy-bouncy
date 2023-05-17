/* Michael Tripp
 * Computer Graphics
 * Project 2
 *
 * Program that bounces a user defined object inside
 * a user defined boundary.
 */

#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <vector>
using namespace std;

int width = 500;
int height = 500;

vector<vector<GLfloat>> boundPoints;
vector<vector<GLfloat>> objPoints;

int mousePosY, mousePosX;
GLfloat xtrans, ytrans;
GLfloat xinc, yinc, angle, angleInc, xdir, ydir, angleDir;
GLfloat prevXInc, prevYInc, prevAngleInc;
double backgroundColor[3];
double shapeColor[3];
double boundColor[3];

double matrix[16];

bool defBound, defObj, animate, goCrazy;
bool closeBound, closeObj;
bool on;

void drawBoundary () {
    int n = boundPoints.size();
    glBegin(GL_LINE_STRIP);
        for (int i = 0; i < n; i++)
            glVertex2f(boundPoints[i][0], boundPoints[i][1]);
        if (defBound)
            glVertex2f(mousePosX, height-mousePosY);
        if (closeBound) {
            glVertex2f(boundPoints[0][0], boundPoints[0][1]);
        }
    glEnd();
}

void drawObject () {
    int n = objPoints.size();
    glBegin(GL_POLYGON);
        for (int i = 0; i < n; i++)
            glVertex2f(objPoints[i][0], objPoints[i][1]);
        if (defObj)
            glVertex2f(mousePosX, height-mousePosY);
        if (closeObj) {
            glVertex2f(objPoints[0][0], objPoints[0][1]);
        }
    glEnd();
}

vector<GLfloat> getAvgCenter (vector<vector<GLfloat>> obj) {
    int n = obj.size();
    GLfloat avgX, avgY;
    for (int i = 0; i < n; i++) {
        avgX += obj[i][0];
        avgY += obj[i][1];
    }
    avgX /= n;
    avgY /= n;
    vector<GLfloat> avgCenter { avgX, avgY };
    return avgCenter;
}

void clearPoints (int val)
{
    if (val == 0) {
        boundPoints.clear();
    } else if (val == 1) {
        objPoints.clear();
    }
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(boundColor[0], boundColor[1], boundColor[2]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    if (boundPoints.size() > 0)
        drawBoundary();
    if (objPoints.size() > 0) {
        glColor3f(shapeColor[0], shapeColor[1], shapeColor[2]);
        if (animate) {
            glTranslated(xtrans, ytrans, 0);
            vector<GLfloat> center = getAvgCenter(objPoints);
            glTranslated(center[0], center[1], 0);
            glRotated(angle, 0, 0, 1.0);
            glTranslated(-center[0], -center[1], 0);
        }
        drawObject();
    }
	glutSwapBuffers();
}

double* matrixMultiply (double* matrix, double* p) {
    double* newp = new double[4];
    double m [4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m[i][j] = matrix[i+4*j];
        }
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            newp[i] += m[i][j] * p[j];
        }
    }
    return newp;
}

void reflectVector (int nx, int ny)
{
    GLfloat ax = xinc;
    GLfloat ay = yinc;
    GLfloat unx = nx/(sqrt(nx*nx+ny*ny));
    GLfloat uny = ny/(sqrt(nx*nx+ny*ny));

    GLfloat rx = ax-2*ax*unx*unx-2*ay*uny*unx;
    GLfloat ry = ay-2*ax*unx*uny-2*ay*uny*uny;

    xinc = rx;
    yinc = ry;
}

void setColor (double r, double g, double b, int type)
{
    if (type == 0) {
        backgroundColor[0] = r;
        backgroundColor[1] = g;
        backgroundColor[2] = b;
    } else if (type == 1) {
        shapeColor[0] = r;
        shapeColor[1] = g;
        shapeColor[2] = b;
    } else if (type == 2) {
        boundColor[0] = r;
        boundColor[1] = g;
        boundColor[2] = b;
    }
}

void idle()
{
    if (animate) {
        glGetDoublev(GL_MODELVIEW_MATRIX, matrix);
        GLfloat px, py, nx, ny, qx, qy;
        int bn = boundPoints.size();
        double p [4] = {0, 0, 0, 1};
        double* newp;
        bool needToBounce;
        for (int i = 0; i < objPoints.size(); i++) {
            needToBounce = true;
            p[0] = objPoints[i][0];
            p[1] = objPoints[i][1];
            newp = matrixMultiply(matrix, p);
            px = newp[0];
            py = newp[1];

            for (int j = 0; j < boundPoints.size()-1; j++) {
                nx = boundPoints[j+1][1]-boundPoints[j][1];
                ny = -1*(boundPoints[j+1][0]-boundPoints[j][0]);
                qx = boundPoints[j][0];
                qy = boundPoints[j][1];
                if ((nx * (px - qx) + ny * (py - qy)) <= 0 && needToBounce) {
                    if (!on) {
                        on = true;
                        return;
                    }
                    reflectVector(nx, ny);
                    needToBounce = false;
                    angleDir *= -1.0;
                    if (goCrazy) {
                        setColor((double) 1/(rand() % 10), (double) 1/(rand() % 10), (double) 1/(rand() % 10), 0);
                        glClearColor (backgroundColor[0], backgroundColor[1], backgroundColor[2], 0.0);
                        setColor((double) 1/(rand() % 10), (double) 1/(rand() % 10), (double) 1/(rand() % 10), 1);
                        setColor((double) 1/(rand() % 10), (double) 1/(rand() % 10), (double) 1/(rand() % 10), 2);
                    }
                }
            }
            nx = boundPoints[0][1]-boundPoints[bn-1][1];
            ny = -1*(boundPoints[0][0]-boundPoints[bn-1][0]);
            qx = boundPoints[bn-1][0];
            qy = boundPoints[bn-1][1];
            if ((nx * (px - qx) + ny * (py - qy)) <= 0 && needToBounce) {
                if (!on) {
                    on = true;
                    return;
                }
                reflectVector(nx, ny);
                needToBounce = false;
                angleDir *= -1.0;
                if (goCrazy) {
                    setColor((double) 1/(rand() % 10), (double) 1/(rand() % 10), (double) 1/(rand() % 10), 0);
                    glClearColor (backgroundColor[0], backgroundColor[1], backgroundColor[2], 0.0);
                    setColor((double) 1/(rand() % 10), (double) 1/(rand() % 10), (double) 1/(rand() % 10), 1);
                    setColor((double) 1/(rand() % 10), (double) 1/(rand() % 10), (double) 1/(rand() % 10), 2);
                }
        }
        }

        angle += angleDir*angleInc;
        if (angle >= 360.0)
            angle -= 360.0;
        xtrans += xdir*xinc;
        ytrans += ydir*yinc;
        glutPostRedisplay();
    }
}

void myinit()
{
	setColor(1.0, 1.0, 1.0, 0);
	setColor(1.0, 0.0, 0.0, 1);
	setColor(0.0, 0.0, 1.0, 2);

    glClearColor (backgroundColor[0], backgroundColor[1], backgroundColor[2], 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);

	defBound = defObj = animate = closeBound = closeObj = false;
	xtrans = ytrans = 0;
	xinc = 0.2;
	yinc = 0.05;
	angle = 0;
	angleInc = 0.2;
	xdir = 1;
	ydir = 1;
	angleDir = 1;

	on = false;
	goCrazy = false;
}

void mouseFunc(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        if (button == GLUT_LEFT_BUTTON) {
            if (defBound) {
                vector<GLfloat> p {(GLfloat) x, (GLfloat) height-y};
                boundPoints.push_back(p);
            } else if (defObj) {
                vector<GLfloat> p {(GLfloat) x, (GLfloat) height-y};
                objPoints.push_back(p);
            }
        }
    }
}

void myPassiveMotionFunc(int x, int y)
{
    mousePosX = x;
    mousePosY = y;
    if (defBound || defObj) {
        glutPostRedisplay();
    }
}

void mykeyboardFunc(unsigned char key, int x, int y)
{
    if (key == 't') {
        if (xinc != 0.0) {
            prevXInc = xinc;
            xinc = 0.0;
        } else
            xinc = prevXInc;

        if (yinc != 0.0) {
            prevYInc = yinc;
            yinc = 0.0;
        } else
            yinc = prevYInc;
    } else if (key == 'r') {
        if (angleInc > 0) {
            prevAngleInc = angleInc;
            angleInc = 0.0;
        } else
            angleInc = prevAngleInc;
    } else if (key == 'q')
        exit(-1);
}

void mySpecialFunc(int key, int x, int y)
{
    if (key == GLUT_KEY_UP) {
        if (xinc < 0)
            xinc -= 0.05;
        else
            xinc += 0.05;

        if (yinc < 0)
            yinc -= 0.05;
        else
            yinc += 0.05;
    } else if (key == GLUT_KEY_DOWN) {
        if (xinc <= 0.05 && xinc >= -0.05)
            xinc = 0.0;
        else if (xinc > 0.05)
            xinc -= 0.05;
        else
            xinc += 0.05;

        if (yinc <= 0.05 && yinc >= -0.05)
            yinc = 0.0;
        else if (yinc > 0.05)
            yinc -= 0.05;
        else
            yinc += 0.05;
    } else if (key == GLUT_KEY_LEFT) {
        angleInc += 0.05;
    } else if (key == GLUT_KEY_RIGHT) {
        angleInc -= 0.05;
        if (angleInc < 0.05)
            angleInc = 0.0;
    }

}

void myreshape(int w, int h)
{
    width = w;
    height = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, width, height);
}

void menu(int id)
{
    switch(id) {
    case 1: if (defBound) {
                defBound = false;
                closeBound = true;
                glutPostRedisplay();
            } else {
                animate = false;
                clearPoints(0);
                defBound = true;
                closeBound = false;
                if (defObj) {
                    defObj = false;
                    closeObj = true;
                }
            }
            break;
    case 2: if (defObj) {
                defObj = false;
                closeObj = true;
                glutPostRedisplay();
            } else {
                animate = false;
                xtrans = ytrans = 0;
                clearPoints(1);
                defObj = true;
                closeObj = false;
                if (defBound) {
                    defBound = false;
                    closeBound = true;
                }
            }
            break;
    case 3: if (animate)
                animate = false;
            else {
                if (defBound) {
                    defBound = false;
                    closeBound = true;
                } else if (defObj) {
                    defObj = false;
                    closeObj = true;
                }
                animate = true;
            }
            break;
    case 4: if (goCrazy)
                goCrazy = false;
            else
                goCrazy = true;
    }
    glutPostRedisplay();
}
void createMenu()
{
	glutCreateMenu(menu);
	glutAddMenuEntry("Define Boundary", 1);
	glutAddMenuEntry("Define Object", 2);
	glutAddMenuEntry("Start Movement", 3);
	glutAddMenuEntry("Go Crazy", 4);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Bouncy Bouncy");
    myinit();
    glutDisplayFunc(display);
    glutIdleFunc(idle);
	glutMouseFunc(mouseFunc);
	glutPassiveMotionFunc(myPassiveMotionFunc);
	glutKeyboardFunc(mykeyboardFunc);
	glutSpecialFunc(mySpecialFunc);
	glutReshapeFunc(myreshape);

	createMenu();
    glutMainLoop();
}
