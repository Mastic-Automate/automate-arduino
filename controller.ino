#include <ArduinoJson.h>
//Carrega a biblioteca Servo
#include "Servo.h"
#include <SoftwareSerial.h>
#include <Wire.h>
SoftwareSerial BT(10, 11); // TX, RX | TX para enviar dados e RX para receber dados. 
String command = "";
int rele = 7;
int button = 6;
int val = 0;      // variável para guardar o valor lido
String umidadeArduino;
String umidade;
String calor;
String vezes;
int times;
Servo servo1; 
//String calor;
StaticJsonBuffer<200> jsonBuffer;

void setUmidade(String umidade){
  String log = "Umidade Antiga: "+umidadeArduino;
  umidadeArduino = umidade;
  log = "Umidade nova: "+umidadeArduino;
}

void writeString(String stringData){
  BT.println(stringData);
}

void executeCommand(String umidade, String arg, int times){
  if(umidade != umidadeArduino) {
    Serial.println("Umidade alterada para: "+umidade);
  
   setUmidade(umidade);
  }
    delay(500);
    servo(times);
    delay(500);
   writeString("Agora foi. tenho certeza!!");
    delay(3000);
}



void regar(boolean val) {
  if(val){
  digitalWrite(rele, LOW);
  } else {
    digitalWrite(rele, HIGH);
  }
}

void servo(int times) {
  for(times > 0; times--;){
   
    int i = 180;
  // Repassa o angulo ao ServoWrite
  
  // Delay de 15ms para o Servo alcançar a posição
  servo1.write(i); 
  delay(500);
 digitalWrite(2,HIGH);

//delay(100); 
 regar(true);
  while (i > 0) {
    //i=map(i, 0, 1023, 0, 180);
  // Repassa o angulo ao ServoWrite
  servo1.write(i); 
  // Delay de 15ms para o Servo alcançar a posição
  delay(7);
  i--;
  }
  regar(false);
  delay(500);
}
//regar(false);
  servo1.write(0); 
  digitalWrite(2, LOW);
}


void setup()
{
  Serial.begin(9600);
  Serial.println("Tipo de Comando realizado");
  BT.begin(9600); // HC-05 usually default baud-rate
  pinMode(rele, OUTPUT);  
  pinMode(button, INPUT);  
  pinMode(2, OUTPUT); //led que indica transmissão de dados para o celular 
  pinMode(A0, INPUT);
  servo1.attach(5); 
  digitalWrite(rele, HIGH);
}

void loop(){



   
   if (Serial.available()){
       digitalWrite(2, HIGH);
      BT.write(Serial.read());
      
      // delay(500);
       digitalWrite(2, LOW);
      }
      
  //Leitura da porta serial via bluetooth
  if (BT.available()){ //Caso aconteça alguma alteração na leitura da porta...
    while(BT.available()){
     delay(15); 
     Serial.read();
     String c = String(BT.readString()); //Converte os dados recebidos em um caractere.
     command = c;
     JsonObject& root = jsonBuffer.parseObject(c);
     vezes = root["vezes"].as<String>();
    umidade = root["umidade"].as<String>();
     calor = root["calor"].as<String>();
     
     times = vezes.toInt();
     executeCommand(umidade,calor,times);
     delay(1000);
     Serial.println("Umidade recebida do celular: "+umidade);
    Serial.println("Calor recebido do celular: "+calor);
    } 
 }
}
