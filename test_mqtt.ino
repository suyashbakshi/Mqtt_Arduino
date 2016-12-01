#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <WiFi101.h>

#define DHTTYPE           DHT22
#define DHTPIN            12
#define LEDPIN            13

DHT_Unified dht(DHTPIN, DHTTYPE);

int highT = 0, lowT = 0, curT = 0;

uint32_t delayMS;
char message[100];
char high[10], low[10];
WiFiClient ethClient;
PubSubClient mqttClient(ethClient);
sensors_event_t event;

void callback(char* topic, byte* payload, unsigned int length) {

  //read the sensor reading here

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int i;
  for (i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message[i] = payload[i];
  }

  message[i] = '\0';
  Serial.println();
  String msg = String(message);

  if (msg.equals("Alive?")) {
    mqttClient.publish("status/", "I'm Alive");
  }
  else if (msg.equals("temp")) {
    //return temperature value
    Serial.print("Temperature: ");
    Serial.println(event.temperature);
    //    }
    int t = (int)event.temperature;
    String curTempStr = String(event.temperature);
    char buff[10];
    curTempStr.toCharArray(buff, 10);
    Serial.println(buff);
    mqttClient.publish("status/", buff);
  }
  else {
    // incoming message = "t1:t2"
    int colonIdx = msg.indexOf(":");
    String l = msg.substring(0, colonIdx);
    String h = msg.substring(colonIdx + 1, msg.length());
    Serial.println("HIGH AND LOW: " + h + ":::" + l);
    highT = h.toInt();
    lowT = l.toInt();
    curT = (int)event.temperature;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("status/", "I'm Alive");
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
  WiFi.setPins(8, 7, 4, 2);
  Serial.begin(9600);
  dht.begin();
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN,LOW);
  sensor_t sensor;
  dht.temperature().getEvent(&event);
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);

  WiFi.begin("Tampi", "aman1995");
  delay(10000);

  mqttClient.setServer("ec2-52-91-85-14.compute-1.amazonaws.com", 1883);
  mqttClient.setCallback(callback);

  // Allow the hardware to sort itself out
  delay(1500);
}

void loop()
{
  if (!mqttClient.connected()) {
    reconnect();
  }

  if(curT < highT && curT > lowT){
    digitalWrite(LEDPIN,HIGH);  
  }
  else{
    digitalWrite(LEDPIN,LOW);
  }

  mqttClient.loop();
}
