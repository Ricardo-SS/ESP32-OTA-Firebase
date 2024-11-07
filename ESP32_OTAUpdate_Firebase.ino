/*****************************************************************************************************************************
**********************************    Author  : Ehab Magdy Abdullah                      *************************************
**********************************    Linkedin: https://www.linkedin.com/in/ehabmagdyy/  *************************************
**********************************    Youtube : https://www.youtube.com/@EhabMagdyy      *************************************
******************************************************************************************************************************/

// Include Libraries
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <HTTPClient.h>
#include <Update.h>

// WiFi credentials
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"

// Firebase Project credentials
#define FIREBASE_API_KEY "FIREBASE_API_KEY"
#define FIREBASE_RTDB_URL "FIREBASE_RTDB_URL"
#define FIREBASE_USER_EMAIL "FIREBASE_USER_EMAIL"
#define FIREBASE_USER_PASSWORD "FIREBASE_USER_PASSWORD"

// Variables for Receiving Version and Update URL
String Update_Version = "";
String Firebase_Firmawre_Update_URL = "";

// Current firmware version
#define CURRENT_FIRMWARE_VERSION "1.0"

// Firebase instance
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup()
{
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Configure Firebase
    config.api_key = FIREBASE_API_KEY;
    config.database_url = FIREBASE_RTDB_URL;
    auth.user.email = FIREBASE_USER_EMAIL;
    auth.user.password = FIREBASE_USER_PASSWORD;

    // Initialize Firebase
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

bool CheckForUpdate()
{
    if (Firebase.ready())
    {
        // Read Available update Version Number
        if(Firebase.RTDB.getString(&fbdo, "/update/version"))
        {
            Update_Version = fbdo.stringData();
            Serial.println(Update_Version);

            char update_version = Update_Version.compareTo(CURRENT_FIRMWARE_VERSION);
            
            // if Version Higher than current version
            if(update_version > 0)
            {
                // Read update URL
                if(Firebase.RTDB.getString(&fbdo, "/update/url"))
                {
                    Firebase_Firmawre_Update_URL = fbdo.stringData();
                    Serial.println(Firebase_Firmawre_Update_URL);
                    return true;
                }
                else{ Serial.println(fbdo.errorReason().c_str()); }
            }
            else if(0 == update_version){ Serial.println("Application is Up To Date"); }
            else{ Serial.println("Firebase version is old!"); }

        }
        else{ Serial.println(fbdo.errorReason().c_str()); }
    }

    return false;
}

void downloadAndUpdateFirmware()
{
    // Get the download URL from Firebase
    Serial.print("Firmware URL: ");
    Serial.println(Firebase_Firmawre_Update_URL);

    HTTPClient http;
    http.begin(Firebase_Firmawre_Update_URL);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
      WiFiClient& client = http.getStream();
      int firmwareSize = http.getSize();
      Serial.print("Firmware Size: ");
      Serial.println(firmwareSize);

      if (Update.begin(firmwareSize))
      {
          size_t written = Update.writeStream(client); //////////// Takes Time

          if (Update.size() == written)
          {
              Serial.println("Update successfully completed. Rebooting...");

              if (Update.end())
              {
                  Serial.println("Rebooting...");
                  ESP.restart();
              } 
              else 
              {
                  Serial.print("Update failed: ");
                  Serial.println(Update.errorString());
              }
          }
          else
          {
              Serial.println("Not enough space for OTA.");
          }
      } 
        else
        {
            Serial.println("Failed to begin OTA update.");
        }
    }
    else
    {
        Serial.print("Failed to download firmware. HTTP code: ");
        Serial.println(httpCode);
    }

    http.end();
}

void loop()
{
    delay(1000);
    Serial.println("> Version 1.0");
    delay(1000);
    
    // if there is a new update, download it
    if(true == CheckForUpdate()){ downloadAndUpdateFirmware() ; }

    delay(5000);
}
