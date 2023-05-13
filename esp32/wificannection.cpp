
#include "wificonnection.h"
void wifi_init(char*  ssid,char* pw)
{
  WiFi.setAutoReconnect(true);

  WiFi.begin(ssid,pw);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}
