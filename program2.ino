#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include <RH_ASK.h>
#include <SPI.h>

RH_ASK transmitter;
RH_ASK receiver;
bool Transmit = true;

LiquidCrystal_I2C lcd(0x27, 20, 4);

char msg[128] = "A VERY LONG MESSAGE AAAAAAAAAAAAAAAAAAAAAAAAAA@";

void setup() {
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    Serial.begin(9600); // Debugging only
    if (!transmitter.init())
        Serial.println("Transmitter Init failed");
    if (!receiver.init())
        Serial.println("Receiver Init failed");

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("I'm Awake.");
}

int chk = 0;

void loop() {
    if (Transmit) {
        transmitter.send((uint8_t *)msg, strlen(msg));
        transmitter.waitPacketSent();
        Serial.println("message sent");
        Transmit = false;
    } else {
        uint8_t buf[128];
        uint8_t buflen = sizeof(buf);
        if (receiver.recv(buf, &buflen)) { // Non-blocking
            int i;
            Serial.print("Message: ");
            Serial.println((char*)buf);
            lcd.print((char*) buf);
        }
    }
}
