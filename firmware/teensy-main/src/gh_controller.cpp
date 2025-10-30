/**
 * Guitar Hero Controller USB HID Driver Implementation
 */

#include "gh_controller.h"
#include <Arduino.h>

void GuitarHeroController::init() {
    isConnected = false;
    driver = nullptr;
    device = nullptr;
    vendorID = 0;
    productID = 0;
    reportAvailable = false;
    reportLength = 0;

    memset(&state, 0, sizeof(state));
    memset(&previousState, 0, sizeof(previousState));
    memset(controllerName, 0, sizeof(controllerName));
    memset(reportBuffer, 0, sizeof(reportBuffer));
    memset(rumbleData, 0, sizeof(rumbleData));

    // Initialize rumble packet structure
    rumbleData[0] = 0x00;  // Report ID
    rumbleData[1] = 0x08;  // Report size
    rumbleData[2] = 0x00;  // Left motor
    rumbleData[3] = 0x00;  // Right motor
}

hidclaim_t GuitarHeroController::claim_collection(USBHIDParser *driver, Device_t *dev, uint32_t topusage) {
    // Check if this is a Guitar Hero controller
    if (dev->idVendor == XBOX360_VID &&
        (dev->idProduct == XBOX360_PID_GH_GUITAR || dev->idProduct == XBOX360_PID_GH_XPLORER)) {

        Serial.print(F("Guitar Hero Controller detected! VID: 0x"));
        Serial.print(dev->idVendor, HEX);
        Serial.print(F(" PID: 0x"));
        Serial.println(dev->idProduct, HEX);

        this->driver = driver;
        this->device = dev;
        this->vendorID = dev->idVendor;
        this->productID = dev->idProduct;

        // Set controller name based on PID
        if (dev->idProduct == XBOX360_PID_GH_GUITAR) {
            strcpy(controllerName, "Guitar Hero Guitar");
        } else if (dev->idProduct == XBOX360_PID_GH_XPLORER) {
            strcpy(controllerName, "Guitar Hero X-plorer");
        }

        isConnected = true;
        return CLAIM_INTERFACE;  // Claim this device
    }

    // Also check for generic Xbox 360 controllers that might be GH guitars
    if (dev->idVendor == 0x045E && dev->idProduct == 0x028E) {  // Microsoft Xbox 360 controller
        // Could be a Guitar Hero controller in Xbox mode
        Serial.println(F("Xbox 360 controller detected - checking if Guitar Hero..."));
        this->driver = driver;
        this->device = dev;
        this->vendorID = dev->idVendor;
        this->productID = dev->idProduct;
        strcpy(controllerName, "Xbox 360 Controller (Possible GH)");
        isConnected = true;
        return CLAIM_INTERFACE;
    }

    return CLAIM_NO;  // Don't claim this device
}

bool GuitarHeroController::hid_process_in_data(const Transfer_t *transfer) {
    if (!transfer || !transfer->buffer) return false;

    uint16_t len = transfer->length;
    if (len > sizeof(reportBuffer)) len = sizeof(reportBuffer);

    // Copy the HID report data
    memcpy(reportBuffer, transfer->buffer, len);
    reportLength = len;
    reportAvailable = true;

    // Parse the HID report
    return parseHIDReport(reportBuffer, reportLength);
}

bool GuitarHeroController::hid_process_out_data(const Transfer_t *transfer) {
    // Handle output reports (rumble, LEDs, etc.)
    return true;
}

void GuitarHeroController::hid_input_begin(uint32_t topusage, uint32_t type, int lgmin, int lgmax) {
    // Called at the beginning of HID input processing
}

void GuitarHeroController::hid_input_data(uint32_t usage, int32_t value) {
    // Process individual HID usage values
    // For Guitar Hero controllers, we mainly use the raw report parsing
}

void GuitarHeroController::hid_input_end() {
    // Called at the end of HID input processing
}

void GuitarHeroController::disconnect_collection(Device_t *dev) {
    if (dev == device) {
        Serial.println(F("Guitar Hero Controller disconnected"));
        isConnected = false;
        device = nullptr;
        driver = nullptr;
        memset(&state, 0, sizeof(state));
    }
}

bool GuitarHeroController::parseHIDReport(const uint8_t* data, uint16_t len) {
    // Xbox 360 controller HID report format
    // Byte 0: Report ID (usually 0x00)
    // Byte 1: Report length
    // Bytes 2-3: Button state (16 bits)
    // Byte 4: Left trigger
    // Byte 5: Right trigger
    // Bytes 6-7: Left stick X (signed 16-bit)
    // Bytes 8-9: Left stick Y (signed 16-bit)
    // Bytes 10-11: Right stick X (signed 16-bit)
    // Bytes 12-13: Right stick Y (signed 16-bit)

    if (len < 14) return false;  // Not enough data

    // Save previous state for edge detection
    previousState = state;

    // Parse button states (bytes 2-3)
    uint16_t buttons = (data[3] << 8) | data[2];
    updateButtonState(buttons);

    // Parse analog controls
    updateAnalogState(data);

    // Store raw button state for debugging
    state.buttonsRaw = buttons;

    return true;
}

