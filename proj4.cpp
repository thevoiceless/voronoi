// Riley Moses

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <ctime>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "Site.h"

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

// Number of sites
static const double MAX_COORD = 1.0;
static const double MIN_COORD = -1.0;
static const int INITIAL_NUM_SITES = 32;
static const int MAX_NUM_SITES = 65536;
int currentNumSites = 32;
// File to use for color lookup
string colorFile = "input.ppm";
// Whether or not to animate
bool animate = false;
// Whether or not sites are visible
bool sitesVisible = true;
// Coloring mode
bool useTextureColor = false;
// Vector of sites
vector<Site> sites;

// Generate MAX_NUM_SITES between (-1, -1) and (1, 1) with z values between 0 and 1
void generateSites()
{
	for (int i = 0; i < MAX_NUM_SITES; ++i)
	{
		double x = (MAX_COORD - MIN_COORD) * ((double) rand() / (double) RAND_MAX) + MIN_COORD;
		double y = (MAX_COORD - MIN_COORD) * ((double) rand() / (double) RAND_MAX) + MIN_COORD;
		double z = ((double) rand() / (double) RAND_MAX);
		sites.push_back(Site(x, y, z));
	}
}

// Double the number of sites, up to MAX_NUM_SITES
void doubleSites()
{
	if (currentNumSites < MAX_NUM_SITES)
	{
		currentNumSites *= 2;
	}
	glutPostRedisplay();
}

// Show or hide the sites
void toggleSiteVisibility()
{
	sitesVisible = !sitesVisible;
	glutPostRedisplay();
}

// Reset
void resetEverything()
{
	currentNumSites = INITIAL_NUM_SITES;
	animate = false;
	sitesVisible = true;
	useTextureColor = false;
}

GLuint draw_sites()
{
	glBegin(GL_POINTS);
		for (int i = 0; i < currentNumSites; ++i)
		{
			glColor3f(1.0f,1.0f,1.0f);
			glVertex3f(sites[i].x, sites[i].y, sites[i].z);
		}
	glEnd();
}

GLuint draw_scene()
{
	if (sitesVisible)
	{
		draw_sites();
	}
}

// Draw callback
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

	// Draw
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
			doubleSites();
			break;
		case MENU_TOGGLE_SITES_VISIBLE:
			toggleSiteVisibility();
			break;
		case MENU_TOGGLE_ANIMATE:
			cout << "Toggle animation" << endl;
			break;
		case MENU_TOGGLE_COLORING:
			cout << "Toggle coloring" << endl;
			break;
		case MENU_RESET:
			cout << "Reset" << endl;
			resetEverything();
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

	// Set background color
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	//glPointSize(2.0f);
}

GLint main(GLint argc, char *argv[])
{
	// Check for color filename argument
	if (argc == 2)
	{
		colorFile = argv[1];
	}

	// Initialize random seed
	srand(time(NULL));

	generateSites();

	// Initialize GLUT: register callbacks, etc.
	windowID = init_glut(&argc, argv);

	// Take care of any OpenGL state initialization
	init_opengl();

	// Start loop
	glutMainLoop();

	return 0;
}