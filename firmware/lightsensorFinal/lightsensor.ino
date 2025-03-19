#include <WiFi.h> //Wifi library
#include <HTTPClient.h> // library used for HTTP requests
#include "esp_eap_client.h" //wpa2 library for connections to Enterprise networks
#include <ArduinoJson.h> // Library to convert strings to json objects in arduino
#include <LiquidCrystal_I2C.h>

#include  <Wire.h>


#define EAP_IDENTITY "e0702014" //if connecting from another corporation, use identity@organisation.domain in Eduroam 
#define EAP_USERNAME "e0702014" //oftentimes just a repeat of the identity
#define EAP_PASSWORD "Rvhs3103YTPS1762!!!!" //your Eduroam password

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

LiquidCrystal_I2C lcd(0x27,  16, 2);

const char* ssid = "NUS_STU"; // Eduroam SSID
const char* host = "www.google.com"; //external server domain for HTTP connection after authentification
#define ENDPOINT "http://167.71.198.73:3002/washers/update"
#define ENDPOINTD "http://167.71.198.73:3002/dryers/update"
int counter = 0;


String machine_name(int index) {
  if (index == 0) {
    return "Elm Washer 1";
  }  else if (index == 1) {
    return "Elm Washer 2";
  }  else if (index == 2) {
    return "Elm Washer 3";
  }  else if (index == 3) {
    return "Elm Washer 4";
  }  else if (index == 4) {
    return "Elm Washer 5";
  }  else if (index == 5) {
    return "Elm Washer 6";
  }  else if (index == 6) {
    return "Elm Dryer 1";
  }  else if (index == 7) {
    return "Elm Dryer 2";
  }  else if (index == 8) {
    return "Elm Dryer 3";
  }
    else if (index == 9) {
    return "Elm Dryer 4";
    }
    else if (index == 10) {
    return "Elm Dryer 5";
    }
    else if (index == 11) {
    return "Elm Dryer 6";
  }  
}

