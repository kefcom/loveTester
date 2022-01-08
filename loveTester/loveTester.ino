#include <WS2812FX.h>



#define LED_COUNT 10
#define LED_PIN D5



int minVal = 2;
int maxVal = 100;
int readings = 20;
int currentReading = 0;
int totalReading = 0;
const long delayTime = 200;
unsigned long previousMillis = 0;
int ledsOn = 0;

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(A0, INPUT);
  Serial.begin(9600);
  Serial.println("Wait for reading...");
  ws2812fx.init();
  ws2812fx.setBrightness(30);
  ws2812fx.setSpeed(3000);
  ws2812fx.setColor(0xff0000);
  ws2812fx.setMode(43);
  ws2812fx.start();
}

void loop() {
  ws2812fx.service();
   unsigned long currentMillis = millis();


  if (currentMillis - previousMillis >= delayTime) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    
    if(currentReading < readings)
    {
      int inVal = analogRead(A0);
      int perC = map(inVal, minVal, maxVal, 0,100);
      totalReading = totalReading + perC;
      currentReading++;
    }
    else
    {
      int perC = (totalReading / readings);
      if (perC > 100)
      {
        perC = 100;
      }
      Serial.print("Average reading over ");
      Serial.print(readings);
      Serial.print(" readings: ");
      Serial.print(perC);
      Serial.print("% (");
      currentReading = 0;
      ledsOn = perC / 10;
      Serial.print(ledsOn);
      Serial.println(" leds)");
      if(ledsOn > 0)
      {
        //ledsOn leds ON
        // parameters: index, first, last, mode, color, speed, reverse
        ws2812fx.setSegment(0, 0, ledsOn-1, FX_MODE_STATIC , RED, 2000, false);
        if(ledsOn < LED_COUNT)
        {
          //all other leds off:
          ws2812fx.setSegment(1, ledsOn, LED_COUNT-1, FX_MODE_STATIC , BLACK, 2000, false);
        }else{
          //all leds on:
          ws2812fx.setSegment(1, 0, LED_COUNT-1, FX_MODE_STATIC , RED, 2000, false);
        }
      }else{
        //all leds off
        ws2812fx.setSegment(0, 0, LED_COUNT-1, FX_MODE_STATIC , BLACK, 2000, false);
        ws2812fx.setSegment(1, 0, LED_COUNT-1, FX_MODE_STATIC , BLACK, 2000, false);
      }


      
      ws2812fx.service();
      Serial.println("Wait for new reading...");
      totalReading = 0;
    }
  }
}
