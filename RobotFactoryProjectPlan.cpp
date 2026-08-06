#include <GL/glut.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstdio>

// =====================================================================
// GLOBAL VARIABLES
// =====================================================================

float beltOffset = 0.0f;
bool  isPaused   = false;

// Belt / box geometry
static const float BELT_Y    = 180.0f;        // Belt center Y
static const float BELT_TOP  = BELT_Y + 30.0f;// Top surface of belt

// Finger geometry
static const float FINGER_LENGTH = 90.0f;     // Total finger reach downward

// ── ARM EXTENSION ──────────────────────────────────────────────────────
static const float HOUSING_Y      = 420.0f;   // Fixed Y of the housing center
static const float ARM_MIN_EXT    = 0.0f;     // Fully retracted
static const float ARM_EXTEND_SPD = 2.5f;     // px per frame

float armExtension       = 0.0f;   // Current extension (px downward from housing)
float armExtensionTarget = 0.0f;   // Target extension

// Derived: Y position of the finger-unit center (moves, housing doesn't)
inline float ClawHeadY() { return HOUSING_Y - armExtension; }

// Claw angle
float clawAngle  = 30.0f;   // 30 = open, 0 = closed
float targetAngle = 30.0f;

bool isHoldingBox = false;
int  heldBoxIndex = -1;

// =====================================================================
// FINITE STATE MACHINE
// =====================================================================
enum RobotState { IDLE, LOWERING, GRABBING, LIFTING, RETURNING };
RobotState robotState = IDLE;

// =====================================================================
// BOX STRUCTURE
// =====================================================================
struct Box {
    float x, y, size;
    float color[3];
    bool  onBelt;
    bool  lifted;
};
std::vector<Box> boxes;

