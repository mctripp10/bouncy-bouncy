/* Michael Tripp
 * Computer Graphics
 *
 * Program that bounces a user defined object inside
 * a user defined boundary using the OpenGL API.
 * Additional keyboard input was added to influence
 * object movement. Also added an additional 'go crazy mode'
 * just for fun :]
 */

#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <vector>
using namespace std;

/*
 * GLOBAL VARIABLES
 */

int width = 500;
int height = 500;

vector<vector<GLfloat>> boundPoints;		// vector to store user defined points making up outer boundary
vector<vector<GLfloat>> objPoints;		// vector to store user defined points making up inner object

int mousePosY, mousePosX;
GLfloat xtrans, ytrans;
GLfloat xinc, yinc, angle, angleInc, xdir, ydir, angleDir;
GLfloat prevXInc, prevYInc, prevAngleInc;
double backgroundColor[3];
double shapeColor[3];		// used to define the color of the inner object (shape)
double boundColor[3];		// used to define the color of the outer boundary

double matrix[16];

bool defBound, defObj, animate, goCrazy;
bool closeBound, closeObj;
bool on;

/*
 * METHODS
 */

// Allows the user to draw a boundary by storing the x and y mouse coordinates upon selecting a location on the screen
// Line from last selected point will follow mouse for user convenience and leaves center of object transparent upon completion
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

// Allows the user to draw the inner object by storing the x and y mouse coordinates upon selecting a location on the screen
// Fill in center of object upon completion
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

// Gets the average center of a polygon by averaging all x and y point values
// (particularly useful since the polygons are user defined and can be any shape!)
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

// Clears all current boundary points or object points based on the value of val
void clearPoints (int val)
{
    if (val == 0) {
        boundPoints.clear();
    } else if (val == 1) {
        objPoints.clear();
    }
}

// Necessary display method to draw screen
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(boundColor[0], boundColor[1], boundColor[2]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    if (boundPoints.size() > 0)
	// Draw boundary given there are boundary points
        drawBoundary();
    if (objPoints.size() > 0) {
	// Draw object given there are object points
        glColor3f(shapeColor[0], shapeColor[1], shapeColor[2]);
        if (animate) {
	    // Begin translation and rotation of the object around the inside of the outer boundary using matrix transformations
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

// Applies a matrix transformation to a point p given a 4x4 matrix matrix (only 4x4 matrices are used in this program)
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

// Solves for the reflect vector rx, ry (vector produced from collision of object against the outer boundary) based on normal vector nx, ny
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

// Automates coloring of the background, shape, or boundary given type variable and r, g, b color
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

// Idle loop constantly running in the background
void idle()
{
    if (animate) {
	// Begin animation
        glGetDoublev(GL_MODELVIEW_MATRIX, matrix);
        GLfloat px, py, nx, ny, qx, qy;
        int bn = boundPoints.size();
        double p [4] = {0, 0, 0, 1};
        double* newp;
        bool needToBounce; 	// Flag to determine if object has hit boundary yet

	// Apply matrix transformation to each object point to transform and rotate entire object as necessary
        for (int i = 0; i < objPoints.size(); i++) {
            needToBounce = true;
            p[0] = objPoints[i][0];
            p[1] = objPoints[i][1];
            newp = matrixMultiply(matrix, p);
            px = newp[0];
            py = newp[1];

	    // Determine if there is a collision between the object and boundary by going through all boundary points
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
			// If goCrazy is enabled by user, this will change color of background, shape, and boundary to a random color on collision
                        setColor((double) 1/(rand() % 10), (double) 1/(rand() % 10), (double) 1/(rand() % 10), 0);
                        glClearColor (backgroundColor[0], backgroundColor[1], backgroundColor[2], 0.0);
                        setColor((double) 1/(rand() % 10), (double) 1/(rand() % 10), (double) 1/(rand() % 10), 1);
                        setColor((double) 1/(rand() % 10), (double) 1/(rand() % 10), (double) 1/(rand() % 10), 2);
                    }
                }
	    }

	    // Code for last iteration (repeat of code above in for loop)
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

	// Code to adjust object rotation angle and speed
        angle += angleDir*angleInc;
        if (angle >= 360.0)
            angle -= 360.0;
        xtrans += xdir*xinc;
        ytrans += ydir*yinc;
        glutPostRedisplay();
    }
}

// Runs at start of program to initialize necessary variables
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

// Mouse function that stores x and y point coordinates based on user mouse click screen position
void mouseFunc(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        if (button == GLUT_LEFT_BUTTON) {
	    // Use left button to choose position, differing based on if user has selected to define boundary or object
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

// Used to keep track of mouse position across the screen by updating global variables mousePosX and mousePosY
void myPassiveMotionFunc(int x, int y)
{
    mousePosX = x;
    mousePosY = y;
    if (defBound || defObj) {
        glutPostRedisplay();
    }
}

// Used to define functions that are run on keyboard key presses
void mykeyboardFunc(unsigned char key, int x, int y)
{
    //Starts or stops translation (movement) of shape
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

    //Starts or stops rotation of shape
    } else if (key == 'r') {
	// Enables or disables rotation
        if (angleInc > 0) {
            prevAngleInc = angleInc;
            angleInc = 0.0;
        } else
            angleInc = prevAngleInc;

    // Exits program (quit)
    } else if (key == 'q')
        exit(-1);
}

// Used to define additional keyboard key press functions
void mySpecialFunc(int key, int x, int y)
{
    // UP ARROW: increase translation (movement) speed
    if (key == GLUT_KEY_UP) {
        if (xinc < 0)
            xinc -= 0.05;
        else
            xinc += 0.05;

        if (yinc < 0)
            yinc -= 0.05;
        else
            yinc += 0.05;

    // DOWN ARROW: decrease translation (movement) speed
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

    // LEFT ARROW: increase rotation speed counterclockwise
    } else if (key == GLUT_KEY_LEFT) {
        angleInc += 0.05;

    // RIGHT ARROW: increase rotation speed clockwise
    } else if (key == GLUT_KEY_RIGHT) {
        angleInc -= 0.05;
        if (angleInc < 0.05)
            angleInc = 0.0;
    }

}

// Used to ensure proper resizing of screen upon the user resizing the GUI window by keeping screen proportional and drawn objects fixed
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

// Defines a GUI menu object and its contents, with each entry of the menu associated with a different id
void menu(int id)
{
    switch(id) {
    // Option that allows the user to define the boundary or close the boundary if already selected
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

    // Option that allows the user to define the object, or close the object if already selected
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

    // Option that allows the user to begin translation and rotation of the object
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

    // Option that enables 'go crazy mode', which changes the boundary, shape, and background to random colors every time the shape hits the boundary
    case 4: if (goCrazy)
                goCrazy = false;
            else
                goCrazy = true;
    }
    glutPostRedisplay();
}

// Initializes the menu and each of its options and attaches it to the user's mouse right click
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
    glutCreateWindow("Bouncy Bouncy");		// Initialize window given previous parameters
    myinit();					// Initialize any necessary initial variables

    // Initialize all necessary glut functions
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutMouseFunc(mouseFunc);
    glutPassiveMotionFunc(myPassiveMotionFunc);
    glutKeyboardFunc(mykeyboardFunc);
    glutSpecialFunc(mySpecialFunc);
    glutReshapeFunc(myreshape);

    createMenu();				// Initialize menu
    glutMainLoop();				// Begin display output loop
}
