#include <SoftwareSerial.h>
#include <ArduinoJson.h>

/* This file is for use on the mock Edge Auto Switch Demo.  *
 * This is NOT compatible with the PCB Edge Auto Switch.    */

const byte numChars = 175;
char received[numChars];   // an array to store the received data
StaticJsonDocument<numChars> settings;


/* Bluetooth Module */
SoftwareSerial bt(11, 10);
const int state = 9, en = 12;



enum TEST { BINARY_COUNT, WALKING_1, WALKING_0 };

/* Setup variables for main loop */

int monitors[5] = {2, 3, 4, 5, 6};

const bool is_HDMI[5] = {false, false, false, false, true};

/* Default test parameters */
TEST mode = BINARY_COUNT;
int num_monitors = 5,
    num_cycles = 4,
    max = pow(2, num_monitors) - 1; // Pattern where all monitors are on

double 
    low_dwell_time = 3 * 1000,      // 10 seconds default
    high_dwell_time = 3 * 1000;    // 30 seconds default
    
bool received_settings = false,
     testing           = false;

void setup() {
    /* Initialize Communications */
    Serial.begin(9600);
    Serial.println("Started");
    bt.begin(9600);
    bt.println("Ready!!!");

    /* Initialize pins */
    pinMode(state, INPUT);
    pinMode(en, OUTPUT);
    digitalWrite(en, LOW);
    for (int j = 0; j < 5; j++)
        pinMode(monitors[j], OUTPUT);
    all_on();
}

void loop() {
    
    // Wait for bt connection
    if (!received_settings) {
        recvWithEndMarker();    // Check if there's data.
        showreceived_settings();
        
        if (received_settings) {      // If there's data, deserialize
            DeserializationError err = deserializeJson(settings, received);
            if (err) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(err.f_str());
            } else {
                num_monitors = settings["num_monitors"];
                low_dwell_time = settings["low_dwell"];
                high_dwell_time = settings["high_dwell"];
            }
            delay(100);
        }
        
        
        
    }






        /*
        // Parse settings JSON
        
        // Check json was parsed successfully, abort if not.
        if (err) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(err.f_str());
        } else {
            num_monitors = settings["num_monitors"];
            low_dwell_time = settings["low_dwell"];
            high_dwell_time = settings["high_dwell"];
            // Remake monitor array

        }
        */
        
       

   
    // Wait for start signal from bt




    /* Testing Logic */
    
    else if (testing) {
        for (int cycle = 0; cycle < num_cycles; cycle++) {
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
}

// Test a pattern and go to high_dwell afterwards
void test_pattern(byte pattern) {
    // Low Dwell
    if (pattern != max) {
        for (int i = 0; i < num_monitors; i++) {
            if (is_HDMI[i])
                digitalWrite(monitors[i], !bitRead(pattern, i));
            else
                digitalWrite(monitors[i], bitRead(pattern, i));
        }
        delay(low_dwell_time);
    }
    // High Dwell
    all_on();
    delay(high_dwell_time);
}

/* Utility */
/* Convert decimal number string to binary number string */
String byte_to_bits(byte pattern) {
    String p = "";
    for (int i = 0; i < num_monitors; i++)
        p += bitRead(pattern, i);
    return p;
}

double round_to(double val, int precision) {
    int c = 10 * precision;
    return (roundf(val * c) / c);
}

/* Sets all monitors being tested to on */
void all_on() {
    for (int i = 0; i < num_monitors; i++) {
        if (is_HDMI[i])
            digitalWrite(monitors[i], LOW);
        else
            digitalWrite(monitors[i], HIGH);
    }
}

/* Sets all monitors being tested to off */
void all_off() {
    for (int i = 0; i < num_monitors; i++) {
        if (is_HDMI[i])
            digitalWrite(monitors[i], HIGH);
        else
            digitalWrite(monitors[i], LOW);
    }
}

/* Bluetooth functions */
void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '#';
    char rc;
    
    while (bt.available() > 0 && received_settings == false) {
        rc = bt.read();

        if (rc != endMarker) {
            received[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            received[ndx] = '\0'; // terminate the string
            ndx = 0;
            received_settings = true;
        }
    }
}

void showreceived_settings() {
    if (received_settings == true) {
        Serial.print("This just in ... ");
        Serial.println(received);
        received_settings = false;
    }
}
