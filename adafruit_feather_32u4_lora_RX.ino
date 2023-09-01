// taken from https://learn.adafruit.com/adafruit-feather-32u4-radio-with-lora-radio-module?view=all#receiver-example-code-2567689

// RX
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS    8
#define RFM95_INT   7
#define RFM95_RST   4
#define BUTTON_PIN  A0  // Pin for the button

#define RF95_FREQ 915.0

LiquidCrystal_I2C lcd(0x27, 16, 2);

const unsigned long loopDuration = 500; // 5 seconds in milliseconds
const unsigned long backlightToggleInterval = 100; // 100 milliseconds

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.begin(115200);
  while (!Serial) delay(1);
  delay(100);

  lcd.init();
  lcd.backlight();
  lcd.print("Feather LoRa RX");
  lcd.setCursor(0, 1);
  lcd.print("RSSI: - dBm");

  if (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  rf95.setTxPower(23, false);
}

void loop() {
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      digitalWrite(LED_BUILTIN, HIGH);
      RH_RF95::printBuffer("Received: ", buf, len);
      Serial.print("Got: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

      // Update LCD with RSSI value
      lcd.setCursor(6, 1);
      lcd.print("      ");  // Clear previous value
      lcd.setCursor(6, 1);
      lcd.print(rf95.lastRssi(), DEC);
      
      // Update LCD with "Hello World" index
      lcd.setCursor(0, 0);
      lcd.println((char*)buf);

      // Send a reply
      uint8_t data[] = "And hello back to you";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Sent a reply");
      digitalWrite(LED_BUILTIN, LOW);
      unsigned long loopStartTime = millis();
      while (millis() - loopStartTime < loopDuration) {
        lcd.backlight(); // Turn on the backlight
        delay(backlightToggleInterval);
        lcd.noBacklight(); // Turn off the backlight
        delay(backlightToggleInterval);
      }
      lcd.backlight();
    } else {
      Serial.println("Receive failed");
    }
  }
}
