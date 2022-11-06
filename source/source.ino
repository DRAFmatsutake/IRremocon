/*
 * esp32 dev kit
 * https://github.com/espressif/arduino-esp32
 * Arduino-IRremote
 * https://github.com/Arduino-IRremote/Arduino-IRremote
 * 
*/
#include <WiFi.h>
#include <WebServer.h>

#include "Config.h" // 'ssid' と 'password'　の定義

//赤外線センサ ライブラリ設定
#define IR_SEND_PIN     4   //D4
#define IR_RECEIVE_PIN 21   //D21
#define RAW_BUFFER_LENGTH 400
#define RECORD_GAP_MICROS 12000
//#define RAW_BUFFER_LENGTH  n
#define DECODE_DISTANCE     // universal decoder for pulse width or pulse distance protocols
//#define DECODE_HASH         // special decoder for all protocols
#include <IRremote.hpp>

//ピン設定
#define SELETMODE_PIN   5  //pull up


//プロトタイプ宣言
void Index(void);
String SelectSignal(String key,String html);
void IRrecProc();

//変数定義
String recSignal="";
String sendSignal="";
char servermode;
char sendFlug=false;

//webサーバー初期化
WebServer server(80);

//----------------------------------------------------------
//           esp32 set up and main prosess
//----------------------------------------------------------


//+===========================================
void setup() {
  //Pin設定
  pinMode(SELETMODE_PIN,INPUT);
  
  Serial.begin(9600);         //シリアル通信　初期化
  
  //Wifi　初期化＆接続
  WiFi.begin(ssid, password); 
  Serial.println("\nConnecing...");   
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
  }
  // ESP32のIPアドレスを出力
  Serial.println("WiFi Connected.");
  Serial.print("IP = ");
  Serial.println(WiFi.localIP());

  //webサーバーの開始
  server.on("/",Index);
  server.begin();
  IrSender.begin();
  IrReceiver.begin(IR_RECEIVE_PIN);
}

//+===========================================
void loop() {
    servermode = digitalRead(SELETMODE_PIN);    //モード状態の取得
    if(servermode == 0){
      IRrecProc();
    }
    if(sendFlug==true){
      sendFlug=false;
      IrSender.sendPronto(sendSignal.c_str());
      delay(2000);  
    }
     
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
    
    //ここからモードによって分岐する
    if(servermode == 1){    //送信モード
        html += "<h1>IRserver MODE</h1>";
        if (server.method()==HTTP_POST){
          html += SelectSignal(server.arg("switch"));
         }
        html += "<form action=\"./\" method=\"POST\">";
        html += "<input name=\"switch\" type=\"text\">";
        html += "<input type=\"submit\" value=\"送信\">";
        html += "</form>";
     }
     else{                  //受信モード
        html += "<h1>IRreceiver MODE</h1>";
        html += "<h2>受信信号</h2>";
        if(recSignal=="")
          html += "<div>No signal</div>";
        else
          html += "<div>"+recSignal+"</div>";
     }
      
     html += "</body></html>";
    server.send(200, "text/html", html);    // HTMLを出力する
 }


 
//----------------------------------------------------------
//                  signal prosess
//----------------------------------------------------------
String SelectSignal(String key){
  sendFlug=true;
  String result;
  if(key=="light_on")
    {result="light_on";}
  else if(key=="light_off")
    {result="light_off";}
  else
    {
      result="\""+key+"\" sended";
      sendSignal=key;
      if(!SignalCheack(key)){
        sendFlug=false;
        result="信号のフォーマットが異なります";
      }
    }
  return result;
}

//+===========================================
void IRrecProc(){
    if (IrReceiver.decode()) {
      recSignal="";
      IrReceiver.compensateAndStorePronto(&recSignal);
      Serial.print("rec signal is : ");
      Serial.println(recSignal);  
      IrReceiver.resume(); // Enable receiving of the next value
  }
}
//+===========================================
//信号が正しいフォーマットであるかの確認
bool SignalCheack(String str){
  int count=0;
  char c;
  if((str.length()-4)%5!=0){
    return false;
  }
  for(int i=0;i<str.length();i++){
    c=str[i];
    if(count==4){
      count=0;
      if(c!=' '){                          //4文字刻みのスペースを確認
        return false;
      }
    }
      else{
        if(c<'0'||'F'<c||('9'<c&&c<'A')){   //16進数文字か確認
          return false;
        }
    count++;
      }
  }
  return true;
}
