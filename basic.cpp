
/*
 * ============================================================
 * Waveshare RP2040-PiZero DVI — Automatic Component Demo
 * ============================================================
 */

#include <PicoDVI.h>
#include <math.h>

DVIGFX16 display(DVI_RES_320x240p60, rp2040pizero_cfg);


// ============================================================
// DEMO 
// ============================================================

enum DemoState {
  STATE_ANIMATION,
  STATE_PLACEMENT,
  STATE_DEMOSCENE
};

DemoState state = STATE_ANIMATION;

const unsigned long DEMO_DURATION = 10000; // 10 seconds per demo
unsigned long demoStartTime = 0;


// ============================================================
// 1. ANIMATION DEMO
// ============================================================

struct MovingBall {
  float x, y;
  float vx, vy;
  int radius;
  uint16_t color;
};

MovingBall ball = {
  160,       // x
  120,       // y
  2.2,       // x velocity
  1.6,       // y velocity
  10,        // radius
  0x07FF     // cyan
};

bool animFirstFrame = true;


void resetAnimation() {
  ball.x = 160;
  ball.y = 120;
  ball.vx = 2.2;
  ball.vy = 1.6;

  animFirstFrame = true;
}


void handleAnimationDemo() {

  // Draw static screen elements once
  if (animFirstFrame) {

    display.fillScreen(0x0000);

    display.setTextSize(2);
    display.setTextColor(0xFFFF);
    display.setCursor(55, 20);
    display.print("ANIMATION");

    display.setTextSize(1);
    display.setTextColor(0x8410);
    display.setCursor(80, 45);
    display.print("Moving Ball Demo");

    animFirstFrame = false;
  }

  // Erase old ball
  display.fillCircle(
    (int)ball.x,
    (int)ball.y,
    ball.radius,
    0x0000
  );

  // Update physics
  ball.x += ball.vx;
  ball.y += ball.vy;

  // Bounce horizontally
  if (
    ball.x - ball.radius < 0 ||
    ball.x + ball.radius > 320
  ) {
    ball.vx = -ball.vx;
  }

  // Bounce vertically
  if (
    ball.y - ball.radius < 60 ||
    ball.y + ball.radius > 240
  ) {
    ball.vy = -ball.vy;
  }

  // Draw new position
  display.fillCircle(
    (int)ball.x,
    (int)ball.y,
    ball.radius,
    ball.color
  );

  delay(16);
}


// ============================================================
// 2. PLACEMENT / LAYERING DEMO
// ============================================================

struct Sprite {
  int x, y, w, h;
  uint16_t color;
  const char* label;
};

Sprite sprites[] = {

  { 20,  80, 100, 60, 0xF800, "BACK"  },
  { 70,  110, 100, 60, 0x07E0, "MID"   },
  { 120, 140, 100, 60, 0x001F, "FRONT" }

};

const int spriteCount = 3;

bool placementDrawn = false;


void resetPlacement() {
  placementDrawn = false;
}


void handlePlacementDemo() {

  if (!placementDrawn) {

    display.fillScreen(0x0000);

    display.setTextSize(2);
    display.setTextColor(0xFFFF);
    display.setCursor(40, 20);
    display.print("PLACEMENT");

    display.setTextSize(1);
    display.setTextColor(0x8410);
    display.setCursor(65, 45);
    display.print("Draw Order / Layering");

    // Draw objects back -> front
    for (int i = 0; i < spriteCount; i++) {

      Sprite &s = sprites[i];

      display.fillRect(
        s.x,
        s.y,
        s.w,
        s.h,
        s.color
      );

      display.drawRect(
        s.x,
        s.y,
        s.w,
        s.h,
        0xFFFF
      );

      display.setTextColor(0xFFFF);
      display.setCursor(
        s.x + 25,
        s.y + s.h / 2 - 4
      );

      display.print(s.label);
    }

    placementDrawn = true;
  }
}


// ============================================================
// 3. DEMOSCENE
// ============================================================

uint16_t plasmaPalette[256];


void buildPalette() {

  for (int i = 0; i < 256; i++) {

    uint8_t r =
      (uint8_t)(sinf(i * 0.024f) * 127 + 128);

    uint8_t g =
      (uint8_t)(sinf(i * 0.024f + 2.0f) * 127 + 128);

    uint8_t b =
      (uint8_t)(sinf(i * 0.024f + 4.0f) * 127 + 128);

    plasmaPalette[i] =
      display.color565(r, g, b);
  }
}


float plasmaTime = 0;


void drawPlasma() {

  const int step = 8;

  for (int y = 0; y < 200; y += step) {

    for (int x = 0; x < 320; x += step) {

      float v =
        sinf(x * 0.05f + plasmaTime)
        +
        sinf(y * 0.05f + plasmaTime * 1.3f)
        +
        sinf((x + y) * 0.03f + plasmaTime * 0.7f)
        +
        sinf(
          sqrtf(
            (x - 160) * (x - 160) +
            (y - 100) * (y - 100)
          )
          * 0.05f
          - plasmaTime
        );

      int idx =
        (int)((v + 4.0f) * 32.0f) & 0xFF;

      display.fillRect(
        x,
        y,
        step,
        step,
        plasmaPalette[idx]
      );
    }
  }

  plasmaTime += 0.08f;
}


const char* scrollMsg =
  "  RP2040 * PICODVI * 320x240 DEMOSCENE * "
  "BUILD YOUR OWN EFFECTS HERE  ";

float scrollX = 320;
float scrollWave = 0;


void drawSineScroller() {

  display.setTextSize(2);

  int charW = 12;
  int len = strlen(scrollMsg);

  for (int i = 0; i < len; i++) {

    int cx =
      (int)scrollX + i * charW;

    if (cx < -charW || cx > 320)
      continue;

    int cy =
      210 +
      (int)(sinf(scrollWave + i * 0.3f) * 15.0f);

    display.setTextColor(0xFFFF);
    display.setCursor(cx, cy);
    display.print(scrollMsg[i]);
  }

  scrollX -= 2.5f;

  if (scrollX < -len * charW)
    scrollX = 320;

  scrollWave += 0.15f;
}


void handleDemoscene() {

  drawPlasma();
  drawSineScroller();

  delay(1);
}


// ============================================================
// START / NEXT DEMO
// ============================================================

void startDemo(DemoState newState) {

  state = newState;
  demoStartTime = millis();

  switch (state) {

    case STATE_ANIMATION:
      resetAnimation();
      break;

    case STATE_PLACEMENT:
      resetPlacement();
      break;

    case STATE_DEMOSCENE:

      display.fillScreen(0x0000);

      plasmaTime = 0;
      scrollX = 320;
      scrollWave = 0;

      break;
  }
}


void nextDemo() {

  switch (state) {

    case STATE_ANIMATION:
      startDemo(STATE_PLACEMENT);
      break;

    case STATE_PLACEMENT:
      startDemo(STATE_DEMOSCENE);
      break;

    case STATE_DEMOSCENE:
      startDemo(STATE_ANIMATION);
      break;
  }
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  display.begin();

  buildPalette();

  startDemo(STATE_ANIMATION);
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  // Run the current demo
  switch (state) {

    case STATE_ANIMATION:
      handleAnimationDemo();
      break;

    case STATE_PLACEMENT:
      handlePlacementDemo();
      break;

    case STATE_DEMOSCENE:
      handleDemoscene();
      break;
  }


  // Automatically advance
  if (millis() - demoStartTime >= DEMO_DURATION) {

    nextDemo();
  }
}

