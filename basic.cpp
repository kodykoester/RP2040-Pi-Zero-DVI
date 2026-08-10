/*
 * ============================================================
 * Waveshare RP2040-PiZero DVI — Component Demo
 * ============================================================
 *
 * Purpose: one sketch, five reusable building blocks.
 * Each section works standalone — copy the section + its
 * helper struct/array into your own project.
 *
 *   1. MENU        - selectable list, cursor, wraparound
 *   2. SUBMENU      - nested menu, "back" navigation
 *   3. ANIMATION    - moving object, velocity, bounce
 *   4. PLACEMENT    - multiple objects, draw order / layering
 *   5. DEMOSCENE    - plasma effect + sine-wave scrolling text
 *
 * Navigation in this demo uses two GPIO buttons:
 *   BTN_NEXT (pin 2)  -> move cursor / advance
 *   BTN_SELECT (pin 3) -> choose / enter / back
 * Wire buttons to GND with INPUT_PULLUP, adjust pins to taste.
 * (Swap this for a rotary encoder or your ESP32-fed UART
 *  commands later — the state machine below doesn't care
 *  where "next" and "select" events come from.)
 *
 * ============================================================
 */

#include <PicoDVI.h>

DVIGFX16 display(DVI_RES_320x240p60, rp2040pizero_cfg);

// ------------------------------------------------------------
// INPUT (swap this block out for encoder / serial / etc.)
// ------------------------------------------------------------
#define BTN_NEXT   2
#define BTN_SELECT 3

bool readButtonEdge(int pin) {
  static uint32_t lastPress[40] = {0};
  if (digitalRead(pin) == LOW && millis() - lastPress[pin] > 180) {
    lastPress[pin] = millis();
    return true;
  }
  return false;
}

// ------------------------------------------------------------
// APP STATE MACHINE
// ------------------------------------------------------------
enum AppState {
  STATE_MAIN_MENU,
  STATE_SUBMENU,
  STATE_ANIMATION_DEMO,
  STATE_PLACEMENT_DEMO,
  STATE_DEMOSCENE
};
AppState state = STATE_MAIN_MENU;

// ============================================================
// 1. MENU COMPONENT
// ============================================================
// A menu is just: an array of labels + an index + a draw loop.
// Everything else (submenus, settings screens) reuses this.

const char* mainMenuItems[] = {
  "Animation Demo",
  "Placement Demo",
  "Demoscene",
  "Settings >"
};
const int mainMenuCount = 4;
int mainMenuCursor = 0;

void drawMenu(const char* items[], int count, int cursor, const char* title) {
  display.fillScreen(0x0000);
  display.setTextSize(2);
  display.setTextColor(0xFFFF);
  display.setCursor(10, 10);
  display.print(title);
  display.drawFastHLine(10, 32, 300, 0x7BEF);

  display.setTextSize(1);
  for (int i = 0; i < count; i++) {
    int y = 50 + i * 20;
    if (i == cursor) {
      // highlight bar behind the selected row
      display.fillRect(10, y - 2, 200, 14, 0x3186);
      display.setTextColor(0xFFE0); // yellow = selected
    } else {
      display.setTextColor(0xFFFF);
    }
    display.setCursor(16, y);
    display.print(items[i]);
  }
}

bool mainMenuDirty = true; // set true whenever something on screen needs to change

void handleMainMenu() {
  if (mainMenuDirty) {
    drawMenu(mainMenuItems, mainMenuCount, mainMenuCursor, "MAIN MENU");
    mainMenuDirty = false;
  }

  if (readButtonEdge(BTN_NEXT)) {
    mainMenuCursor = (mainMenuCursor + 1) % mainMenuCount; // wraparound
    mainMenuDirty = true; // cursor moved -> redraw needed
  }
  if (readButtonEdge(BTN_SELECT)) {
    switch (mainMenuCursor) {
      case 0: state = STATE_ANIMATION_DEMO; break;
      case 1: state = STATE_PLACEMENT_DEMO; break;
      case 2: state = STATE_DEMOSCENE; break;
      case 3: state = STATE_SUBMENU; break;
    }
    mainMenuDirty = true; // returning here later should redraw fresh
  }
}

