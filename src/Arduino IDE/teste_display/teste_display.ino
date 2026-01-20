#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// Configuração para o SH1106 de 1.3" I2C
// Modo _1_ (Page Buffer) para economizar memória FLASH
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

const int PIN_TECLADO = PA0;

// Função com os SEUS valores medidos (0, 145, 305, 492, 780)
String identificarTecla(int valor) {
  if (valor > 3500) return "SOLTO"; 
  
  if (valor >= 0   && valor < 80)   return "ESQUERDO";   // Seu: 0
  if (valor >= 100 && valor < 200)  return "CIMA";       // Seu: 145
  if (valor >= 250 && valor < 380)  return "BAIXO";      // Seu: 304/305
  if (valor >= 430 && valor < 600)  return "DIREITO";    // Seu: 492/494
  if (valor >= 700 && valor < 950)  return "ESPECIAL";   // Seu: 780/797
  
  return "CALIBRANDO..."; 
}

void setup() {
  u8g2.begin();
  pinMode(PIN_TECLADO, INPUT_ANALOG);
  pinMode(PC13, OUTPUT);
}

void loop() {
  // 1. Lê o valor bruto do ADC
  int adc_val = analogRead(PIN_TECLADO);
  
  // 2. Identifica a tecla
  String nomeTecla = identificarTecla(adc_val);

  // 3. Desenha na tela (Ciclo Page Buffer)
  u8g2.firstPage();
  do {
    // Moldura e Título
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(10, 15, "TESTE DE INPUT");
    u8g2.drawHLine(0, 20, 128);

    // Mostra o valor ADC bruto (ajuda na conferência)
    u8g2.setCursor(10, 35);
    u8g2.print("ADC: "); 
    u8g2.print(adc_val);

    // Mostra o nome da tecla em fonte maior
    u8g2.setFont(u8g2_font_ncenB12_tr);
    int largura = u8g2.getStrWidth(nomeTecla.c_str());
    u8g2.setCursor((128 - largura) / 2, 55); // Centraliza
    u8g2.print(nomeTecla);

  } while (u8g2.nextPage());

  // Feedback no LED (acende se qualquer tecla for pressionada)
  digitalWrite(PC13, (adc_val < 3500) ? LOW : HIGH);

  delay(30); // Taxa de atualização rápida para resposta imediata
}