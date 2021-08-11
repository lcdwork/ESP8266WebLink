/*
 * 简易web配网程序，上电后自动连接保存过的wifi，5秒后连接超时，自动进入配网模式
 * 默认配网SSID:ESP8266，密码：12345678
 * 默认地址：192.168.4.1
 * 默认主机名：esp8266.local
 * 封装成库后可能会在获取ip时卡住，多等待一会即可，原因暂时不明，未封装成库使用正常
 */
#include <WebLinkEE.h>

void setup() {
  link();
  ESP.wdtEnable(10000);//设定看门狗
}

void loop() {
  if(WiFi.status() == WL_CONNECTED){
    ESP.wdtFeed();//喂狗释放资源
    Serial.println("link ok!");
  }
  delay(2000);

}