/*  ___   ___  ___  _   _  ___   ___   ____ ___  ____  
 * / _ \ /___)/ _ \| | | |/ _ \ / _ \ / ___) _ \|    \ 
 *| |_| |___ | |_| | |_| | |_| | |_| ( (__| |_| | | | |
 * \___/(___/ \___/ \__  |\___/ \___(_)____)___/|_|_|_|
 *                  (____/ 
 * Remotely control LED with NodeMCU through MQTT IOT broker
 * Tutorial URL http://osoyoo.com/2016/11/25/remotely-control-led-with-nodemcu-through-mqtt-iot-broker/
 * CopyRight John Yu
 */
// mqqt command to publish for LED on/off
// mosquitto_pub -h 192.168.1.9 -t "ledonoff" -m "1"
// mosquitto_pub -h 192.168.1.9 -t "ledonoff" -m "0"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
// Define NodeMCU D3 pin  connect to LED
#define LED_PIN 13

// Update these with values suitable for your network.
const char* ssid = "Kehsi Haw";
const char* password = "1Qaz2Wsx";
 const char* mqtt_server = "192.168.1.9";
//const char* mqtt_server = "iot.eclipse.org";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
   delay(100);
  // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Command from MQTT broker is : [");
  Serial.print(topic);
  int p =(char)payload[0]-'0';
  // if MQTT comes a 0 turn off LED on D2
  if(p==0) 
  {
     digitalWrite(LED_PIN, LOW); 
    Serial.println(" Turn Off LED! " );
  } 
  // if MQTT comes a 1, turn on LED on pin D2
  if(p==1)
  {
     digitalWrite(LED_PIN, HIGH); 
    Serial.println(" Turn On LED! " );
  }
  Serial.println();
} //end callback

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to    if (client.connect(clientId,userName,passWord))
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
     //once connected to MQTT broker, subscribe command if any
      client.subscribe("ledonoff");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 6 seconds before retrying
      delay(6000);
    }
  }
} //end reconnect()

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
    pinMode(LED_PIN, OUTPUT);
 digitalWrite(LED_PIN, LOW); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}
