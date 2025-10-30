/**
 * Guitar Hero Controller USB HID Driver
 * For Xbox 360 Wireless Guitar Hero Controller
 *
 * Handles USB enumeration, HID report parsing, and control mapping
 */

#ifndef GH_CONTROLLER_H
#define GH_CONTROLLER_H

#include <USBHost_t36.h>

// Xbox 360 Guitar Hero Controller USB IDs
#define XBOX360_VID 0x1430  // RedOctane (Guitar Hero)
#define XBOX360_PID_GH_GUITAR 0x4748  // Guitar Hero guitar
#define XBOX360_PID_GH_XPLORER 0x474C  // X-plorer guitar

// HID Report structure for Xbox 360 Guitar Hero controller
struct GHControllerState {
    // Fret buttons
    bool greenFret;   // A button
    bool redFret;     // B button
    bool yellowFret;  // Y button
    bool blueFret;    // X button
    bool orangeFret;  // LB button

    // Strum bar
    bool strumUp;     // D-pad up
    bool strumDown;   // D-pad down

    // Control buttons
    bool starPower;   // Back button
    bool plusButton;  // Start button
    bool minusButton; // Xbox button

    // D-pad (for navigation)
    bool dpadUp;
    bool dpadDown;
    bool dpadLeft;
    bool dpadRight;

    // Analog controls
    uint8_t whammyBar;       // 0-255 (0 = not pressed, 255 = fully pressed)
    uint8_t pickupSelector;  // 0-2 (3 positions)
    int16_t tiltX;          // -32768 to 32767 (accelerometer X)
    int16_t tiltY;          // -32768 to 32767 (accelerometer Y)

    // Raw button state for debugging
    uint16_t buttonsRaw;
};

class GuitarHeroController : public USBHIDInput {
public:
    GuitarHeroController(USBHost &host) : myHost(&host) { init(); }

    // USBHIDInput virtual function overrides
    virtual hidclaim_t claim_collection(USBHIDParser *driver, Device_t *dev, uint32_t topusage);
    virtual bool hid_process_in_data(const Transfer_t *transfer);
    virtual bool hid_process_out_data(const Transfer_t *transfer);
    virtual void hid_input_begin(uint32_t topusage, uint32_t type, int lgmin, int lgmax);
    virtual void hid_input_data(uint32_t usage, int32_t value);
    virtual void hid_input_end();
    virtual void disconnect_collection(Device_t *dev);

    // Public interface
    bool connected() const { return isConnected; }
    GHControllerState getState() const { return state; }
    void rumble(uint8_t leftMotor, uint8_t rightMotor);
    const char* getControllerName() const { return controllerName; }
    uint16_t getVendorID() const { return vendorID; }
    uint16_t getProductID() const { return productID; }

    // Debug functions
    void printRawReport(const uint8_t* data, uint16_t len);
    void printState();

protected:
    void init();
    bool parseHIDReport(const uint8_t* data, uint16_t len);
    void updateButtonState(uint16_t buttons);
    void updateAnalogState(const uint8_t* data);

private:
    USBHost* myHost;
    USBHIDParser* driver;
    Device_t* device;

    bool isConnected;
    GHControllerState state;
    GHControllerState previousState;

    // Device information
    uint16_t vendorID;
    uint16_t productID;
    char controllerName[64];

    // HID report parsing
    uint8_t reportBuffer[64];
    uint16_t reportLength;
    bool reportAvailable;

    // Rumble output report
    uint8_t rumbleData[8];

    // Button mapping (Xbox 360 format)
    static const uint16_t BUTTON_GREEN = 0x0002;   // A
    static const uint16_t BUTTON_RED = 0x0004;     // B
    static const uint16_t BUTTON_YELLOW = 0x0008;  // Y
    static const uint16_t BUTTON_BLUE = 0x0001;    // X
    static const uint16_t BUTTON_ORANGE = 0x0100;  // LB
    static const uint16_t BUTTON_STAR = 0x0020;    // Back
    static const uint16_t BUTTON_PLUS = 0x0010;    // Start
    static const uint16_t BUTTON_MINUS = 0x0040;   // Xbox Guide
    static const uint16_t DPAD_UP = 0x0001;
    static const uint16_t DPAD_DOWN = 0x0002;
    static const uint16_t DPAD_LEFT = 0x0004;
    static const uint16_t DPAD_RIGHT = 0x0008;
};

#endif // GH_CONTROLLER_H