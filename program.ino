#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include <RH_ASK.h>
#include <SPI.h>

//par FS
RH_ASK transmitter;
RH_ASK receiver;
bool Transmit = false;

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
    pinMode(3, INPUT);
    pinMode(4, INPUT);
    pinMode(5, INPUT);
    
    Serial.begin(9600); // Debugging only
    if (!transmitter.init())
        Serial.println("Transmitter Init failed");
    if (!receiver.init())
        Serial.println("Receiver Init failed");

    lcd.init();
    lcd.backlight();
    lcd.cursor();
    lcd.setCursor(0,0);
    lcd.print("0/0/0:A");
    lcd.setCursor(0,0);
}

int chk = 0;

int cntL = 0;
int cntM = 0;
int cntR = 0;

int noiseLim = 4;
int varchoice = 0;
int varL = 0;
int varM = 0;
int varR = 0;
bool interacted = false;

char charset[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
              '(', ')', '[', ']', '{', '}', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '#', '+' , '-', '*', '/', '=',
              '?', '!', ' ', '-', '_', ';', ':', ',', '&', '"', '\'', '^', '%', 's', 's', 's'};

char msg[41];
int msgpos = 0;

int cntRecv = 0;
int cntTran = 0;
int RadioLimit = 100;

int timestamp = 0;

void loop() {
    timestamp++;
    cntRecv++;
    cntTran++;
    
    interacted = false;
    int LeftInput = digitalRead(5);
    int MidInput = digitalRead(4);
    int RightInput = digitalRead(3);
    
    bool Left = false;
    bool Mid = false;
    bool Right = false;
    
    if (LeftInput == HIGH)
      cntL++;
    else
      cntL = 0;
    if (MidInput == HIGH)
      cntM++;
    else
      cntM = 0;
    if (RightInput == HIGH)
      cntR++;
    else
      cntR = 0;

    if (cntL == noiseLim) {
      Left = true;
      interacted = true;
    }

    if (cntM == noiseLim) {
      Mid = true;
      interacted = true;
    }
    
    if (cntR == noiseLim) {
      Right = true;
      interacted = true;
    }

    if (interacted) {
      lcd.cursor();
      lcd.clear();
      if (Left) {
        if (varchoice == 0) {
          varL++;
          if (varL == 4)
            varL = 0;
        } else if (varchoice == 1) {
          varM++;
          if (varM == 4)
            varM = 0;
        } else if (varchoice == 2) {
          varR++;
          if (varR == 4)
            varR = 0;
        }
      }
      
      if (Mid) {
        varchoice++;
        if (varchoice == 3)
          varchoice = 0;
      }

      int value = varL * 16 + varM * 4 + varR;
  
      if (Right) {
          if (value == 62) {
            msg[msgpos - 1] = '\0';
            msgpos--;
            
            if (msgpos == -1)
              msgpos = 0;
          } else if (value == 61) {
            for (int i = msgpos - 1; i >= 0; i--)
              msg[i] = '\0';
            msgpos = 0;
          } else if (value == 63) {
            if (msgpos > 0) {
              transmitter.send((uint8_t *)msg, strlen(msg));
              transmitter.waitPacketSent();
            }
          } else if (msgpos < 41)
            msg[msgpos++] = charset[value];
      }
      
      lcd.setCursor(0, 0);
      
      lcd.print(varL);
      lcd.print("/");
      lcd.print(varM);
      lcd.print("/");
      lcd.print(varR);
      lcd.print(':');

      if (value == 63)
        lcd.print("ENVOYER MSG.");
      else if (value == 62)
        lcd.print("EFFACER 1 CHR");
      else if (value == 61)
        lcd.print("EFFACER TOUT.");
      else {
        if (msgpos == 41) {
          lcd.print ("40 CHR. LIMIT");
        } else {
            lcd.print(charset[value]);
        }
      }
      
      lcd.setCursor(0, 1);
      if (msgpos >= 20)
        lcd.print (msg + (msgpos - 20));
      else
        lcd.print(msg);
        
      lcd.setCursor(varchoice * 2, 0);
    }
    
    if (cntRecv == RadioLimit) {
        cntRecv = 0;
        uint8_t buf[41];
        uint8_t buflen = sizeof(buf);
        if (receiver.recv(buf, &buflen)) { // Non-blocking
          
            lcd.noCursor();
            int i;
            lcd.setCursor(0, 2);
            char tempbuf[41];
            
            for (int i = 0; i < buflen; i++) {
              if (buf[i] == 's')
                break;
              tempbuf[i] = buf[i];
              tempbuf[i+1] = '\0';
            }
            
            for (int i = buflen; i < 40; i++) {
              tempbuf[i] = ' ';
            }
            tempbuf[40] = '\0';
            lcd.print((char*) tempbuf);
        }
    }

    delay(10);
}
