//quick refs
//- https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html#pinout-and-design-files
//- https://www.raspberrypi.com/documentation/pico-sdk/hardware.html
//- https://github.com/hathach/tinyusb/tree/master/examples/device/hid_composite
//- https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid_device.h




#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "tusb.h"

//called when bs windows asks what this usb device can do
extern "C"
{
    uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance);
    uint8_t const *tud_descriptor_device_cb(void);
    uint8_t const *tud_descriptor_configuration_cb(uint8_t index);
    uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t language_id);
}




namespace
{
    //tell windows to treat the pico like a regular mouse
    uint8_t const report_descriptor[] = {TUD_HID_REPORT_DESC_MOUSE()};

    //usb labels that windows sees
    tusb_desc_device_t const device_descriptor = {
        sizeof(tusb_desc_device_t),
        TUSB_DESC_DEVICE,
        0x0200,
        0,
        0,
        0,
        CFG_TUD_ENDPOINT0_SIZE,
        0xCafe,
        0x4010,
        0x0100,
        1,
        2,
        0,
        1,
    };

    //one mouse interface
    uint8_t const configuration_descriptor[] = {
        TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
        TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_MOUSE, sizeof(report_descriptor), 0x81, CFG_TUD_HID_EP_BUFSIZE, 10),
    };

    int8_t axis_speed(unsigned raw, bool invert = false)
    {
        //0-4095 analog range
        int offset = static_cast<int>(raw) - 2048;
        if (invert) offset = -offset;

        //deadzone
        if (offset > -180 && offset < 180) return 0;

        int speed = offset / 180;
        if (speed > 10) speed = 10;
        if (speed < -10) speed = -10;
        return static_cast<int8_t>(speed);
    }
}




extern "C" uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    //tusb mouse report format
    return report_descriptor;
}

extern "C" uint8_t const *tud_descriptor_device_cb(void)
{
    //tusb details
    return reinterpret_cast<uint8_t const *>(&device_descriptor);
}

extern "C" uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    //tusb config
    return configuration_descriptor;
}

extern "C" uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t language_id)
{
    (void)language_id;
    static uint16_t string_descriptor[32];
    const char *text = "";
    if (index == 0)
    {
        string_descriptor[0] = (TUSB_DESC_STRING << 8) | 4;
        string_descriptor[1] = 0x0409;
        return string_descriptor;
    }
    if (index == 1) text = "raspberry pi";
    if (index == 2) text = "pico joystick mouse";
    //usb labels are sent as 16bit chars
    uint8_t length = 0;
    while (text[length] && length < 31)
    {
        string_descriptor[1 + length] = static_cast<uint8_t>(text[length]);
        ++length;
    }
    string_descriptor[0] = (TUSB_DESC_STRING << 8) | (2 * length + 2);
    return string_descriptor;
}




int main()
{
    adc_init();
    adc_gpio_init(26);  // vrx -> gp26 / adc0
    adc_gpio_init(27);  // vry -> gp27 / adc1
    gpio_init(14);      // sw -> gp14
    gpio_set_dir(14, GPIO_IN);
    gpio_pull_up(14);

    //start the usb device stack in mouse mode
    tusb_rhport_init_t usb_init = {};
    usb_init.role = TUSB_ROLE_DEVICE;
    usb_init.speed = TUSB_SPEED_AUTO;
    tusb_init(0, &usb_init);

    while (true)
    {
        tud_task();
        if (tud_mounted() && tud_hid_ready())
        {
            //each axis gives a value from 0 to 4095
            adc_select_input(0);
            const unsigned x = adc_read();
            adc_select_input(1);
            const unsigned y = adc_read();

            //bit 0 in a mouse report is the left button
            const uint8_t buttons = gpio_get(14) ? 0 : 1;

            //send button state and relative x/y
            tud_hid_mouse_report(0, buttons, axis_speed(x), axis_speed(y), 0, 0);
        }
        sleep_ms(10);
    }
}
