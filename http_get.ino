#include <WiFi.h>
#include <HTTPClient.h>
 
//Conexión a Internet
const char* ssid = "Totalplay Casa_2.4Gnormal"; //Nombre de su internet
const char* password = "36830440Hola"; //Contraseña de su internet

void setup() {
 
  Serial.begin(9600);
  delay(4000);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
 
}

void loop() {
 
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
 
    HTTPClient http;
 
    http.begin("https://e3vb3nsln2.execute-api.us-east-1.amazonaws.com/test/"); //Specify the URL

    int httpCode = http.GET();                                        //Make the request
 
    if (httpCode > 0) { //Check for the returning code
 
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
      }
 
    else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources
  }
 
  delay(10000);
 
}