// ============================================================
// 2. SUBMENU COMPONENT
// ============================================================
// Identical mechanics to the main menu — the "nesting" is just
// which state you return to on select/back. This is the whole
// trick to arbitrarily deep menu trees: same draw/select code,
// different parent state stored per-menu.

const char* settingsItems[] = {
  "Brightness: --",
  "Demo Speed: --",
  "< Back"
};
const int settingsCount = 3;
int settingsCursor = 0;

bool submenuDirty = true;

void handleSubmenu() {
  if (submenuDirty) {
    drawMenu(settingsItems, settingsCount, settingsCursor, "SETTINGS");
    submenuDirty = false;
  }

  if (readButtonEdge(BTN_NEXT)) {
    settingsCursor = (settingsCursor + 1) % settingsCount;
    submenuDirty = true;
  }
  if (readButtonEdge(BTN_SELECT)) {
    if (settingsCursor == settingsCount - 1) {
      state = STATE_MAIN_MENU;       // "Back" returns to parent
      settingsCursor = 0;
      mainMenuDirty = true;          // wake the main menu's redraw too
    }
    // real settings would adjust a value here instead
    submenuDirty = true;
  }
}

// ============================================================
// 3. ANIMATION COMPONENT
// ============================================================
// Core idea: position + velocity, updated each frame, redrawn
// each frame. "Erase old, draw new" is the key technique on a
// framebuffer display with no hardware sprites.

struct MovingBall {
  float x, y;
  float vx, vy;
  int radius;
  uint16_t color;
};

MovingBall ball = { 160, 120, 2.2, 1.6, 10, 0x07FF };
bool animFirstFrame = true; // draw the static UI chrome only once

void handleAnimationDemo() {
  if (animFirstFrame) {
    // Everything that DOESN'T move gets drawn exactly once, not
    // every frame. This is the other half of avoiding flicker:
    // static content should never be part of the redraw loop.
    display.fillScreen(0x0000);
    display.setTextSize(1);
    display.setTextColor(0x8410);
    display.setCursor(4, 4);
    display.print("hold SELECT to return to menu");
    animFirstFrame = false;
  }

  // 1. ERASE only the ball's OLD position (small black circle),
  //    not the whole screen. Everything else on screen is left alone.
  display.fillCircle((int)ball.x, (int)ball.y, ball.radius, 0x0000);

  // 2. update physics
  ball.x += ball.vx;
  ball.y += ball.vy;

  // bounce off screen edges
  if (ball.x - ball.radius < 0 || ball.x + ball.radius > 320) ball.vx = -ball.vx;
  if (ball.y - ball.radius < 20 || ball.y + ball.radius > 240) ball.vy = -ball.vy;

  // 3. draw the ball at its NEW position
  display.fillCircle((int)ball.x, (int)ball.y, ball.radius, ball.color);

  if (readButtonEdge(BTN_SELECT)) { state = STATE_MAIN_MENU; animFirstFrame = true; }
  delay(16); // ~60fps pacing
}

// ============================================================
// 4. PLACEMENT / LAYERING COMPONENT
// ============================================================
// Multiple independent objects on screen at once. "Layering" on
// a simple framebuffer is just draw order — later draws sit on
// top of earlier ones. No true z-buffer needed at this scale.

struct Sprite {
  int x, y, w, h;
  uint16_t color;
  const char* label;
};

Sprite sprites[] = {
  { 20,  40, 60, 40, 0xF800, "BACK"  },  // drawn first  -> bottom layer
  { 50,  70, 60, 40, 0x07E0, "MID"   },  // drawn second -> middle
  { 80, 100, 60, 40, 0x001F, "FRONT" }   // drawn last   -> top layer
};
const int spriteCount = 3;

bool placementDrawn = false; // nothing here moves, so draw it exactly once

