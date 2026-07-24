

#include <GL/glut.h>
#include <math.h>
#include <cstdlib>
#include <cstdio>

// =====================================================================
// GLOBAL VARIABLES
// =====================================================================

float clawAngle = 30.0f;      // 30° = open, 0° = closed
float targetAngle = 30.0f;    // Target angle (for smooth animation)
bool isPaused = false;

// =====================================================================
// INITIALIZATION
// =====================================================================

void Initial() {
    glClearColor(0.12f, 0.14f, 0.18f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
}

// =====================================================================
// DRAW A SINGLE FINGER – now pointing DOWN
// =====================================================================

void DrawFinger(float baseX, float baseY, float angle, bool isLeft, float length) {
    // angle controls how much the finger bends
    // We'll draw the finger going downward (negative Y)
    float jointAngle = angle * 0.6f;
    float tipAngle = angle * 0.3f;

    glPushMatrix();
    glTranslatef(baseX, baseY, 0.0f);

    // Finger base (thick part)
    glColor3f(0.4f, 0.4f, 0.5f);
    glBegin(GL_QUADS);
    if (isLeft) {
        glVertex2f(-8, 0);
        glVertex2f(-4, 0);
        glVertex2f(-4, -length * 0.3f);
        glVertex2f(-8, -length * 0.3f);
    }
    else {
        glVertex2f(4, 0);
        glVertex2f(8, 0);
        glVertex2f(8, -length * 0.3f);
        glVertex2f(4, -length * 0.3f);
    }
    glEnd();

    // ----- FIRST JOINT (main bend) -----
    glTranslatef(0, -length * 0.3f, 0);
    if (isLeft) {
        glRotatef(jointAngle, 0, 0, 1);   // bend inward
    }
    else {
        glRotatef(-jointAngle, 0, 0, 1);
    }

    // Finger middle segment
    glColor3f(0.5f, 0.5f, 0.6f);
    glBegin(GL_QUADS);
    if (isLeft) {
        glVertex2f(-6, 0);
        glVertex2f(-3, 0);
        glVertex2f(-3, -length * 0.4f);
        glVertex2f(-6, -length * 0.4f);
    }
    else {
        glVertex2f(3, 0);
        glVertex2f(6, 0);
        glVertex2f(6, -length * 0.4f);
        glVertex2f(3, -length * 0.4f);
    }
    glEnd();

    // Joint circle
    glColor3f(0.3f, 0.3f, 0.4f);
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= 20; i++) {
        float a = 2.0f * 3.14159f * i / 20;
        glVertex2f(0 + 5 * cos(a), 0 + 5 * sin(a));
    }
    glEnd();

    // ----- SECOND JOINT (tip bend) -----
    glTranslatef(0, -length * 0.4f, 0);
    if (isLeft) {
        glRotatef(tipAngle, 0, 0, 1);
    }
    else {
        glRotatef(-tipAngle, 0, 0, 1);
    }

    // Finger tip (with grip pad)
    glColor3f(0.6f, 0.6f, 0.7f);
    glBegin(GL_QUADS);
    if (isLeft) {
        glVertex2f(-5, 0);
        glVertex2f(-2, 0);
        glVertex2f(-2, -length * 0.3f);
        glVertex2f(-5, -length * 0.3f);
    }
    else {
        glVertex2f(2, 0);
        glVertex2f(5, 0);
        glVertex2f(5, -length * 0.3f);
        glVertex2f(2, -length * 0.3f);
    }
    glEnd();

    // Grip pad (rubber tip)
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    if (isLeft) {
        glVertex2f(-5, -length * 0.3f + 2);
        glVertex2f(-2, -length * 0.3f + 2);
        glVertex2f(-2, -length * 0.3f);
        glVertex2f(-5, -length * 0.3f);
    }
    else {
        glVertex2f(2, -length * 0.3f + 2);
        glVertex2f(5, -length * 0.3f + 2);
        glVertex2f(5, -length * 0.3f);
        glVertex2f(2, -length * 0.3f);
    }
    glEnd();

    glPopMatrix();
}

// =====================================================================
// DRAW THE COMPLETE MECHANICAL CLAW (FLIPPED – fingers point DOWN)
// =====================================================================

