#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include "ctimer.h"
#include <Servo.h>

#define DRY_SOIL (1023) //VALOR MEDIDO COM O SOLO SECO (VOCÊ PODE FAZER TESTES E AJUSTAR ESTE VALOR)
#define WET_SOIL (224) //VALOR MEDIDO COM O SOLO MOLHADO (VOCÊ PODE FAZER TESTES E AJUSTAR ESTE VALOR)
#define ZERO_PERCENT (0)
#define ONE_HUNDRED_PERCENT (100)

#define WATER_BOMB (3)
#define SERVO_PIN (6)
#define HUMIDITY_SENSOR (A0)      // variável para guardar o VALor lido
int HUMIDITYBD = 0;
float REPORTDAYS = 0;
int REPORT[] = {0, 0, 0};
Servo servo;
const int CAPACITY = JSON_OBJECT_SIZE(7);
SoftwareSerial BT(10, 11); // TX, RX | TX para enviar dados e RX para receber dados.]
cTimer dayTimer(true);
cTimer irrigateTimer(false);
cTimer drainTimer(true);

void setup() {
  dayTimer.SetTimeOut(86400000); //Número de milissegundos que tem em um dia
  irrigateTimer.SetTimeOut(1000);
  drainTimer.SetTimeOut(5000);
  servo.attach(SERVO_PIN);
  servo.write(0);
  Serial.begin(9600);
  BT.begin(9600); // HC-05 usually default baud-rate
  pinMode(WATER_BOMB, OUTPUT);  
  pinMode(HUMIDITY_SENSOR, INPUT);
  Serial.println("RODOU");
}

void loop(){
  StaticJsonDocument<CAPACITY> jsonDoc;
  
  //Leitura da porta serial via bluetooth
  if (BT.available()){ //Caso aconteça alguma alteração na leitura da porta...
    while(BT.available()){
      Serial.read();
      String json = String(BT.readString()); //Converte os dados recebidos em uma String.
      DeserializationError error = deserializeJson(jsonDoc, json); //Descompacta o JSON recebido
      Serial.println("Erro em baixo: ");
      Serial.println(error.c_str());
      Serial.println(json);
      String response;
      String errorText = error.c_str();
      if(!error) {
       bool plantData = jsonDoc["plantData"].as<bool>(); //Pega o atributo plantData do JSON
       bool getReport = jsonDoc["getReport"].as<bool>(); //Pega o atributo plantData do JSON
       if (plantData){ //SE ESSE ATRIBUTO FOR TRUE, ELE RESETA O RELATÓRIO E COMEÇA A UTILIZAR ESSA NOVA MEDIDA DE UMIDADE
        HUMIDITYBD = jsonDoc["humidity"].as<int>();
        //RESETA O REPORT PARA SEGUIR COM A NOVA UMIDADE
        REPORT[0] = 0;
        REPORT[1] = 0;
        REPORT[2] = 0;
        REPORTDAYS = 0;
        deserializeJson(jsonDoc, "{}");
       } else if (getReport) {
        float humidityAverage = 0;
        int wateredTimes = 0;
        if (REPORT && HUMIDITYBD) {
          humidityAverage = REPORT[0] / REPORT[1];
          wateredTimes = REPORT[2];
        } else {
          errorText = "noData";  
        }
        deserializeJson(jsonDoc, "{}");
        jsonDoc["humidityAverage"] = humidityAverage;
        jsonDoc["wateredTimes"] = wateredTimes;
        
       }
      } else {
        deserializeJson(jsonDoc, "{}");  
      }
      jsonDoc["error"] = errorText;
      serializeJson(jsonDoc, response);
      BT.println(response);
    } 
  }
  
  if (HUMIDITYBD) {
    //IMPORTANTE IMPLEMENTAR FUNÇÃO DE LER UMIDADE PARA FUNCIONAR ↙
    int sensorRead = analogRead(HUMIDITY_SENSOR);
    float humidity = map(sensorRead,WET_SOIL,DRY_SOIL,ONE_HUNDRED_PERCENT,ZERO_PERCENT); // tá em porcentagem    
    Serial.println(humidity);
  
    if (irrigateTimer.IsEnabled()) {
       if (irrigateTimer.ReadTimeOut() > 500) {
        servo.write(180);
       } else {
        servo.write(0);
       }
       
       if (irrigateTimer.IsTimeOut(false)) {
        digitalWrite(WATER_BOMB, LOW);
        irrigateTimer.Enable(false);  
        drainTimer.IsTimeOut(true);
      }
      
    } else {
        if (humidity < HUMIDITYBD && drainTimer.IsTimeOut()) {
          digitalWrite(WATER_BOMB, HIGH);
          irrigateTimer.Enable(true);  
          irrigateTimer.IsTimeOut(true);
          REPORT[2] = REPORT[2] + 1;
        }
    }
    
    if (!REPORTDAYS || dayTimer.IsTimeOut(true)) {
      REPORT[1] = REPORT[1] + 1;
      REPORT[0] = REPORT[0] + humidity;  
      REPORTDAYS = REPORTDAYS + 1;
    }
  }
}

/**
MODELO DE JSON RECEBIDO:
  PARA RECEBER O REPORT:
{
  "getReport": true,
  "plantData": false,
  "humidity": 0,
}
  PARA INSERIR A UMIDADE:
{
  "getReport": false,
  "plantData": true,
  "humidity": 324,
}
O ARDUINO SÓ PODE EXECUTAR UM POR VEZ, NÃO TEM COMO RECCEBER O RELATÓRIO E, AO MESMO TEMPO, ENVIAR OS DADOS DA PLANTA
O ARDUINO ENVIA:
{
  "error": mensagem de erro ou null,
  "humidityAverage": a média da umidade ou null,
  "wateredTimes": a quantidade de vezes que a planta foi regada ou null
}
*/
