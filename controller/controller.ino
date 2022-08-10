#include <ArduinoJson.h>
#include "Servo.h" //Carrega a biblioteca Servo
#include <SoftwareSerial.h>
#include <Wire.h>



SoftwareSerial BT(10, 11); // TX, RX | TX para enviar dados e RX para receber dados. 
String command = "";
int rele = 7;
int button = 6;
int val = 0;      // variável para guardar o valor lido
String umidadeArduino;
int vezes;
String umidade;
String calor;
Servo servo1;
DynamicJsonDocument jsonBuffer(1024); 
//String calor;


//function that sets the humidity
void setUmidade(String umidade){
  String log = "Umidade Antiga: "+umidadeArduino;
  umidadeArduino = umidade;
  log = "Umidade nova: "+umidadeArduino;
}

//Function that write on the phone with the bluetooth connection
void writeString(String stringData){
  BT.println(stringData);
}

//Function that check the humidity
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


//Function that irrigate
void regar(boolean val) {
  if(val){
  digitalWrite(rele, LOW);
  } else {
    digitalWrite(rele, HIGH);
  }
}

//Function that move the servomotor
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
     DeserializationError error = deserializeJson(jsonBuffer, c);
     if(!error) {
       umidade = jsonBuffer["umidade"].as<String>();
       vezes = jsonBuffer["vezes"];
       calor = jsonBuffer["calor"].as<String>();
       executeCommand(umidade,calor,vezes);
       delay(1000);
       Serial.println("Umidade recebida do celular: "+umidade);
       Serial.println("Calor recebido do celular: "+calor); 
     }
     
    } 
 }
}

/*
const int pinoSensor = A0; //PINO UTILIZADO PELO SENSOR
int valorLido; //VARIÁVEL QUE ARMAZENA O PERCENTUAL DE UMIDADE DO SOLO
 
int analogSoloSeco = 900; //VALOR MEDIDO COM O SOLO SECO (VOCÊ PODE FAZER TESTES E AJUSTAR ESTE VALOR)
int analogSoloMolhado = 320; //VALOR MEDIDO COM O SOLO MOLHADO (VOCÊ PODE FAZER TESTES E AJUSTAR ESTE VALOR)
int percSoloSeco = 0; //MENOR PERCENTUAL DO SOLO SECO (0% - NÃO ALTERAR)
int percSoloMolhado = 100; //MAIOR PERCENTUAL DO SOLO MOLHADO (100% - NÃO ALTERAR)
int valorSensor; 
void setup(){
 Serial.begin(9600); //INICIALIZA A SERIAL
 Serial.println("Lendo a umidade do solo..."); //IMPRIME O TEXTO NO MONITOR SERIAL
 pinMode(4, OUTPUT);
 delay(2000); //INTERVALO DE 2 SEGUNDOS
}
 
void loop(){  
 valorSensor = analogRead(pinoSensor);
 valorLido = constrain(valorSensor,analogSoloMolhado,analogSoloSeco); //MANTÉM valorLido DENTRO DO INTERVALO (ENTRE analogSoloMolhado E analogSoloSeco)
 valorLido = map(valorLido,analogSoloMolhado,analogSoloSeco,percSoloMolhado,percSoloSeco); //EXECUTA A FUNÇÃO "map" DE ACORDO COM OS PARÂMETROS PASSADOS
 int ml = (1024 - valorSensor)/70;
 Serial.print("Umidade do solo: "); //IMPRIME O TEXTO NO MONITOR SERIAL
 Serial.print(valorLido); //IMPRIME NO MONITOR SERIAL O PERCENTUAL DE UMIDADE DO SOLO
 Serial.println("%"); //IMPRIME O CARACTERE NO MONITOR SERIAL
 Serial.println(valorSensor);
  Serial.println("ml:");
 Serial.println(ml);
 if(valorLido > 70){
  digitalWrite(4, HIGH);
 } else {
  digitalWrite(4, LOW);
 }
 delay(1000);  //INTERVALO DE 1 SEGUNDO
}

*/
