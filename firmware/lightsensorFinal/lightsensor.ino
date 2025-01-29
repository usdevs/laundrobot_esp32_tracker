#include <WiFi.h> //Wifi library
#include <HTTPClient.h> // library used for HTTP requests
#include "esp_eap_client.h" //wpa2 library for connections to Enterprise networks
#include <ArduinoJson.h> // Library to convert strings to json objects in arduino

#define EAP_IDENTITY "e" //if connecting from another corporation, use identity@organisation.domain in Eduroam 
#define EAP_USERNAME "e" //oftentimes just a repeat of the identity
#define EAP_PASSWORD "" //your Eduroam password

const int analogInPin0 = 36;  // Analog input pin that the potentiometer is attached to - Sensor 1
const int analogInPin1 = 39; // Sensor 2 // these reefer to the pins on the pcb
const int analogInPin2 = 34; // Sensor 3
const int analogInPin3 = 35; // Sensor 4
const int analogInPin4 = 32; // Sensor 5

int sensorValue0 = 0;  // Washer 1 3.4k - 4k
int sensorValue1 = 0;  // Washer 2 2.1k - 3k
int sensorValue2 = 0;  // Washer 3 1.8k - 4k
int sensorValue3 = 0;  // Washer 4 3.2k - 4k 
int sensorValue4 = 0;  // Washer 5 1.7k - 4k

const char* ssid = "NUS_STU"; // Eduroam SSID
const char* host = "www.google.com"; //external server domain for HTTP connection after authentification
#define ENDPOINT "http://167.71.198.73:3002/washers/update"
int counter = 0;

void setup() {
  pinMode(2, OUTPUT);

  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.print("Connecting to network: ");
  Serial.println(ssid);
  WiFi.disconnect(true);  //disconnect form wifi to set new wifi connection
  WiFi.mode(WIFI_STA); //init wifi mode
  
  // Example1 (most common): a cert-file-free eduroam with PEAP (or TTLS)
  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);

  // Example 2: a cert-file WPA2 Enterprise with PEAP
  //WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD, ca_pem, client_cert, client_key);
  
  // Example 3: TLS with cert-files and no password
  //WiFi.begin(ssid, WPA2_AUTH_TLS, EAP_IDENTITY, NULL, NULL, ca_pem, client_cert, client_key);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
    if(counter>=60){ //after 30 seconds timeout - reset board
      ESP.restart();
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address set: "); 
  Serial.println(WiFi.localIP()); //print LAN IP
}

