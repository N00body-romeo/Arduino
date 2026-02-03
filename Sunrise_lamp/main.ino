#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ! CONFIGURAZIONE UTENTE
const char *ssid     = "";
const char *password = "";

// Orario Sveglia
const int ALARM_HOUR   = 7;  
const int ALARM_MINUTE = 30; 

// CONFIGURAZIONE TEMPI
const int SUNRISE_DURATION_MIN = 30; // 1. Alba inizia 30 min PRIMA
const int DAYLIGHT_DURATION_MIN = 30; // 2. Luce resta accesa 30 min DOPO lo stop

const long UTC_OFFSET = 3600; 

// --- PIN ---
const int PIN_RED    = 13; 
const int PIN_GREEN  = 12; 
const int PIN_BLUE   = 14; 
const int PIN_BUZZ   = 5;  
const int PIN_BUTTON = 4;  

// --- STATI DEL SISTEMA ---
enum SystemState {
  IDLE,       // Tutto spento, aspetta l'ora
  SUNRISE,    // Alba in corso (luce sale)
  ALARM,      // Ore X: Suono attivo + Luce 100%
  DAYLIGHT    // Utente ha fermato suono, Luce resta 100% (timer auto-off)
};

SystemState currentState = IDLE;

// Variabili
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", UTC_OFFSET);

unsigned long stateStartTime = 0; // Per calcolare i timer
unsigned long lastLoopTime = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_BUZZ, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  
  turnOffEverything(); // Reset iniziale

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nConnesso!");
  timeClient.begin();
}

void loop() {
  timeClient.update();

  // --- LOGICA PULSANTE ---
  if (digitalRead(PIN_BUTTON) == LOW) {
    handleButtonPress();
    // Debounce: aspetta che lasci il dito
    while(digitalRead(PIN_BUTTON) == LOW) delay(50);
    delay(200); 
  }

  // --- GESTIONE TEMPO E STATI ---
  if (millis() - lastLoopTime > 1000) {
    lastLoopTime = millis();
    checkTimeSchedule();
    
    // Debug stato
    Serial.print(timeClient.getFormattedTime());
    Serial.print(" | Stato: ");
    if(currentState == IDLE) Serial.println("IDLE");
    else if(currentState == SUNRISE) Serial.println("ALBA IN CORSO");
    else if(currentState == ALARM) Serial.println("DRIIIN!");
    else if(currentState == DAYLIGHT) Serial.println("LUCE GIORNO (Auto-Off attivo)");
  }

  // Esecuzione effetti continui
  runStateEffects();
}

void checkTimeSchedule() {
  int currentH = timeClient.getHours();
  int currentM = timeClient.getMinutes();
  int currentS = timeClient.getSeconds();
  
  int currentTotalMin = (currentH * 60) + currentM;
  int alarmTotalMin   = (ALARM_HOUR * 60) + ALARM_MINUTE;

  // 1. START ALBA (30 min prima)
  if (currentState == IDLE) {
    if (currentTotalMin == (alarmTotalMin - SUNRISE_DURATION_MIN) && currentS == 0) {
      changeState(SUNRISE);
    }
  }

  // 2. START SVEGLIA (Ora X)
  if (currentState == SUNRISE || currentState == IDLE) {
    if (currentTotalMin == alarmTotalMin && currentS == 0) {
      changeState(ALARM);
    }
  }
  
  // 3. AUTO-STOP GIORNO (30 min dopo aver fermato la sveglia)
  if (currentState == DAYLIGHT) {
    unsigned long elapsedMillis = millis() - stateStartTime;
    // Se sono passati i minuti di "Daylight", spegni tutto
    if (elapsedMillis > (DAYLIGHT_DURATION_MIN * 60 * 1000UL)) {
      Serial.println("Tempo Daylight scaduto -> Spegnimento automatico.");
      changeState(IDLE);
    }
  }
}

void runStateEffects() {
  switch (currentState) {
    case SUNRISE:
      updateSunriseColor();
      break;
      
    case ALARM:
      // Luce al massimo + Suono
      analogWrite(PIN_RED, 1023);
      analogWrite(PIN_GREEN, 1023);
      analogWrite(PIN_BLUE, 1023);
      playAlarmSound();
      break;
      
    case DAYLIGHT:
      // Solo Luce fissa al massimo (Niente suono)
      analogWrite(PIN_RED, 1023);
      analogWrite(PIN_GREEN, 1023);
      analogWrite(PIN_BLUE, 1023);
      noTone(PIN_BUZZ);
      break;
      
    case IDLE:
      // Assicurati che sia tutto spento
      
      break;
  }
}

void handleButtonPress() {
  switch (currentState) {
    case IDLE:

      break;
      
    case SUNRISE:
      // Se ti svegli durante l'alba e premi -> Spegni tutto (vuoi dormire ancora)
      Serial.println("Alba interrotta.");
      changeState(IDLE);
      break;
      
    case ALARM:
      // Se suona e premi -> FERMA SUONO, MA TIENI LUCE (Passa a Daylight)
      Serial.println("Suono fermato. Luce attiva per 30 min.");
      changeState(DAYLIGHT);
      break;
      
    case DAYLIGHT:
      // Se c'è la luce fissa e premi -> SPEGNI TUTTO (Esci di casa)
      Serial.println("Spegnimento manuale.");
      changeState(IDLE);
      break;
  }
}

void changeState(SystemState newState) {
  currentState = newState;
  stateStartTime = millis(); // Resetta il timer per il nuovo stato
  
  if (newState == IDLE) {
    turnOffEverything();
  }
}

void updateSunriseColor() {
  unsigned long elapsedTime = millis() - stateStartTime;
  unsigned long durationMillis = SUNRISE_DURATION_MIN * 60 * 1000UL;
  float progress = (float)elapsedTime / (float)durationMillis;
  if (progress > 1.0) progress = 1.0;

  int valRed = (int)(progress * 1023);
  int valGreen = 0;
  if (progress > 0.3) valGreen = map((long)((progress - 0.3) * 1000), 0, 700, 0, 1023);
  int valBlue = 0;
  if (progress > 0.7) valBlue = map((long)((progress - 0.7) * 1000), 0, 300, 0, 1023);

  analogWrite(PIN_RED, valRed);
  analogWrite(PIN_GREEN, valGreen);
  analogWrite(PIN_BLUE, valBlue);
}

void playAlarmSound() {
  unsigned long currentMillis = millis();
  if (currentMillis % 1000 < 500) tone(PIN_BUZZ, 2000);
  else noTone(PIN_BUZZ);
}

void turnOffEverything() {
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE, 0);
  noTone(PIN_BUZZ);
}
