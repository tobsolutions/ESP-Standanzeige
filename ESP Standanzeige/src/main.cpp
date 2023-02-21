#include <Arduino.h>
#include "ESP8266WiFi.h"
#include "WebSocketClient.h"
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "Adafruit_MCP23X17.h"

#define ANZ_STAENDE 12
#define ERST_STAND 1
#define SIMULATION_MODE false

struct settings {
  char ssid[30];
  char password[30];
} user_settings = {};

int freeLanes[ERST_STAND+ANZ_STAENDE];

ESP8266WebServer server(80);

WebSocketClient ws(false);

StaticJsonDocument<512> msg_rx_json;

Adafruit_MCP23X17 mcp0;
const uint8_t mcp0_addr = 0; // Adresse 0x20 / 0

void handleConfig() {
  if (server.method() == HTTP_POST) {
    strncpy(user_settings.ssid,     server.arg("ssid").c_str(),     sizeof(user_settings.ssid) );
    strncpy(user_settings.password, server.arg("password").c_str(), sizeof(user_settings.password) );
    user_settings.ssid[server.arg("ssid").length()] = user_settings.password[server.arg("password").length()] = '\0';
    EEPROM.put(0, user_settings);
    EEPROM.commit();

    server.send(200,   "text/html",  "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Setup</title><style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}</style> </head> <body><main class='form-signin'> <h1>Wifi Setup</h1> <br/> <p>Your settings have been saved successfully!<br />Please restart the device.</p></main></body></html>" );
  } else {
    String html;
    html = "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Setup</title> <style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{cursor: pointer;border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1{text-align: center}</style> </head> <body><main class='form-signin'> <form action='/' method='post'> <h1 class=''>Wifi Setup</h1><br/><div class='form-floating'><label>SSID</label><input type='text' class='form-control' name='ssid' value='";
    html += user_settings.ssid;
    html += "'> </div><div class='form-floating'><br/><label>Password</label><input type='password' class='form-control' name='password'></div><br/><br/><button type='submit'>Save</button><p style='text-align: right'><a href='https://www.esp-standanzeige.de' style='color: #32C5FF'>esp-standanzeige.de</a></p></form></main> </body></html>";
    server.send(200,   "text/html", html);
  }
}

void handleFreeLanes() {
  if (server.method() == HTTP_POST) {
    strncpy(user_settings.ssid,     server.arg("ssid").c_str(),     sizeof(user_settings.ssid) );
    strncpy(user_settings.password, server.arg("password").c_str(), sizeof(user_settings.password) );
    user_settings.ssid[server.arg("ssid").length()] = user_settings.password[server.arg("password").length()] = '\0';
    EEPROM.put(0, user_settings);
    EEPROM.commit();

    server.send(200,   "text/html",  "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title><style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}</style> </head> <body><main class='form-signin'> <h1>Wifi Setup</h1> <br/> <p>Your settings have been saved successfully!<br />Please restart the device.</p></main></body></html>" );
  } else {
    String html;
    html = "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>ESP Standanzeige</title> <style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{cursor: pointer;border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1{text-align: center}</style> </head> <body><main class='form-signin'><h2>ESP Standanzeige</h2>";
    html += "<p>";
    for(int i = ERST_STAND;i < ANZ_STAENDE+ERST_STAND;i++){
      html += "Stand ";
      html += i;
      if(freeLanes[i] == 1){
        html += " ist frei</br>";
      } else {
        html += " ist belegt</br>";
      }
    }
    html += "</p>";
    html += "<p style='text-align: right'><a href='/config' style='color: #32C5FF'>Config</a></p><p style='text-align: right'><a href='https://www.esp-standanzeige.de' style='color: #32C5FF'>esp-standanzeige.de</a></p></main></body></html>";
    server.send(200,   "text/html", html);
  }
}

void setup()
{
  Serial.begin(9600);
  delay(6000);

  if (!mcp0.begin_I2C()) {
  //if (!mcp.begin_SPI(CS_PIN)) {
    Serial.println("Error on I2C Bus.");
    while (1);
  }
  for(int i = ERST_STAND;i < ANZ_STAENDE+ERST_STAND;i++){
    mcp0.pinMode(i-ERST_STAND, OUTPUT);
  }

  Serial.println();
  Serial.println("Running Firmware.");

  EEPROM.begin(sizeof(struct settings));
  EEPROM.get(0, user_settings);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(user_settings.ssid);
   
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(user_settings.ssid, user_settings.password);
  Serial.print("Connecting...");

  byte tries = 0;
  byte wifistat = 0; //1: WLAN Connection 2: AP for WIFI Config
  while (WiFi.status() != WL_CONNECTED) {  
    Serial.print("..");
    wifistat = 1;
    if (tries++ > 15) {
      WiFi.mode(WIFI_AP);
      WiFi.softAP("ESP-Standanzeige", "12345678");
      wifistat = 2;
      Serial.println(".");
      Serial.println("WiFi AP started.");
      Serial.println("Connect to AP for WIFI config.");
      Serial.println("SSID: ESP-Standanzeige Pass: 12345678");
      Serial.println("IP address: 192.168.4.1");
      break;
    }
    delay(1000);
  }
  if(wifistat == 1) {
    Serial.println(".");
    Serial.print("WiFi connected to ");
    Serial.println(user_settings.ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    server.on("/",  handleFreeLanes);
    server.on("/config",  handleConfig);
  } else if(wifistat == 2) {
    server.on("/",  handleConfig);
  }
  server.begin();
}

void loop()
{
  server.handleClient();

  if (!ws.isConnected()) {
    if(!SIMULATION_MODE) {
      Serial.println("Websocket not connected. Try to connect..");
		  ws.connect("192.168.10.200","/",49472);
    } else {
      Serial.println("## SIMULATION MODE ## No connection to websocket. Random free Lanes.");
      for(int i = ERST_STAND;i < ANZ_STAENDE+ERST_STAND;i++){
        int rand = random(100);
        if(rand > 50) {
          freeLanes[i] = 1;
        } else {
          freeLanes[i] = 0;
        }
      }
    }
	} else {
    // Send request to Websocket
    Serial.println("Websocket connected. Send message:");
    String msg_tx;
    msg_tx = "{\"Prot\": \"MEWS\",\"VerP\": \"00000100\",\"SubProt\": \"LA\",\"VerSP\": \"00000100\",\"SeqNo\": 1,\"Cmd\": \"GetFreeLanes\",\"Data\": {\"StartlistID\": 215}}";
    Serial.println(msg_tx);
    ws.send(msg_tx);

    // Check response from Websocket
		String msg_rx;
		if (ws.getMessage(msg_rx)) {
      Serial.println("Response:");
			Serial.println(msg_rx);

      DeserializationError error = deserializeJson(msg_rx_json, msg_rx);
      if(error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
      }

      JsonArray data = msg_rx_json["Data"];
      //Serial.print("data[0]: ");
      //Serial.println(data[0].as<String>());
      for(int i = ERST_STAND;i < ANZ_STAENDE+ERST_STAND;i++){
        freeLanes[i] = 0;
      }
      for(JsonVariant v : data){
        //Serial.println(v.as<int>());
        freeLanes[v.as<int>()] = 1;
      }
		}
	}
  for(int i = ERST_STAND;i < ANZ_STAENDE+ERST_STAND;i++){
    if(freeLanes[i] == 1){
      Serial.print("Stand ");
      Serial.print(i);
      Serial.println(" ist frei");
      mcp0.digitalWrite(i-ERST_STAND, LOW);
    } else {
      mcp0.digitalWrite(i-ERST_STAND, HIGH);
    }
  }
	delay(2000);
}

