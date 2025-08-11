#include <RunningMedian.h>

#define PH_PIN A0

RunningMedian samples = RunningMedian(20); // média de 20 leituras

// Valores de calibração — ajuste após medir com soluções padrão
float calibrationSlope = -5.70; // inclinação da curva (mV/pH)
float calibrationIntercept = 21.34; // offset (pH no ponto zero)

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Coleta múltiplas leituras para média
  for (int i = 0; i < 20; i++) {
    int analogValue = analogRead(PH_PIN);
    samples.add(analogValue);
    delay(10); // pequena pausa para estabilizar
  }

  float avgAnalog = samples.getMedian(); // filtragem por mediana
  float voltage = avgAnalog * (5.0 / 1023.0); // conversão para volts

  // Converte para pH usando a equação calibrada
  float pHValue = calibrationSlope * voltage + calibrationIntercept;

  Serial.print("pH: ");
  Serial.println(pHValue, 2);
  delay(500);
}
