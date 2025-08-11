// Pinos
#define TDS_PIN A1

// Configurações de leitura
const int SAMPLE_COUNT = 30;      // número de leituras por ciclo
const int SAMPLE_DELAY_MS = 50;   // delay entre leituras (ajuste se necessário)

// Parâmetros de compensação
float tempCoef = 0.02;            // coeficiente de compensação por °C (típico ~0.02)
float tdsFactor = 0.5;            // fator TDS = EC(µS/cm) * factor -> ppm (ajuste na calibração)
float tempC = 25.0;               // temperatura fixa (sem sensor)

// Se seu módulo requer Vcc estável, defina aqui (ex: 5.0 ou 3.3)
const float VREF = 5.0;

// Para conversão ADC (Arduino UNO 10 bits)
const float ADC_MAX = 1023.0;

// ----- Funções utilitárias -----
float readMedianAnalog(uint8_t pin, int samples) {
  float vals[50];
  if(samples > 50) samples = 50;
  for(int i=0;i<samples;i++){
    vals[i] = analogRead(pin);
    delay(5);
  }
  // ordena (insertion sort)
  for(int i=1;i<samples;i++){
    float key = vals[i];
    int j = i-1;
    while(j>=0 && vals[j] > key){
      vals[j+1] = vals[j];
      j--;
    }
    vals[j+1] = key;
  }
  // retorna mediana
  if(samples % 2 == 1) return vals[samples/2];
  return (vals[samples/2 - 1] + vals[samples/2]) / 2.0;
}

float movingAverage(float *buf, int len) {
  float s = 0;
  for(int i=0;i<len;i++) s += buf[i];
  return s / len;
}

void setup() {
  Serial.begin(9600);
  delay(1000);
}

void loop() {
  // 1) coletar leituras TDS e filtrar (mediana + média)
  static float buffer[10];
  int bufferIdx = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    float med = readMedianAnalog(TDS_PIN, 7); // mediana de 7 rápidas
    float voltage = med * (VREF / ADC_MAX); // volts
    buffer[bufferIdx++ % 10] = voltage;
    delay(SAMPLE_DELAY_MS);
  }
  float avgVoltage = movingAverage(buffer, 10);

  // 2) converter voltagem -> condutividade EC (µS/cm)
  const float SCALE_EC = 2000.0; // ajuste via calibração
  float ecRaw = (avgVoltage / VREF) * SCALE_EC; // µS/cm (raw)

  // 3) compensação de temperatura (referência 25°C)
  float ec25 = ecRaw / (1.0 + tempCoef * (tempC - 25.0));

  // 4) converter EC -> TDS (ppm)
  float tds = ec25 * tdsFactor; // ppm

  // Impressão
  Serial.print("Temp (C): ");
  Serial.print(tempC, 2);
  Serial.print("  Vavg: ");
  Serial.print(avgVoltage, 3);
  Serial.print(" V  ECraw(µS/cm): ");
  Serial.print(ecRaw, 1);
  Serial.print("  EC(25C): ");
  Serial.print(ec25, 1);
  Serial.print("  TDS(ppm): ");
  Serial.println(tds, 1);

  delay(1000);
}