// =====================================================================
// INITIALIZATION
// =====================================================================
void Initial() {
    glClearColor(0.12f, 0.14f, 0.18f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    srand(0);
}

// =====================================================================
// CONVEYOR BELT
// =====================================================================
void DrawConveyorBelt() {
    float beltX = 100.0f, beltY = BELT_Y, beltW = 600.0f, beltH = 60.0f;

    // Support legs
    glColor3f(0.2f, 0.2f, 0.25f);
    glBegin(GL_QUADS);
        glVertex2f(beltX+30,       beltY-beltH/2-60); glVertex2f(beltX+50,       beltY-beltH/2-60);
        glVertex2f(beltX+50,       beltY-beltH/2);    glVertex2f(beltX+30,       beltY-beltH/2);
        glVertex2f(beltX+beltW-50, beltY-beltH/2-60); glVertex2f(beltX+beltW-30, beltY-beltH/2-60);
        glVertex2f(beltX+beltW-30, beltY-beltH/2);    glVertex2f(beltX+beltW-50, beltY-beltH/2);
    glEnd();

    // Belt surface
    glColor3f(0.12f, 0.12f, 0.15f);
    glBegin(GL_QUADS);
        glVertex2f(beltX,       beltY-beltH/2); glVertex2f(beltX+beltW, beltY-beltH/2);
        glVertex2f(beltX+beltW, beltY+beltH/2); glVertex2f(beltX,       beltY+beltH/2);
    glEnd();

    // Animated stripes
    float gap = 40.0f, sw = 8.0f, off = fmod(beltOffset, gap);
    for (float x = beltX+20; x < beltX+beltW-20; x += gap) {
        float pos = x + off;
        glColor4f(0.45f,0.45f,0.5f, 0.7f+0.3f*sin((pos/gap)*3.14159f));
        glBegin(GL_QUADS);
            glVertex2f(pos-sw/2, beltY-beltH/2+6); glVertex2f(pos+sw/2, beltY-beltH/2+6);
            glVertex2f(pos+sw/2, beltY+beltH/2-6); glVertex2f(pos-sw/2, beltY+beltH/2-6);
        glEnd();
    }

    // Rollers
    auto Roller = [](float cx, float cy, float r) {
        glColor3f(0.35f,0.35f,0.4f);
        glBegin(GL_TRIANGLE_FAN); glVertex2f(cx,cy);
        for (int i=0;i<=24;i++) { float a=2.f*3.14159f*i/24; glVertex2f(cx+r*cos(a),cy+r*sin(a)); }
        glEnd();
    };
    Roller(beltX+15, beltY, 22); Roller(beltX+beltW-15, beltY, 22);

    glColor3f(0.4f,0.4f,0.45f);
    glRasterPos2f(beltX+beltW/2-30, beltY-30);
    for (const char* c="CONVEYOR BELT"; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10,*c);
}

// =====================================================================
// BOX FUNCTIONS
// =====================================================================
void GenerateBox() {
    Box b; b.x=120.f; b.y=BELT_TOP; b.onBelt=true; b.lifted=false;
    b.size = 20.f+(rand()%3)*10.f;
    float cols[][3]={{1,.2f,.2f},{.2f,.8f,.2f},{.2f,.4f,1},{1,.8f,0},{.8f,.2f,.8f},{1,.5f,0}};
    int idx=rand()%6; b.color[0]=cols[idx][0]; b.color[1]=cols[idx][1]; b.color[2]=cols[idx][2];
    boxes.push_back(b);
}

void UpdateBoxes() {
    if (!isPaused)
        for (auto& b : boxes) if (b.onBelt) b.x += 1.8f;

    boxes.erase(std::remove_if(boxes.begin(),boxes.end(),
        [](const Box& b){ return b.x>720.f && !b.lifted; }), boxes.end());

    if (isHoldingBox && (heldBoxIndex<0 || heldBoxIndex>=(int)boxes.size())) {
        isHoldingBox=false; heldBoxIndex=-1; robotState=RETURNING;
    }

    static int fc=0;
    if (!isPaused && ++fc%80==0 && boxes.size()<8) GenerateBox();
}

void DrawBoxes() {
    for (int i=0;i<(int)boxes.size();i++) {
        if (isHoldingBox && i==heldBoxIndex) continue;
        const Box& b=boxes[i];
        glColor3f(b.color[0],b.color[1],b.color[2]);
        glBegin(GL_QUADS);
            glVertex2f(b.x-b.size/2,b.y-b.size/2); glVertex2f(b.x+b.size/2,b.y-b.size/2);
            glVertex2f(b.x+b.size/2,b.y+b.size/2); glVertex2f(b.x-b.size/2,b.y+b.size/2);
        glEnd();
        glColor3f(1,1,1); glLineWidth(1);
        glBegin(GL_LINES);
            glVertex2f(b.x-b.size/2+2,b.y+b.size/2-2); glVertex2f(b.x+b.size/2-2,b.y+b.size/2-2);
        glEnd();
    }
}

// =====================================================================
// DRAWING – FINGER
// =====================================================================
void DrawFinger(float bx, float by, float angle, bool isLeft, float len) {
    float ja=angle*0.6f, ta=angle*0.3f;
    glPushMatrix();
    glTranslatef(bx,by,0);

    glColor3f(0.4f,0.4f,0.5f);
    glBegin(GL_QUADS);
    if(isLeft){ glVertex2f(-8,0);glVertex2f(-4,0);glVertex2f(-4,-len*.3f);glVertex2f(-8,-len*.3f); }
    else       { glVertex2f(4,0); glVertex2f(8,0); glVertex2f(8,-len*.3f); glVertex2f(4,-len*.3f); }
    glEnd();

    glTranslatef(0,-len*.3f,0);
    glRotatef(isLeft?ja:-ja,0,0,1);

    glColor3f(0.5f,0.5f,0.6f);
    glBegin(GL_QUADS);
    if(isLeft){ glVertex2f(-6,0);glVertex2f(-3,0);glVertex2f(-3,-len*.4f);glVertex2f(-6,-len*.4f); }
    else       { glVertex2f(3,0); glVertex2f(6,0); glVertex2f(6,-len*.4f); glVertex2f(3,-len*.4f); }
    glEnd();

    glColor3f(0.3f,0.3f,0.4f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(0,0);
    for(int i=0;i<=20;i++){float a=2.f*3.14159f*i/20;glVertex2f(5*cos(a),5*sin(a));} glEnd();

    glTranslatef(0,-len*.4f,0);
    glRotatef(isLeft?ta:-ta,0,0,1);

    glColor3f(0.6f,0.6f,0.7f);
    glBegin(GL_QUADS);
    if(isLeft){ glVertex2f(-5,0);glVertex2f(-2,0);glVertex2f(-2,-len*.3f);glVertex2f(-5,-len*.3f); }
    else       { glVertex2f(2,0); glVertex2f(5,0); glVertex2f(5,-len*.3f); glVertex2f(2,-len*.3f); }
    glEnd();

    // Grip pad
    glColor3f(0.2f,0.2f,0.2f);
    glBegin(GL_QUADS);
    if(isLeft){ glVertex2f(-5,-len*.3f+2);glVertex2f(-2,-len*.3f+2);glVertex2f(-2,-len*.3f);glVertex2f(-5,-len*.3f); }
    else       { glVertex2f(2,-len*.3f+2); glVertex2f(5,-len*.3f+2); glVertex2f(5,-len*.3f); glVertex2f(2,-len*.3f); }
    glEnd();

    glPopMatrix();
}

// =====================================================================
// DRAWING – FULL CLAW ASSEMBLY
// =====================================================================
void DrawClaw(float cx) {
    float hy  = HOUSING_Y;           // housing center Y – FIXED
    float chy = ClawHeadY();         // finger-unit center Y – moves
    float ext = armExtension;        // how far the arm has extended (px)
    float spread = 30.0f;

    // Top mount plate
    glColor3f(0.3f,0.3f,0.4f);
    glBegin(GL_QUADS);
        glVertex2f(cx-20, hy+155); glVertex2f(cx+20, hy+155);
        glVertex2f(cx+15, hy+145); glVertex2f(cx-15, hy+145);
    glEnd();

    // Housing body (trapezoid)
    glColor3f(0.25f,0.25f,0.35f);
    glBegin(GL_QUADS);
        glVertex2f(cx-35, hy+25); glVertex2f(cx+35, hy+25);
        glVertex2f(cx+30, hy-10); glVertex2f(cx-30, hy-10);
    glEnd();

    // Housing dome
    glColor3f(0.3f,0.3f,0.4f);
    glBegin(GL_TRIANGLE_FAN);
    for(int i=0;i<=20;i++){
        float a=3.14159f*i/20;
        glVertex2f(cx+30*cos(a-3.14159f), hy+25+30*sin(a-3.14159f));
    }
    glEnd();

    // Motor/gearbox detail on housing
    glColor3f(0.2f,0.2f,0.28f);
    glBegin(GL_QUADS);
        glVertex2f(cx-14,hy+10); glVertex2f(cx+14,hy+10);
        glVertex2f(cx+14,hy+22); glVertex2f(cx-14,hy+22);
    glEnd();
    glColor3f(0.45f,0.45f,0.55f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(cx,hy+16);
    for(int i=0;i<=16;i++){float a=2.f*3.14159f*i/16;glVertex2f(cx+7*cos(a),hy+16+7*sin(a));}
    glEnd();

    // ── TELESCOPING RODS (extend from housing bottom to finger unit) 
    // Only draw if arm is extended at all
    if (ext > 0.5f) {
        float rodTop = hy - 10.0f;   // bottom of housing
        float rodBot = chy + 25.0f;  // top of finger-unit housing

        // Outer rod (wider, behind)
        glColor3f(0.28f,0.28f,0.38f);
        glBegin(GL_QUADS);
            glVertex2f(cx-12, rodTop); glVertex2f(cx+12, rodTop);
            glVertex2f(cx+12, rodBot); glVertex2f(cx-12, rodBot);
        glEnd();
        // Inner rod (narrower, in front) – lighter colour
        glColor3f(0.38f,0.38f,0.50f);
        glBegin(GL_QUADS);
            glVertex2f(cx-7, rodTop+4); glVertex2f(cx+7, rodTop+4);
            glVertex2f(cx+7, rodBot);   glVertex2f(cx-7, rodBot);
        glEnd();
        // Edge highlights on rods
        glColor3f(0.55f,0.55f,0.65f);
        glLineWidth(1.0f);
        glBegin(GL_LINES);
            glVertex2f(cx-12, rodTop); glVertex2f(cx-12, rodBot);
            glVertex2f(cx+12, rodTop); glVertex2f(cx+12, rodBot);
        glEnd();

        // ── SPRING COILS along the rod ─────────────────────────────
        // Number of coils scales with extension
        int   coils    = 4 + (int)(ext / 20.0f);
        float coilSpan = (rodBot - rodTop - 8.0f);
        float coilStep = (coils > 1) ? coilSpan / (coils - 1) : coilSpan;

        glColor3f(0.5f,0.5f,0.55f);
        glLineWidth(1.5f);
        for (int i = 0; i < coils; i++) {
            float cy2 = rodTop + 4.0f + i * coilStep;
            glBegin(GL_LINE_LOOP);
            for (int j=0;j<=18;j++){
                float a=2.f*3.14159f*j/18;
                glVertex2f(cx+10*cos(a), cy2+4*sin(a));
            }
            glEnd();
        }

        // Connection rods between housing and spring top
        glColor3f(0.35f,0.35f,0.45f);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
            glVertex2f(cx-10, rodTop); glVertex2f(cx-10, rodTop+10);
            glVertex2f(cx+10, rodTop); glVertex2f(cx+10, rodTop+10);
        glEnd();
    }

    // ── FINGER UNIT (the moving claw head) ─────────────────────────
    // Housing base of finger unit
    glColor3f(0.25f,0.25f,0.35f);
    glBegin(GL_QUADS);
        glVertex2f(cx-30, chy-10); glVertex2f(cx+30, chy-10);
        glVertex2f(cx+35, chy+25); glVertex2f(cx-35, chy+25);
    glEnd();

    // Dome on finger unit (only when retracted, merges with main housing)
    if (ext < 1.0f) {
        glColor3f(0.3f,0.3f,0.4f);
        glBegin(GL_TRIANGLE_FAN);
        for(int i=0;i<=20;i++){
            float a=3.14159f*i/20;
            glVertex2f(cx+30*cos(a-3.14159f), chy+25+30*sin(a-3.14159f));
        }
        glEnd();
    }

    // Pivot joints
    glColor3f(0.4f,0.4f,0.5f);
    for (float sx : {-spread, spread}) {
        glBegin(GL_TRIANGLE_FAN); glVertex2f(cx+sx, chy-10);
        for(int i=0;i<=20;i++){float a=2.f*3.14159f*i/20;glVertex2f(cx+sx+7*cos(a),chy-10+7*sin(a));}
        glEnd();
    }

    // Fingers
    DrawFinger(cx-spread, chy-10, clawAngle, true,  FINGER_LENGTH);
    DrawFinger(cx+spread, chy-10, clawAngle, false, FINGER_LENGTH);

    // ── HELD BOX ───────────────────────────────────────────────────
    if (isHoldingBox && heldBoxIndex>=0 && heldBoxIndex<(int)boxes.size()) {
        const Box& b = boxes[heldBoxIndex];
        float gripY  = chy - 10.0f - FINGER_LENGTH * 0.5f;
        glColor3f(b.color[0],b.color[1],b.color[2]);
        glBegin(GL_QUADS);
            glVertex2f(cx-b.size/2, gripY-b.size/2); glVertex2f(cx+b.size/2, gripY-b.size/2);
            glVertex2f(cx+b.size/2, gripY+b.size/2); glVertex2f(cx-b.size/2, gripY+b.size/2);
        glEnd();
        glColor3f(1,1,1); glLineWidth(1);
        glBegin(GL_LINES);
            glVertex2f(cx-b.size/2+2, gripY+b.size/2-2);
            glVertex2f(cx+b.size/2-2, gripY+b.size/2-2);
        glEnd();
    }
}

// =====================================================================
// MACHINES
// =====================================================================
void DrawMachines() {
    static float lp=0.f; lp+=0.03f; if(lp>6.28f) lp-=6.28f;
    auto Blink=[&](float phase){return 0.3f+0.7f*(0.5f+0.5f*sin(phase));};

    glColor3f(0.35f,0.35f,0.45f);
    glBegin(GL_QUADS); glVertex2f(40,30);glVertex2f(90,30);glVertex2f(90,90);glVertex2f(40,90); glEnd();
    glColor3f(Blink(lp),0,0);
    glBegin(GL_QUADS); glVertex2f(55,90);glVertex2f(75,90);glVertex2f(75,98);glVertex2f(55,98); glEnd();

    glColor3f(0.35f,0.35f,0.45f);
    glBegin(GL_QUADS); glVertex2f(710,30);glVertex2f(760,30);glVertex2f(760,90);glVertex2f(710,90); glEnd();
    glColor3f(Blink(lp+2.f),0,0);
    glBegin(GL_QUADS); glVertex2f(725,90);glVertex2f(745,90);glVertex2f(745,98);glVertex2f(725,98); glEnd();
}

// =====================================================================
// HUD
// =====================================================================
void DrawHUD() {
    glColor3f(1,1,1); glRasterPos2f(20,580);
    for(const char* c="ROBOT FACTORY  -  Conveyor + Arm Extension Pickup";*c;c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*c);

    char info[140];
    const char* sn[]={"IDLE","LOWERING","GRABBING","LIFTING","RETURNING"};
    sprintf_s(info, sizeof(info),
        "Boxes:%zu  Claw:%.0fdeg  Ext:%.0fpx  State:%s  %s",
        boxes.size(), clawAngle, armExtension, sn[robotState],
        isPaused?"PAUSED":"RUNNING");
    glColor3f(0.7f,0.7f,0.7f); glRasterPos2f(20,560);
    for(char* c=info;*c;c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*c);

    glColor3f(0.5f,0.5f,0.5f); glRasterPos2f(20,20);
    for(const char* c="SPACE: Pause   |   R: Reset   |   ESC: Exit";*c;c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10,*c);

    glColor3f(0.6f,0.6f,0.6f); glRasterPos2f(20,40);
    for(const char* c="Housing stays fixed - arm rod extends down to pick up boxes";*c;c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10,*c);
}

// =====================================================================
// DISPLAY
// =====================================================================
void Display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.12f,0.14f,0.18f);
    glBegin(GL_QUADS); glVertex2f(0,0);glVertex2f(800,0);glVertex2f(800,600);glVertex2f(0,600); glEnd();

    glColor4f(0.2f,0.2f,0.25f,0.3f); glLineWidth(1);
    for(int i=0;i<800;i+=40){glBegin(GL_LINES);glVertex2f(i,0);glVertex2f(i,600);glEnd();}
    for(int i=0;i<600;i+=40){glBegin(GL_LINES);glVertex2f(0,i);glVertex2f(800,i);glEnd();}

    DrawMachines();
    DrawConveyorBelt();
    DrawBoxes();
    DrawClaw(400);   // X is fixed; extension controls Y reach
    DrawHUD();

    glFlush();
}

// =====================================================================
// TIMER – FSM
// =====================================================================
void Timer(int value) {
    if (!isPaused) {
        beltOffset += 1.5f;
        if (beltOffset > 40.f) beltOffset -= 40.f;

        UpdateBoxes();

        switch (robotState) {

            case IDLE: {
                for (int i=0; i<(int)boxes.size(); i++) {
                    Box& b = boxes[i];
                    if (!b.onBelt || b.x<250.f || b.x>380.f) continue;

                    heldBoxIndex = i;
                    targetAngle  = 30.f;  // open
                    float needed = HOUSING_Y - b.y - FINGER_LENGTH - 8.f;
                    if (needed < 0.f) needed = 0.f;
                    // Clamp: can't extend past housing to belt distance minus some slack
                    float maxExt = HOUSING_Y - BELT_TOP - FINGER_LENGTH - 4.f;
                    if (needed > maxExt) needed = maxExt;
                    armExtensionTarget = needed;

                    robotState = LOWERING;
                    break;
                }
                break;
            }

            case LOWERING:
                if (armExtension < armExtensionTarget) {
                    armExtension += ARM_EXTEND_SPD;
                    if (armExtension > armExtensionTarget) armExtension = armExtensionTarget;
                } else {
                    armExtension = armExtensionTarget;
                    targetAngle  = 0.f;  // close claw
                    robotState   = GRABBING;
                }
                break;

            case GRABBING:
                if (clawAngle > targetAngle) {
                    clawAngle -= 0.8f;
                    if (clawAngle < targetAngle) clawAngle = targetAngle;
                } else {
                    if (heldBoxIndex>=0 && heldBoxIndex<(int)boxes.size()) {
                        boxes[heldBoxIndex].onBelt = false;
                        boxes[heldBoxIndex].lifted = true;
                        isHoldingBox = true;
                    }
                    armExtensionTarget = ARM_MIN_EXT;  // retract back up
                    robotState = LIFTING;
                }
                break;

            case LIFTING:
                if (armExtension > armExtensionTarget) {
                    armExtension -= ARM_EXTEND_SPD;
                    if (armExtension < armExtensionTarget) armExtension = armExtensionTarget;
                } else {
                    armExtension = armExtensionTarget;
                    // Release box at the top
                    if (isHoldingBox && heldBoxIndex>=0 && heldBoxIndex<(int)boxes.size()) {
                        Box& b   = boxes[heldBoxIndex];
                        b.x      = 400.f;
                        b.y      = ClawHeadY() - 10.f - FINGER_LENGTH * 0.5f;
                        b.onBelt = false;
                        b.lifted = false;
                        heldBoxIndex = -1;
                        isHoldingBox = false;
                    }
                    targetAngle = 30.f;  // open claw
                    robotState  = RETURNING;
                }
                break;

            case RETURNING:
                if (clawAngle < targetAngle) {
                    clawAngle += 0.8f;
                    if (clawAngle > targetAngle) clawAngle = targetAngle;
                } else {
                    clawAngle  = targetAngle;
                    robotState = IDLE;
                }
                break;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0);
}

// =====================================================================
// KEYBOARD
// =====================================================================
void Keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case ' ': isPaused = !isPaused; break;
        case 'r': case 'R':
            boxes.clear();
            clawAngle=30.f; targetAngle=30.f;
            armExtension=0.f; armExtensionTarget=0.f;
            beltOffset=0.f; robotState=IDLE;
            heldBoxIndex=-1; isHoldingBox=false;
            break;
        case 27: exit(0);
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
    glutCreateWindow("Robot Factory - Arm Extension Pickup");

    Initial();
    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard);
    glutTimerFunc(16, Timer, 0);
    glutMainLoop();
    return 0;
}
