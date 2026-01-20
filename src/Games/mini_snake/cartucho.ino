#include <Arduino.h>
#include <SPI.h>


//ESSE CODIGO FUNCIONA COMO UM CARTUCHO DE VIDEO GAME: ELE GRAVA NA FLASH EXTERNA COISAS DO JOGO COMO STRINGS E INTERFACE
//PARA LIBERAR A MEMORIA INTERNA DA BLUEPILL. ELE DEVE SER RODADO UMA UNICA VEZ!

#define CS_PIN PA12

void st(uint8_t d) { SPI.transfer(d); }

void wait() {
  digitalWrite(CS_PIN, LOW);
  st(0x05); while (SPI.transfer(0) & 0x01);
  digitalWrite(CS_PIN, HIGH);
}

void writeStr(uint32_t adr, const char* s) {
  digitalWrite(CS_PIN, LOW); st(0x06); digitalWrite(CS_PIN, HIGH);
  digitalWrite(CS_PIN, LOW);
  st(0x02); st(adr >> 16); st(adr >> 8); st(adr);
  while (*s) st(*s++);
  st(0); digitalWrite(CS_PIN, HIGH);
  wait();
}



void setup() {
  pinMode(CS_PIN, OUTPUT); digitalWrite(CS_PIN, HIGH);
  SPI.begin();
  
  // Apagar setor inicial
  digitalWrite(CS_PIN, LOW); st(0x06); digitalWrite(CS_PIN, HIGH);
  digitalWrite(CS_PIN, LOW); st(0x20); st(0); st(0); st(0); digitalWrite(CS_PIN, HIGH);
  wait();

  // GRAVANDO ASSETS
  writeStr(100, "SNAKE VOLT"); // Nome do Jogo
  writeStr(200, "GAME OVER");  // Tela de morte
  writeStr(300, "PONTOS:");    // Placar
  writeStr(400, "RECORDE:");   // Recorde
  
  // Gravando um "Mapa" (Bytes que definem bordas ou obstáculos)
  // Exemplo: Endereço 500 guarda se o nível tem borda (1) ou não (0)
  uint8_t mapaConfig = 1; 
  digitalWrite(CS_PIN, LOW); st(0x06); digitalWrite(CS_PIN, HIGH);
  digitalWrite(CS_PIN, LOW); st(0x02); st(0); st(1); st(244); // Adr 500
  st(mapaConfig); digitalWrite(CS_PIN, HIGH);
  
  pinMode(PC13, OUTPUT); digitalWrite(PC13, LOW); // LED acende quando pronto
}
void loop() {}