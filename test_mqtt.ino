#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <PubSubClient.h>

#define DHTTYPE           DHT22
#define DHTPIN            12
#include <WiFi101.h>

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;
char message[100];
WiFiClient ethClient;
PubSubClient mqttClient(ethClient);
 
void callback(char* topic, byte* payload, unsigned int length) {

  //read the sensor reading here
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int i;
  for (i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    message[i]=payload[i];
  }
  
  message[i]='\0';
  Serial.println();
  String msg = String(message);

  if(msg.equals("Alive?")){
    mqttClient.publish("status/","I'm Alive");
  }
  else if(msg.equals("temp")){
    //return temperature value
    sensors_event_t event;  
    dht.temperature().getEvent(&event);
//    if (isnan(event.temperature)) {
//      Serial.println("Error reading temperature!");
//    }
//    else {
     Serial.print("Temperature: ");
     Serial.println(event.temperature);
//    }  
     int t = (int)event.temperature;
     String curTempStr = String(event.temperature);
     char buff[10];
     curTempStr.toCharArray(buff,10);
     Serial.println(buff);
     mqttClient.publish("status/",buff);
  }
  else{
    String reqTempStr = msg.substring(5);
    int reqTemp = reqTempStr.toInt();
    //Serial.println(reqTemp);
    Serial.println("REQUIRED TEMPERATURE: " + reqTempStr);
    delay(reqTemp*1000);
    

  }
  
  //look for temp in message,compare it to what is now and turn on fan for that amount of time
  //::To calculate time, invent a formula for amount of time required to keep the fan running.
  //hint: write a loop to keep fan running since callback will be called only when the message is received(only once)
    
}
 
void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("status/","I'm Alive");
      // ... and resubscribe
      mqttClient.subscribe("control/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 
void setup()
{
  WiFi.setPins(8,7,4,2);
  Serial.begin(9600);
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  
  WiFi.begin("UHWireless");
  delay(10000);
 
  mqttClient.setServer("ec2-54-165-84-13.compute-1.amazonaws.com", 1883);
  mqttClient.setCallback(callback);
 
  // Allow the hardware to sort itself out
  delay(1500);
}
 
void loop()
{
  if (!mqttClient.connected()) {
    reconnect();
  }
//  mqttClient.publish("status/","Alive");
//  delay(10000);
  mqttClient.loop();
}
