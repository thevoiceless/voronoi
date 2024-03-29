// Riley Moses

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <ctime>
#include <fstream>
#include <cassert>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "Point.h"
#include "RGB.h"

using namespace std;

// OpenGL variables
static const int VIEWPORT_DEFAULT = 700;
static const double TWOPI = (2.0 * M_PI);
static const int MENU_ADD_SITES = 1;
static const int MENU_REMOVE_SITES = 2;
static const int MENU_TOGGLE_SITES_VISIBLE = 3;
static const int MENU_TOGGLE_ANIMATE = 4;
static const int MENU_TOGGLE_COLORING = 5;
static const int MENU_RESET = 6;
// GLUT window id; value asigned in main() and should stay constant
GLint windowID;
// Viewport dimensions; change when window is resized (via resize callback)
GLint viewport_width = VIEWPORT_DEFAULT;
GLint viewport_height = VIEWPORT_DEFAULT;
// Variables for sites
static const double MAX_COORD = 1.0;
static const double MIN_COORD = -1.0;
static const int INITIAL_NUM_SITES = 32;
static const int MAX_NUM_SITES = 65536;
int currentNumSites = 32;
static const double ONE_THIRD = 1.0 / 3.0;
static const int NUM_TRIANGLES_IN_CONE = 200;
// File to use for color lookup
string colorFile = "input.ppm";
int ppmResX, ppmResY;
RGB* ppm;
// Whether or not to animate
bool animate = false;
int t = 0;
// Whether or not sites are visible
bool sitesVisible = true;
// Coloring mode
bool useColorFromTexture = false;
// Mouse down boolean
bool mouseDown = false;
// Vector of sites
vector<Point> sites;
// Vector of velocities for the sites
vector<double> velocities;
// Vector of points for the cone
vector<Point> cone;
// Vector of colors for the cones
vector<Point> coneColors;
// Vector of RGB for the texture colors
vector<RGB> ppmColors;

// Return a random color as a Point
Point getRandomColor()
{
	return Point(((double) rand() / (double) RAND_MAX),
		((double) rand() / (double) RAND_MAX),
		((double) rand() / (double) RAND_MAX),
		0);
}

// Generate MAX_NUM_SITES between (-1, -1) and (1, 1)
void generateSites()
{
	for (int i = 0; i < MAX_NUM_SITES; ++i)
	{
		double x = (MAX_COORD - MIN_COORD) * ((double) rand() / (double) RAND_MAX) + MIN_COORD;
		double y = (MAX_COORD - MIN_COORD) * ((double) rand() / (double) RAND_MAX) + MIN_COORD;
		// z values between 0 and 1
		//double z = ((double) rand() / (double) RAND_MAX);
		double z = 0;
		sites.push_back(Point(x, y, z, 1));
	}
}

// Generate a random velocity between -1 and 1 for each site
void generateVelocities()
{
	for (int i = 0; i < MAX_NUM_SITES; ++i)
	{
		velocities.push_back(2 * ((double) rand() / (double) RAND_MAX) - 1);
	}
}

// Double the number of sites, up to MAX_NUM_SITES
void doubleSites()
{
	if (currentNumSites < MAX_NUM_SITES)
	{
		currentNumSites *= 2;
		cout << currentNumSites << " sites" << endl;
	}
}

// Halve the number of sites, down to ITITIAL_NUM_SITES
void halveSites()
{
	if (currentNumSites > INITIAL_NUM_SITES)
	{
		currentNumSites /= 2;
		cout << currentNumSites << " sites" << endl;
	}
}

// Show or hide the sites
void toggleSiteVisibility()
{
	sitesVisible = !sitesVisible;
	glutPostRedisplay();
}

// Toggle animation
void toggleAnimation()
{
	if (!useColorFromTexture)
	{
		animate = !animate;
	}
}

// Toggle color lookup between random and PPM
void toggleColorFromTexture()
{
	useColorFromTexture = !useColorFromTexture;
	if (useColorFromTexture)
	{
		t = 0;
		animate = false;
	}
}

