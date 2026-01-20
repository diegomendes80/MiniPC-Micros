#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define CS_PIN      PA12
#define PIN_TECLADO PA0

int contador = 0;
int ultimoADC = 4095;

// --- FUNÇÕES SIMPLES DE FLASH ---

void flashWait() {
  uint8_t status;
  do {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(0x05); 
    status = SPI.transfer(0);
    digitalWrite(CS_PIN, HIGH);
  } while (status & 0x01);
}

void salvarContador(int valor) {
  // Habilita escrita e apaga setor
  digitalWrite(CS_PIN, LOW); SPI.transfer(0x06); digitalWrite(CS_PIN, HIGH);
  digitalWrite(CS_PIN, LOW); SPI.transfer(0x20); 
  SPI.transfer(0); SPI.transfer(0); SPI.transfer(0);
  digitalWrite(CS_PIN, HIGH);
  flashWait();

  // Habilita escrita e grava o valor (2 bytes)
  digitalWrite(CS_PIN, LOW); SPI.transfer(0x06); digitalWrite(CS_PIN, HIGH);
  digitalWrite(CS_PIN, LOW); SPI.transfer(0x02); 
  SPI.transfer(0); SPI.transfer(0); SPI.transfer(0);
  SPI.transfer(highByte(valor)); 
  SPI.transfer(lowByte(valor));
  digitalWrite(CS_PIN, HIGH);
  flashWait();
}

int lerContador() {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x03); 
  SPI.transfer(0); SPI.transfer(0); SPI.transfer(0);
  byte hi = SPI.transfer(0);
  byte lo = SPI.transfer(0);
  digitalWrite(CS_PIN, HIGH);
  return (hi << 8) | lo;
}

void setup() {
  u8g2.begin();
  pinMode(PIN_TECLADO, INPUT_ANALOG);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  SPI.begin();
  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));

  // Carrega o valor salvo ao iniciar
  contador = lerContador();
  if (contador < 0 || contador > 999) contador = 0; // Limpa se a memória estiver vazia
}

void loop() {
  int adc = analogRead(PIN_TECLADO);

  // Clique no Botão Especial (700-950)
  if (adc >= 700 && adc < 950) {
    contador++;
    salvarContador(contador);
    delay(200); // Debounce
  }
  ultimoADC = adc;

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_9x15_tf);
    u8g2.drawStr(10, 20, "CONTADOR:");
    u8g2.setFont(u8g2_font_logisoso32_tn);
    u8g2.setCursor(35, 60);
    u8g2.print(contador);
  } while (u8g2.nextPage());
}