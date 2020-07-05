#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// If not enabled, all EPROM counters will show zero
#define EEPROM_ENABLE    0
#define EEPROM_DOORSTATE 0
#define EEPROM_COUNTLOW  1
#define EEPROM_COUNTHIGH 2

// If enabled, test mode will be invoked, which will turn on and off the relay every 2 seconds
#define TEST_MODE   0

#define LED_INTERVAL 2000
#define PIN_LED  2
#define PIN_D1   5
#define PIN_D2   4
#define PIN_REED 0

const char* ssid = "GFIOTI";
const char* password = "";
ESP8266WebServer server(80);

// These values are stored in EEPROM (actually flash on ESP8266)
byte status;
byte reedswitch;
long counter;

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

void door_toggle() {
  digitalWrite(PIN_D2, HIGH);
  delay(6000);
  digitalWrite(PIN_D2, LOW);
}

void door_close() {
  // Blink the LED
  digitalWrite(PIN_LED, LOW);

  // Update Status
  status = 0;

  // Trigger Relay
  door_toggle();

  // Increment counter and write status to EEPROM
  if (EEPROM_ENABLE == 1) {
    EEPROM.write(EEPROM_DOORSTATE,status);
    counter++;
    char a = lowByte(counter);
    char b = highByte(counter);
    EEPROM.write(EEPROM_COUNTLOW, a);
    EEPROM.write(EEPROM_COUNTHIGH, b);

    // Commit changes to EEPROM
    EEPROM.commit();
  }

  // Blink the LED
  delay(LED_INTERVAL);
  digitalWrite(PIN_LED, HIGH);
}

void door_open() {
  // Blink the LED
  digitalWrite(PIN_LED, LOW);

  // Update Status
  status = 1;

  // Trigger Relay
  door_toggle();

  // Increment counter and write status to EEPROM
  if (EEPROM_ENABLE == 1) {
    EEPROM.write(EEPROM_DOORSTATE,status);
    counter++;
    char a = lowByte(counter);
    char b = highByte(counter);
    EEPROM.write(EEPROM_COUNTLOW, a);
    EEPROM.write(EEPROM_COUNTHIGH, b);

    // Commit changes to EEPROM
    EEPROM.commit();
  }

  // Blink the LED
  delay(LED_INTERVAL);
  digitalWrite(PIN_LED, HIGH);
}

String eeprom_data() {

  String ep;
  ep += "<h3>EEPROM Data</h3>";
  ep += "<table>";
  ep += "<tr><td><b>Location</b></td><td><b>Data</b></td></tr>";
  ep += "<tr><td><b>Doorstate (0)</b></td><td>";
  if (EEPROM_ENABLE == 1) {
    ep += EEPROM.read(EEPROM_DOORSTATE);
  } else {
    ep += "0";
  }
  ep += "</td></tr>";
  ep += "<tr><td><b>Counter Low (1)</b></td><td>";
  if (EEPROM_ENABLE == 1) {
    ep += EEPROM.read(EEPROM_COUNTLOW);
  } else {
    ep += "0";
  }
  ep += "</td></tr>";
  ep += "<tr><td><b>Counter High (2)</b></td><td>";
  if (EEPROM_ENABLE == 1) {
    ep += EEPROM.read(EEPROM_COUNTHIGH);
  } else {
    ep += "0";
  }
  ep += "</td></tr>";
  ep += "</table>";

  return ep;
  
}

String reed_status() {

  String resp;
  resp += "<tr><td><b>Reed Switch Status</b></td>";
  if (reedswitch == 0) {
    resp += "<td>Closed</td></tr>";
  } else {
    resp += "<td>Open</td></tr>";
  }

  return resp;
  
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

  // Initialize EEPROM
  EEPROM.begin(512);
  
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  // Initialize D2 as output pin
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_D2, OUTPUT);
  digitalWrite(PIN_D2, LOW);
  pinMode(PIN_REED, INPUT_PULLUP);
  digitalWrite(PIN_LED, HIGH);

  // Read the current door state from EEPROM
  status = EEPROM.read(EEPROM_DOORSTATE);
  Serial.print("Retrieved door state from EEPROM: ");
  Serial.println(status);

  // Read the current counter value from EEPROM
  counter = EEPROM.read(EEPROM_COUNTHIGH);
  counter = counter << 8;
  counter = counter | EEPROM.read(EEPROM_COUNTLOW);
  Serial.print("Retrieved counter value from EEPROM: ");
  Serial.println(counter);

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
   ltoa(counter, buf, 10);
      
    String resp;
    resp = "<html><head><title>Garage Opener</title></head>";
    resp += "<body>";
    resp += "<h1>Garage Door Opener</h1>";

    // Status table
    resp += "<h3>Status</h3>";
    resp += "<table><tr>";
    resp += "<td><b>Door is currently</b></td>";
    if (status == 0) {
      resp += "<td>Closed</td>";
    } else {
      resp += "<td>Open</td>"; 
    }
    resp += "</tr><tr>";
    resp += "<td><b>Door has been operated</b></td>";
    resp += "<td>";
    resp += buf;
    resp += " times</td></tr>";
    resp += "<tr><td><b>System Uptime</b></td><td>";
    resp += uptime();
    resp += "</td></tr>";
    resp += reed_status();
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

    // EEPROM Data
    resp += eeprom_data();
    
    resp += "</body></html>";
    server.send(200, "text/html", resp);
  });

  server.on("/api", [](){
    String reqstate = server.arg("plain");
    String txtstate;
    if (status == 0) {
      txtstate = "true";
    } else {
      txtstate = "false";
    }
    if (reqstate == "toggle") {
      if (status == 0) {
        door_open();
      } else {
        door_close();
      }
    }
    server.send(200, "application/json", "{\"closed\": \"" + txtstate + "\",\"req\": \"" + reqstate + "\"}");
  });
  
  server.on("/door", [](){
    String state=server.arg("state");
    if (state == "close") {

      if (status == 0) {
        server.send(500, "text/plain", "Door is already closed");
      } else {
        door_close();
        server.send(200, "text/plain", "Closing Garage Door");    
      }
    }
    else if (state == "open") {

      if (status == 1) {
        server.send(500, "text/plain", "Door is already opened");
      } else {
        door_open();
        server.send(200, "text/plain", "Opening Garage Door");    
      }
    }
    else if (state == "setcounter") {
      counter = server.arg("counter").toInt();
      server.send(200, "text/plain", "OK. Counter set to "+server.arg("counter"));
    }
    else if (state == "setclosed") {
      status = 0;
      server.send(200, "text/plain", "OK. Door set closed.");
    }
    else if (state == "setopen") {
      status = 1;
      server.send(200, "text/plain", "OK. Door set opened.");
    }
    else if (state == "query") {

      // Buffer to store long as a char
      String resp;
      char buf[16];
      ltoa(counter, buf, 10);
      
      if (status == 0) {
        resp = "closed,";
      } else {
        resp = "open,";
      }
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
  if (digitalRead(PIN_REED) == HIGH) {
    reedswitch = 1;
  } else {
    reedswitch = 0;
  }

  if (TEST_MODE == 1) {
    // If test mode is enabled, toggle the relay every 2 seconds and print to serial monitor
    // This helps with 
    digitalWrite(PIN_D2, HIGH);
    Serial.println("Turn On");
    delay(2000);
    digitalWrite(PIN_D2, LOW);
    Serial.println("Turn Off");
    delay(2000);
  }
  
  delay(1);
}
