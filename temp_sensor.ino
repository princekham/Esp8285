/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/





#include <FS.h> //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          // https://github.com/bblanchon/ArduinoJson

#include<stdlib.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

#define IP "api.thingspeak.com" // thingspeak.com
String GET = "GET /update?key="; // 


const int buttonPin = 2;    // the number of the pushbutton pin for interrupt
const int ledPin = 13;
// Set web server port number to 80
WiFiServer server(80);
WiFiManager wifiManager;
// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String outputState = "off";

// Assign output variables to GPIO pins
char API[18];
char Interval[4];//default is 3 minutes
//flag for saving data
bool shouldSaveConfig = false;
void Reset (){
Serial.println("interrupt function");
wifiManager.resetSettings();
}
//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup() {
  Serial.begin(115200);
  
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  attachInterrupt(digitalPinToInterrupt(buttonPin), Reset, CHANGE);
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(API, json["API"]); // needs to add more lines if more custom fields
          strcpy(Interval, json["Interval"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
  
  WiFiManagerParameter custom_output("API", "API",API, 20); // id, placeholder, default value, length; needs more lines if more custom fields
  WiFiManagerParameter custom_interval("Interval", "Interval (mins)",Interval ,4);
  
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  //WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //add all your parameters here
  wifiManager.addParameter(&custom_output); // needs more lines if more custom fields
  wifiManager.addParameter(&custom_interval);
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  // fetches  and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("EntreTech");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  
  strcpy(API, custom_output.getValue());
  strcpy(Interval, custom_interval.getValue());
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["API"] = API;
    json["Interval"]=Interval;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  // Initialize the output variables as outputs
//  pinMode(atoi(output), OUTPUT);
//  // Set outputs to LOW
//  digitalWrite(atoi(output), LOW);;
  
  server.begin();
}
void loop(){

  float temp;
  delay (2000);
  DS18B20.begin();
  DS18B20.requestTemperatures();
  temp = DS18B20.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.println(temp);
 
  char charVal[12];
 
  dtostrf(temp, 8, 2, charVal);
 
  Serial.print("connecting to ");
  Serial.println(IP);
 
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(IP, httpPort)) {
    Serial.println("connection failed");
    return;
  }
 
 
 
 
  // We now create a URI for the request
  String url = "/update?key=";
 // url +="R42ISKK935LK27L8";
  url += API;
  
  url += "&field1=";
  url += charVal;//String(temp);
 
 
 
  Serial.print("Requesting URL: ");
  Serial.println(url);
 
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + IP + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);
 
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
 
  Serial.println();
  Serial.println("closing connection");
  
  digitalWrite(ledPin, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(1500);                       // wait for a second
  digitalWrite(ledPin, HIGH);    // turn the LED off by making the voltage LOW
  
  delay((atoi(Interval)*1000*60)-3500);
}
