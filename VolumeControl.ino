#include "VolumeControl.h"


#define PIN_SENSOR_CATHODE          1  
#define PIN_SENSOR_ANODE            2
#define PIN_LED                     1

#define MAX_LIGHT_LEVEL             50000
#define AVERAGE_SAMPLE_SIZE         50
#define LIGHT_DIFFERENCE_THRESHOLD  0.2

void blink(byte n);
float measureLight(void);

float averageValue;

void setup() {
    averageValue = 0;
    pinMode(PIN_LED, OUTPUT);
    blink(3);
}

/* Blink the LED */
void blink(byte n) {

    // Make sure the sensor LED is won't turn on through pin 2.
    pinMode(PIN_SENSOR_ANODE, OUTPUT);
    digitalWrite(PIN_SENSOR_ANODE, HIGH);

    while (n > 0) {
        digitalWrite(PIN_LED, HIGH);
        DigiVolumeControl.delay(50);
        digitalWrite(PIN_LED, LOW);
        DigiVolumeControl.delay(50);
        n--;
    }
}

/* Return the light intensity as a float between 0.0 and 1.0 */
float measureLight(void) {
    unsigned int n;
    
    // Set N to the Max light level. The loop will not measure beyond this value
    n = MAX_LIGHT_LEVEL;
    
    // Set both Anode and Cathode of the LED to OUTPUT
    pinMode(PIN_SENSOR_CATHODE, OUTPUT);
    pinMode(PIN_SENSOR_ANODE, OUTPUT);
    
    // Reverse bias the LED to charge up the junction
    digitalWrite(PIN_SENSOR_CATHODE, LOW);
    digitalWrite(PIN_SENSOR_ANODE, HIGH);
    
    // Switch the anode to an input, and turn off the internal pullup resistor
    // The LED will now discharge slowly through the micro. The more light that
    // lands on the PN junction, the quicker the discharge.
    pinMode(PIN_SENSOR_ANODE, INPUT);
    digitalWrite(PIN_SENSOR_ANODE, LOW);
 
    // Poll the digital value of the anode pin, once below the input threshold
    // voltage ~0.5V it will read 0. To prevent looping forever, n is set to the
    // maximum light level first, and the loop will end once n == 0.
    while(n) {
        if (digitalRead(PIN_SENSOR_ANODE) == 0) {
            break;
        }
        n--;    
    }
    // Return the value between 0.0 (dark) and 1.0 (light)
    return (float)((float)(n) / (float)(MAX_LIGHT_LEVEL));
}

void loop() {
    float currentValue;

    // Measue the light level, between 0.0 (dark) and 1.0 (light)
    currentValue = measureLight();

    // Is there big difference from current measurement to the average measurement
    if (averageValue - currentValue > LIGHT_DIFFERENCE_THRESHOLD) {
        blink(1);
        // Send zero to make sure the USB bus is awake
        DigiVolumeControl.sendData(0);
        // Mute the PC using the HID interface
        DigiVolumeControl.sendData(VOLUME_MUTE);
        // Reset the averageValue so that it does not trigger again straight away.
        averageValue = currentValue;
    }
    else
    {
        // Calculate the next averageValue by adding a percentage of the difference
        averageValue = averageValue + ((currentValue - averageValue) / (float)(AVERAGE_SAMPLE_SIZE));
    }
}
