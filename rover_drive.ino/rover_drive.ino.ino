// ESP8266 Built-in LED Blink Code
#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266WebServer.h>
const char *ssid = "ESP8266 Access Point"; // The name of the Wi-Fi network that will be created
const char *password = "1234";   // The password required to connect to it, leave blank for an open network

ESP8266WebServer server(80);
void handleRoot();              // function prototypes for HTTP handlers
void handleLED();
void handleNotFound();
void setup() {
  WiFi.softAP(ssid, password);
  // Initialize the built-in LED pin as an output
  pinMode(LED_BUILTIN, OUTPUT); 
  pinMode(D0,OUTPUT);
  pinMode(D1,OUTPUT);
  pinMode(D2,OUTPUT);
    pinMode(D5,OUTPUT);
  pinMode(D6,OUTPUT);
  pinMode(D7,OUTPUT);
    digitalWrite(D0,LOW);
  digitalWrite(D1,LOW);
  digitalWrite(D2,LOW); 
      digitalWrite(D5,LOW);
  digitalWrite(D6,LOW);
  digitalWrite(D7,LOW); 
  server.on("/", HTTP_GET, handleRoot);     // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/LED", HTTP_POST, handleLED);  // Call the 'handleLED' function when a POST request is made to URI "/LED"
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();         
}

void loop() {
  server.handleClient();
  // digitalWrite(LED_BUILTIN, LOW);  // Turn the LED on (Active Low)

  // digitalWrite(D0,HIGH);
  // digitalWrite(D1,LOW);
  // digitalWrite(D2,HIGH); 
  // delay(5000);                     // Wait for a second
  // digitalWrite(LED_BUILTIN, HIGH);
  // digitalWrite(D0,HIGH);
  // digitalWrite(D1,HIGH);
  // digitalWrite(D2,LOW);  // Turn the LED off
  // delay(5000);                      // Wait for a second
}
void handleRoot(){
  server.send(200, "text/html", "<form action=\"/LED\" method=\"POST\"><input type=\"submit\" value=\"Toggle MOTOR\"></form>");
}
void handleLED() {                          // If a POST request is made to URI /LED
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));  
      // Change the state of the LED
  digitalWrite(D0,!digitalRead(D0));
  digitalWrite(D1,LOW);
  digitalWrite(D2,HIGH); 
  digitalWrite(D5,!digitalRead(D5));
  digitalWrite(D6,LOW);
  digitalWrite(D7,HIGH); 
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}