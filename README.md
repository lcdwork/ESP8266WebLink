### 参考自wifi_link_tool



## 当前版本v0.0.1 开源协议：GPL3.0



## 说明
ESP8266简易配网库，默认上电连接wifi，5秒内未连接成功启动配网模式

默认SSID:ESP8266

默认密码:12345678

默认IP:192.168.4.1

默认主机名:esp8266.local

wifi信息保存在EEPROM

网页采用文件方式存储



## 使用方法
- 1.安装本库
- 2.安装arduino-esp8266fs-plugin插件
- 3.示例代码
``` c
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
```

- 4.上传网页文件



## 存在问题

封装为arduino库后有时卡在获取ip，需要多等待一会，暂时不明是否为代码问题还是硬件问题



## 版本变化
0.0.1基本功能实现，待进一步测试和改进



