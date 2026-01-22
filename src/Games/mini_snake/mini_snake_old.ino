#include <Arduino.h>
#include <U8g2lib.h>

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define PIN_TECLADO PA0
#define SIZE 4
#define MAX_SNAKE 30 


void st(uint8_t d);
void waitFlash();
void writeStr(uint32_t adr, const char* s);
void wait();
void writeInt(uint32_t adr, int32_t v);



// MUDANÇA CRÍTICA: int8_t economiza 180 bytes de Flash/RAM
int8_t sX[MAX_SNAKE], sY[MAX_SNAKE]; 
int tam, dX, dY, mX, mY, score;
bool vivo = true;
unsigned long lastT = 0;

void novaMaca() {
  mX = (random(1, 30)) * SIZE;
  mY = (random(2, 14)) * SIZE; 
}

void reset() {
  tam = 3; score = 0; dX = 1; dY = 0; vivo = true;
  for(int i = 0; i < tam; i++) { 
    sX[i] = 60 - (i * SIZE); 
    sY[i] = 32; 
  }
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
  }

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_04b_03_tr);
    u8g2.setDrawColor(1);
    if (vivo) {
      for (int x = 0; x < 128; x += 16) for (int y = 8; y < 64; y += 16) u8g2.drawPixel(x, y);
      u8g2.setCursor(2, 6); u8g2.print(F("SC:")); // Macro F() economiza Flash
      u8g2.setCursor(18, 6); u8g2.print(score);
      u8g2.drawHLine(0, 7, 128); 
      u8g2.drawFrame(mX, mY, SIZE, SIZE);
      for (int i = 0; i < tam; i++) u8g2.drawBox(sX[i], sY[i], SIZE, SIZE);
    } else {

      writeInt(900, score);
      
      u8g2.drawFrame(38, 15, 52, 12); 
      u8g2.setCursor(44, 25); u8g2.print(F("GAME OVER"));
      
      u8g2.setCursor(0, 40); u8g2.print(F("Recorde:"));
      int highScore = 1000;
      u8g2.setCursor(55, 40); u8g2.print(highScore);

      if (adc >= 700 && adc < 950 ) reset();
    }
  } while (u8g2.nextPage());
}