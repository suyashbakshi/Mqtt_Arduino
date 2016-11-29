#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>
 
char message[100];
int ledPin = 13;
 
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
  String msg = String(message);

  //if message contains request to get temperature, publish the temperature
  //look for temp in message,compare it to what is now and turn on fan for that amount of time
  //::To calculate time, invent a formula for amount of time required to keep the fan running.
  //hint: write a loop since callback will be called only when the message is received(only once)
  
   
  if(msg.equals("on")){
      digitalWrite(ledPin,HIGH);
    }
   else if(msg.equals("off")){
      digitalWrite(ledPin,LOW);
    }
  Serial.println();
}
 
WiFiClient ethClient;
PubSubClient mqttClient(ethClient);
 
void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("status/","hello world");
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
  pinMode(ledPin,OUTPUT);
  
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
