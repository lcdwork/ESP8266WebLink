#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <FS.h>

#ifndef STASSID
#define STASSID "ESP8266"
#define STAPSK  "12345678"
#endif

String ssid;
String psw;

struct config_type
{
  char stassid[32];//定义配网得到的WIFI名长度(最大32字节)
  char stapsw[64];//定义配网得到的WIFI密码长度(最大64字节)
};

config_type config;//声明定义内容

const char* ap_ssid = STASSID;
const char* ap_password = STAPSK;
int Signal_filtering = -200;//最低信号强度

ESP8266WebServer server(80);
bool LED_Flag = false;

void init(void);
void saveConfig(void);
void loadConfig(void);
void reboot(void);
String wifiType(int typecode);
void handleRoot(void);
void wifiScan(void);
void wifiConfig(void);
void handleNotFound(void);
void htmlConfig(void);
bool autoConfig(void);


/*
 * 初始化
 * 无返回值
 */
void link() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.begin(115200);
    SPIFFS.begin();
    loadConfig();
    Serial.printf("");
//    Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
    bool wifiConfig = autoConfig();
    
    if(wifiConfig == false){
		digitalWrite(LED_BUILTIN, LOW);		
        htmlConfig();//HTML配网
	}
    else{
      digitalWrite(LED_BUILTIN, HIGH);
    }
}

/*
 * EEPROM写入函数
 * 无返回值
 */
void saveConfig()//保存函数
{
 EEPROM.begin(1024);//向系统申请1024kb ROM
 //开始写入
 uint8_t *p = (uint8_t*)(&config);
  for (int i = 0; i < sizeof(config); i++)
  {
    EEPROM.write(i, *(p + i)); //在闪存内模拟写入
  }
  EEPROM.commit();//执行写入ROM
}
/*
 * EEPROM读取函数
 * 无返回值
 */
void loadConfig()//读取函数
{
  EEPROM.begin(1024);
  uint8_t *p = (uint8_t*)(&config);
  for (int i = 0; i < sizeof(config); i++)
  {
    *(p + i) = EEPROM.read(i);
  }
  EEPROM.commit();
  ssid = config.stassid;
  psw = config.stapsw;
}

/*
 * 重启ESP8266
 * 无返回值
 * 尚未使用
 */
void reboot(){
  IPAddress ips;
  ips = WiFi.localIP();
  server.send(200, "text/plain", String(ips[0])+"."+String(ips[1])+"."+String(ips[2])+"."+String(ips[3]));
  delay(1000);
  WiFi.softAPdisconnect();
  ESP.restart();
}
/*
 * WIFI加密类型
 * 返回字符串
 */
String wifiType(int typecode) {
  if (typecode == ENC_TYPE_NONE) return "Open";
  if (typecode == ENC_TYPE_WEP) return "WEP ";
  if (typecode == ENC_TYPE_TKIP) return "WPA ";
  if (typecode == ENC_TYPE_CCMP) return "WPA2";
  if (typecode == ENC_TYPE_AUTO) return "WPA*";
}
/*
 * web主页
 * 无返回值
 */
void handleRoot() {
  File file = SPIFFS.open("/index.html", "r");
//  if (!file) {
//    Serial.print("File not found");
//    return;
//  }
//  //如果未找到文件  打印输出并且返回
//  while (file.available()) {
//    Serial.write(file.read());
//  }
  server.streamFile(file, "text/html");
  file.close();
}
/*
 * 扫描附近WIFI
 * 无返回值
 */
