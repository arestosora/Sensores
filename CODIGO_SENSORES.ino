#include <FirebaseESP32.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Set your Wi-Fi credentials
#define WIFI_SSID "Network"
#define WIFI_PASSWORD "2023unicor"

// Set your Firebase project's API Key and Firestore URL
#define API_KEY "AIzaSyCIZmSGZVMMG7jO-MAG9vJvAFZofx0Tq7E"
#define DATABASE_URL "https://piralarm-df5d9-default-rtdb.firebaseio.com"

const int PIN_TO_SENSOR = 19; // GIOP19 pin connected to OUTPUT pin of sensor
const int alarmPin = 5;
int pinStateCurrent   = LOW;  // current state of pin
int pinStatePrevious  = LOW;  // previous state of pin

FirebaseData firebaseData;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setup() {
  Serial.begin(9600);     
  pinMode(PIN_TO_SENSOR, INPUT);
  pinMode(alarmPin, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Wi-Fi connection established");

  // Initialize Firebase
  Firebase.begin(DATABASE_URL, API_KEY);

    // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone
  // For example: GMT +1 = 3600, GMT +8 = 28800, GMT -1 = -3600, GMT 0 =
}

void loop() {
  timeClient.update();
  Firebase.getBool(firebaseData, "/sensores/sensor_1/state");
  bool sensorState = firebaseData.boolData();

  unsigned long timestamp = timeClient.getEpochTime() ;
 
  pinStatePrevious = pinStateCurrent; // store old state
  pinStateCurrent = digitalRead(PIN_TO_SENSOR);   // read new state

  if (pinStatePrevious == LOW && pinStateCurrent == HIGH && sensorState==true) {   // pin state change: LOW -> HIGH
    String sensorPath = "/sensores/sensor_1/reg/" + String(timestamp)+"000";
    Serial.println("Pir Active");
    Firebase.setBool(firebaseData, sensorPath + "/alarm_state", true);
    Firebase.setTimestamp(firebaseData, sensorPath + "/date");
    Firebase.setBool(firebaseData, "/alarm/state", true);
    Firebase.setBool(firebaseData, "/sensores/sensor_1/alarm", true);
    delay(5000);
    Firebase.setBool(firebaseData, "/alarm/state", false);
    Firebase.setBool(firebaseData, "/sensores/sensor_1/alarm", false);
  }
  else
  if (pinStatePrevious == HIGH && pinStateCurrent == LOW && sensorState==true)  {
    Serial.println("Pir Inactive");
    Firebase.setBool(firebaseData, "/alarm/state", false);
    digitalWrite(alarmPin, LOW);
  }
  delay(1000);
}
