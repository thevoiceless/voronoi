// Riley Moses

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <string>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>

using namespace std;

// OpenGL variables
static const int VIEWPORT_DEFAULT = 800;
static const double TWOPI = (2.0 * M_PI);
static const int MENU_ADD_SITES = 1;
static const int MENU_TOGGLE_SITES_VISIBLE = 2;
static const int MENU_TOGGLE_ANIMATE = 3;
static const int MENU_TOGGLE_COLORING = 4;
static const int MENU_RESET = 5;
// GLUT window id; value asigned in main() and should stay constant
GLint windowID;
// Viewport dimensions; change when window is resized (via resize callback)
GLint viewport_width = VIEWPORT_DEFAULT;
GLint viewport_height = VIEWPORT_DEFAULT;

// File to use for color lookup
string colorFile = "input.ppm";
// Whether or not to animate
bool animate = false;

void toggleAnimation()
{
	animate = !animate;
	glutPostRedisplay();
}

GLuint draw_scene()
{
}

// Draw callback: clear window, set up matrices, draw all cubes
void draw()
{
	// Set the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Ensure we're drawing to the correct GLUT window
	glutSetWindow(windowID);

	// Clear the color buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw; smooth shading is the default (use glShadeModel to change)
	draw_scene();

	// Flush the pipeline
	glFlush();

	// Look at our handiwork
	glutSwapBuffers();

	// Make sure everything is redrawn in the animation mode
	if (animate)
	{
		glutPostRedisplay();
	} 
}

// Handle keyboard events
void keyboard(GLubyte key, GLint x, GLint y)
{
	switch (key)
	{
		// Exit when escape is pressed
		case 27:
			exit(0);
		default:
			break;
	}
}

// Menu callback
void menu(int value)
{
	switch (value)
	{
		case MENU_ADD_SITES:
			cout << "Add sites" << endl;
			break;
		case MENU_TOGGLE_SITES_VISIBLE:
			cout << "Toggle site visibility" << endl;
			break;
		case MENU_TOGGLE_ANIMATE:
			cout << "Toggle animation" << endl;
			break;
		case MENU_TOGGLE_COLORING:
			cout << "Toggle coloring" << endl;
			break;
		case MENU_RESET:
			cout << "Reset" << endl;
			break;
		default:
			break;
	}
}

// Handle resizing the window
GLvoid reshape(GLint sizex, GLint sizey)
{
	glutSetWindow(windowID);

	viewport_width = sizex;
	viewport_height = sizey;

	glViewport(0, 0, viewport_width, viewport_height);
	glutReshapeWindow(viewport_width, viewport_height);

	glutPostRedisplay();
}

// Set up GLUT
GLint init_glut(GLint *argc, char **argv)
{
	GLint id;

	glutInit(argc, argv);

	// Size and placement hints to the window system
	glutInitWindowSize(viewport_width, viewport_height);
	glutInitWindowPosition(10,10);

	// Double-buffered, RGB color mode
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// Create a GLUT window (not drawn until glutMainLoop() is entered)
	id = glutCreateWindow("CSCI441 Project 4");    

	// Register callbacks
	// Window size changes
	glutReshapeFunc(reshape);

	// Keypress handling when the current window has input focus
	glutKeyboardFunc(keyboard);

	// Window obscured/revealed event handler
	glutVisibilityFunc(NULL);

	// What to do when mouse cursor enters/exits the current window
	glutEntryFunc(NULL);

	// What to do on each display loop iteration
	glutDisplayFunc(draw);

	// Create menu
	GLint mainMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Add more sites", MENU_ADD_SITES);
	glutAddMenuEntry("Toggle site visibility", MENU_TOGGLE_SITES_VISIBLE);
	glutAddMenuEntry("Toggle animation", MENU_TOGGLE_ANIMATE);
	glutAddMenuEntry("Toggle coloring method", MENU_TOGGLE_COLORING);
	glutAddMenuEntry("Reset", MENU_RESET);

	glutSetMenu(mainMenu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	return id;
}

// OpenGL settings for 3D
void init_opengl()
{
	// Automatically scale normals to unit length after transformation
	glEnable(GL_NORMALIZE);

	// Clear to BLACK
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
}

GLint main(GLint argc, char *argv[])
{
	// Check for color filename argument
	if (argc == 2)
	{
		colorFile = argv[1];
	}

	// Initialize GLUT: register callbacks, etc.
	windowID = init_glut(&argc, argv);

	// Take care of any OpenGL state initialization
	init_opengl();

	// Start loop
	glutMainLoop();

	return 0;
}