void wifiScan() {
  String req_json = "";
  Serial.println("Scan WiFi");
  int n = WiFi.scanNetworks();
  int m = 0;
  if (n > 0) {
    req_json = "{\"req\":[";
    for (int i = 0; i < n; i++) {
      if ((int)WiFi.RSSI(i) >= Signal_filtering)
        m++;
      String a="{\"ssid\":\"" + (String)WiFi.SSID(i) + "\"," + "\"encryptionType\":\"" + wifiType(WiFi.encryptionType(i)) + "\"," + "\"rssi\":" + (int)WiFi.RSSI(i) + "},";
      if(a.length()>15)
        req_json += a;
    }
  }
  req_json.remove(req_json.length() - 1);
  req_json += "]}";
  server.send(200, "text/json;charset=UTF-8", req_json);
  Serial.print("Found ");
  Serial.print(m);
  Serial.print(" WiFi!  >");
  Serial.print(Signal_filtering);
  Serial.println("dB");
}
/*
 * 设置WIFI
 * 无返回值
 */
void wifiConfig() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    int ssid_len = server.arg("ssid").length();
    int password_len = server.arg("password").length();
    if ((ssid_len > 0) && (ssid_len < 33) && (password_len > 7) && (password_len < 65)) {
      String ssid_str = server.arg("ssid");
      String password_str = server.arg("password");
      const char *ssid = ssid_str.c_str();
      const char *password = password_str.c_str();
      Serial.print("SSID: ");
      Serial.println(ssid);
//      Serial.print("Password: ");
//      Serial.println(password);
      WiFi.begin(ssid, password);
      Serial.print("Connenting");
      unsigned long millis_time = millis();
      while ((WiFi.status() != WL_CONNECTED) && (millis() - millis_time < 8000)) {
        delay(500);
        Serial.print(".");
      }
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("Connected successfully!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("http://");
        // webServer.send(200, "text/plain", "1");
        IPAddress ips;
        ips = WiFi.localIP();
        server.send(200, "text/plain", String(ips[0])+"."+String(ips[1])+"."+String(ips[2])+"."+String(ips[3]));
        delay(300);
        strcpy(config.stassid,WiFi.SSID().c_str());//名称复制
        strcpy(config.stapsw,WiFi.psk().c_str());//密码复制
        saveConfig();//调用保存函数
        //取消注释，配网成功后直接重启
        /*
        WiFi.softAPdisconnect();
        delay(1000);
        ESP.restart();
        */
      } else {
        Serial.println("Connenting failed!");
        server.send(200, "text/plain", "0");
      }
    } else {
      Serial.println("Password format error");
      server.send(200, "text/plain", "0");
    }
  } else {
    Serial.println("Request parameter error");
    server.send(200, "text/plain", "0");
  }
}
/*
 * 返回404
 * 无返回值
 */
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

/*
 * 网页服务配置
 * 无返回值
 */
void htmlConfig()
{
    WiFi.mode(WIFI_AP_STA);//设置模式为AP+STA
    WiFi.softAP(ap_ssid, ap_password);
    Serial.println("AP设置完成");
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  
    if (MDNS.begin("esp8266")) {
      Serial.println("MDNS responder started");
    }
  
    server.on("/", handleRoot);
    server.on("/wificonfig", wifiConfig);
    server.on("/wifiscan", wifiScan);
    server.on("/reboot", reboot);
    
    server.onNotFound(handleNotFound);//请求失败回调函数
  
    server.begin();//开启服务器
    Serial.println("HTTP server started");
    while(1)
    {
        server.handleClient();
        MDNS.update();  
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("HtmlConfig Success");
            Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
//            Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
            Serial.println("HTML连接成功");
            break;
        }
    }  
}
/*
 * 自动连接WIFI
 * 无返回值
 */
bool autoConfig()
{
  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, psw);
  if(ssid==0||psw==0){
    return false;
  }
  Serial.print("AutoConfig Waiting......");
  for (int i = 0; i < 20; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("AutoConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
//      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.printDiag(Serial);
      return true;
      //break;
    }
    else
    {
      Serial.print(".");
      LED_Flag = !LED_Flag;
      if(LED_Flag)
          digitalWrite(LED_BUILTIN, HIGH);
      else
          digitalWrite(LED_BUILTIN, LOW); 
      delay(500);
    }
  }
  Serial.println("AutoConfig Faild!" );
  return false;
}
