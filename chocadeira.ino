#include "DHT.h"
#include <stdio.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 18     // temperature sensor
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors
#define Rele_cooler_02 16 // Rele estado sólido porta 4
#define Rele_lamp_04 17 // Rele estado sólido porta 4
#define Rele_motor_DC 19 // rele


#define an_hour 3600000UL
#define thirty_seconds 8000UL

#define col 20 // Serve para definir o numero de colunas do display utilizado
#define lin  4 // Serve para definir o numero de linhas do display utilizado
#define ende 0x27 // Serve para definir o endereço do display.

LiquidCrystal_I2C lcd(ende, col, lin);

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
  lcd.init();
  lcd.backlight(); 
  lcd.clear(); 
  Serial.begin(9600);
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while(!Serial) { }

  pinMode(Rele_lamp_04, OUTPUT);
  digitalWrite(Rele_lamp_04, HIGH);
  pinMode(Rele_motor_DC, OUTPUT);
  digitalWrite(Rele_motor_DC, HIGH);
  pinMode(Rele_cooler_02, OUTPUT);
  digitalWrite(Rele_cooler_02, LOW);

  
  
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

   

  if(t >= tempMax) {
   digitalWrite(Rele_lamp_04, HIGH);
   lampActived = false;
   Serial.println(" LAMP OFF");
  }
  
  if(t <= tempMin ) {
    digitalWrite(Rele_lamp_04, LOW);
    lampActived = true;
    Serial.println(" LAMP ON");
  }

  if(t == 0.00 || isnan(t)) {
      digitalWrite(Rele_lamp_04, LOW);
      delay(2000);
      lampActived = true;
            
    if(lampActived) {
      digitalWrite(Rele_lamp_04, HIGH);
      delay(2000);
      lampActived = false;
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

   writeLcd(t, h, qtdDias, hora, min, sec);
}

void writeLcd(double temp, double hum, int qtdDias, int hora, int min, int sec) {
  char display_hora[20];
  sprintf(display_hora,"%.2d:%.2d:%.2d - Dia%2d",hora,min,sec, qtdDias);

  lcd.clear();

  //dados QTD dias 
  lcd.setCursor(1, 0);
  lcd.print(display_hora);
    
  // dados de temperatura e humidade
  lcd.setCursor(0,2);
  lcd.print("Temp: ");
  lcd.setCursor(6, 1);
  lcd.print(temp);
  lcd.setCursor(12, 1);
  lcd.print("Hum: ");
  lcd.setCursor(17, 1);
  lcd.print(int(hum));
  lcd.setCursor(19, 1);
  lcd.print("%");

  // dados rolagem
  lcd.setCursor(0, 2);
  lcd.print("Ult. Rolagem: ");
  lcd.setCursor(14, 2);
  char display_rolagem[5];
  sprintf(display_rolagem,"%.2d:00",horaUltimaRolagem);
  lcd.print(display_rolagem);

  lcd.setCursor(0, 3);   
  if(rollActived) {
   lcd.print("rolagem ativada");
  } else {
   lcd.print("rolagem desativada");
  }
  
  consoleLogs(temp, hum, hora, min, sec);
}

void consoleLogs(double temp, double hum, int hora, int min, int sec) {
  Serial.printf("%02d:%02d:%02d", hora, min, sec);
  Serial.print(" - ");
     
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" *C ");
}

void acionaRolagem(long hora, long nowTimestamp) {
  horaUltimaRolagem = hora;
  digitalWrite(Rele_motor_DC, LOW);
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
    digitalWrite(Rele_motor_DC, HIGH);
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