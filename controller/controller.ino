#include <ArduinoJson.h>
#include "Servo.h" //Carrega a biblioteca Servo
#include <SoftwareSerial.h>
#include <Wire.h>

int RELE = 7;
int BUTTON = 6;
int VAL = 0;      // variável para guardar o VALor lido
int UMIDADEBD = 0;
int DIASRELATORIO = 0;
int RELATORIO[] = {0, 0};
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

void regar(int times, Servo servo) {
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
}

void loop(){
  DynamicJsonDocument jsonDoc(1024);
  
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
      
      if(!error) {
       bool dadosPlanta = jsonDoc["dadosPlanta"].as<bool>(); //Pega o atributo dadosPlanta do JSON
       bool getRelatorio = jsonDoc["getRelatorio"].as<bool>(); //Pega o atributo dadosPlanta do JSON
       
       if (dadosPlanta){ //SE ESSE ATRIBUTO FOR TRUE, ELE RESETA O RELATÓRIO E COMEÇA A UTILIZAR ESSA NOVA MEDIDA DE UMIDADE
        UMIDADEBD = jsonDoc["umidade"].as<int>();
        //RESETA O RELATORIO PARA SEGUIR COM A NOVA UMIDADE
        RELATORIO[0] = 0;
        RELATORIO[1] = 0;
        DIASRELATORIO = 0;
        
       } else if (getRelatorio) {
        String response;
        if (RELATORIO && UMIDADEBD) {
          int mediaUmidade = RELATORIO[0] / RELATORIO[1];
          jsonDoc["mediaUmidade"] = mediaUmidade;
        } else {
          jsonDoc["error"] = "Não há umidade inserida ou relatório suficiente para enviar um relatório";
        }
        serializeJson(jsonDoc, response);
        BT.println(response);
       }
      }
    } 
  }

  int umidade = 0;
  if (UMIDADEBD) {
    //IMPORTANTE IMPLEMENTAR PARA FUNCIONAR ↙
    //umidade = LeituraDoSensorDeUmidade();
    if (umidade < UMIDADEBD) {
      regar(2, SERVO1);
    }
    float diasLigados = ((((millis() / 1000)/60)/60)/24); //Transforma milissegundos para segundos, depois para minutos, depois para horas e depois para dias
    if (diasLigados > DIASRELATORIO) {
      RELATORIO[0] = RELATORIO[0] + umidade;  
      RELATORIO[1] = RELATORIO[1] + 1;
      DIASRELATORIO = DIASRELATORIO + 1;
    }
    delay(5000);
  }
}

/**
MODELO DE JSON RECEBIDO:
  PARA RECEBER O RELATORIO:
{
  "getRelatorio": true,
  "dadosPlanta": false,
  "umidade": 0,
}
  PARA INSERIR A UMIDADE:
{
  "getRelatorio": false,
  "dadosPlanta": true,
  "umidade": 324,
}

O ARDUINO SÓ PODE EXECUTAR UM POR VEZ, NÃO TEM COMO RECCEBER O RELATÓRIO E, AO MESMO TEMPO, ENVIAR OS DADOS DA PLANTA
*/
