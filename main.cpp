#include <stdlib.h>
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
// #include<iostream>
// using namespace std;

int score = 0;
int gamePaused = 0, playing_and_paused = 0;
int brick_color = 1, ball_color = 3, level = 0, paddle_color = 2, text_color = 5, size = 1, shapeType = 1;
bool obstacleEnabled = false;
GLfloat twoModel[] = {GL_TRUE};
int game_level[] = {50, 20, 10};
float rate = game_level[level];

GLfloat brick_color_array[][3] = {{1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {1, 0, 1}, {1, 1, 0}, {0, 1, 1}};
GLfloat paddle_color_array[][3] = {{1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {1, 0, 1}, {1, 1, 0}, {0, 1, 1}};
GLfloat text_color_array[][4] = {{1, 0, 0, 1}, {0, 0, 1, 1}, {0, 1, 0, 1}, {1, 0, 1, 1}, {1, 1, 0, 1}, {0, 1, 1, 1}};
GLfloat paddle_size[] = {2, 4, 6};
// The grid parameters for the bricks
int rows = 4;
int columns = 10;

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
        int cnt = 0, spcl = 0;
        // Draw regular bricks
        for (i = 1; i <= rows; i += 1)
        {
                for (j = 1; j <= columns; j += 1)
                {
                        cnt++;
                        if (brick_array[i][j].x != 0 && brick_array[i][j].y != 0)
                        {
                                if (cnt % 6 == 0)
                                    spcl = 1;
                                brick(brick_array[i][j].x, brick_array[i][j].y, 0, 3, 1.5, shapeType, spcl);
                                if (cnt % 6 == 0)
                                    spcl = 0;
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

        float materialColours[][3] = {{1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {1, 0, 1}, {1, 1, 0}, {0, 1, 1}};
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
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialColours[ball_color]);

        glPushMatrix();
        glTranslatef(bx, by, 0);
        glScalef(1.0, 1.0, 0.5);
        // glScalef(size[i], size[], size[]);
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
        char text[50];
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
                start = 0;
                by = -12.8;
                bx = 0;
                dirx = 0;
                diry = 0;
                px = 0;
        }
        if (playing_and_paused == 1)
            sprintf(text,"Difficulty: %s    Your Score: %d    Paused!!",difficulty, sc);
        // The color
        glColor4fv(text_color_array[text_color]);
        // Position of the text to be printer
        glPushMatrix();
        glTranslatef(-1, 0, 0);
        glRasterPos3f(0, 0, 20);
        for (int i = 0; text[i] != '\0'; i++)
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
        glEnable(GL_LIGHTING);
        glPopMatrix();
}

// The main display function
void display(void)
{

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
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
        if (!gamePaused) {
            glutPostRedisplay();
        }
}

void hit()
{
        int i, j;
        for (i = 1; i <= rows; i++)
        {
                for (j = 1; j <= columns; j++)
                {
                        if ((bx >= brick_array[i][j].x - 19.5 - 0.1) && (bx <= brick_array[i][j].x + 3 - 19.5 + 0.1))
                        {
                                if (by >= brick_array[i][j].y + 5 - 0.1 && by <= brick_array[i][j].y + 5 + 1.2 + 0.1)
                                {
                                        // Emit particles at the collision point
                                        emitParticles(bx, by, 0);
                                        brick_array[i][j].x = 0;
                                        brick_array[i][j].y = 0;
                                        diry = diry * -1;
                                        score++;
                                }
                        }
                        else if (by >= brick_array[i][j].y + 5 - 0.1 && by <= brick_array[i][j].y + 5 + 1.2 + 0.1)
                        {
                                if ((bx >= brick_array[i][j].x - 19.5 - 0.1) && (bx <= brick_array[i][j].x + 3 - 19.5 + 0.1))
                                {
                                        // Emit particles at the collision point
                                        emitParticles(bx, by, 0);
                                        brick_array[i][j].x = 0;
                                        brick_array[i][j].y = 0;
                                        dirx = dirx * -1;
                                        score++;
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
    if (!gamePaused) {
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
                start = 0;
                by = -12.8;
                bx = 0;
                dirx = 0;
                diry = 0;
                px = 0;
        }
        glutPostRedisplay();
    }
}

int main(int argc, char **argv)
{
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
        glutInitWindowSize(900, 900);
        glutInitWindowPosition(100, 100);
        glutCreateWindow("Brick Breaker");
        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glEnable(GL_DEPTH_TEST);
        glutIdleFunc(idle);
        glutPassiveMotionFunc(mousemotion);
        glutKeyboardFunc(keyboard);
        lightsOn();
        addMenu();
        glutMainLoop();
        return 0;
}
