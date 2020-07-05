#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define LED_INTERVAL 2000
#define PIN_LED  2
#define PIN_D8   15

const char* ssid = "GFIOTI";
const char* password = "K5nwsV7UVaRcPcgW";
ESP8266WebServer server(80);

String uptime()
{
 long days=0;
 long hours=0;
 long mins=0;
 long secs=0;
 String ut;
 
 secs = millis()/1000; //convect milliseconds to seconds
 mins=secs/60; //convert seconds to minutes
 hours=mins/60; //convert minutes to hours
 days=hours/24; //convert hours to days
 secs=secs-(mins*60); //subtract the coverted seconds to minutes in order to display 59 secs max
 mins=mins-(hours*60); //subtract the coverted minutes to hours in order to display 59 minutes max
 hours=hours-(days*24); //subtract the coverted hours to days in order to display 23 hours max
 
 // Return results
 if (days>0) // days will displayed only if value is greater than zero
 {
   ut += days;
   ut += " days, ";
 }
 ut += hours;
 ut += " hours, ";
 ut += mins;
 ut += " mins, ";
 ut += secs;
 ut += " seconds.";

 return ut;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

   // Initialize D8 as input pin
  pinMode(PIN_D8, INPUT);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  server.on("/", [](){

   // Buffer to store long as a char
   char buf[16];
      
    String resp;
    resp = "<html><head><title>Garage Opener</title></head>";
    resp += "<body>";
    resp += "<h1>Garage Door Opener</h1>";

    // Status table
    resp += "<h3>Status</h3>";
    resp += "<table><tr>";
    resp += "<td><b>Door is currently</b></td>";
    resp += "</tr><tr>";
    resp += "<td><b>Door has been operated</b></td>";
    resp += "<td>";
    resp += buf;
    resp += " times</td></tr>";
    resp += "<tr><td><b>System Uptime</b></td><td>";
    resp += uptime();
    resp += "</td></tr>";
    resp += "</table>";

    resp += "<h3>Commands</h3>";
    resp += "<a href='/door?state=close'>Close</a>";
    resp += "<br /><a href='/door?state=open'>Open</a>";
    resp += "<br /><a href='/door?state=query'>Query</a>";
    resp += "<br /><a href='/door?state=setopen'>Set Open</a>";
    resp += "<br /><a href='/door?state=setclosed'>Set Closed</a>";

    // Counter update form
    resp += "<h3>Update Door Counter</h3>";
    resp += "<form method=GET action='/door'>";
    resp += "<input type=hidden name=state value=setcounter>";
    resp += "<input type=input name=counter value='";
    resp += buf;
    resp += "'>";
    resp += "<input type=submit value='Update'>";
    resp += "</form>";
   
    resp += "</body></html>";
    server.send(200, "text/html", resp);
  });

  server.on("/api", [](){
    String reqstate = server.arg("plain");
    String txtstate;
    if (reqstate == "toggle") {
    }
    server.send(200, "application/json", "{\"closed\": \"" + txtstate + "\",\"req\": \"" + reqstate + "\"}");
  });
  
  server.on("/door", [](){
    String state=server.arg("state");
    if (state == "close") {

    }
    else if (state == "open") {

    }
    else if (state == "setcounter") {
      server.send(200, "text/plain", "OK. Counter set to "+server.arg("counter"));
    }
    else if (state == "query") {

      // Buffer to store long as a char
      String resp;
      char buf[16];
      
      resp += buf;
      server.send(200, "text/plain", resp);
    }
    else {
      server.send(404, "text/plain", "The state that you specified, " + state + " is not valid. Try one of open or close");    
    }
  });

  ArduinoOTA.begin();
  server.begin();
  
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  // Determine Reed Switch status
  if (digitalRead(PIN_D8) == HIGH) {
    Serial.println("Turn On");
  } else {
    Serial.println("Turn Off");
  }

  delay(2000);
}
