#ifndef MAIN_H
#define MAIN_H
#include "wificonnection.h"
#define PROJECT_ID "projet-corrige" //Taken from "project_id" key in JSON file.
#define CLIENT_EMAIL "pfacorrige@projet-corrige.iam.gserviceaccount.com" //Taken from "client_email" key in JSON file.
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#define WIFI_SSID "*****"
#define WIFI_PASSWORD "*********"
#define led 15
void success(void);
void fail(void);
#endif // MAIN_H