// Reset
void resetEverything()
{
	currentNumSites = INITIAL_NUM_SITES;
	animate = false;
	sitesVisible = true;
	useColorFromTexture = false;
	t = 0;
}

// Generate cone with point at (0, 0, 0) expanding out to infinity
void generateCone()
{
	// Tip of cone has homogenous coordinate of 1
	cone.push_back(Point(0, 0, 0, 1));

	double thetaStep = TWOPI / NUM_TRIANGLES_IN_CONE;
	for (double i = 0; i < TWOPI - 0.5 * thetaStep; i += thetaStep)
	{
		cone.push_back(Point(sin(i), cos(i), ONE_THIRD, 0));
	}

	// Repeat first vertex from the loop to close the circle
	cone.push_back(Point(sin(0), cos(0), ONE_THIRD, 0));
}

// Generate a random color for each cone
void generateConeColors()
{
	for (int i = 0; i < MAX_NUM_SITES; ++i)
	{
		coneColors.push_back(getRandomColor());
	}
}

// Load color information from PPM lookup file
void loadPPM()
{
	// Open the texture file
	ifstream inFile(colorFile.c_str(), ios::binary);
	if (!inFile.is_open())
	{
	  cout << "Error reading from " << colorFile << endl;
	  exit(1);
	}
	// Binary PPMs start with 'P6'
	char c;
	inFile >> c;
	assert(c == 'P');
	inFile >> c;
	assert(c == '6');
	inFile >> ppmResX >> ppmResY;
	// All images we'll use have 255 color levels
	int i;
	inFile >> i;
	assert(i == 255);
	// Need to skip one more byte
	inFile.get();
	// Allocate space
	ppm = new RGB[ppmResX * ppmResY];
	inFile.read((char*) ppm, ppmResX * ppmResY * sizeof(RGB));
	inFile.close();
}

void setColorsFromPPM()
{
	//Look up color for the cone of a site at (x, y)
	for (int i = 0; i < MAX_NUM_SITES; ++i)
	{
		// Scale x and y from -1...1 to 0...resolution, flip y-axis orientation
		double sx = ppmResX * (sites[i].x + 1) / 2;
		double sy = ppmResY * (-sites[i].y + 1) / 2;
		// Round to nearest integer
		int ix = (int) floor(sx + 0.5);
		int iy = (int) floor(sy + 0.5);
		// Clamp to the range 0...(resolution-1)
		if (ix < 0)
		{
			ix = 0;
		}
		if (ix >= ppmResX)
		{
			ix = ppmResX - 1;
		}
		if (iy < 0)
		{
			iy = 0;
		}
		if (iy >= ppmResY)
		{
			iy = ppmResY - 1;
		}
		// Color to be used: ppm[ix+iy*resolution_x]
		ppmColors.push_back(ppm[ix + iy * ppmResX]);
	}
}

GLuint draw_sites()
{
	// Disable depth test when drawing points so that they are always shown
	// This is to handle possible numerical errors with the location of the cones' points
	glDisable(GL_DEPTH_TEST);
	for (int i = 0; i < currentNumSites; ++i)
	{
		glPushMatrix();
			glLoadIdentity();
			glRotatef(t * velocities[i], 0, 0, 1);
			glBegin(GL_POINTS);
				glColor3f(0.0, 00.0, 0.0);
				glVertex3f(sites[i].x, sites[i].y, sites[i].z);
			glEnd();
		glPopMatrix();
	}
	// Re-enable depth test when finished
	glEnable(GL_DEPTH_TEST);
}

GLuint draw_cones()
{
	// Set the modelview matrix
	glMatrixMode(GL_MODELVIEW);

	glPointSize(2.0);
	
	for (int i = 0; i < currentNumSites; ++i)
	{
		// Clear each cone translation when finished
		glPushMatrix();
			glLoadIdentity();
			glRotatef(t * velocities[i], 0, 0, 1);
			glTranslatef(sites[i].x, sites[i].y, sites[i].z);
			glBegin(GL_TRIANGLE_FAN);
				for (int j = 0; j < cone.size(); ++j)
				{
					if (useColorFromTexture)
					{
						glColor3ub(ppmColors[i].r, ppmColors[i].g, ppmColors[i].b);
					}
					else
					{
						glColor3f(coneColors[i].x, coneColors[i].y, coneColors[i].z);
					}
					glVertex4f(cone[j].x, cone[j].y, cone[j].z, cone[j].homogenous);
				}
			glEnd();
		glPopMatrix();
	}
	
}

