#include <Arduino.h>
#include <U8g2lib.h>

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define PIN_TECLADO PA0
#define SIZE 4
#define MAX_SNAKE 30 // Reduzi mais 5 segmentos para garantir a compilação

int sX[MAX_SNAKE], sY[MAX_SNAKE], tam, dX, dY, mX, mY, score, posMenu = 0;
bool vivo = true;
unsigned long lastT = 0;

void novaMaca() {
  mX = (random(1, 30)) * SIZE;
  mY = (random(2, 14)) * SIZE; 
}

void reset() {
  tam = 3; score = 0; dX = 1; dY = 0; vivo = true;
  for(int i = 0; i < tam; i++) { sX[i] = 60 - (i * SIZE); sY[i] = 32; }
  novaMaca();
}

void setup() {
  u8g2.begin();
  pinMode(PIN_TECLADO, INPUT_ANALOG);
  reset();
}

void loop() {
  int adc = analogRead(PIN_TECLADO);
  
  if (vivo) {
    if (adc < 80 && dX == 0) { dX = -1; dY = 0; }
    else if (adc >= 100 && adc < 200 && dY == 0) { dX = 0; dY = -1; }
    else if (adc >= 250 && adc < 380 && dY == 0) { dX = 0; dY = 1; }
    else if (adc >= 430 && adc < 600 && dX == 0) { dX = 1; dY = 0; }
    
    if (millis() - lastT >= 100) {
      for (int i = tam - 1; i > 0; i--) { sX[i] = sX[i-1]; sY[i] = sY[i-1]; }
      sX[0] += dX * SIZE; sY[0] += dY * SIZE;
      if (sX[0] < 0 || sX[0] >= 128 || sY[0] < 8 || sY[0] >= 64) vivo = false;
      for(int i = 1; i < tam; i++) if(sX[0] == sX[i] && sY[0] == sY[i]) vivo = false;
      if (sX[0] == mX && sY[0] == mY) { if (tam < MAX_SNAKE) tam++; score += 10; novaMaca(); }
      lastT = millis();
    }
  } else {
    if (adc < 80) posMenu = 0;
    else if (adc >= 430 && adc < 600) posMenu = 1;
    if (adc >= 700 && adc < 950 && posMenu == 0) reset();
  }

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_04b_03_tr);
    u8g2.setDrawColor(1);
    if (vivo) {
      for (int x = 0; x < 128; x += 16) for (int y = 8; y < 64; y += 16) u8g2.drawPixel(x, y);
      char buf[5]; itoa(score, buf, 10);
      u8g2.drawStr(2, 6, "SC:"); u8g2.drawStr(18, 6, buf);
      u8g2.drawHLine(0, 7, 128); 
      u8g2.drawFrame(mX, mY, SIZE, SIZE);
      for (int i = 0; i < tam; i++) u8g2.drawBox(sX[i], sY[i], SIZE, SIZE);
    } else {
      u8g2.drawFrame(38, 15, 52, 12); u8g2.drawStr(44, 24, "GAME OVER");
      
      // Desenho simplificado dos botões
      if (posMenu == 0) u8g2.drawBox(0, 48, 45, 10);
      else u8g2.drawBox(104, 48, 22, 10);
      
      u8g2.setDrawColor(0);
      u8g2.drawStr(2, 55, "REINICIAR");
      u8g2.drawStr(106, 55, "MENU");
      u8g2.setDrawColor(1);
      
      // Escreve o texto não selecionado por cima em branco
      if (posMenu == 1) u8g2.drawStr(2, 55, "REINICIAR");
      else u8g2.drawStr(106, 55, "MENU");
    }
  } while (u8g2.nextPage());
}