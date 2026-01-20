#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <U8g2lib.h>

// Driver em modo "Page" (o mais leve)
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);


volatile int tela = 1;

// task 1 - Gerencia o que aparece na tela
void TaskInterface(void *pvParameters) {
  for (;;) {
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_04b_03_tr); 
      if (tela == 1) {
        u8g2.drawStr(40, 35, "TAREFA 1");
      } else {
        u8g2.drawStr(40, 35, "TAREFA 2");
      }
      u8g2.drawStr(10, 60, "Botao K5 p/ mudar");
    } while (u8g2.nextPage());
    
    vTaskDelay(pdMS_TO_TICKS(100)); // Taxa de atualização
  }
}

// task 2 - Monitora o Botão (Independente)
void TaskBotao(void *pvParameters) {
  for (;;) {
    int adc = analogRead(PA0);
    // Range do K5: 700 a 950
    if (adc >= 700 && adc < 950) {
      tela = (tela == 1) ? 2 : 1; // Alterna entre 1 e 2
      vTaskDelay(pdMS_TO_TICKS(400)); // Debounce para não alternar loucamente
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void setup() {
  u8g2.begin();
  pinMode(PA0, INPUT_ANALOG);

//chama as duas tarefas
//nesse codigo as duas tarefas estao rodando ao mesmo tempo, a tarefa de criar a interface e a de ler o valor do botao
  xTaskCreate(TaskInterface, "I", 192, NULL, 1, NULL);
  xTaskCreate(TaskBotao,     "B", 64,  NULL, 2, NULL);

  vTaskStartScheduler();
}

void loop() {}