
#include "wificonnection.h"
void wifi_init(char*  ssid,char* pw)
{
  WiFi.setAutoReconnect(true);

  WiFi.begin(ssid,pw);
}
