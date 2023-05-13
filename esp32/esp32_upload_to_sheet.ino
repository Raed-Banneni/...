#include "main.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 0;
String GOOGLE_SCRIPT_ID = "AKfycbynttO9aC2dqHAPtJ9L6BS7tRFdljS06F3DMf-j_eZTvLtepgQRk79j9dT4JLVKp9kf";    // change Gscript ID
String msg = "";
//int count = 1;
void setup() {

  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // Initialize UART2 with 9600 baud rate, 8 data bits, no parity, and 1 stop bit. Pin 16 is the RX pin and pin 17 is the TX pin.
  delay(1000);
  wifi_init(WIFI_SSID, WIFI_PASSWORD);
  //Begin the access token generation for Google API authentication
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  if (Serial2.available()) { // Check if there's data available on UART2
    String data = Serial2.readStringUntil('\n'); // Read the incoming data from UART2
    Serial.println(data); // Print the data to the Serial Monitor
    if (data == "1") {
       Serial.println("fall detected");
      if (WiFi.status() == WL_CONNECTED) {
        static bool flag = false;
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
          Serial.println("Failed to obtain time");
          return;
        }
        char timeStringBuff[50]; //50 chars should be enough
        strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
        String asString(timeStringBuff);
        asString.replace(" ", "-");
        Serial.print("Time:");
        Serial.println(asString);

        String urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + "chute=" + asString /*+ "&fall=" + String(count)*/ ;
        Serial.print("POST data to spreadsheet:");
        Serial.println(urlFinal);
        HTTPClient http;
        http.begin(urlFinal.c_str());
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        int httpCode = http.GET();
        Serial.print("HTTP Status Code: ");
        Serial.println(httpCode);
        //---------------------------------------------------------------------
        //getting response from google sheet
        String payload;
        if (httpCode > 0) {
          payload = http.getString();
          Serial.println("Payload: " + payload);
        }
        //---------------------------------------------------------------------
        http.end();
      }
    }
  }
}
