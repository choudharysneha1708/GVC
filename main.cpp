#include <stdlib.h>
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

using namespace std;

int score = 0;
int gamePaused = 0, playing_and_paused = 0;
int brick_color = 1, ball_color = 3, level = 0, paddle_color = 2, text_color = 5, size = 1, shapeType = 1;
bool obstacleEnabled = false;
GLfloat twoModel[] = {GL_TRUE};
int game_level[] = {10, 7, 2};
float rate = game_level[level];
int PowerCount = 6;

GLfloat brick_color_array[][3] = {{1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {1, 0, 1}, {1, 1, 0}, {0, 1, 1}, {1, 1, 1}};
GLfloat paddle_color_array[][3] = {{1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {1, 0, 1}, {1, 1, 0}, {0, 1, 1}};
GLfloat text_color_array[][4] = {{1, 0, 0, 1}, {0, 0, 1, 1}, {0, 1, 0, 1}, {1, 0, 1, 1}, {1, 1, 0, 1}, {0, 1, 1, 1}};
GLfloat paddle_size[] = {2, 4, 6};
GLfloat original_paddle_size[] = {2, 4, 6};
GLfloat ball_size = 1.0; // Set the initial ball size
// The grid parameters for the bricks
int rows = 4;
int columns = 10;

const int numStars = 1000;
float stars[numStars][3]; // x, y, z positions of stars

void initializeStars()
{
        srand(static_cast<unsigned>(time(nullptr)));
        for (int i = 0; i < numStars; ++i)
        {
                stars[i][0] = static_cast<float>(rand() % 100 - 50);  // x position (-50 to 50)
                stars[i][1] = static_cast<float>(rand() % 100 - 50);  // y position (-50 to 50)
                stars[i][2] = static_cast<float>(rand() % 200 - 100); // z position (-100 to 100)
        }
}

void drawStars()
{
        glColor3f(1.0, 1.0, 1.0); // Set color to white for stars
        glPointSize(2.0);         // Set the point size

        glBegin(GL_POINTS);
        for (int i = 0; i < numStars; ++i)
        {
                glVertex3f(stars[i][0], stars[i][1], stars[i][2]);
        }
        glEnd();
}

// Structure to store the coordinates of each brick
struct brick_coords
{
        GLfloat x;
        GLfloat y;
};

// Array to store the bricks
brick_coords brick_array[50][50];
GLfloat px, bx = 0, by = -12.8, speed = 0, dirx = 0, diry = 0, start = 0;

#define MAX_PARTICLES 1000
#define NUM_PARTICLES 50

struct Particle
{
        GLfloat x, y, z;    // Position
        GLfloat vx, vy, vz; // Velocity
        GLfloat r, g, b;    // Color
        GLfloat size;       // Size
        int lifespan;       // Remaining lifespan
        GLfloat splashDirectionX, splashDirectionY, splashSpeed;
};

// Create an array to store particles
Particle particles[MAX_PARTICLES];

int numParticles = 0;
int PowerUpUsed = 0;
int powerUpStartTime[6] = {-10000, -10000, -10000, -10000, -10000, -10000};

// Power-up duration in milliseconds (5 seconds)
#define powerUpDuration 5000

// Function to handle the paddle oscillation power-up effect
void OscillatePaddleSize()
{
        int elapsedTime = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[0];
        // Check if the power-up duration is within the specified time
        if (elapsedTime < 5000)
        {
                float frequency = 0.002;
                float amplitude = 2.0;

                float oscillationFactor = sin(elapsedTime * frequency);

                int originalPaddleSize = 4;
                paddle_size[size] = originalPaddleSize + amplitude * oscillationFactor;

                // Ensure the paddle size stays within the desired range (2 to 6)
                paddle_size[size] = fmin(6, fmax(2, paddle_size[size]));
        }
        else
        {
                // Power-up duration expired, reset paddle size to original
                paddle_size[size] = original_paddle_size[size];
        }
}

// Function to handle the ball oscillation power-up effect
void OscillateBallSize()
{
        int elapsedTime = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[1];
        // Check if the power-up duration is within the specified time
        if (elapsedTime < 5000)
        {
                float frequency = 0.002;
                float amplitude = 0.5;

                float oscillationFactor = sin(elapsedTime * frequency);

                ball_size = 1.0 + amplitude * oscillationFactor;

                ball_size = fmin(1.5, fmax(0.5, ball_size));
        }
        else
        {
                // Power-up duration expired, reset ball size to original
                ball_size = 1.0; // Set the initial ball size
        }
}

// Function to emit particles at a specific location
void emitParticles(GLfloat x, GLfloat y, GLfloat z)
{
        for (int i = 0; i < NUM_PARTICLES; i++)
        {
                // Initialize particle properties (randomize as needed)
                particles[numParticles].x = x;
                particles[numParticles].y = y;
                particles[numParticles].z = z;
                particles[numParticles].vx = (rand() % 20 - 10) / 50.0;
                particles[numParticles].vy = (rand() % 20 - 10) / 50.0;
                particles[numParticles].vz = (rand() % 20 - 10) / 50.0;
                particles[numParticles].r = (rand() % 256) / 255.0;
                particles[numParticles].g = (rand() % 256) / 255.0;
                particles[numParticles].b = (rand() % 256) / 255.0;
                particles[numParticles].size = 0.2;
                particles[numParticles].lifespan = 100; // Adjust as needed
                particles[numParticles].splashDirectionX = (rand() % 200 - 100) / 100.0;
                particles[numParticles].splashDirectionY = (rand() % 200 - 100) / 100.0;
                particles[numParticles].splashSpeed = (rand() % 200 - 100) / 100.0;

                numParticles++;
                if (numParticles >= MAX_PARTICLES)
                {
                        numParticles = 0; // Restart from the beginning
                }
        }
}

// Function to update and draw particles
void updateAndDrawParticles()
{
        for (int i = 0; i < numParticles; i++)
        {
                // Update particle position and other properties
                particles[i].x += particles[i].vx;
                particles[i].y += particles[i].vy;
                particles[i].z += particles[i].vz;
                particles[i].lifespan--;

                if (particles[i].lifespan <= 0)
                {
                        // Remove the particle
                        particles[i] = particles[numParticles - 1];
                        numParticles--;
                        i--;
                }
                else
                {
                        // Update particle position based on the splash direction and speed
                        particles[i].x += particles[i].splashDirectionX * particles[i].splashSpeed;
                        particles[i].y += particles[i].splashDirectionY * particles[i].splashSpeed;
                }
        }

        // Render particles as points
        glDisable(GL_LIGHTING);
        glPointSize(5.0);
        glBegin(GL_POINTS);
        for (int i = 0; i < numParticles; i++)
        {
                glColor3f(particles[i].r, particles[i].g, particles[i].b);
                glVertex3f(particles[i].x, particles[i].y, particles[i].z);
        }
        glEnd();
        glEnable(GL_LIGHTING);
}

// Function to draw the paddle
void draw_paddle()
{
        glDisable(GL_LIGHTING);
        glColor3fv(paddle_color_array[paddle_color]);
        glBegin(GL_POLYGON);
        glVertex3f(-paddle_size[size] + px, 0 - 15, 0);
        glVertex3f(paddle_size[size] + px, 0 - 15, 0);
        glVertex3f(paddle_size[size] + px, 1 - 15, 0);
        glVertex3f(-paddle_size[size] + px, 1 - 15, 0);
        glEnd();
        glEnable(GL_LIGHTING);
}

// Function to draw the inverted triangle-shaped brick
void drawInvertedTriangleBrick(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height, int isSpcl)
{
        glDisable(GL_LIGHTING);

        // Draw the black borders
        glLineWidth(2.0);         // Set the line width for the black border
        glColor3f(0.0, 0.0, 0.0); // Set the border color

        glBegin(GL_LINES);
        // Border lines
        glVertex3f(x, y, z);
        glVertex3f(x + width / 2, y + height, z);

        glVertex3f(x, y, z);
        glVertex3f(x + width, y + height, z);

        glVertex3f(x + width / 2, y + height, z);
        glVertex3f(x + width, y + height, z);
        glEnd();

        int index = brick_color;
        if (isSpcl)
                index = (index + 1) % 6;
        // Draw the inner colored part of the brick
        int elapsedTime = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[3];
        if (elapsedTime < powerUpDuration)
        {
                glColor3fv(brick_color_array[6]);
        }
        else
                glColor3fv(brick_color_array[index]);
        glBegin(GL_TRIANGLES);
        glVertex3f(x + width / 2, y, z);
        glVertex3f(x, y + height, z);
        glVertex3f(x + width, y + height, z);
        glEnd();

        glEnable(GL_LIGHTING);
}

// Function to draw the vertical bars-shaped brick
void drawVerticalBarsBrick(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height, int isSpcl)
{
        glDisable(GL_LIGHTING);

        // Set the line width for the black border
        glLineWidth(2.0);
        glColor3f(1.0, 1.0, 1.0); // Set the border color

        glBegin(GL_LINES);
        // Left border
        glVertex3f(x, y, z);
        glVertex3f(x, y + height, z);

        // Right border
        glVertex3f(x + width, y, z);
        glVertex3f(x + width, y + height, z);

        // Top border
        glVertex3f(x, y, z);
        glVertex3f(x + width, y, z);

        // Bottom border
        glVertex3f(x, y + height, z);
        glVertex3f(x + width, y + height, z);
        glEnd();

        // Draw the inner colored part of the brick
        int index = brick_color;
        if (isSpcl)
                index = (index + 1) % 6;
        int elapsedTime = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[3];
        if (elapsedTime < powerUpDuration)
        {
                glColor3fv(brick_color_array[6]);
        }
        else
                glColor3fv(brick_color_array[index]);
        glBegin(GL_QUADS);
        glVertex3f(x, y, z);
        glVertex3f(x + width, y, z);
        glVertex3f(x + width, y + height, z);
        glVertex3f(x, y + height, z);
        glEnd();

        glEnable(GL_LIGHTING);
}

// Function to draw a semi-circle-shaped brick with borders
void drawSemiCircleBrick(GLfloat x, GLfloat y, GLfloat z, GLfloat radius, int isSpcl)
{
        glDisable(GL_LIGHTING);

        // Set the line width for the black border
        glLineWidth(2.0);
        glColor3f(0.0, 0.0, 0.0); // Set the border color

        // Draw the black borders
        glBegin(GL_LINES);
        for (int i = 0; i <= 180; i++)
        {
                float angle1 = i * M_PI / 180;
                float angle2 = (i + 1) * M_PI / 180;
                glVertex3f(x + radius * cos(angle1), y + radius * sin(angle1), z);
                glVertex3f(x + radius * cos(angle2), y + radius * sin(angle2), z);
        }
        glEnd();

        int index = brick_color;
        if (isSpcl)
                index = (index + 1) % 6;
        // Draw the inner colored part of the brick
        int elapsedTime = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[3];
        if (elapsedTime < powerUpDuration)
        {
                glColor3fv(brick_color_array[6]);
        }
        else
                glColor3fv(brick_color_array[index]);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(x, y, z); // Center of the semi-circle
        for (int i = 0; i <= 180; i++)
        {
                float angle = i * M_PI / 180;
                glVertex3f(x + radius * cos(angle), y + radius * sin(angle), z);
        }
        glEnd();

        glEnable(GL_LIGHTING);
}

void drawSmallGreyBrick(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height)
{
        glDisable(GL_LIGHTING);

        // Set the line width for the black border
        glLineWidth(2.0);
        glColor3f(0.5, 0.5, 0.5); // Set the border color to grey

        glBegin(GL_LINES);
        // Left border
        glVertex3f(x, y, z);
        glVertex3f(x, y + height, z);

        // Right border
        glVertex3f(x + width, y, z);
        glVertex3f(x + width, y + height, z);

        // Top border
        glVertex3f(x, y, z);
        glVertex3f(x + width, y, z);

        // Bottom border
        glVertex3f(x, y + height, z);
        glVertex3f(x + width, y + height, z);
        glEnd();

        // Draw the inner colored part of the brick (grey)
        glColor3f(0.5, 0.5, 0.5);
        glBegin(GL_QUADS);
        glVertex3f(x, y, z);
        glVertex3f(x + width, y, z);
        glVertex3f(x + width, y + height, z);
        glVertex3f(x, y + height, z);
        glEnd();

        glEnable(GL_LIGHTING);
}

// Modify the brick function to accept shapeType
void brick(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height, int shapeType, int isSpcl)
{
        // Call the appropriate shape-drawing function based on shapeType
        switch (shapeType)
        {
        case 1:
                drawInvertedTriangleBrick(x, y, z, width, height, isSpcl);
                break;
        case 2:
                drawVerticalBarsBrick(x, y, z, width, height, isSpcl);
                break;
        case 3:
                drawSemiCircleBrick(x, y, z, width / 2, isSpcl); // Adjust the radius as needed
                break;
        case 4:
                drawSmallGreyBrick(x, y, z, width, height);
                break;
        }
}
struct brick_coords obstacle_positions[5];

// Function to draw the grid of bricks
void draw_bricks()
{
        int i, j;
        if (start == 0)
        {
                for (i = 1; i <= rows; i++)
                {
                        for (j = 1; j <= columns; j++)
                        {
                                brick_array[i][j].x = (GLfloat)(j * 4 * 0.84);
                                brick_array[i][j].y = (GLfloat)(i * 2 * 0.6);
                        }
                }

                if (obstacleEnabled)
                {
                        for (int k = 0; k < 5; k++)
                        {
                                int randomRow = (rand() + rows) % rows - 7;
                                int randomColumn = rand() % columns + 1;
                                obstacle_positions[k].x = (GLfloat)(randomColumn * 4 * 0.84);
                                obstacle_positions[k].y = (GLfloat)(randomRow * 2 * 0.6);
                        }
                }
        }

        glPushMatrix();
        glTranslatef(-19.5, 5, 0);

        if (obstacleEnabled)
        {
                for (int k = 0; k < 5; k++)
                {
                        brick(obstacle_positions[k].x, obstacle_positions[k].y, 0, 1.5, 1, 4, 0);
                }
        }
        int cnt = 0;
        int bit = rand() % 2;
        if (bit == 1)
                PowerCount = 4;
        // Draw regular bricks
        for (i = 1; i <= rows; i += 1)
        {
                for (j = 1; j <= columns; j += 1)
                {
                        cnt++;
                        if (brick_array[i][j].x != 0 && brick_array[i][j].y != 0)
                        {
                                brick(brick_array[i][j].x, brick_array[i][j].y, 0, 3, 1.5, shapeType, cnt % PowerCount == 0);
                        }
                }
        }

        glPopMatrix();
}

// Function to draw the spherical ball
void draw_ball()
{
        GLfloat ambient1[] = {1, 1, 1};
        GLfloat diffuse1[] = {0.4, 0.4, 0.4};
        GLfloat specular1[] = {1, 1, 1};

        GLfloat position[] = {0, 0, -50, 1};
        GLfloat ambient2[] = {0, 0, 0};
        GLfloat diffuse2[] = {1, 1, 1};
        GLfloat specular2[] = {0, 1, 1};

        float materialColours[][3] = {{1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {1, 0, 1}, {1, 1, 0}, {0, 1, 1}, {0.2, 0.2, 0.2}, {1, 1, 1}};
        GLfloat matAmbient1[] = {1, 1, 1};
        GLfloat matDiffuse1[] = {1, 1, 1};
        GLfloat matSpecular1[] = {1, 1, 1};
        GLfloat shininess[] = {1000};

        glLightfv(GL_LIGHT0, GL_SPECULAR, specular1);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient1);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse1);

        glLightfv(GL_LIGHT1, GL_POSITION, position);
        glLightfv(GL_LIGHT1, GL_SPECULAR, specular2);
        glLightfv(GL_LIGHT1, GL_AMBIENT, ambient2);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse2);

        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

        int elapsedTime1 = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[2];
        int elapsedTime = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[3];
        if (elapsedTime < powerUpDuration)
        {
                // Indestructive bricks
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialColours[7]);
        }
        else if (elapsedTime1 < powerUpDuration)
        {
                // Heavy ball powerup
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialColours[6]);
        }
        else
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialColours[ball_color]);

        glPushMatrix();
        glTranslatef(bx, by, 0);

        // Osicllating ball powerup
        elapsedTime = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[1];
        if (elapsedTime < powerUpDuration)
        {
                glScalef(ball_size, ball_size, 0.5);
        }
        else
                glScalef(1.0, 1.0, 0.5);

        glutSolidSphere(1.0, 52, 52);

        glPopMatrix();
}

