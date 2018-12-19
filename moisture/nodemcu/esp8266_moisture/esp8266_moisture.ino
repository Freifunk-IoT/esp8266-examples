/*
 Freifunk-IoT-Example-Sketch
 Github: https://github.com/FFS-IoT/
 Needs PubSubClient and ESP8266 Enviroment
 Connects via A0
 by Marvin Gaube <dev@marvingaube.de>
 Input: A0
 You have to change "MQTT_MAX_PACKET_SIZE to at least 512 in your PubSubClient.h (usually under (your arduino folder)\libraries\PubSubClient\src
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>


// Insert your sensorId here
char sensorId[]="replaceId";
// Insert here where your moisture sensor is connected
int sensorPin = A0;

// Raw value when sensor is completly dry
int calibration_dry = 821;
// Raw value when value is completly wet
int calibration_wet = 360;

// Config for FFS-IoT

const char* ssid = "Freifunk";
// if you use an open SSID
const char* password = NULL;
// if you use a protected SSID
// const char* password = "yourwifipassword";

// MQTT-Server of your ffiot-instance
const char* mqtt_server = "demo.freifunk-iot.de";

WiFiClient espClient;
PubSubClient client(espClient);

char msg[512];
char str_raw[10];
int raw_moisture = 0;
char str_rel[10];
int relative_moisture = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
   setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {

  delay(10);
  // Connect with wifi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
 //only Station, no AP
  WiFi.mode(WIFI_STA);
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
  
  digitalWrite(LED_BUILTIN, LOW);
  //Read out sensor
    raw_moisture = analogRead(sensorPin);
    relative_moisture = map(raw_moisture, calibration_dry, calibration_wet, 0, 100);
    //Make Strings
    /* 4 is mininum width, 2 is precision; float value is copied onto str_raw*/
    dtostrf(raw_moisture, 4, 2, str_raw);
    dtostrf(relative_moisture, 4, 2, str_rel);
    
    //Handle sensor errors
    if (isnan(raw_moisture)) {
      Serial.println("Failed to read from sensor!");
      snprintf(msg, sizeof(msg), "{\"%s._error\":2}",sensorId);
    } else {
      //Make JSON-message
      snprintf(msg, sizeof(msg), "{\"%s.raw_moisture\":%s, \"%s.relative_moisture\":%s, \"%s._error\":0}",sensorId,str_raw,sensorId,str_rel,sensorId);
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
    delay(5000);
}
