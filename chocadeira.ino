#include "DHT.h"
#include <stdio.h>

#define DHTPIN 4     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors
#define Rele_lamp_02 5
#define Rele_motor_03 0
#define an_hour 3600000UL
#define thirty_seconds 6000UL

unsigned long startTime;
double tempMax = 37.80;
double tempMin = 36.40;
int horasDia = 0;
int horaUltimaRolagem;
int qtdDias = 0;
bool primeiraExecucao = true;
bool rollActived = false;
bool lampActived = false;
unsigned long startTimeRoll;


DHT dht(DHTPIN, DHTTYPE);


void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while(!Serial) { }

  pinMode(Rele_lamp_02, OUTPUT);
  digitalWrite(Rele_lamp_02, HIGH);
  pinMode(Rele_motor_03, OUTPUT);
  digitalWrite(Rele_motor_03, HIGH);
  
  dht.begin();

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!");

  startTime = millis();
}

void loop() {
  delay(2000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();


  long time = (millis() - startTime);

  long hora = time / an_hour;
  time = time - an_hour * hora;
  long min = time / 60000;
  time = time - 60000 * min;
  long sec = time / 1000;
  time = time - 1000 * sec;
  
  stopRolagem();

  if(primeiraExecucao){

    primeiraExecucao = false;
    acionaRolagem(hora, millis());
  }


    Serial.printf("%02d:%02d:%02d", hora, min, sec);
    Serial.print(" - ");
      
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");

  if(t >= tempMax) {
   digitalWrite(Rele_lamp_02, HIGH);
   lampActived = false;
   Serial.println(" LAMP OFF");
  }
  
  if(t <= tempMin ) {
    digitalWrite(Rele_lamp_02, LOW);
    lampActived = true;
    Serial.println(" LAMP ON");
  }

  if(t == 0.0 || isnan(t)) {
    if(lampActived){
      digitalWrite(Rele_lamp_02, HIGH);
      delay(2000);
      lampActived = false;
            
    }else {
      digitalWrite(Rele_lamp_02, LOW);
      delay(2000);
      lampActived = true;
    }
  }
  Serial.println();

  if(hora > horasDia) {
    // acrescenta hora
    horasDia = hora;

    if(horasDia == 24) {
      startTime = millis();
      horasDia = 0;
      horaUltimaRolagem = 0;
      qtdDias++;
      acionaRolagem(horasDia, millis());
    }

    if(horasDia > horaUltimaRolagem && 
        horaUltimaRolagem % 2 == 0 && 
        hora % 2 == 0 && 
        qtdDias < 18) {
      
      acionaRolagem(hora, millis());
    }
  }
}

void acionaRolagem(long hora, long nowTimestamp) {
  horaUltimaRolagem = hora;
  digitalWrite(Rele_motor_03, LOW);
  startTimeRoll = nowTimestamp;
  rollActived = true;
  mostraPainelLogs(horasDia, qtdDias, horaUltimaRolagem);
  Serial.println();
  Serial.println("###################################");
  Serial.println("######### Rolagem Ativa ###########");
  Serial.println("###################################"); 
  Serial.println();    
}

void stopRolagem() {

   long timeRolling = (millis() - startTimeRoll);
  if(rollActived && (timeRolling > thirty_seconds)) {
    digitalWrite(Rele_motor_03, HIGH);
    rollActived = false;    
    Serial.println();
    Serial.println("###################################");
    Serial.println("#############Rolou ################");
    Serial.println("###################################"); 
    Serial.println(); 
  }  
}

void mostraPainelLogs(long horasDia, long qtdDias, long ultimaRolagem) {
Serial.println();
    Serial.println("----------------------------------");
    Serial.print("\t Horas: ");
    Serial.print(horasDia);
    Serial.print("\t Dias: ");
    Serial.println(qtdDias);
    Serial.print("\t Ultima Rolagem: ");
    Serial.println(ultimaRolagem);
    Serial.println("----------------------------------");
}