
#include <SPI.h>

/* ===== DEFINIÇÕES ===== */
#define CS_PIN   PA12
#define LED_PIN  PC13

uint8_t jedec[3];

void setup() {
  /* LED */
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED OFF (PC13 é ativo em LOW)

  /* CS */
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);  // Flash deselecionada

  /* SPI */
  SPI.begin();
  SPI.beginTransaction(SPISettings(
    500000,        // 500 kHz (bem lento)
    MSBFIRST,
    SPI_MODE0
  ));

  delay(100);

  /* === LEITURA JEDEC === */
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x9F);
  jedec[0] = SPI.transfer(0x00);
  jedec[1] = SPI.transfer(0x00);
  jedec[2] = SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);

  SPI.endTransaction();

  /* === INDICAÇÃO === */
  if (jedec[0] == 0xEF) {
    digitalWrite(LED_PIN, LOW);   // LED ON → Flash OK
  } else {
    digitalWrite(LED_PIN, HIGH);  // LED OFF → Falha
  }

  /* trava aqui para debug */
  while (1);
}

void loop() {
  // nunca chega aqui
}
