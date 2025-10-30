/**
 * Test Code for Guitar Hero Controller Detection
 * Simple test to verify USB Host and controller detection
 */

#include <Arduino.h>
#include <USBHost_t36.h>

// USB Host objects
USBHost myusb;
USBHub hub1(myusb);
USBHIDParser hid1(myusb);

// LED for status indication
const int LED_PIN = 13;
bool ledState = false;
elapsedMillis ledTimer;

// Controller detection
bool controllerFound = false;
uint16_t vendorID = 0;
uint16_t productID = 0;

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    // Wait for serial or timeout
    while (!Serial && millis() < 3000);

    Serial.println(F("================================="));
    Serial.println(F("Guitar Hero Controller Test"));
    Serial.println(F("================================="));
    Serial.println(F("Starting USB Host..."));

    myusb.begin();

    Serial.println(F("Waiting for controller..."));
    Serial.println(F("Expected VIDs: 0x1430 (RedOctane), 0x045E (Microsoft)"));
    Serial.println(F("Expected PIDs: 0x4748, 0x474C, 0x028E"));
    Serial.println();
}

void loop() {
    myusb.Task();

    // Check for connected devices
    for (uint8_t i = 0; i < CNT_DEVICES; i++) {
        if (*drivers[i] != driver_active[i]) {
            if (*drivers[i]) {
                Serial.print(F("*** Device "));
                Serial.print(i);
                Serial.print(F(" - VID: 0x"));
                Serial.print(drivers[i]->idVendor(), HEX);
                Serial.print(F(" PID: 0x"));
                Serial.print(drivers[i]->idProduct(), HEX);

                // Check if it's a Guitar Hero controller
                uint16_t vid = drivers[i]->idVendor();
                uint16_t pid = drivers[i]->idProduct();

                if ((vid == 0x1430 && (pid == 0x4748 || pid == 0x474C)) ||
                    (vid == 0x045E && pid == 0x028E)) {
                    Serial.println(F(" - Guitar Hero Controller DETECTED!"));
                    controllerFound = true;
                    vendorID = vid;
                    productID = pid;
                    digitalWrite(LED_PIN, HIGH);  // Solid LED
                } else {
                    Serial.println(F(" - Not a Guitar Hero controller"));
                }
            } else {
                Serial.print(F("*** Device "));
                Serial.print(i);
                Serial.println(F(" - disconnected"));
                if (controllerFound) {
                    controllerFound = false;
                    Serial.println(F("Guitar Hero Controller disconnected"));
                }
            }
            driver_active[i] = *drivers[i];
        }
    }

    // Blink LED if no controller found
    if (!controllerFound && ledTimer >= 500) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        ledTimer = 0;
    }

    // Print status every 5 seconds
    static elapsedMillis statusTimer;
    if (statusTimer >= 5000) {
        if (controllerFound) {
            Serial.print(F("Controller connected: VID=0x"));
            Serial.print(vendorID, HEX);
            Serial.print(F(" PID=0x"));
            Serial.println(productID, HEX);
        } else {
            Serial.println(F("No controller detected..."));
        }
        statusTimer = 0;
    }
}