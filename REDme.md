# 🏭 Robot Factory

A computer graphics project for CS341 – Industrial simulation with conveyor belt, robotic arm, and mechanical claw.

Built with **C++**, **OpenGL**, and **GLUT**.

---

## 📦 What's Inside

- `RobotFactoryProjectPlan.cpp` – Main factory with conveyor belt, boxes, and robotic arm
- `MechanicalClaw.cpp` – Standalone claw demo with spring mechanism
- `README.md` – You're reading it

---

## ⚠️ Important – Run One File at a Time

Both files have a `main()` function. **You can only run one at a time.**

### To run the main factory:
- Use `RobotFactoryProjectPlan.cpp`
- Remove or comment out `MechanicalClaw.cpp`

### To run the claw demo:
- Use `MechanicalClaw.cpp`
- Remove or comment out `RobotFactoryProjectPlan.cpp`

**Quick way:** Right-click the file you don't want → **Exclude From Project**.

---

## 🛠️ What You Need


**Visual Studio 2022** 
**FreeGLUT**
**Git**

### Install GLUT


---


## 🎮 Controls

### Main Factory
| Key | Action |
|:---|:---|
| **Space** | Pause / Resume |
| **R** | Reset everything |
| **ESC** | Exit |

### Claw Demo
| Key | Action |
|:---|:---|
| **O** | Open claw |
| **C** | Close claw |
| **Space** | Auto open/close |
| **R** | Reset |
| **ESC** | Exit |

---

## 🐛 Common Problems

| Error | Fix |
|:---|:---|
| `Cannot open include file: 'GL/glut.h'` | Add `C:\freeglut\include` to Include Directories |
| `LNK2005: _main already defined` | You have two .cpp files – remove one |
| `freeglut.dll not found` | Copy DLL to your Debug folder |
| `unresolved external symbol` | Add `glut32.lib` `opengl32.lib` `glu32.lib` to Additional Dependencies |
| `exit: identifier not found` | Add `#include <cstdlib>` at the top |

---

## 👨‍💻 Author

[YDEEM BASSAM] – CS341 Computer Graphics

---

## 📅 Date

July 2026
