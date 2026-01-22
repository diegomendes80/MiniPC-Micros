#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>

// Definições de Hardware
#define CS_PIN PA12
#define PIN_TECLADO PA0
#define SIZE 4
#define MAX_SNAKE 25
#define LED_PIN PC13

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Variáveis de Jogo
int8_t sX[MAX_SNAKE], sY[MAX_SNAKE], dX, dY, mX, mY;
uint8_t tam;
int score, high = 0;
bool vivo = true;
unsigned long lastT = 0;
bool recordeSalvo = false;

// Buffers para assets da Flash Externa
char tMorte[15], tScore[15], tHigh[15];

/* ===== FUNÇÕES DE COMUNICAÇÃO FLASH (SPI IDÊNTICO AO SEU TESTE) ===== */

// Configuração SPI segura: 500kHz, Modo 0
SPISettings flashSettings(500000, MSBFIRST, SPI_MODE0);

void rdStr(uint32_t a, char* b, uint8_t l) {
  digitalWrite(CS_PIN, LOW);
  SPI.beginTransaction(flashSettings);
  
  SPI.transfer(0x03); // Comando Read Data
  SPI.transfer((a >> 16) & 0xFF);
  SPI.transfer((a >> 8) & 0xFF);
  SPI.transfer(a & 0xFF);
  
  for(uint8_t i = 0; i < l - 1; i++) {
    uint8_t c = SPI.transfer(0x00);
    if(c == 0 || c == 0xFF) break;
    b[i] = (char)c;
  }
  b[l-1] = 0;
  
  SPI.endTransaction();
  digitalWrite(CS_PIN, HIGH);
}

int rdInt(uint32_t a) {
  int valor = 0;
  digitalWrite(CS_PIN, LOW);
  SPI.beginTransaction(flashSettings);
  
  SPI.transfer(0x03);
  SPI.transfer((a >> 16) & 0xFF);
  SPI.transfer((a >> 8) & 0xFF);
  SPI.transfer(a & 0xFF);
  
  for(int i = 0; i < 4; i++) {
    valor |= ((int)SPI.transfer(0x00) << (8 * i));
  }
  
  SPI.endTransaction();
  digitalWrite(CS_PIN, HIGH);
  return (valor == -1 || valor == 65535) ? 0 : valor;
}

void writeInt(uint32_t adr, int valor) {
  // Write Enable
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x06);
  digitalWrite(CS_PIN, HIGH);

  // Program Page
  digitalWrite(CS_PIN, LOW);
  SPI.beginTransaction(flashSettings);
  SPI.transfer(0x02);
  SPI.transfer((adr >> 16) & 0xFF);
  SPI.transfer((adr >> 8) & 0xFF);
  SPI.transfer(adr & 0xFF);
  
  for(int i = 0; i < 4; i++) {
    SPI.transfer((valor >> (8 * i)) & 0xFF);
  }
  digitalWrite(CS_PIN, HIGH);
  
  // Wait Busy (Status Register)
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x05);
  while (SPI.transfer(0) & 0x01);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
}

/* ===== LÓGICA DO JOGO ===== */

void novaMaca() {
  mX = ((millis() % 30) + 1) * SIZE;
  mY = ((millis() % 12) + 2) * SIZE;
}

void eraseSector(uint32_t adr) {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x06); // Write Enable
  digitalWrite(CS_PIN, HIGH);

  digitalWrite(CS_PIN, LOW);
  SPI.beginTransaction(flashSettings);
  SPI.transfer(0x20); // Sector Erase (4KB)
  SPI.transfer((adr >> 16) & 0xFF);
  SPI.transfer((adr >> 8) & 0xFF);
  SPI.transfer(adr & 0xFF);
  digitalWrite(CS_PIN, HIGH);
  
  // Aguarda apagar (pode levar até 400ms)
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x05);
  while (SPI.transfer(0) & 0x01);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
}

void reset() {
  tam = 3; score = 0; dX = 1; dY = 0; vivo = true; recordeSalvo = false;
  for(uint8_t i = 0; i < tam; i++) { sX[i] = 60 - (i * SIZE); sY[i] = 32; }
  novaMaca();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED OFF (PC13 é invertido)
  
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  
  u8g2.begin();
  pinMode(PIN_TECLADO, INPUT_ANALOG);
  
  SPI.begin();
  delay(300); // Aguarda o módulo W25Q32 acordar

  // 1. Tenta carregar o nome do jogo para testar a Flash
  char testeID[11];
  rdStr(100, testeID, 11);

  // 2. Se a Flash respondeu "SNAKE", acende o LED de SUCESSO
  if (testeID[0] == 'S') {
    digitalWrite(LED_PIN, LOW); 
  }

  // 3. Carrega os outros textos e o recorde
  rdStr(200, tMorte, 12); 
  rdStr(300, tScore, 10); 
  rdStr(400, tHigh, 10);  
  high = rdInt(4096);

  // // Fallback (Somente se a leitura falhar totalmente)
  // if (tScore[0] == 0) strcpy(tScore, "SC2ORE:");
  // if (tMorte[0] == 0) strcpy(tMorte, "GAME OVER");
  // if (tHigh[0] == 0)  strcpy(tHigh, "RECORDE:");

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
      for (uint8_t i = tam - 1; i > 0; i--) { sX[i] = sX[i-1]; sY[i] = sY[i-1]; }
      sX[0] += dX * SIZE; sY[0] += dY * SIZE;
      
      if (sX[0] < 0 || sX[0] >= 128 || sY[0] < 8 || sY[0] >= 64) vivo = false;
      for(uint8_t i = 1; i < tam; i++) if(sX[0] == sX[i] && sY[0] == sY[i]) vivo = false;
      
      if (sX[0] == mX && sY[0] == mY) { 
        if (tam < MAX_SNAKE) tam++; 
        score += 10; 
        if (score > high) high = score;
        novaMaca(); 
      }
      lastT = millis();
    }
  } else {
    if(!recordeSalvo){
      int savedHigh = rdInt(4096); // Novo endereço para não apagar os textos
      if(score > savedHigh){
        eraseSector(4096);    // LIMPA o setor antes de gravar
        writeInt(4096, score); // GRAVA o novo valor
        high = score;
      }
      recordeSalvo = true;
    }

    if (adc >= 700 && adc < 950) reset();
  }

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_04b_03_tr);
    if (vivo) {
      u8g2.drawStr(2, 6, tScore); 
      u8g2.setCursor(45, 6); u8g2.print(score);
      u8g2.drawHLine(0, 7, 128); 
      u8g2.drawFrame(mX, mY, SIZE, SIZE);
      for (uint8_t i = 0; i < tam; i++) u8g2.drawBox(sX[i], sY[i], SIZE, SIZE);
    } else {
      u8g2.drawFrame(38, 15, 52, 12); 
      u8g2.drawStr(44, 25, tMorte);
      u8g2.drawStr(20, 40, tHigh); 
      u8g2.setCursor(70, 40); u8g2.print(high);
      u8g2.drawBox(0, 52, 55, 10);
      u8g2.setDrawColor(0); u8g2.drawStr(2, 60, "REINICIAR");
      u8g2.setDrawColor(1);
    }
  } while (u8g2.nextPage());
}