// mouse function
void mousemotion(int x, int y)
{
        if (start == 1)
        {
                // Calculate the desired position based on the mouse cursor
                float targetPx = (x - glutGet(GLUT_WINDOW_WIDTH) / 2) / 20;

                // Add some acceleration or damping factor for smoother movement
                const float acceleration = 0.2; // Adjust as needed
                px += (targetPx - px) * acceleration;

                // Clamp the paddle position within the game area
                if (px > 15)
                {
                        px = 15;
                }
                if (px < -15)
                {
                        px = -15;
                }
        }
        else
        {
                glutSetCursor(GLUT_CURSOR_INHERIT);
        }
        glutPostRedisplay();
}

// handle brick color
void change_brick_color(int action)
{

        brick_color = action - 1;
}

// handle ball color
void change_ball_color(int action)
{

        ball_color = action - 1;
}

// handle level
void change_difficulty(int action)
{

        level = action - 1;
}

// handle menu
void handle_menu(int action)
{
}

// handle paddle color
void change_paddle_color(int action)
{
        paddle_color = action - 1;
}

// handle paddle color
void change_text_color(int action)
{
        text_color = action - 1;
}

// handle paddle size
void change_paddle_size(int action)
{
        size = action - 1;
}

void change_brick_shape(int action)
{
        // Update the shapeType variable based on the selected shape
        shapeType = action;
}

