#include <ArduinoJson.h>

#include <SoftwareSerial.h>
#include <Wire.h>
SoftwareSerial BT(8, 7); // TX, RX | TX para enviar dados e RX para receber dados. 
String command = "";
int rele = 7;

StaticJsonBuffer<200> jsonBuffer;

void setUmidade(int umidade){
  
}

void executeCommand(String command, String arg){
  if(command == "setUmidade") {
    Serial.println("Umidade alterada para: "+arg);
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Tipo de Comando realizado");
  BT.begin(9600); // HC-05 usually default baud-rate
  pinMode(rele, OUTPUT);  
}

void loop(){
  //Leitura da porta serial via bluetooth
  if (BT.available()){ //Caso aconteça alguma alteração na leitura da porta...
    while(BT.available()){
     delay(15); 
     String c = String(BT.readString()); //Converte os dados recebidos em um caractere.
     command = c;
     JsonObject& root = jsonBuffer.parseObject(command);
     String arg = root["arg"];
     String command = root["command"];
     executeCommand(command, arg);
     Serial.println(arg);
    }
    Serial.println(command);
        
    if (command == "F"){
      digitalWrite(rele,HIGH);
    }
    if(command == "B"){
      digitalWrite(rele, LOW);
    }
 }
}
