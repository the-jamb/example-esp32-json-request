// JSON Example
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <cstdio>

const char* ssid = "poco";
const char* password = "12344321";

// wybrac dowolne
#define SSID "stud_wifi"
#define PASS ""

const char* address = "http://10.0.2.32:80/tcmed-platform-web/api/web/command/find/2";

HTTPClient http;

class Command
{
public:
    int mType;
    String mBody;

    Command(int type, String body)
        : mType(type), mBody(body) {}

};

uint64_t ledTimer;
uint64_t programTimer;
int blinkInterval;

#define PROG_INTERVAL 200


// #################################################
// ################# DECLARATIONS ##################
// #################################################

bool wifiConnect();
Command deserialize(const char* payload);
void blinkLed();

// #################################################
// ############### SETUP AND MAIN LOOP #############
// #################################################


void setup() {
    Serial.begin(115200);
    pinMode(BUILTIN_LED, OUTPUT);

    if (wifiConnect()) {
        Serial.printf("Connected to %s, on pass %s\n", SSID, PASS);
    }
}

void loop() {

    if (millis() - programTimer > PROG_INTERVAL) {
        http.begin(address);
        auto httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK)        {
            String payload = http.getString();

            auto myCommand = deserialize(payload.c_str());
            Serial.printf("Type is %d\n", myCommand.mType);
            Serial.printf("Body is %s\n", myCommand.mBody.c_str());

            if (sscanf(myCommand.mBody.c_str(), "%d", &blinkInterval) && myCommand.mType == 2) {
                Serial.printf("Value is %d\n", blinkInterval);
                blinkLed();
            }
        }
        else if (httpCode != HTTP_CODE_NOT_FOUND) {
            Serial.printf("Code is %d\n", httpCode);
        }
        programTimer = millis();
    }
}

// #################################################
// #################### DEFINITIONS ################
// #################################################

bool wifiConnect()
{
    unsigned int retries = 0;
    WiFi.begin(SSID, PASS);

    while(WiFi.status() != WL_CONNECTED){
        Serial.printf("Connect to %s, on pass %s\n", SSID, PASS);

        retries++;
        if(retries > 50){
            Serial.println("Failed to connect after 50 attempts");

            return false;
        }
    }
    
    return true;
}
Command deserialize(const char* payload)
{
    // Stream& input;
    StaticJsonDocument<500> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return Command(0, "");
    }

    int type = doc["type"];             
    const char* buffer = doc["body"];      

    return Command(type, buffer);
}

void blinkLed()
{
    static bool state = false;

    if(millis() - ledTimer > blinkInterval){
        digitalWrite(BUILTIN_LED, state);
        Serial.printf("State is %d\n", state);
        state = !state;
        ledTimer = millis();
    }
}
// #################################################