#include <SPI.h>

// Pino de Seleção do Chip (CS)
const int CH376_CS = 12;

void setup() {
  Serial.begin(9600);
  while (!Serial); // Aguarda abrir o Monitor Serial

  pinMode(CH376_CS, OUTPUT);
  digitalWrite(CH376_CS, HIGH); // Desativa o chip inicialmente

  // Inicia o barramento SPI
  SPI.begin();
  // CH376S trabalha bem em 1MHz ou 2MHz para testes
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

  Serial.println("========================================");
  Serial.println("   TESTE DE COMUNICACAO CH376S (UNO)    ");
  Serial.println("========================================");
  delay(500);
}

void loop() {
  Serial.print("Enviando comando 0x06... ");

  // Inicia a conversa com o chip
  digitalWrite(CH376_CS, LOW);
  
  // 1. Envia o comando CHECK_EXIST (0x06)
  SPI.transfer(0x06); 
  
  // 2. Envia um valor qualquer para o chip inverter (0x55)
  // O CH376S deve retornar o inverso bit a bit (0xAA)
  uint8_t resposta = SPI.transfer(0x55); 
  
  digitalWrite(CH376_CS, HIGH);
  // Finaliza a conversa

  Serial.print("Recebido: 0x");
  if (resposta < 16) Serial.print("0"); // Formatação HEX
  Serial.println(resposta, HEX);

  if (resposta == 0xAA) {
    Serial.println("[SUCESSO] O modulo esta VIVO e respondendo!");
    digitalWrite(PC13, HIGH);
  } else if (resposta == 0xFF) {
    Serial.println("[ERRO] Resposta 0xFF: Verifique a fiacao do MISO/SDO.");
  } else if (resposta == 0x00) {
    Serial.println("[ERRO] Resposta 0x00: O chip nao esta alimentado ou RSTI esta solto.");
  } else {
    Serial.println("[ERRO] Resposta invalida. Verifique o Cristal de 12MHz.");
  }

  Serial.println("----------------------------------------");
  delay(3000); // Espera 3 segundos para o proximo teste
}