#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

/* This file is for use on the mock Edge Auto Switch Demo.  *
 * This is NOT compatible with the PCB Edge Auto Switch.    */


enum TEST { BINARY_COUNT, WALKING_1, WALKING_0 };

/* Setup variables for main loop */
const int monitors[5] = {2, 3, 4, 5, 6};
const bool is_HDMI[5] = {false, false, false, false, true};
const int start = 7;

/* LCD Related Variables */
const int rs = A5, en = A4, d4 = A3, d5 = A2, d6 = A1, d7 = A0;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);



/* Default test parameters */
TEST mode           = BINARY_COUNT;
int num_monitors    = 5,
    num_cycles      = 4,
    low_dwell_time  = 3 * 1000,     // 10 seconds default
    high_dwell_time = 3 * 1000,     // 30 seconds default
    max = pow(2, num_monitors) - 1; // Pattern where all monitors are on

void setup() {
    /* Initialize modules */
    lcd.begin(16, 2);
    Serial.begin(9600);
    
    /* Initialize pins */
    pinMode(start, INPUT);
    for (int i = 0; i < 5; i++)
        pinMode(monitors[i], OUTPUT);

}

void loop() {

    all_on();
    lcd.clear();
    print_status("Waiting");

    // Wait for button push
    while (digitalRead(start) == LOW){}
    lcd.clear();

    /* Testing Logic */
    for (int cycle = 0; cycle < num_cycles; cycle++) {
        print_cycle_info(cycle + 1, num_cycles);
        switch (mode) {
            case BINARY_COUNT:
                for (byte pattern = 0; pattern < max; pattern++)
                    test_pattern(pattern);
                break;
            case WALKING_1:
                for (byte pattern = 1; pattern < max; pattern *= 2)
                    test_pattern(pattern);
                break;
            case WALKING_0:
                for (int delta = 1; delta <= max; delta *= 2)
                    test_pattern(max - delta); // Subtract the binary value of the monitor, if second monitor is off: base (11111) - 2^off_monitor (2) = 11101
                break;
        }
    }
}

/* Monitor switch related functions  */
void test_pattern(byte pattern) {
    low_dwell(pattern);
    high_dwell();
}
/* Sets monitors to specific testing pattern for specified low_dwell_time */
void low_dwell(byte pattern) {
    if (pattern == max) 
        return;
    print_status("Low Dwell");
    print_pattern(pattern);
    for (int i = 0; i < num_monitors; i++) {
        if (is_HDMI[i]) {
            digitalWrite(monitors[i], !bitRead(pattern, i));
        }
        else {
            digitalWrite(monitors[i], bitRead(pattern, i));
        }
    }
    delay(low_dwell_time);
}
/* Sets all monitors to on for specified high_dwell_time */
void high_dwell() {
    print_status("High Dwell");
    print_pattern(byte(max));
    all_on();
    delay(high_dwell_time);
}

/* LCD Utility functions */
/* Writes the status to the top right of the LCD Screen */
void print_status(String status)
{
    String msg = status;
    lcd.setCursor(6, 0);
    while (msg.length() < 10)
    {
        msg += " ";
    } // Ensure any left over characters are overwritten
    lcd.print(msg);
}
void print_pattern(byte pattern)
{
    String labels = "";
    for (int i = 0; i < num_monitors; i++)
    {
        labels += i + 1;
    }
    lcd.setCursor(0, 0);
    lcd.print(labels);
    lcd.setCursor(0, 1);
    lcd.print(byte_to_bits(pattern));
}
void print_cycle_info(int num, int denom)
{
    lcd.setCursor(6, 1);
    lcd.print(String(num) + "/" + String(denom));
}

/* Utility */
String byte_to_bits(byte pattern)
{
    String p = "";
    for (int i = 0; i < num_monitors; i++)
    {
        p += bitRead(pattern, i);
    }
    return p;
}

/* Sets all monitors being tested to on */
void all_on() {
    for (int i = 0; i < num_monitors; i++) {
        if (is_HDMI[i]) {
            digitalWrite(monitors[i], LOW);
        } else {
            digitalWrite(monitors[i], HIGH);
        }
    }
}

/* Sets all monitors being tested to off */
void all_off() {
    for (int i = 0; i < num_monitors; i++) {
        if (is_HDMI[i]) {
            digitalWrite(monitors[i], HIGH);
        } else {
            digitalWrite(monitors[i], LOW);
        }
    }
}