/*
 Freifunk-IoT-Example-Sketch
 Github: https://github.com/FFS-IoT/

 Needs Adafruit-Sensor and BME280-Libary, PubSubClient and ESP8266 Enviroment
 Connects via I2C
 by Marvin Gaube <dev@marvingaube.de>

 SDA: GPIO0 
 SCL: GPIO2

 You have to change "MQTT_MAX_PACKET_SIZE to at least 512 in your PubSubClient.h (usually under (your arduino folder)\libraries\PubSubClient\src
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


//Hier deine Sensor-ID einsetzen:
char sensorId[]="ReplaceId";
//Hier deinen Sensor-Pin und Typ anpassen anpassen:
Adafruit_BME280 bme; // BME280 via I2C

// Config for FFS-IoT

const char* ssid = "Freifunk";
const char* password = "";
const char* mqtt_server = "mqtt.stg.freifunk-iot.de";

WiFiClient espClient;
PubSubClient client(espClient);

char msg[512];
char str_temp[10];
char str_humi[10];
char str_press[10];
float temperature = 0;
float humidity = 0;
float pressure = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Wire.pins(0, 2);           
  Wire.begin();  
   setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {

  delay(10);
  // Verbindung zu Freifunk aufbauen
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Reconnect
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  //Reconnect MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //Try to reach sensor, address 0x76 (maybe change to 0x77)
  while (! bme.begin(0x76)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        snprintf(msg, sizeof(msg), "{\"%s._error\":1}",sensorId);
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish("iot_input", msg);
        //Wait for transmit
        delay(200);
  }
  digitalWrite(LED_BUILTIN, LOW);
  //Read out sensor
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure()/100; //in hPa
    //Make Strings
    /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
    dtostrf(temperature, 4, 2, str_temp);
    dtostrf(humidity, 4, 2, str_humi);
    dtostrf(pressure, 4, 2, str_press);
    
    //Handle sensor errors
    if (isnan(humidity) || isnan(temperature) || isnan(pressure)) {
      Serial.println("Failed to read from DHT sensor!");
      snprintf(msg, sizeof(msg), "{\"%s._error\":2}",sensorId);
    } else {
      //Make JSON-message
      snprintf(msg, sizeof(msg), "{\"%s.temperature\":%s, \"%s.humidity\":%s, \"%s.pressure\":%s, \"%s._error\":0}",sensorId,str_temp,sensorId,str_humi,sensorId,str_press,sensorId);
    }
    Serial.print("Publish message: ");
    Serial.println(msg);
    if(client.publish("iot_input", msg, strlen(msg))) {  
    } else {
      //Handle transmit errors as far as possible (Buffer Overflows)
      snprintf(msg, sizeof(msg), "{\"%s._error\":100,\"%s._debug\":\"MQTT Error\"}",sensorId, sensorId);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("iot_input", msg);
    }
    //Wait for transmit
    client.loop();
    delay(2000);
    digitalWrite(LED_BUILTIN, HIGH);
    
    //If you have the "Deep Sleep mod" uncomment
    //ESP.deepSleep(30e6);
    //else normal delay 30s
    delay(30000);
}
