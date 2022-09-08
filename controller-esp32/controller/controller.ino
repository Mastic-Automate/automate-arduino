#include "ctimer.h"
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

int DRY_SOIL = 1023; //VALOR MEDIDO COM O SOLO SECO (VOCÊ PODE FAZER TESTES E AJUSTAR ESTE VALOR)
int WET_SOIL = 224; //VALOR MEDIDO COM O SOLO MOLHADO (VOCÊ PODE FAZER TESTES E AJUSTAR ESTE VALOR)
int ZERO_PERCENT = 0;
int ONE_HUNDRED_PERCENT = 100;

int WATER_BOMB = 26;
int BUTTON = 6;
int HUMIDITY_SENSOR = 25;      // variável para guardar o VALor lido
int HUMIDITYBD = 0;
bool SEND_FEEDBACK = false;
String FEEDBACK = "Ok";
bool SEND_REPORT = false;
float REPORTDAYS = 0;
int REPORT[] = {0, 0, 0};
const int CAPACITY = JSON_OBJECT_SIZE(18);
cTimer dayTimer(true);
cTimer irrigateTimer(false);
cTimer drainTimer(true);

#define SERVICE_UUID           "ab0828b1-198e-4351-b779-901fa0e0371e"
#define CHARACTERISTIC_UUID_RX "4ac8a682-9736-4e5d-932b-e9b31405049c"
#define CHARACTERISTIC_UUID_TX "0972EF8C-7613-4075-AD52-756F33D4DA91"

BLECharacteristic *characteristicTX;

bool deviceConnected = false;
class ServerCallbacks: public BLEServerCallbacks { //callback para receber os eventos de conexão de dispositivos
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class CharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *characteristic) {
      std::string rxValue = characteristic->getValue(); 
      String strRxValue;
      
      if (rxValue.length() > 0) { //verifica se existe dados (tamanho maior que zero)
        StaticJsonDocument<CAPACITY> jsonDoc;
        
        for (int i = 0; i < rxValue.length(); i++) {
          strRxValue = strRxValue + rxValue[i];
        }
        DeserializationError error = deserializeJson(jsonDoc, strRxValue); //Descompacta o JSON recebido

        if (!error) {
          bool plantData = jsonDoc["plantData"].as<bool>(); //Pega o atributo plantData do JSON
          bool getReport = jsonDoc["getReport"].as<bool>(); //Pega o atributo plantData do JSON
          if (plantData){ //SE ESSE ATRIBUTO FOR TRUE, ELE RESETA O RELATÓRIO E COMEÇA A UTILIZAR ESSA NOVA MEDIDA DE UMIDADE
            HUMIDITYBD = jsonDoc["humidity"].as<int>();
            //RESETA O REPORT PARA SEGUIR COM A NOVA UMIDADE
            REPORT[0] = 0;
            REPORT[1] = 0;
            REPORT[2] = 0;
            REPORTDAYS = 0;
            SEND_FEEDBACK = true;
            FEEDBACK = error.c_str();
          } else if (getReport) {
            SEND_REPORT = true;
          }
        } else {
          SEND_FEEDBACK = true;
        }
      }
    }
};

void setup() {
  Serial.begin(115200);
  
  dayTimer.SetTimeOut(86400000); //Número de milissegundos que tem em um dia
  irrigateTimer.SetTimeOut(1000);
  drainTimer.SetTimeOut(5000);
   
  pinMode(WATER_BOMB, OUTPUT);  
  pinMode(BUTTON, INPUT);  
  pinMode(HUMIDITY_SENSOR, INPUT);
  
  
  BLEDevice::init("AUTOMATE-ESP"); // nome do dispositivo bluetooth
  BLEServer *server = BLEDevice::createServer(); //cria um BLE server 
  server->setCallbacks(new ServerCallbacks()); //seta o callback do server
  
  // Create the BLE Service
  BLEService *service = server->createService(SERVICE_UUID);
  
  // Create a BLE Characteristic para envio de dados
  characteristicTX = service->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  characteristicTX->addDescriptor(new BLE2902());

  // Create a BLE Characteristic para recebimento de dados
  BLECharacteristic *characteristic = service->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  characteristic->setCallbacks(new CharacteristicCallbacks());
  // Start the service
  service->start();
  // Start advertising (descoberta do ESP32)
  server->getAdvertising()->start();  
  
  Serial.println("Aguardando notificação - RODOU");

}

void loop() {
     //se existe algum dispositivo conectado
    if (deviceConnected) {
      StaticJsonDocument<CAPACITY> jsonDoc;
      String errorText = "Ok";
      String responseBT;
      if (SEND_REPORT){
        float humidityAverage = 0;
        int wateredTimes = 0;
        if (REPORT && HUMIDITYBD) {
          humidityAverage = REPORT[0] / REPORT[1];
          wateredTimes = REPORT[2];
        } else {
          errorText = "noData";  
        }
        jsonDoc["humidityAverage"] = humidityAverage;
        jsonDoc["wateredTimes"] = wateredTimes;
        SEND_REPORT = false;
        
      } else if (SEND_FEEDBACK) {
        errorText = FEEDBACK;
      }
      jsonDoc["error"] = errorText;
      serializeJson(jsonDoc, responseBT);
      SEND_FEEDBACK = false;

      char buffer[responseBT.length()+1];
      responseBT.toCharArray(buffer,responseBT.length()+1);
      characteristicTX->setValue((char*)&buffer);
      characteristicTX->notify();
    }

    if (HUMIDITYBD) {
      //IMPORTANTE IMPLEMENTAR FUNÇÃO DE LER UMIDADE PARA FUNCIONAR ↙
      int sensorRead = analogRead(HUMIDITY_SENSOR);
      float humidity = map(sensorRead,WET_SOIL,DRY_SOIL,ONE_HUNDRED_PERCENT,ZERO_PERCENT); // tá em porcentagem    
   
      if (drainTimer.IsTimeOut() && !irrigateTimer.IsEnabled()) {
        if (humidity < HUMIDITYBD) {
          digitalWrite(WATER_BOMB, HIGH);
          irrigateTimer.Enable(true);  
          irrigateTimer.IsTimeOut(true);
          REPORT[2] = REPORT[2] + 1;
        }
      }
  
      if (irrigateTimer.IsTimeOut(false) && irrigateTimer.IsEnabled()) {
        digitalWrite(WATER_BOMB, LOW);
        irrigateTimer.Enable(false);  
        drainTimer.IsTimeOut(true);
      }
      
      if (!REPORTDAYS || dayTimer.IsTimeOut(true)) {
        REPORT[1] = REPORT[1] + 1;
        REPORT[0] = REPORT[0] + humidity;  
        REPORTDAYS = REPORTDAYS + 1;
      }
  }
}