void handlePlacementDemo() {
  if (!placementDrawn) {
    display.fillScreen(0x0000);
    display.setTextSize(1);
    display.setTextColor(0xFFFF);
    display.setCursor(4, 4);
    display.print("draw order = layer order (back->front)");

    for (int i = 0; i < spriteCount; i++) {
      Sprite &s = sprites[i];
      display.fillRect(s.x, s.y, s.w, s.h, s.color);
      display.drawRect(s.x, s.y, s.w, s.h, 0xFFFF); // outline for clarity
      display.setCursor(s.x + 6, s.y + s.h / 2 - 4);
      display.setTextColor(0x0000);
      display.print(s.label);
    }
    placementDrawn = true;
  }

  if (readButtonEdge(BTN_SELECT)) { state = STATE_MAIN_MENU; placementDrawn = false; }
}

// ============================================================
// 5. DEMOSCENE COMPONENT — Plasma + Sine-Scroll Text
// ============================================================
// The two most classic 256-byte-demo effects, combined:
//   a) Plasma: sum of sine waves sampled per-pixel, mapped
//      through a colour ramp. Pure math, no assets.
//   b) Sine scroller: scrolling text where each character's
//      Y offset follows a sine wave over time.
// Both are cheap enough for RP2040 at reduced resolution.

uint16_t plasmaPalette[256];

void buildPalette() {
  // precompute a rainbow ramp once at startup (cheap trick:
  // never recompute colour math per-frame, only per-pixel index)
  for (int i = 0; i < 256; i++) {
    uint8_t r = (uint8_t)(sinf(i * 0.024f) * 127 + 128);
    uint8_t g = (uint8_t)(sinf(i * 0.024f + 2.0f) * 127 + 128);
    uint8_t b = (uint8_t)(sinf(i * 0.024f + 4.0f) * 127 + 128);
    plasmaPalette[i] = display.color565(r, g, b);
  }
}

float plasmaTime = 0;

void drawPlasma() {
  // Downsampled to an 8px block grid — a full per-pixel plasma
  // at 320x240 is too slow on RP2040 without PIO tricks; blocky
  // plasma still reads as "plasma" and runs comfortably.
  const int step = 8;
  for (int y = 0; y < 200; y += step) {
    for (int x = 0; x < 320; x += step) {
      float v = sinf(x * 0.05f + plasmaTime)
              + sinf(y * 0.05f + plasmaTime * 1.3f)
              + sinf((x + y) * 0.03f + plasmaTime * 0.7f)
              + sinf(sqrtf((x - 160) * (x - 160) + (y - 100) * (y - 100)) * 0.05f - plasmaTime);
      int idx = (int)((v + 4.0f) * 32.0f) & 0xFF;
      display.fillRect(x, y, step, step, plasmaPalette[idx]);
    }
  }
  plasmaTime += 0.08f;
}

const char* scrollMsg = "  RP2040 * PICODVI * 320x240 DEMOSCENE * BUILD YOUR OWN EFFECTS HERE  ";
float scrollX = 320;
float scrollWave = 0;

void drawSineScroller() {
  display.setTextSize(2);
  int charW = 12; // approx width per char at size 2
  int len = strlen(scrollMsg);
  for (int i = 0; i < len; i++) {
    int cx = (int)scrollX + i * charW;
    if (cx < -charW || cx > 320) continue; // skip offscreen chars
    int cy = 210 + (int)(sinf(scrollWave + i * 0.3f) * 15.0f);
    display.setTextColor(0xFFFF);
    display.setCursor(cx, cy);
    display.print(scrollMsg[i]);
  }
  scrollX -= 2.5f;
  if (scrollX < -len * charW) scrollX = 320;
  scrollWave += 0.15f;
}

void handleDemoscene() {
  drawPlasma();
  drawSineScroller();

  display.setTextSize(1);
  display.setTextColor(0x0000);
  display.setCursor(4, 4);
  display.print("SELECT: back");

  if (readButtonEdge(BTN_SELECT)) state = STATE_MAIN_MENU;
}

// ============================================================
// SETUP / LOOP
// ============================================================
void setup() {
  display.begin();
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  buildPalette();
}

void loop() {
  switch (state) {
    case STATE_MAIN_MENU:      handleMainMenu();      break;
    case STATE_SUBMENU:        handleSubmenu();       break;
    case STATE_ANIMATION_DEMO: handleAnimationDemo(); break;
    case STATE_PLACEMENT_DEMO: handlePlacementDemo(); break;
    case STATE_DEMOSCENE:      handleDemoscene();     break;
  }
}