GLuint draw_scene()
{
	// draw_cones();
	// if (sitesVisible)
	// {
	// 	draw_sites();
	// }

	// Below is a combination of draw_cones() and draw_sites()
	// Wraps everything in a single loop over the current number of sites
	glMatrixMode(GL_MODELVIEW);
	glPointSize(2.0);

	for (int i = 0; i < currentNumSites; ++i)
	{
		// Load identity
		glLoadIdentity();
		// Rotate according to velocity
		glRotatef(t * velocities[i], 0, 0, 1);
		// Translate the cone to the site
		glTranslatef(sites[i].x, sites[i].y, sites[i].z);
		// Draw the cone
		glBegin(GL_TRIANGLE_FAN);
			for (int j = 0; j < cone.size(); ++j)
			{
				// Look up colors from PPM
				if (useColorFromTexture)
				{
					glColor3ub(ppmColors[i].r, ppmColors[i].g, ppmColors[i].b);
				}
				// Use randomly chosen colors
				else
				{
					glColor3f(coneColors[i].x, coneColors[i].y, coneColors[i].z);
				}
				glVertex4f(cone[j].x, cone[j].y, cone[j].z, cone[j].homogenous);
			}
		glEnd();
		// Draw the sites
		if (sitesVisible)
		{
			// Load identity
			glLoadIdentity();
			// Rotate according to velocity
			glRotatef(t * velocities[i], 0, 0, 1);
			// Draw the site
			glBegin(GL_POINTS);
				glColor3f(0.0, 0.0, 0.0);
				glVertex3f(sites[i].x, sites[i].y, sites[i].z);
			glEnd();
		}
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

	if (animate)
	{
		t++;
	}

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
			delete[] ppm;
			exit(0);
		case 'm':
		case 'M':
			doubleSites();
			glutPostRedisplay();
			break;
		case 'n':
		case 'N':
			halveSites();
			glutPostRedisplay();
			break;
		case 'v':
		case 'V':
			toggleSiteVisibility();
			break;
		case 'r':
		case 'R':
			resetEverything();
			glutPostRedisplay();
			break;
		case 'a':
		case 'A':
			toggleAnimation();
			glutPostRedisplay();
			break;
		case 'c':
		case 'C':
			toggleColorFromTexture();
			glutPostRedisplay();
			break;
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
			glutPostRedisplay();
			break;
		case MENU_REMOVE_SITES:
			halveSites();
			glutPostRedisplay();
			break;
		case MENU_TOGGLE_SITES_VISIBLE:
			toggleSiteVisibility();
			break;
		case MENU_TOGGLE_ANIMATE:
			toggleAnimation();
			glutPostRedisplay();
			break;
		case MENU_TOGGLE_COLORING:
			toggleColorFromTexture();
			glutPostRedisplay();
			break;
		case MENU_RESET:
			resetEverything();
			glutPostRedisplay();
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
	glutInitWindowPosition(10, 10);

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
	glutAddMenuEntry("Reduce number of sites", MENU_REMOVE_SITES);
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

	// Generate the sites
	generateSites();
	// Generate velocities
	generateVelocities();
	// Generate the cone
	generateCone();
	// Gnerate the random cone colors
	generateConeColors();
	// Load PPM texture
	loadPPM();
	// Set colors based on texture
	setColorsFromPPM();

	// Initialize GLUT: register callbacks, etc.
	windowID = init_glut(&argc, argv);

	// Take care of any OpenGL state initialization
	init_opengl();

	// Start loop
	glutMainLoop();

	return 0;
}