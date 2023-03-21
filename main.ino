#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#define LED_PIN 4   // 連接 WS2812 LED 的腳位
#define LED_COUNT 7 // LED 燈條的 LED 數量
WiFiManager wm;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// const char* ssid = "lyuchan2.4";
// const char* password = "david0831";
WiFiUDP udp;
const int udpPort = 8080;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
int id = 0;
void setup()
{
    // wm.resetSettings();
    EEPROM.begin(4);
    strip.begin();
    strip.show();
    pinMode(13, INPUT);
    /*while (digitalRead(D7) == HIGH) {
      flashWhitec();
      if (digitalRead(D7) == LOW) {
        delay(1000);
        if (digitalRead(D7) == LOW) {
          return;
        }
      }
    }*/
    light(0, 0, 255);
    wm.autoConnect("set_my_tally", "super_password");
    light(0, 0, 0);
    // 初始化 LED 燈條，並將所有 LED 關閉
    // Serial.begin(115200);
    // id = reeprom(eepromaddress);
    // Serial.println(id);
    // WiFi.begin(ssid, password);
    // while (WiFi.status() != WL_CONNECTED) {
    //  delay(1000);
    // Serial.println("Connecting to WiFi...");
    //}
    // Serial.println("Connected to WiFi");
    udp.begin(udpPort);
    // Serial.print("Listening on UDP port ");
    // Serial.println(udpPort);
    id = reeprom();
}

void loop()
{
    int packetSize = udp.parsePacket();
    if (packetSize)
    {
        // Serial.print("Received packet of size ");
        // Serial.println(packetSize);
        int len = udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
        if (len > 0)
        {
            packetBuffer[len] = 0;
        }
        ////Serial.println("Contents:");
        ////Serial.println(packetBuffer);
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, packetBuffer);
        if (error)
        {
            // Serial.print(F("de//SerializeJson() failed: "));
            // Serial.println(error.f_str());
            return;
        }
        const char *get = doc[0]["get"];
        // int pgm = doc[0]["pgm"];

        if (strcmp(get, "ping") == 0)
        { // 偵測上線用
            udp.beginPacket(udp.remoteIP(), udp.remotePort());
            udp.write("pong!");
            udp.endPacket();
            // Serial.println("pong!");
        }
        else if (strcmp(get, "tallyidset") == 0)
        {                                // 設定tally id
            int idbuffer = doc[0]["id"]; // id
            weeprom(idbuffer);
            id = reeprom();
            // Serial.print("setid:");
            // Serial.println(id);
        }
        else if (strcmp(get, "find") == 0)
        { // 尋找tally
            // Serial.println("find!");
            flashWhite(10);
        }
        else if (strcmp(get, "tally") == 0)
        {                            // 設定tally
            int pwv = doc[0]["pwv"]; // pwv
            JsonArray pgmArray = doc[0]["pgm"];
            bool found = false; // 初始化为未找到
            for (JsonVariant value : pgmArray)
            {
                int pgmValue = value; // 获取数组中的值
                if (pgmValue == id)
                {                 // 如果找到了A
                    found = true; // 标记为已找到
                    break;        // 停止查找
                }
            }
            if (found)
            { // pgm
                light(255, 0, 0);
            }
            else
            { // no pgm
                if (id == pwv)
                {
                    light(0, 255, 0);
                }
                else
                {
                    light(0, 0, 0);
                }
            }
        }
    }
}
void light(int r, int g, int b)
{
    for (int i = 0; i < LED_COUNT; i++)
    {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}
void light(uint32_t color)
{
    for (int i = 0; i < LED_COUNT; i++)
    {
        strip.setPixelColor(i, color);
    }
    strip.show();
}
void flashWhite(int numFlashes)
{
    uint32_t color = strip.getPixelColor(0);
    for (int i = 0; i < numFlashes; i++)
    {
        light(255, 255, 255);
        delay(50); // 等待 0.1 秒
        light(0, 0, 0);
        delay(50); // 等待 0.1 秒
    }
    light(color);
}
void flashWhitec()
{
    uint32_t color = strip.getPixelColor(0);
    for (int i = 0; i <= 128; i++)
    {
        light(0, i, i);
        delay(5); // 等待 0.1 秒
    }
    for (int i = 0; i <= 128; i++)
    {
        light(0, 128 - i, 128 - i);
        delay(10); // 等待 0.1 秒
    }
    delay(500);
}
void weeprom(int number)
{
    EEPROM.write(0, number);
    EEPROM.commit();
}
int reeprom()
{
    return EEPROM.read(0);
}
s