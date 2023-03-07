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
#define RAW_BUFFER_LENGTH 800
#define RECORD_GAP_MICROS 12000
//#define RAW_BUFFER_LENGTH  n
#define DECODE_DISTANCE     // universal decoder for pulse width or pulse distance protocols
//#define DECODE_HASH         // special decoder for all protocols
#include <IRremote.hpp>

//プロトタイプ宣言
void Index(void);
String SelectSignal(String key,String html);
void IRrecProc();

//変数定義
String recSignal="No signal";
String sendSignal="";
char sendFlug=false;
char sendLoop=1;

//webサーバー初期化
WebServer server(80);

//----------------------------------------------------------
//           esp32 set up and main prosess
//----------------------------------------------------------


//+===========================================
void setup() {
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

  //IRセンサの開始
  IrSender.begin();
  IrReceiver.begin(IR_RECEIVE_PIN);
}

//+===========================================
void loop() {
    IRrecProc();          //信号受信処理
    if(sendFlug==true){   //信号送信処理　フラグが立った場合
      sendFlug=false;
      while(sendLoop--){
        IrSender.sendPronto(sendSignal.c_str());
        delay(3000);
      }
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

    //送信フォーム
    html += "<h1>送信</h1>";
    if (server.method()==HTTP_POST){
      html += SelectSignal(server.arg("switch"));
     }
    html += "<form action=\"./\" method=\"POST\">";
    html += "<input name=\"switch\" type=\"text\">";
    html += "<input type=\"submit\" value=\"送信\">";
    html += "</form>";
    
    //受信フォーム
    html += "<h1>受信</h1>";
    html += "<h2>受信信号</h2>";
    html += "<div id=\"sig\">"+recSignal+"</div>";
    
    
    html += "</body></html>";
    server.send(200, "text/html", html);    // HTMLを出力する
 }
 
//----------------------------------------------------------
//                  signal prosess
//----------------------------------------------------------
String SelectSignal(String key){
  sendFlug=true;
  String result;
  if(key=="light_on"){
    result="ライトオン";sendSignal=SIGNAL_LIGHT;sendLoop=1;
  }
  else if(key=="light_off"){
    result="ライトオフ";sendSignal=SIGNAL_LIGHT;sendLoop=2;
  }
  else{
    sendLoop=1;
    result="入力信号送信";
    sendSignal=key;
    if(!SignalCheack(key)){
      sendFlug=false;
      result="信号のフォーマットが異なります";
    }
  }
  return result;
}

//+===========================================
//信号受信処理
void IRrecProc(){
    if (IrReceiver.decode()) {    //信号が受信されていたら
      recSignal="";
      IrReceiver.compensateAndStorePronto(&recSignal);
      Serial.print("rec signal is : ");
      Serial.println(recSignal);  
      IrReceiver.resume();        //信号受信の許可
  }
}
//+===========================================
//信号が正しいフォーマットであるかの確認
//4バイト+スペース+4バイトの繰り返しの確認
bool SignalCheack(String str){
  int count=0;
  char c;
  if((str.length()-4)%5!=0)
    return false;
  for(int i=0;i<str.length();i++){
    c=str[i];
    if(count==4){
      count=0;
      if(c!=' ')                            //4文字刻みのスペースを確認
        return false;
    }
    else{
      if(c<'0'||'F'<c||('9'<c&&c<'A'))      //16進数文字か確認
        return false;
      count++;
    }
  }
  return true;
}
