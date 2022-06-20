#include <ArduinoJson.h>

#include <SoftwareSerial.h>
#include <Wire.h>
SoftwareSerial BT(10, 11); // TX, RX | TX para enviar dados e RX para receber dados. 
String command = "";
int rele = 0;
int button = 6;
int val = 0;      // variável para guardar o valor lido
String umidadeArduino;
//String calor;
StaticJsonBuffer<200> jsonBuffer;

void setUmidade(String umidade){
  String log = "Umidade Antiga: "+umidadeArduino;
  umidadeArduino = umidade;
  log = "Umidade nova: "+umidadeArduino;
 // BT.write(log);

}

void executeCommand(String umidade, String arg){
  if(umidade != umidadeArduino) {
    Serial.println("Umidade alterada para: "+arg);
  int bytesSent = BT.write("hello");
  Serial.println(bytesSent);
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


void setup()
{
  Serial.begin(9600);
  Serial.println("Tipo de Comando realizado");
  BT.write(1000);
  BT.begin(9600); // HC-05 usually default baud-rate
  pinMode(rele, OUTPUT);  
  pinMode(button, INPUT);  
  pinMode(2, OUTPUT); //led que indica transmissão de dados para o celular 
  pinMode(A5, INPUT);
 
}

void loop(){
   val = analogRead(A5);
  // if (calor > "500") {
      //Serial.println(val);
  //     BT.write("Muito Quente");
  //     calor="300";
   //}
  
   if (Serial.available()){
       digitalWrite(2, HIGH);
      
      String txt = Serial.readString();
      writeString(txt);
    
     

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
     //executeCommand(command, arg);
       Serial.println(umidade);
     Serial.println(command);
     executeCommand(umidade, calor);
      Serial.println("Umidade recebida do celular: "+umidade);
    Serial.println("Calor recebido do celular: "+calor);
    }
   
     

        
    if (command == "F"){
      digitalWrite(rele,HIGH);
    }
    if(command == "B"){
      digitalWrite(rele, LOW);
    }
 }
}
