#include "main.h"
int bpm, spo2;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 0;
HTTPClient http;
String GOOGLE_SCRIPT_ID = "**************";  // change Gscript ID
String msg = "";
const char* frenchWeekdays[] = { "Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi" };
const char* frenchMonths[] = { "janvier", "février", "mars", "avril", "mai", "juin", "juillet", "août", "septembre", "octobre", "novembre", "décembre" };

void setup() {

  // put your setup code here, to run once:
  ////Serial.begin(19200);
  Serial1.begin(115200, SERIAL_8N1, 3, 1);  // Initialize UART2 with 9600 baud rate, 8 data bits, no parity, and 1 stop bit. Pin 3 is the RX pin and pin 1 is the TX pin.
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // Initialize UART2 with 9600 baud rate, 8 data bits, no parity, and 1 stop bit. Pin 16 is the RX pin and pin 17 is the TX pin.
  pinMode(led, OUTPUT);
  delay(1000);
  wifi_init(WIFI_SSID, WIFI_PASSWORD);
  //Begin the access token generation for Google API authentication
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  
  if (Serial1.available()) {
     success();
    String bpmspo2 = Serial1.readStringUntil('\n');  // Read the incoming data from UART0
    char charvals[bpmspo2.length() + 1];
    bpmspo2.toCharArray(charvals, sizeof(charvals));
    sscanf(charvals, "%d BPM %d SpO2", &bpm, &spo2);
    http.begin("http://***************/api/");       //Specify the URL
    http.addHeader("Content-Type", "application/json");  //set the content type
    String famille = "80";                                                                                        // Famille value to send
    String postBody = "{\"bpm\":" + String(bpm) + ",\"famille\":" + famille + ",\"spo2\":" + String(spo2) + "}";  // Create the JSON string with both fields
    int httpCode = http.POST(postBody);
    if (httpCode > 0) {  //Check for the returning code
      String response = http.getString();
      success();
      //Serial.print(postBody);
      //Serial.println(response);
    } else {

      fail();
      
    }


    ////Serial.println(bpmspo2); // Print the data to the Serial Monitor
  }
  if (Serial2.available()) {                      // Check if there's data available on UART2
    String data = Serial2.readStringUntil('\n');  // Read the incoming data from UART2
    ////Serial.println(data); // Print the data to the Serial Monitor
    if (data == "1") {
      ////Serial.println("fall detected");
      if (WiFi.status() == WL_CONNECTED) {
        static bool flag = false;
        time_t now;
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
          ////Serial.println("Failed to obtain time");
          return;
        }
        if (time(&now)) {
          localtime_r(&now, &timeinfo);
          char strftime_buf[50];
          setlocale(LC_TIME, "fr_FR");  // Set locale to French                                                              //50 chars should be enough
          snprintf(strftime_buf, sizeof(strftime_buf), "%s %d %s %d, %02d:%02d:%02d",
                   frenchWeekdays[timeinfo.tm_wday], timeinfo.tm_mday, frenchMonths[timeinfo.tm_mon], timeinfo.tm_year + 1900,
                   timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
          String asString(strftime_buf);
          asString.replace(" ", "-");
          ////Serial.print("Time:");
          ////Serial.println(asString);

          String urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + "chute=" + asString /*+ "&fall=" + String(count)*/;
          ////Serial.print("POST data to spreadsheet:");
          ////Serial.println(urlFinal);
          HTTPClient http;
          http.begin(urlFinal.c_str());
          http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
          int httpCode = http.GET();
          ////Serial.print("HTTP Status Code: ");
          ////Serial.println(httpCode);
          //---------------------------------------------------------------------
          //getting response from google sheet
          String payload;
          if (httpCode > 0) {
            payload = http.getString();
            ////Serial.println("Payload: " + payload);
          }
          //---------------------------------------------------------------------
          http.end();
        }
      }
    }
  }
}