void DrawClaw() {
    float centerX = 400.0f;
    float centerY = 350.0f;      // Base is now higher up
    float fingerLength = 90.0f;
    float spread = 25.0f;

    // ----- CLAW BASE / HOUSING (top) -----
    glColor3f(0.25f, 0.25f, 0.35f);
    glBegin(GL_QUADS);
    glVertex2f(centerX - 30, centerY - 10);
    glVertex2f(centerX + 30, centerY - 10);
    glVertex2f(centerX + 35, centerY + 25);
    glVertex2f(centerX - 35, centerY + 25);
    glEnd();

    // Housing top (rounded)
    glColor3f(0.3f, 0.3f, 0.4f);
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= 20; i++) {
        float angle = 3.14159f * i / 20;
        glVertex2f(centerX + 30 * cos(angle - 3.14159f), centerY + 25 + 30 * sin(angle - 3.14159f));
    }
    glEnd();

    // Connection rod (spring mechanism) – above the base
    glColor3f(0.35f, 0.35f, 0.45f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(centerX - 10, centerY + 25);
    glVertex2f(centerX - 10, centerY + 55);
    glVertex2f(centerX + 10, centerY + 25);
    glVertex2f(centerX + 10, centerY + 55);
    glEnd();

    // ----- SPRING VISUAL (zigzag) – above the base -----
    glColor3f(0.5f, 0.5f, 0.55f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 20; i++) {
        float t = (float)i / 20.0f;
        float x = centerX + 15 * cos(t * 3.14159f * 4) * 0.5f;
        float y = centerY + 55 + t * 80;
        glVertex2f(x, y);
    }
    glEnd();
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 20; i++) {
        float t = (float)i / 20.0f;
        float x = centerX - 15 * cos(t * 3.14159f * 4) * 0.5f;
        float y = centerY + 55 + t * 80;
        glVertex2f(x, y);
    }
    glEnd();

    // Spring coils (circles)
    glColor3f(0.4f, 0.4f, 0.5f);
    for (int i = 0; i < 6; i++) {
        float y = centerY + 60 + i * 14;
        glBegin(GL_LINE_LOOP);
        for (int j = 0; j <= 16; j++) {
            float angle = 2.0f * 3.14159f * j / 16;
            glVertex2f(centerX + 8 * cos(angle), y + 4 * sin(angle));
        }
        glEnd();
    }

    // ----- SPRING COVER (top mount) -----
    glColor3f(0.3f, 0.3f, 0.4f);
    glBegin(GL_QUADS);
    glVertex2f(centerX - 15, centerY + 135);
    glVertex2f(centerX + 15, centerY + 135);
    glVertex2f(centerX + 20, centerY + 145);
    glVertex2f(centerX - 20, centerY + 145);
    glEnd();

    // ----- FINGERS (pointing DOWN) -----
    float angleRad = clawAngle * 3.14159f / 180.0f;
    float fingerSpread = 30.0f + angleRad * 30.0f;

    // Back left finger (behind)
    DrawFinger(centerX - spread - 5, centerY - 10, clawAngle * 1.1f, true, fingerLength * 0.7f);

    // Back right finger (behind)
    DrawFinger(centerX + spread + 5, centerY - 10, clawAngle * 1.1f, false, fingerLength * 0.7f);

    // Front left finger (main)
    DrawFinger(centerX - spread, centerY - 10, clawAngle, true, fingerLength);

    // Front right finger (main)
    DrawFinger(centerX + spread, centerY - 10, clawAngle, false, fingerLength);

    // ----- PIVOT JOINTS (circles at finger base) -----
    glColor3f(0.4f, 0.4f, 0.5f);
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= 20; i++) {
        float a = 2.0f * 3.14159f * i / 20;
        glVertex2f(centerX - spread + 7 * cos(a), centerY - 10 + 7 * sin(a));
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= 20; i++) {
        float a = 2.0f * 3.14159f * i / 20;
        glVertex2f(centerX + spread + 7 * cos(a), centerY - 10 + 7 * sin(a));
    }
    glEnd();
}

// =====================================================================
// HUD
// =====================================================================

void DrawHUD() {
    // Title
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(20, 570);
    const char* title = "MECHANICAL CLAW - FLIPPED (Downward Fingers)";
    for (const char* c = title; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Status
    glColor3f(0.7f, 0.7f, 0.7f);
    glRasterPos2f(20, 550);
    char info[100];
    sprintf_s(info, "Claw Angle: %.0f°   |   %s", clawAngle, isPaused ? "PAUSED" : "RUNNING");
    for (char* c = info; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    // Controls
    glColor3f(0.5f, 0.5f, 0.5f);
    glRasterPos2f(20, 20);
    const char* controls = "O: Open   |   C: Close   |   SPACE: Auto Demo   |   R: Reset   |   ESC: Exit";
    for (const char* c = controls; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
    }

    // Instructions
    glColor3f(0.6f, 0.6f, 0.6f);
    glRasterPos2f(20, 40);
    const char* instructions = "Claw fingers now point DOWN – ready to pick from below.";
    for (const char* c = instructions; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
    }
}

// =====================================================================
// DISPLAY
// =====================================================================

void Display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Background
    glColor3f(0.12f, 0.14f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(800, 0);
    glVertex2f(800, 600); glVertex2f(0, 600);
    glEnd();

    // Subtle grid
    glColor4f(0.2f, 0.2f, 0.25f, 0.3f);
    glLineWidth(1.0f);
    for (int i = 0; i < 800; i += 40) {
        glBegin(GL_LINES);
        glVertex2f(i, 0); glVertex2f(i, 600);
        glEnd();
    }
    for (int i = 0; i < 600; i += 40) {
        glBegin(GL_LINES);
        glVertex2f(0, i); glVertex2f(800, i);
        glEnd();
    }

    DrawClaw();
    DrawHUD();

    glFlush();
}

// =====================================================================
// TIMER
// =====================================================================

void Timer(int value) {
    if (!isPaused) {
        // Smoothly move toward target angle
        if (clawAngle < targetAngle) {
            clawAngle += 0.3f;
            if (clawAngle > targetAngle) clawAngle = targetAngle;
        }
        else if (clawAngle > targetAngle) {
            clawAngle -= 0.3f;
            if (clawAngle < targetAngle) clawAngle = targetAngle;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0);
}

// =====================================================================
// KEYBOARD
// =====================================================================

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'o': case 'O':
        targetAngle = 30.0f;
        break;
    case 'c': case 'C':
        targetAngle = 0.0f;
        break;
    case ' ':
        isPaused = !isPaused;
        if (!isPaused) {
            // Toggle auto open/close
            if (targetAngle > 15.0f) targetAngle = 0.0f;
            else targetAngle = 30.0f;
        }
        break;
    case 'r': case 'R':
        clawAngle = 30.0f;
        targetAngle = 30.0f;
        isPaused = false;
        break;
    case 27:
        exit(0);
        break;
    }
}

// =====================================================================
// MAIN
// =====================================================================

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Mechanical Claw - Flipped");

    Initial();

    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard);
    glutTimerFunc(16, Timer, 0);

    glutMainLoop();
    return 0;
}