void toggleObstacles(int action)
{
        obstacleEnabled = (action == 1); // Assuming action 1 enables obstacles, and action 0 disables them
}

// add menu
void addMenu()
{

        int submenu1 = glutCreateMenu(change_brick_color);
        glutAddMenuEntry("Red", 1);
        glutAddMenuEntry("Blue", 2);
        glutAddMenuEntry("Green", 3);
        glutAddMenuEntry("Purple", 4);
        glutAddMenuEntry("Yellow", 5);
        glutAddMenuEntry("Cyan", 6);

        int submenu2 = glutCreateMenu(change_ball_color);
        glutAddMenuEntry("Red", 1);
        glutAddMenuEntry("Blue", 2);
        glutAddMenuEntry("Green", 3);
        glutAddMenuEntry("Purple", 4);
        glutAddMenuEntry("Yellow", 5);
        glutAddMenuEntry("Cyan", 6);

        int submenu4 = glutCreateMenu(change_paddle_color);
        glutAddMenuEntry("Red", 1);
        glutAddMenuEntry("Blue", 2);
        glutAddMenuEntry("Green", 3);
        glutAddMenuEntry("Purple", 4);
        glutAddMenuEntry("Yellow", 5);
        glutAddMenuEntry("Cyan", 6);

        int submenu3 = glutCreateMenu(change_difficulty);
        glutAddMenuEntry("Easy", 1);
        glutAddMenuEntry("Medium", 2);
        glutAddMenuEntry("Hard", 3);

        int submenu5 = glutCreateMenu(change_text_color);
        glutAddMenuEntry("Red", 1);
        glutAddMenuEntry("Blue", 2);
        glutAddMenuEntry("Green", 3);
        glutAddMenuEntry("Purple", 4);
        glutAddMenuEntry("Yellow", 5);
        glutAddMenuEntry("Cyan", 6);

        int submenu6 = glutCreateMenu(change_paddle_size);
        glutAddMenuEntry("Small", 1);
        glutAddMenuEntry("Medium", 2);
        glutAddMenuEntry("Large", 3);

        // Add new menu entries for each shape you want
        int submenu7 = glutCreateMenu(change_brick_shape);
        glutAddMenuEntry("Inverted Triangle", 1);
        glutAddMenuEntry("Vertical Bars", 2);
        glutAddMenuEntry("Semi-Circular", 3);

        int submenu8 = glutCreateMenu(toggleObstacles);
        glutAddMenuEntry("Enable Obstacles", 1);
        glutAddMenuEntry("Disable Obstacles", 0);

        glutCreateMenu(handle_menu);
        glutAddSubMenu("Bricks Color", submenu1);
        glutAddSubMenu("Ball Color", submenu2);
        glutAddSubMenu("Paddle Color", submenu4);
        glutAddSubMenu("Text Color", submenu5);
        glutAddSubMenu("Difficulty", submenu3);
        glutAddSubMenu("Paddle Size", submenu6);
        glutAddSubMenu("Brick Shape", submenu7);
        glutAddSubMenu("Toggle Obstacles", submenu8);
        glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// Function to print the score on the screen
void text(int sc)
{
        glDisable(GL_LIGHTING);
        char text[500];
        char difficulty[10];
        if (level == 0)
        {
                sprintf(difficulty, "Easy");
        }

        if (level == 1)
        {
                sprintf(difficulty, "Medium");
        }

        if (level == 2)
        {
                sprintf(difficulty, "Hard");
        }
        if (sc < 40)
                sprintf(text, "Difficulty: %s    Your Score: %d", difficulty, sc);
        else
        {
                sprintf(text, "You have won !!");
                for (int idx = 0; idx < 6; idx++)
                        powerUpStartTime[idx] = -10000;
                PowerUpUsed = 0;
                start = 0;
                by = -12.8;
                bx = 0;
                dirx = 0;
                diry = 0;
                px = 0;
        }
        if (playing_and_paused == 1)
                sprintf(text, "Difficulty: %s    Your Score: %d    Paused!!", difficulty, sc);
        // The color
        glColor4fv(text_color_array[text_color]);

        // Position of the text to be printer
        glPushMatrix();
        glTranslatef(-1, 0, 0);
        glRasterPos3f(0, 0, 20);
        for (int i = 0; text[i] != '\0'; i++)
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
        glPopMatrix();

        // Power-up is active, render the announcement
        glColor4f(1.0, 1.0, 0.0, 1.0); // Shining yellow color
        glPushMatrix();
        glTranslatef(-1, 0, 0);
        // Initialize an empty string
        sprintf(text, "");
        int sz = 0;

        // Check if a power-up is active
        int elapsedTimePowerUp = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[0];
        if (elapsedTimePowerUp < powerUpDuration)
        {
                strcat(text, "Oscillating Paddle");
                sz += 18;
        }
        elapsedTimePowerUp = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[1];
        if (elapsedTimePowerUp < powerUpDuration)
        {
                if (sz > 0)
                {
                        strcat(text, " | ");
                        sz += 3;
                }
                strcat(text, "Oscillating Ball");
                sz += 16;
        }
        elapsedTimePowerUp = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[2];
        if (elapsedTimePowerUp < powerUpDuration)
        {
                if (sz > 0)
                {
                        strcat(text, " | ");
                        sz += 3;
                }
                strcat(text, "Heavy Ball");
                sz += 10;
        }
        elapsedTimePowerUp = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[3];
        if (elapsedTimePowerUp < powerUpDuration)
        {
                if (sz > 0)
                {
                        strcat(text, " | ");
                        sz += 3;
                }
                strcat(text, "Indestructible Bricks");
                sz += 21;
        }
        if (powerUpStartTime[4] > 0)
        {
                if (sz > 0)
                {
                        strcat(text, " | ");
                        sz += 3;
                }
                strcat(text, "Extra Life Available");
                sz += 20;
        }

        float xPos = -sz * 0.039 / 2.0;
        glRasterPos3f(xPos, -0.29, 18);

        for (int i = 0; text[i] != '\0'; i++)
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
        glPopMatrix();
        glEnable(GL_LIGHTING);
}

// The main display function
void display(void)
{

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        // Draw space background with stars
        drawStars();
        gluLookAt(0, 0, 0, 0, 0, -25, 0, 1, 0);
        glTranslatef(0, 0, -25);
        draw_paddle();
        draw_bricks();
        draw_ball();
        updateAndDrawParticles();
        text(score);
        glutSwapBuffers();
}

// function to turn on lights
void lightsOn()
{
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
}

void reshape(int w, int h)
{
        glViewport(0, 0, (GLsizei)w, (GLsizei)h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60, (GLfloat)w / (GLfloat)h, 1.0, 1000.0);
        glMatrixMode(GL_MODELVIEW);
}

// function to take in keyboard entries
void keyboard(unsigned char key, int x, int y)
{
        switch (key)
        {
        case 'd':
                px += 3;
                break;
        case 'a':
                px -= 3;
                break;
        case 'q':
                exit(0);
                break;
        case 's':
                if (!start)
                {
                        dirx = diry = 1;
                        rate = game_level[level];
                        start = 1;
                        score = 0;
                        glutSetCursor(GLUT_CURSOR_NONE);
                }
                break;
        case ' ':
                // Toggle the game state (play/pause) when the space bar is pressed
                gamePaused = !gamePaused;
                playing_and_paused = !playing_and_paused;
                break;
        }
        if (px > 15)
        {
                px = 15;
        }
        if (px < -15)
        {
                px = -15;
        }
        if (start == 0)
        {
                px = 0;
        }
        // If the game is not paused, update the display
        if (!gamePaused)
        {
                glutPostRedisplay();
        }
}

void hit()
{
        int i, j, cnt = 0;
        for (i = 1; i <= rows; i++)
        {
                for (j = 1; j <= columns; j++)
                {
                        cnt++;
                        if ((bx >= brick_array[i][j].x - 19.5 - 0.1) && (bx <= brick_array[i][j].x + 3 - 19.5 + 0.1))
                        {
                                if (by >= brick_array[i][j].y + 5 - 0.1 && by <= brick_array[i][j].y + 5 + 1.2 + 0.1)
                                {
                                        int elapsedTime = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[3];
                                        if (elapsedTime >= powerUpDuration)
                                        {
                                                // Emit particles at the collision point
                                                emitParticles(bx, by, 0);
                                                brick_array[i][j].x = 0;
                                                brick_array[i][j].y = 0;
                                                score++;
                                                if (cnt % PowerCount == 0)
                                                {
                                                        powerUpStartTime[PowerUpUsed] = glutGet(GLUT_ELAPSED_TIME);
                                                        PowerUpUsed++;
                                                        if (PowerUpUsed == 6)
                                                        {
                                                                for (int r = max(1, i - 1); r <= min(rows, i + 1); r++)
                                                                {
                                                                        for (int c = max(1, j - 1); c <= min(columns, j + 1); c++)
                                                                        {
                                                                                // complete this loop to destroy a box zone when specific brick is hit
                                                                                if (brick_array[r][c].x > 0)
                                                                                {
                                                                                        score++;
                                                                                        // Emit particles at the collision point
                                                                                        emitParticles(brick_array[r][c].x + 19.5, brick_array[r][c].y + 5, 0);
                                                                                        // Destroy the brick at position (r, c)
                                                                                        brick_array[r][c].x = 0;
                                                                                        brick_array[r][c].y = 0;
                                                                                }
                                                                        }
                                                                }
                                                        }
                                                        PowerUpUsed %= 6;
                                                }
                                        }
                                        // Heavy ball powerup
                                        elapsedTime = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[2];
                                        if (elapsedTime < powerUpDuration)
                                        {
                                                // do nothing
                                        }
                                        else
                                                diry = diry * -1;
                                }
                        }
                        else if (by >= brick_array[i][j].y + 5 - 0.1 && by <= brick_array[i][j].y + 5 + 1.2 + 0.1)
                        {
                                if ((bx >= brick_array[i][j].x - 19.5 - 0.1) && (bx <= brick_array[i][j].x + 3 - 19.5 + 0.1))
                                {
                                        int elapsedTime = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[3];
                                        if (elapsedTime >= powerUpDuration)
                                        {
                                                // Emit particles at the collision point
                                                emitParticles(bx, by, 0);
                                                brick_array[i][j].x = 0;
                                                brick_array[i][j].y = 0;
                                                score++;
                                                if (cnt % PowerCount == 0)
                                                {
                                                        powerUpStartTime[PowerUpUsed] = glutGet(GLUT_ELAPSED_TIME);
                                                        PowerUpUsed++;
                                                        if (PowerUpUsed == 6)
                                                        {
                                                                for (int r = max(1, i - 1); r <= min(rows, i + 1); r++)
                                                                {
                                                                        for (int c = max(1, j - 1); c <= min(columns, j + 1); c++)
                                                                        {
                                                                                // complete this loop to destroy a box zone when specific brick is hit
                                                                                if (brick_array[r][c].x > 0)
                                                                                {
                                                                                        score++;
                                                                                        // Emit particles at the collision point
                                                                                        emitParticles(brick_array[r][c].x + 19.5, brick_array[r][c].y + 5, 0);
                                                                                        // Destroy the brick at position (r, c)
                                                                                        brick_array[r][c].x = 0;
                                                                                        brick_array[r][c].y = 0;
                                                                                }
                                                                        }
                                                                }
                                                        }
                                                        PowerUpUsed %= 6;
                                                }
                                        }
                                        elapsedTime = glutGet(GLUT_ELAPSED_TIME) - powerUpStartTime[2];
                                        if (elapsedTime < powerUpDuration)
                                        {
                                                // do nothing
                                        }
                                        else
                                                dirx = dirx * -1;
                                }
                        }
                }
        }
        // Check collision with grey bricks (solid obstacle bricks)
        if (obstacleEnabled)
        {
                for (int k = 0; k < 5; k++)
                {
                        if ((bx >= obstacle_positions[k].x - 19.5 - 0.1) && (bx <= obstacle_positions[k].x + 3 - 19.5 + 0.1))
                        {
                                if (by >= obstacle_positions[k].y + 5 - 0.1 && by <= obstacle_positions[k].y + 5 + 1.2 + 0.1)
                                {
                                        // Emit particles at the collision point

                                        diry = diry * -1;
                                }
                        }
                        else if (by >= obstacle_positions[k].y + 5 - 0.1 && by <= obstacle_positions[k].y + 5 + 1.2 + 0.1)
                        {
                                if ((bx >= obstacle_positions[k].x - 19.5 - 0.1) && (bx <= obstacle_positions[k].x + 3 - 19.5 + 0.1))
                                {
                                        // Emit particles at the collision point

                                        dirx = dirx * -1;
                                }
                        }
                }
        }
}

// The idle function. Handles the motion of the ball along with rebounding from various surfaces
void idle()
{
        if (!gamePaused)
        {
                hit();
                if (bx < -16 || bx > 16 && start == 1)
                {
                        dirx = dirx * -1;
                }
                if (by < -15 || by > 14 && start == 1)
                {
                        diry = diry * -1;
                }
                bx += dirx / (rate);
                by += diry / (rate);
                rate -= 0.001; // Rate at which the speed of ball increases

                float x = paddle_size[size];
                // Make changes here for the different position of ball after rebounded by paddle
                if (by <= -12.8 && bx < (px + x * 2 / 3) && bx > (px + x / 3) && start == 1)
                {
                        dirx = 1;
                        diry = 1;
                }
                else if (by <= -12.8 && bx < (px - x / 3) && bx > (px - x * 2 / 3) && start == 1)
                {
                        dirx = -1;
                        diry = 1;
                }
                else if (by <= -12.8 && bx < (px + x / 3) && bx > (px - x / 3) && start == 1)
                {
                        dirx = dirx;
                        diry = 1;
                }
                else if (by <= -12.8 && bx < (px - (x * 2 / 3)) && bx > (px - (x + 0.3)) && start == 1)
                {
                        dirx = -1.5;
                        diry = 0.8;
                }
                else if (by <= -12.8 && bx < (px + (x + 0.3)) && bx > (px + x / 3) && start == 1)
                {
                        dirx = 1.5;
                        diry = 0.8;
                }
                else if (by < -13)
                {
                        if (powerUpStartTime[4] > 0)
                        {
                                powerUpStartTime[4] = -10000;
                                diry = 1;
                        }
                        else
                        {
                                start = 0;
                                by = -12.8;
                                bx = 0;
                                dirx = 0;
                                diry = 0;
                                px = 0;
                                for (int idx = 0; idx < 6; idx++)
                                        powerUpStartTime[idx] = -10000;
                                PowerUpUsed = 0;
                                std::this_thread::sleep_for(std::chrono::seconds(1));
                        }
                }
                OscillatePaddleSize();
                OscillateBallSize();
                glutPostRedisplay();
        }
}

int main(int argc, char **argv)
{
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);

        // Enters in normal mode
        glutInitWindowSize(1800, 1200);
        glutInitWindowPosition(0, 0);
        glutCreateWindow("Brick Breaker");

        // int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
        // int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);

        // Enters full-screen mode -> game mode
        // glutGameModeString("1920x1080:32@60");
        glutEnterGameMode();

        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glEnable(GL_DEPTH_TEST);
        glutIdleFunc(idle);
        initializeStars();
        glutPassiveMotionFunc(mousemotion);
        glutKeyboardFunc(keyboard);
        lightsOn();
        addMenu();
        glutMainLoop();
        return 0;
}