void GuitarHeroController::updateButtonState(uint16_t buttons) {
    // Parse D-pad first (it's in the lower nibble of byte 2)
    uint8_t dpad = buttons & 0x0F;
    state.dpadUp = (dpad == DPAD_UP) || (dpad == (DPAD_UP | DPAD_LEFT)) || (dpad == (DPAD_UP | DPAD_RIGHT));
    state.dpadDown = (dpad == DPAD_DOWN) || (dpad == (DPAD_DOWN | DPAD_LEFT)) || (dpad == (DPAD_DOWN | DPAD_RIGHT));
    state.dpadLeft = (dpad == DPAD_LEFT) || (dpad == (DPAD_UP | DPAD_LEFT)) || (dpad == (DPAD_DOWN | DPAD_LEFT));
    state.dpadRight = (dpad == DPAD_RIGHT) || (dpad == (DPAD_UP | DPAD_RIGHT)) || (dpad == (DPAD_DOWN | DPAD_RIGHT));

    // Strum bar is mapped to D-pad up/down
    state.strumUp = state.dpadUp;
    state.strumDown = state.dpadDown;

    // Parse fret buttons
    state.greenFret = (buttons & BUTTON_GREEN) != 0;
    state.redFret = (buttons & BUTTON_RED) != 0;
    state.yellowFret = (buttons & BUTTON_YELLOW) != 0;
    state.blueFret = (buttons & BUTTON_BLUE) != 0;
    state.orangeFret = (buttons & BUTTON_ORANGE) != 0;

    // Parse control buttons
    state.starPower = (buttons & BUTTON_STAR) != 0;
    state.plusButton = (buttons & BUTTON_PLUS) != 0;
    state.minusButton = (buttons & BUTTON_MINUS) != 0;
}

void GuitarHeroController::updateAnalogState(const uint8_t* data) {
    // Whammy bar is typically mapped to right trigger (byte 5)
    state.whammyBar = data[5];

    // Pickup selector might be on left trigger or analog stick
    // This varies by controller model, may need adjustment
    state.pickupSelector = (data[4] > 170) ? 2 : (data[4] > 85) ? 1 : 0;

    // Tilt sensor (accelerometer) is typically mapped to analog sticks
    // Left stick X/Y for tilt
    state.tiltX = (int16_t)((data[7] << 8) | data[6]);
    state.tiltY = (int16_t)((data[9] << 8) | data[8]);
}

void GuitarHeroController::rumble(uint8_t leftMotor, uint8_t rightMotor) {
    if (!isConnected || !driver) return;

    rumbleData[2] = leftMotor;
    rumbleData[3] = rightMotor;

    // Send rumble command via USB
    // Note: Implementation depends on the specific USB Host library version
    // driver->sendControlPacket(rumbleData, sizeof(rumbleData));
}

void GuitarHeroController::printRawReport(const uint8_t* data, uint16_t len) {
    Serial.print(F("HID Report ("));
    Serial.print(len);
    Serial.print(F(" bytes): "));

    for (uint16_t i = 0; i < len; i++) {
        if (data[i] < 0x10) Serial.print('0');
        Serial.print(data[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
}

void GuitarHeroController::printState() {
    Serial.println(F("=== Guitar Hero Controller State ==="));
    Serial.print(F("Frets: "));
    if (state.greenFret) Serial.print(F("G "));
    if (state.redFret) Serial.print(F("R "));
    if (state.yellowFret) Serial.print(F("Y "));
    if (state.blueFret) Serial.print(F("B "));
    if (state.orangeFret) Serial.print(F("O "));
    Serial.println();

    Serial.print(F("Strum: "));
    if (state.strumUp) Serial.print(F("UP "));
    if (state.strumDown) Serial.print(F("DOWN "));
    Serial.println();

    Serial.print(F("Controls: "));
    if (state.starPower) Serial.print(F("STAR "));
    if (state.plusButton) Serial.print(F("PLUS "));
    if (state.minusButton) Serial.print(F("MINUS "));
    Serial.println();

    Serial.print(F("Whammy: "));
    Serial.print(state.whammyBar);
    Serial.print(F(" Pickup: "));
    Serial.print(state.pickupSelector);
    Serial.print(F(" Tilt X: "));
    Serial.print(state.tiltX);
    Serial.print(F(" Y: "));
    Serial.println(state.tiltY);

    Serial.print(F("Raw buttons: 0x"));
    Serial.println(state.buttonsRaw, HEX);
    Serial.println(F("==================================="));
}