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
Servo servo1; 
//String calor;
StaticJsonBuffer<200> jsonBuffer;

void setUmidade(String umidade){
  String log = "Umidade Antiga: "+umidadeArduino;
  umidadeArduino = umidade;
  log = "Umidade nova: "+umidadeArduino;
 // BT.write(log);

}

void executeCommand(String umidade, String arg, int times){
  if(umidade != umidadeArduino) {
    Serial.println("Umidade alterada para: "+umidade);
    servo(times);
   setUmidade(umidade);
  }
}

void writeString(String stringData){
 // for (int i =0; i < stringData.length(); i++) {
    //BT.write(stringData[i]);
  //}

  int i = 0;
  String sendData = stringData;
  int numberData = sendData.length();

  while (i <= numberData){
    char oneLetter = sendData[i];
    BT.write(oneLetter);
    //BT.print(oneLetter);
    i++;
    //delay(50);
  }
  //BT.print("");
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
 // delay(5000);
servo1.write(180); 
delay(1000); 
 regar(true);
  while (i > 0) {
    //i=map(i, 0, 1023, 0, 180);
  // Repassa o angulo ao ServoWrite
  servo1.write(i); 
  // Delay de 15ms para o Servo alcançar a posição
  delay(15);
  i--;
  }
  regar(false);
  delay(500);
}
regar(false);
  servo1.write(0); 
}


void setup()
{
  Serial.begin(9600);
  Serial.println("Tipo de Comando realizado");
  BT.write(1000);
  BT.begin(9600); // HC-05 usually default baud-rate
  pinMode(rele, OUTPUT);  
  pinMode(button, INPUT);  
  pinMode(2, OUTPUT); //led que indica transmissão de dados para o celular 
  pinMode(A0, INPUT);
  servo1.attach(5); 
  digitalWrite(rele, HIGH);
}

void loop(){


servo1.write(0); 
   
   if (Serial.available()){
       digitalWrite(2, HIGH);
      
      String txt = String(Serial.readString());
      //writeString(txt);
    
     JsonObject& root = jsonBuffer.parseObject(txt);
     String umidade = root["umidade"];
     String calor = root["calor"];
     String vezes = root["vezes"];
     int times = vezes.toInt();
     executeCommand(umidade, calor,times);
      Serial.println("Umidade recebida do Serial: "+umidade);
    Serial.println("Calor recebido do Serial: "+calor);
              
      // Serial.println(mensagem);
      
       delay(500);
       digitalWrite(2, LOW);
      }
      
  //Leitura da porta serial via bluetooth
  if (BT.available()){ //Caso aconteça alguma alteração na leitura da porta...
    while(BT.available()){
     delay(15); 
     String c = String(BT.readString()); //Converte os dados recebidos em um caractere.
     command = c;
     JsonObject& root = jsonBuffer.parseObject(c);
     String umidade = root["umidade"];
     String calor = root["calor"];
     String vezes = root["vezes"];
     int times = vezes.toInt();
     executeCommand(umidade, calor,times);
      Serial.println("Umidade recebida do celular: "+umidade);
    Serial.println("Calor recebido do celular: "+calor);
    } 
 }
}
