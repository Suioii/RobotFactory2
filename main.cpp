#include <GL/glut.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <cstdlib>

// =====================================================================
// GLOBAL VARIABLES
// =====================================================================

float beltOffset = 0.0f;
bool isPaused = false;

// Box structure
struct Box {
    float x, y;         // Position
    float size;         // Width/height
    float color[3];     // RGB (0-1)
    bool onBelt;
};

std::vector<Box> boxes;  // All active boxes

// =====================================================================
// INITIALIZATION
// =====================================================================

void Initial() {
    glClearColor(0.15f, 0.18f, 0.22f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    srand(0); // Seed random
}

// =====================================================================
// CONVEYOR BELT DRAWING
// =====================================================================

void DrawConveyorBelt() {
    float beltX = 100.0f;
    float beltY = 200.0f;
    float beltWidth = 600.0f;
    float beltHeight = 70.0f;

    // Support legs
    glColor3f(0.2f, 0.2f, 0.25f);
    glBegin(GL_QUADS);
    glVertex2f(beltX + 30, beltY - beltHeight / 2 - 80);
    glVertex2f(beltX + 50, beltY - beltHeight / 2 - 80);
    glVertex2f(beltX + 50, beltY - beltHeight / 2);
    glVertex2f(beltX + 30, beltY - beltHeight / 2);

    glVertex2f(beltX + beltWidth - 50, beltY - beltHeight / 2 - 80);
    glVertex2f(beltX + beltWidth - 30, beltY - beltHeight / 2 - 80);
    glVertex2f(beltX + beltWidth - 30, beltY - beltHeight / 2);
    glVertex2f(beltX + beltWidth - 50, beltY - beltHeight / 2);
    glEnd();

    // Belt surface
    glColor3f(0.12f, 0.12f, 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(beltX, beltY - beltHeight / 2);
    glVertex2f(beltX + beltWidth, beltY - beltHeight / 2);
    glVertex2f(beltX + beltWidth, beltY + beltHeight / 2);
    glVertex2f(beltX, beltY + beltHeight / 2);
    glEnd();

    // Animated stripes (smooth)
    float stripeWidth = 8.0f;
    float gap = 40.0f;
    float offset = fmod(beltOffset, gap);

    for (float x = beltX + 20; x < beltX + beltWidth - 20; x += gap) {
        float pos = x + offset;
        float alpha = 0.7f + 0.3f * sin((pos / gap) * 3.14159f);
        glColor4f(0.45f, 0.45f, 0.5f, alpha);
        glBegin(GL_QUADS);
        glVertex2f(pos - stripeWidth / 2, beltY - beltHeight / 2 + 8);
        glVertex2f(pos + stripeWidth / 2, beltY - beltHeight / 2 + 8);
        glVertex2f(pos + stripeWidth / 2, beltY + beltHeight / 2 - 8);
        glVertex2f(pos - stripeWidth / 2, beltY + beltHeight / 2 - 8);
        glEnd();
    }

    // Roller wheels
    auto DrawRoller = [&](float cx, float cy, float radius) {
        glColor3f(0.35f, 0.35f, 0.4f);
        int segments = 24;
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * 3.14159f * i / segments;
            glVertex2f(cx + radius * cos(angle), cy + radius * sin(angle));
        }
        glEnd();
        };

    DrawRoller(beltX + 15, beltY, 25);
    DrawRoller(beltX + beltWidth - 15, beltY, 25);
}

// =====================================================================
// BOX FUNCTIONS
// =====================================================================

void GenerateBox() {
    Box b;
    b.x = 120.0f;                 // Start at left end of belt
    b.y = 200.0f;                 // Center of belt
    b.onBelt = true;
    b.size = 20.0f + (rand() % 3) * 10.0f; // 20, 30, or 40

    // Random colors: Red, Green, Blue, Yellow, Purple, Orange
    float colors[][3] = {
        {1.0f, 0.2f, 0.2f},  // Red
        {0.2f, 0.8f, 0.2f},  // Green
        {0.2f, 0.4f, 1.0f},  // Blue
        {1.0f, 0.8f, 0.0f},  // Yellow
        {0.8f, 0.2f, 0.8f},  // Purple
        {1.0f, 0.5f, 0.0f}   // Orange
    };
    int idx = rand() % 6;
    b.color[0] = colors[idx][0];
    b.color[1] = colors[idx][1];
    b.color[2] = colors[idx][2];

    boxes.push_back(b);
}

void UpdateBoxes() {
    // Move boxes on belt
    if (!isPaused) {
        for (auto& b : boxes) {
            if (b.onBelt) {
                b.x += 2.0f;  // Speed
            }
        }
    }

    // Remove boxes that go off the right side
    boxes.erase(std::remove_if(boxes.begin(), boxes.end(),
        [](Box& b) { return b.x > 700; }), boxes.end());

    // Spawn new boxes (every 60 frames, max 8 boxes)
    static int frameCount = 0;
    if (!isPaused) {
        frameCount++;
        if (frameCount % 60 == 0 && boxes.size() < 8) {
            GenerateBox();
        }
    }
}

void DrawBoxes() {
    for (const auto& b : boxes) {
        glColor3f(b.color[0], b.color[1], b.color[2]);
        glBegin(GL_QUADS);
        // Draw box with a small border effect
        glVertex2f(b.x - b.size / 2, b.y - b.size / 2);
        glVertex2f(b.x + b.size / 2, b.y - b.size / 2);
        glVertex2f(b.x + b.size / 2, b.y + b.size / 2);
        glVertex2f(b.x - b.size / 2, b.y + b.size / 2);
        glEnd();
        // Add a subtle highlight line on top
        glColor3f(1.0f, 1.0f, 1.0f);
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glVertex2f(b.x - b.size / 2 + 2, b.y + b.size / 2 - 2);
        glVertex2f(b.x + b.size / 2 - 2, b.y + b.size / 2 - 2);
        glEnd();
    }
}

// =====================================================================
// DISPLAY
// =====================================================================

void Display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Background floor
    glColor3f(0.12f, 0.14f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(800, 0);
    glVertex2f(800, 600); glVertex2f(0, 600);
    glEnd();

    DrawConveyorBelt();
    DrawBoxes();

    // Info overlay
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(20, 570);
    const char* title = "CONVEYOR BELT WITH BOXES (Step 2)";
    for (const char* c = title; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glColor3f(0.7f, 0.7f, 0.7f);
    glRasterPos2f(20, 550);
    char info[100];
    sprintf_s(info, "Boxes: %zu   |   SPACE: Pause   |   R: Reset   |   ESC: Exit", boxes.size());
    for (char* c = info; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    glFlush();
}

// =====================================================================
// TIMER
// =====================================================================

void Timer(int value) {
    if (!isPaused) {
        beltOffset += 1.5f;
        if (beltOffset > 40.0f) beltOffset -= 40.0f;
        UpdateBoxes();
    }
    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0);
}

// =====================================================================
// KEYBOARD
// =====================================================================

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case ' ': // Pause
        isPaused = !isPaused;
        break;
    case 'r': // Reset
    case 'R':
        boxes.clear();
        beltOffset = 0.0f;
        break;
    case 27: // ESC
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
    glutCreateWindow("Conveyor Belt with Boxes");

    Initial();

    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard);
    glutTimerFunc(16, Timer, 0);

    glutMainLoop();
    return 0;
}