String httpGET(char *endpoint){
  HTTPClient http;
  String payload = "";
  // Your Domain name with URL path or IP address with path
  http.begin(endpoint);
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
  return payload;
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) { //if we are connected to Eduroam network
    counter = 0; //reset counter
    Serial.println("Wifi is still connected with IP: "); 
    Serial.println(WiFi.localIP());   //inform user about his IP address
  } else if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
    WiFi.begin(ssid);      
  }
  while (WiFi.status() != WL_CONNECTED) { //during lost connection, print dots
    delay(500);
    Serial.print(".");
    counter++;
    if(counter>=60){ //30 seconds timeout - reset board
    ESP.restart();
    }
  }

  Serial.print("Connecting to website: ");
  Serial.println(host);
  WiFiClient client;
  HTTPClient http;

  //syntax - client.connect(ip/URL, port)
  if (client.connect(host, 80)) {
    Serial.println("Connected");

    // read the analog in value:
    sensorValue0 = analogRead(analogInPin0);
    sensorValue1 = analogRead(analogInPin1);
    sensorValue2 = analogRead(analogInPin2);
    sensorValue3 = analogRead(analogInPin3);
    sensorValue4 = analogRead(analogInPin4);
    Serial.print("Voltage at analogInPin0: ");
    Serial.println(sensorValue0);
    Serial.print("Voltage at analogInPin1: ");
    Serial.println(sensorValue1);
    Serial.print("Voltage at analogInPin2: ");
    Serial.println(sensorValue2);
    Serial.print("Voltage at analogInPin3: ");
    Serial.println(sensorValue3);
    Serial.print("Voltage at analogInPin4: ");
    Serial.println(sensorValue4);
    delay(3000);

//washin pause done estimated time
    int sensorValues[4] = {sensorValue0, sensorValue1, sensorValue2, sensorValue3}; //
    int sensorThresholds[4] = {3800, 3650, 5000, 3650}; //figure out circuitry and see whether this works by tseting ldr - does it light up or at least serial print or smth
    // print the results to the Serial Monitor:

    bool inUse[] = {false, false, false, false};

    for (int i = 0; i < 4; i++) {
      if (sensorValues[i] > sensorThresholds[i]) { // VOLTAGe higher than threshold, i.e. indicator OFF --- voltage, not the brightness! // washing phase
        digitalWrite(2, 1);
      } else {
        inUse[i] = true;  // washer is inuse when the sensor values less than threshold, i.e. the LED is  on.
        digitalWrite(2, 0);
      }
    }

    for (int i = 0; i < 4; i++) {
      Serial.print("sensor " + String(i) + "output value: ");
      Serial.print(sensorValues[i]);
      Serial.print(" inUse="+String(inUse[i]));
      Serial.print("\n");
    }

    // wait 2 milliseconds before the next loop for the analog-to-digital
    // converter to settle after the last reading:
    delay(2);

    //Your Domain name with URL path or IP address with path
    http.begin(ENDPOINT);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = -1000000;

    // String payload = httpGET("https://167.71.198.73:3002/seventeenwashers");
    // DynamicJsonDocument doc(1024);
    // deserializeJson(doc, payload);

    // // Extract washer data
    // JsonObject washer1 = doc[0];
    // JsonObject washer2 = doc[1];
    // JsonObject washer3 = doc[2];
    // JsonObject washer4 = doc[3];
    // JsonObject washer5 = doc[4];
    
    //int washingmachine_number = 1;
    char url[] = "";
    char washing_machine[] = "Elm Washer 1";
    httpResponseCode = http.PUT("{\"api_key\":\"" + String(url) + "\", \"name\": \""+String(washing_machine)+ "\", \"timeLeftUserInput\": 30}");
    Serial.println(httpResponseCode);

    if (inUse[0]) { //washer1["timeLeftUserInput"].as<int>() == 0 && inUse[0]) { //if inuse[0] is true // the first indicator, like prewash
      httpResponseCode = http.PUT("{\"api_key\":\"" + String(url) + "\", \"name\": \""+String(washing_machine)+ "\", \"timeLeftUserInput\": 30}");
      Serial.println(httpResponseCode);
      Serial.println("light 1 sent");
    }
    if (inUse[1]) { //washer2["timeLeftUserInput"].as<int>() == 0 && inUse[1]) { //midwash
      httpResponseCode = http.PUT("{\"api_key\":\"" + String(url) + "\", \"name\": \""+String(washing_machine)+ "\", \"timeLeftUserInput\": 20}");
      Serial.println(httpResponseCode);
      Serial.println("light 2 sent");
    }
    if (inUse[2]) { //washer3["timeLeftUserInput"].as<int>() == 0 && inUse[2]) { //end wash
      httpResponseCode = http.PUT("{\"api_key\":\"" + String(url) + "\", \"name\": \""+String(washing_machine)+ "\", \"timeLeftUserInput\": 10}");
      Serial.println(httpResponseCode);
      Serial.println("light 3 sent");
    }
    // if (inUse[3]) { //washer4["timeLeftUserInput"].as<int>()== 0 && inUse[3]) { // lock
    //   httpResponseCode = http.PUT("{\"api_key\":\"" + String(url) + "\", \"name\": \""+String(washing_machine)+ "\", \"timeLeftUserInput\": 0}");
    //   Serial.println("light 4 sent");
    // }
    http.end();
    Serial.println();
  } else{
      Serial.println("Connection unsucessful");
  }
}


