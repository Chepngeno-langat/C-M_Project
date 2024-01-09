#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"
#include <MHZ19.h>

#include <time.h>
#define TIME_ZONE +3

#define MQ4_PIN A4 // Analog pin for MQ-2 sensor
//#define LED_PIN 13 // GPIO pin for the LED
#define METHANE_THRESHOLD 2000

MHZ19 co2Sensor;

// define an esp32:pub/sub topic
#define AWS_IOT_PUBLISH_TOPIC "test/demo"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
int carbon_levels;
int methane_levels;
int methane_concentration;
unsigned long lastMillis = 0;
unsigned long previousMillis = 0;
const long interval = 5 * 60 * 1000;
 
time_t now;
time_t nowish = 1510592825;


void NTPConnect(void)
{
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0 * 3600, "time.google.com", "time.apple.com");
  now = time(nullptr);
  while (now < nowish)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}


// define a function protoype for the message handler
void messageHandler(char* topic, byte* payload, unsigned int length);

void connectAWS(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to WiFi...");
  while(WiFi.status() !=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  NTPConnect();

  // Configure WiFiClientSecure to use AWS IoT device credentials
  net.setCACert(AWS_IOT_ROOT_CA_1);
  net.setCertificate(AWS_IOT_CERTIFICATE);
  net.setPrivateKey(AWS_IOT_PRIVATE_KEY);

  // establish a connection to the MQTT broker on the AWS endpoint.
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  //create a message hander
  client.setCallback(messageHandler);
  Serial.println("Connecting to AWS IoT ...");

  while(!client.connect(THINGNAME)){
    Serial.print(".");
    delay(50);
  } if (!client.connected()){
    Serial.println("AWS IoT connection failed");
    return;
  }
  // subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT connection established");
}

void publishMessage(){
  StaticJsonDocument<200> doc; // create a JSON object, of capacity 200 bytes (the default) 
  doc["carbon_concentration"] = carbon_levels;
  doc["methane_concentration"] = methane_concentration;
  doc["sensor_id"] = 4;
  
  char timestamp[20]; // Buffer to hold the formatted timestamp
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
  doc["timestamp"] = timestamp;

  
  char jsonBuffer[512]; // create a buffer to hold the JSON object
  serializeJson(doc, jsonBuffer); // serialize the JSON object to the buffer;
                                  // prints to client
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);                                  
}

void messageHandler(char* topic, byte* payload, unsigned int length){
  Serial.println("Incoming message: ");
  Serial.println(topic);
  
  StaticJsonDocument<200> doc; // create a JSON object
  deserializeJson(doc, payload); // deserialize the JSON object from the buffer
  const char* message = doc["message"]; // get the message from the JSON object
  Serial.println(message);
}

void setup(){
  Serial.begin(9600);
  connectAWS();
  Serial2.begin(9600);  // Start Serial2 for ESP32 (TX: GPIO17, RX: GPIO16)
  co2Sensor.begin(Serial2);

//  pinMode(LED_PIN, OUTPUT);
  Serial.println("Setup complete");
}

void loop(){
  carbon_levels = co2Sensor.getCO2();
  methane_levels = analogRead(MQ4_PIN);
  
  if (carbon_levels > 0) {
    Serial.print("CO2 Level: ");
    Serial.print(carbon_levels);
    Serial.println(" ppm");
    Serial.print("\t");
  } else {
    Serial.println("Error reading CO2 data. Check your connections.");
  }

  if (methane_levels > METHANE_THRESHOLD) {
    methane_concentration = methane_levels - METHANE_THRESHOLD;
    Serial.print("MQ4 Sensor: ");  
    Serial.print(methane_concentration);   /*Read value printed*/
    Serial.println(" ppm");
    Serial.print("\t");
    }
  else {
    methane_concentration = 0;
    Serial.print("MQ4 Sensor: ");  
    Serial.print(methane_concentration);   /*Read value printed*/
    Serial.println(" ppm");
    Serial.print("\t");
    }

  delay(2000); 

  now = time(nullptr);
 
  if (!client.connected())
  {
    connectAWS();
  }
  else
  {
    client.loop();
    if (millis() - lastMillis >= interval)
    {
      lastMillis = millis();
      publishMessage();
    }
  }
}
