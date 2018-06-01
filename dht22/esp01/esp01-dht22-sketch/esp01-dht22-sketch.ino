/*
 Freifunk-IoT-Example-Sketch
 Github: https://github.com/FFS-IoT/

 Needs Adafruit-Sensor and DHT-Libary, PubSubClient and ESP8266 Enviroment
 by Marvin Gaube <dev@marvingaube.de>
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>


//Hier deine Sensor-ID einsetzen:
char sensorId[]="ReplaceId";
//Hier deinen Sensor-Pin und Typ anpassen anpassen:
#define DHTPIN            2 
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Config for FFS-IoT

const char* ssid = "Freifunk";
const char* password = "";
const char* mqtt_server = "freifunk-iot.de";

WiFiClient espClient;
PubSubClient client(espClient);

char msg[200];

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  dht.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
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
  digitalWrite(LED_BUILTIN, LOW);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();;
    int errorCode = 0;
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      errorCode=1;
      temperature=1;
      humidity=1;
    }
    //Make JSON-message
    snprintf(msg, sizeof(msg), "{\"%s.temperature\":%.3f, \"%s.humidity\":%.3f, \"%s.errorCode\":%i}",sensorId,temperature,sensorId,humidity,sensorId,errorCode);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("iot_input", msg);
    //Wait for transmit
    delay(2000);
    digitalWrite(LED_BUILTIN, HIGH);
    
    //If you have the "Deep Sleep mod" uncomment
    //ESP.deepSleep(30e6);
    //else normal delay 30s
    delay(30000);
}
