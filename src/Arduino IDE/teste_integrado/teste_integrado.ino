#include <Arduino.h>
#include <U8g2lib.h>

// Driver SH1106 em modo econômico (Page Buffer)
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define PIN_TECLADO PA0
#define LARGURA 128
#define ALTURA 64
#define SIZE 4
#define MAX_SNAKE 40 // Reduzi um pouco para economizar RAM também

int sX[MAX_SNAKE], sY[MAX_SNAKE];
int tam = 3;
int dX = 1, dY = 0;
int mX, mY;
bool vivo = true;
unsigned long tMove = 100;
unsigned long lastT = 0;

void novaMaca() {
  mX = (random(1, 30)) * SIZE;
  mY = (random(1, 14)) * SIZE;
}

void reset() {
  tam = 3;
  for(int i = 0; i < tam; i++) { sX[i] = 60 - (i*SIZE); sY[i] = 32; }
  dX = 1; dY = 0;
  novaMaca();
  vivo = true;
}

void setup() {
  u8g2.begin();
  pinMode(PIN_TECLADO, INPUT_ANALOG);
  reset();
}

void loop() {
  int adc = analogRead(PIN_TECLADO);
  
  // Controles simplificados para economizar espaço
  if (adc < 80 && dX == 0) { dX = -1; dY = 0; }
  else if (adc >= 100 && adc < 200 && dY == 0) { dX = 0; dY = -1; }
  else if (adc >= 250 && adc < 380 && dY == 0) { dX = 0; dY = 1; }
  else if (adc >= 430 && adc < 600 && dX == 0) { dX = 1; dY = 0; }

  if (millis() - lastT >= tMove) {
    if (vivo) {
      for (int i = tam - 1; i > 0; i--) { sX[i] = sX[i-1]; sY[i] = sY[i-1]; }
      sX[0] += dX * SIZE; sY[0] += dY * SIZE;

      if (sX[0] < 0 || sX[0] >= LARGURA || sY[0] < 0 || sY[0] >= ALTURA) vivo = false;
      if (sX[0] == mX && sY[0] == mY) { if (tam < MAX_SNAKE) tam++; novaMaca(); }
    }
    lastT = millis();
  }

  u8g2.firstPage();
  do {
    if (vivo) {
      u8g2.drawFrame(mX, mY, SIZE, SIZE);
      for (int i = 0; i < tam; i++) u8g2.drawBox(sX[i], sY[i], SIZE, SIZE);
      u8g2.drawFrame(0, 0, 128, 64);
    } else {
      // Fonte minúscula para economizar memória FLASH
      u8g2.setFont(u8g2_font_04b_03_tr); 
      u8g2.drawStr(40, 32, "GAME OVER");
      if (adc < 3000) reset();
    }
  } while (u8g2.nextPage());
}