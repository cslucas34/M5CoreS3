#include <Arduino.h>
#include <Wire.h>
#include <M5Unified.h>
#include <lvgl.h>
#include "ui.h"

// Change these to your proper screen resolution, they will be used later.
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

static lv_indev_drv_t indev_drv; // Correct type for the input device driver

// Change to 1 if you desire to log via serial port
#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf) {
    if (level <= LV_LOG_LEVEL_WARN) {
        Serial.println(buf);
    }
}
#endif

// Create a function to handle touch input and pass to LVGL
void my_touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    M5.update(); // Update M5Unified library to handle touch events
    // Uncomment lines 29-33 to troubleshoot touch issues with serial monitoring output
    // Serial.print("X: ");
    // Serial.print(M5.Touch.getTouchPointRaw(0).x);
    // Serial.print(", Y: ");
    // Serial.print(M5.Touch.getTouchPointRaw(0).y);
    // Serial.println();

    if (M5.Touch.getTouchPointRaw(0).id == 0xFFFFFFFF) {
        // No touch detected
        data->state = LV_INDEV_STATE_REL; // Set the touch state to released
    } else {
        // Touch is detected
        data->point.x = M5.Touch.getTouchPointRaw(0).x;
        data->point.y = M5.Touch.getTouchPointRaw(0).y;
        data->state = LV_INDEV_STATE_PR; // Set the touch state to pressed
    }
}

// Create the display flushing function
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    M5.Lcd.startWrite();
    M5.Lcd.setAddrWindow(area->x1, area->y1, w, h);
    M5.Lcd.pushColors((uint16_t *)color_p, w * h, true);
    M5.Lcd.endWrite();

    lv_disp_flush_ready(disp_drv);
}

// This is just a simple test to inject for troubleshooting purposes
void m5test() {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.delay(500);
    M5.Lcd.fillScreen(TFT_BLUE);
    M5.delay(500);
    M5.Lcd.fillScreen(TFT_BROWN);
    M5.delay(500);
    M5.Lcd.fillScreen(TFT_DARKGREEN);
    M5.delay(500);
    M5.Lcd.fillScreen(TFT_WHITE);
    M5.delay(500);
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setCursor(0, 120);
    M5.Lcd.setTextSize(1.5);
    M5.Lcd.print("Testing M5 Unified Basic Functions");
    M5.delay(2000);
    M5.Lcd.clearDisplay();
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.delay(1000);
    M5.Lcd.setCursor(40, 120);
    M5.Lcd.setTextSize(1.5);
    M5.Lcd.print("M5 Unified working");
    M5.Lcd.setCursor(40,160);
    M5.Lcd.print("Loading SETUP");
    M5.delay(3000);
}

void setup() {
    Serial.begin(115200); // Used for any serial debug required
    M5.begin();
    m5test();
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

    // Allocate the display driver structure dynamically
    lv_disp_drv_t *disp_drv = (lv_disp_drv_t *)malloc(sizeof(lv_disp_drv_t));
    if (!disp_drv) {
        // Handle memory allocation failure
        return;
    }

    // Initialize the display driver structure
    lv_disp_drv_init(disp_drv);

    // Set display resolution
    disp_drv->hor_res = screenWidth;
    disp_drv->ver_res = screenHeight;

    // Set callback functions
    disp_drv->flush_cb = my_disp_flush;
    disp_drv->draw_buf = &draw_buf;

    // Register the display driver
    lv_disp_drv_register(disp_drv);

    // Initialize I2C for the touch controller
    // Replace these pin numbers with the actual pin numbers you are using
    int sdaPin = 12; // Replace with your actual SDA pin
    int sclPin = 11; // Replace with your actual SCL pin

    Wire.begin(sdaPin, sclPin);

    // Initialize the Touch Controller
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read; // Replace with your touch input read function if different than what I made
    lv_indev_drv_register(&indev_drv);

    // Initialize the UI
    ui_init();
}

void loop() {
    M5.update();
    lv_task_handler();

    // Create an lv_indev_data_t structure to hold touch information
    lv_indev_data_t data;

    // Call the touch input read function
    my_touchpad_read(&indev_drv, &data);
}
