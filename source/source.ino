/*
 * esp32 dev kit
 * https://github.com/espressif/arduino-esp32
 * Arduino-IRremote
 * https://github.com/Arduino-IRremote/Arduino-IRremote
 * 
*/
#include <WiFi.h>
#include <WebServer.h>

#include "Config.h" // definition 'ssid' and 'password'

//赤外線センサ
#define IR_SEND_PIN     4   //D4
#define IR_RECEIVE_PIN 21   //D5
#include <IRremote.hpp>

#define SELETMODE_PIN   5  //pull up

WebServer server(80);
void Index(void);
String SelectSignal(String key,String html);
char prontoData[] = SIGNAL_A;
char recData[1024];
//----------------------------------------------------------
//           esp32 set up and main prosess
//----------------------------------------------------------


void setup() {
  pinMode(PIN_0,OUTPUT);
  Serial.begin(9600);
  WiFi.begin(ssid, password); 
  Serial.println("\nConnecing...");   
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
  }
  // ESP32のIPアドレスを出力
  Serial.println("WiFi Connected.");
  Serial.print("IP = ");
  Serial.println(WiFi.localIP());
  server.on("/",Index);
  server.begin();
  IrSender.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
    server.handleClient();
}



//----------------------------------------------------------
//                  html
//----------------------------------------------------------
void Index(void){
    String html;
    html = "<html><head>";
    html += "<meta charset=\"UTF-8\">";
    html += "<title>リモコン</title>";
    html += "</head>";
    
    html += "<body>";
    if (server.method()==HTTP_POST){
      html += SelectSignal(server.arg("switch"));
     }
    html += "<form action=\"./\" method=\"POST\">";
    html += "<input name=\"switch\" type=\"text\">";
    html += "<input type=\"submit\" value=\"送信\">";
    html += "</form>";
    html += "</body></html>";

    // HTMLを出力する
    server.send(200, "text/html", html);
 }


 
//----------------------------------------------------------
//                  signal prosess
//----------------------------------------------------------
String SelectSignal(String key){
  String result;
  if(key=="light_on")
    {result="light_on";digitalWrite(PIN_0,1);}
  else if(key=="light_off")
    {result="light_off";digitalWrite(PIN_0,0);}
  else
    {result="\""+key+"\" is invalid word";
    //IrSender.sendPronto(key.c_str());
    IrSender.sendPronto(prontoData);
    Serial.println("send trun on at " +  String(millis()));
    delay(2000);
    }
  return result;
}