void setup() {
    //initialize lcd screen
  lcd.init();
  // turn on the backlight
  lcd.backlight();
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

  int no_machines = 12;
  char url[] = "";
  int curr_machine = 0;
  int prevstate0 = 0;
  int prevstate1 = 0;

  pinMode(36, INPUT_PULLUP);
  pinMode(39, INPUT_PULLUP);
  while (1) {

    // tell the screen to write on the top row
    lcd.setCursor(0,0);
    // tell the screen to write “hello, from” on the top  row
    lcd.print(machine_name(curr_machine)+" ");
    lcd.setCursor(0,1);
    lcd.print("Button 1: Start Time");
    lcd.setCursor(0,2);
    lcd.print("Button 2: Next");

    if (digitalRead(36) == 0 && prevstate0 == 1) { // when button is pressed down it is 0
      curr_machine = (curr_machine + 1) % no_machines;
    }
    if (digitalRead(39) == 0 && prevstate1 == 1) {
      
    HTTPClient http;
    if (curr_machine >=6 ){
      http.begin(ENDPOINTD);
    } else {
      http.begin(ENDPOINT);
    }
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = -1000000;
      httpResponseCode = http.PUT("{\"api_key\":\"" + String(url) + "\", \"name\": \""+machine_name(curr_machine)+ "\", \"timeLeftUserInput\": 45}");
      Serial.println(httpResponseCode);
      Serial.println("light 1 sent");
      lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("Set!");
       delay(1000);
    }
    prevstate0 = digitalRead(36);
    prevstate1 = digitalRead(39);
    // Serial.println(digitalRead(36));
    // Serial.println(digitalRead(39));
  }
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

 //just send 45 minutes
void loop() {
//   delay(1000);
//   // tell the screen to write on the top row

//   if (WiFi.status() == WL_CONNECTED) { //if we are connected to Eduroam network
//     counter = 0; //reset counter
//     Serial.println("Wifi is still connected with IP: "); 
//     Serial.println(WiFi.localIP());   //inform user about his IP address
//   } else if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
//     WiFi.begin(ssid);      
//   }
//   while (WiFi.status() != WL_CONNECTED) { //during lost connection, print dots
//     delay(500);
//     Serial.print(".");
//     counter++;
//     if(counter>=60){ //30 seconds timeout - reset board
//     ESP.restart();
//     }
//   }

//   Serial.print("Connecting to website: ");
//   Serial.println(host);
//   WiFiClient client;
//   HTTPClient http;

//   //syntax - client.connect(ip/URL, port)
//   if (client.connect(host, 80)) {
//     Serial.println("Connected");

//     // read the analog in value:
//     sensorValue0 = analogRead(analogInPin0);
//     sensorValue1 = analogRead(analogInPin1);
//     sensorValue2 = analogRead(analogInPin2);
//     sensorValue3 = analogRead(analogInPin3);
//     sensorValue4 = analogRead(analogInPin4);
//     // Serial.print("Voltage at analogInPin0: ");
//     // Serial.println(sensorValue0);
//     // Serial.print("Voltage at analogInPin1: ");
//     // Serial.println(sensorValue1);
//     // Serial.print("Voltage at analogInPin2: ");
//     // Serial.println(sensorValue2);
//     // Serial.print("Voltage at analogInPin3: ");
//     // Serial.println(sensorValue3);
//     // Serial.print("Voltage at analogInPin4: ");
//     // Serial.println(sensorValue4);
//     delay(3000);

// //washin pause done estimated time
//     // int sensorValues[4] = {sensorValue0, sensorValue1, sensorValue2, sensorValue3}; //
//     // int sensorThresholds[4] = {3800, 3650, 5000, 3650}; //figure out circuitry and see whether this works by tseting ldr - does it light up or at least serial print or smth
//     // // print the results to the Serial Monitor:

//     // bool inUse[] = {false, false, false, false};

//     // for (int i = 0; i < 4; i++) {
//     //   if (sensorValues[i] > sensorThresholds[i]) { // VOLTAGe higher than threshold, i.e. indicator OFF --- voltage, not the brightness! // washing phase
//     //     digitalWrite(2, 1);
//     //   } else {
//     //     inUse[i] = true;  // washer is inuse when the sensor values less than threshold, i.e. the LED is  on.
//     //     digitalWrite(2, 0);
//     //   }
//     // }

//     // for (int i = 0; i < 4; i++) {
//     //   Serial.print("sensor " + String(i) + "output value: ");
//     //   Serial.print(sensorValues[i]);
//     //   Serial.print(" inUse="+String(inUse[i]));
//     //   Serial.print("\n");
//     // }

//     // wait 2 milliseconds before the next loop for the analog-to-digital
//     // converter to settle after the last reading:
//   //   delay(2);

//   //   //Your Domain name with URL path or IP address with path
//   //   http.begin(ENDPOINT);
//   //   http.addHeader("Content-Type", "application/json");
//   //   int httpResponseCode = -1000000;

//   //   // String payload = httpGET("https://167.71.198.73:3002/seventeenwashers");
//   //   // DynamicJsonDocument doc(1024);
//   //   // deserializeJson(doc, payload);

//   //   // // Extract washer data
//   //   // JsonObject washer1 = doc[0];
  //   // JsonObject washer2 = doc[1];
  //   // JsonObject washer3 = doc[2];
  //   // JsonObject washer4 = doc[3];
  //   // JsonObject washer5 = doc[4];
    
  //   //int washingmachine_number = 1;
  //   char url[] = "";
  //   char washing_machine[] = "Elm Washer 1";
  //   httpResponseCode = http.PUT("{\"api_key\":\"" + String(url) + "\", \"name\": \""+String(washing_machine)+ "\", \"timeLeftUserInput\": 45}");
  //   Serial.println(httpResponseCode);

  //   if (inUse[0]) { //washer1["timeLeftUserInput"].as<int>() == 0 && inUse[0]) { //if inuse[0] is true // the first indicator, like prewash
  //     httpResponseCode = http.PUT("{\"api_key\":\"" + String(url) + "\", \"name\": \""+String(washing_machine)+ "\", \"timeLeftUserInput\": 30}");
  //     Serial.println(httpResponseCode);
  //     Serial.println("light 1 sent");
  //   }
  //   if (inUse[1]) { //washer2["timeLeftUserInput"].as<int>() == 0 && inUse[1]) { //midwash
  //     httpResponseCode = http.PUT("{\"api_key\":\"" + String(url) + "\", \"name\": \""+String(washing_machine)+ "\", \"timeLeftUserInput\": 20}");
  //     Serial.println(httpResponseCode);
  //     Serial.println("light 2 sent");
  //   }
  //   if (inUse[2]) { //washer3["timeLeftUserInput"].as<int>() == 0 && inUse[2]) { //end wash
  //     httpResponseCode = http.PUT("{\"api_key\":\"" + String(url) + "\", \"name\": \""+String(washing_machine)+ "\", \"timeLeftUserInput\": 10}");
  //     Serial.println(httpResponseCode);
  //     Serial.println("light 3 sent");
  //   }
  //   // if (inUse[3]) { //washer4["timeLeftUserInput"].as<int>()== 0 && inUse[3]) { // lock
  //   //   httpResponseCode = http.PUT("{\"api_key\":\"" + String(url) + "\", \"name\": \""+String(washing_machine)+ "\", \"timeLeftUserInput\": 0}");
  //   //   Serial.println("light 4 sent");
  //   // }
  //   http.end();
  //   Serial.println();
  // } else{
  //     Serial.println("Connection unsucessful");
  // }
}

