#include <ArduinoJson.h>
#include "Servo.h" //Carrega a biblioteca Servo
#include <SoftwareSerial.h>
#include <Wire.h>

int RELE = 7;
int BUTTON = 6;
int VAL = 0;      // variável para guardar o VALor lido
int HUMIDITYBD = 0;
float REPORTDAYS = 0;
const int CAPACITY = JSON_OBJECT_SIZE(24);
int REPORT[] = {0, 0, 0};
SoftwareSerial BT(10, 11); // TX, RX | TX para enviar dados e RX para receber dados.]
Servo SERVO1; 

//BT.println(stringData);

//Function that move the servomotor
void rotateServo(int times, Servo servo) {
  for(times > 0; times--;){
    servo.write(180); 
    delay(500);
    servo.write(0); 
    delay(500);
  }
}

void irrigate(int times, Servo servo) {
  digitalWrite(RELE, HIGH);
  rotateServo(times, servo);
  digitalWrite(RELE, LOW);
}

void setup() {
  
  Serial.begin(9600);
  SERVO1.attach(5); 
  BT.begin(9600); // HC-05 usually default baud-rate
  pinMode(RELE, OUTPUT);  
  //digitalWrite(RELE, HIGH);
  pinMode(BUTTON, INPUT);  
  pinMode(2, OUTPUT); //led que indica transmissão de dados para o celular 
  pinMode(A0, INPUT);
  Serial.println("RODOU");
}

void loop(){
  StaticJsonDocument<CAPACITY> jsonDoc;
  
  if (Serial.available()){
    digitalWrite(2, HIGH);
    BT.write(Serial.read());
    digitalWrite(2, LOW);
  }
  //Leitura da porta serial via bluetooth
  if (BT.available()){ //Caso aconteça alguma alteração na leitura da porta...
    while(BT.available()){
      delay(15); 
      Serial.read();
      String json = String(BT.readString()); //Converte os dados recebidos em uma String.
      DeserializationError error = deserializeJson(jsonDoc, json); //Descompacta o JSON recebido
      Serial.println("Erro em baixo: ");
      Serial.println(error.c_str());
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

  int humidity = 2;
  if (HUMIDITYBD) {
    //IMPORTANTE IMPLEMENTAR FUNÇÃO DE LER UMIDADE PARA FUNCIONAR ↙
    if (humidity < HUMIDITYBD) {
      irrigate(2, SERVO1);
      REPORT[2] = REPORT[2] + 1;
    }
    float secondsOn = millis() / 1000; 
    float minutesOn = secondsOn / 60; 
    float hoursOn = minutesOn / 60; 
    float daysOn = hoursOn / 24; //Transforma milissegundos para segundos, depois para minutos, depois para horas e depois para dias
    Serial.println("Dias de Reportados");
    Serial.println(REPORTDAYS);
    if (daysOn > REPORTDAYS) {
      REPORT[1] = REPORT[1] + 1;
      REPORT[0] = REPORT[0] + humidity;  
      REPORTDAYS = REPORTDAYS + 1;
    }
    //delay(5000);
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
