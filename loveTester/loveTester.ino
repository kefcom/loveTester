#include <WS2812FX.h>



#define LED_COUNT 10
#define LED_PIN D5



int minVal = 2;
int maxVal = 100;
int readings = 20;
int resultTimeOut = 5000;
int baselineReadings = 20;
int currentReading = 0;
int totalReading = 0;
const long delayTime = 200;
unsigned long previousMillis = 0;
int ledsOn = 0;
int state = 0; //0: gather baseline data, 1: show idle animation, 2: show measure animation and measure, 3: show measurement

int animSpeed = 3000;
int startupAnim = 0;
int idleAnim = 55;
int measureAnim = 13;
int resultAnim = 0;

int inVal;
int perC;

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(A0, INPUT);
  Serial.begin(9600);
  Serial.println("Startup");
  ws2812fx.init();
  ws2812fx.setBrightness(100);
  ws2812fx.setSpeed(animSpeed);
  ws2812fx.setColor(0xff0000);
  ws2812fx.setMode(startupAnim);
  ws2812fx.start();
}

void loop() {
  ws2812fx.service();
  unsigned long currentMillis = millis();

  switch(state)
  {
    case 0:  
      //gather a baseline
      if (currentMillis - previousMillis >= delayTime) {
        previousMillis = currentMillis;      
        //do readings until baseline is established
        if(currentReading < baselineReadings)
        {
          inVal = analogRead(A0);
          perC = map(inVal, minVal, maxVal, 0,100);
          totalReading = totalReading + perC;
          currentReading++;
        }else{
          //baseline is established, move to state 1
          minVal = (totalReading / baselineReadings);
          Serial.print("Baseline: ");
          Serial.print(minVal);
          Serial.println("%");
          ws2812fx.setSegment(0, 0, LED_COUNT-1, idleAnim , RED, animSpeed, false);
          currentReading = 0;
          perC = 0;
          totalReading = 0;
          state = 1;
        }
      }
    break;
    case 1:
      //check if measured value > baseline
      inVal = analogRead(A0);
      perC = map(inVal, minVal, maxVal, 0,100);
      if(perC > (minVal+1))
      {
        Serial.println("Higher value detected, measuring");
        // show measuring animation?
        ws2812fx.setSegment(0, 0, LED_COUNT-1, measureAnim , RED, animSpeed, false);
        ws2812fx.setSegment(1, LED_COUNT+1, 1, FX_MODE_STATIC , BLACK, 1, false); //move segment 1 outside of active LEDs
        currentReading = 0;
        perC = 0;
        totalReading = 0;
        state = 2;
      }
    break;
    case 2:
      //measure
      if (currentMillis - previousMillis >= delayTime) 
      {
        previousMillis = currentMillis;
        if(currentReading < readings)
        {
          inVal = analogRead(A0);
          perC = map(inVal, minVal, maxVal, 0,100);
          totalReading = totalReading + perC;
          currentReading++;
        }
        else
        {
          //compile average, set LEDs and move state
          perC = (totalReading / readings);
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
            ws2812fx.setSegment(0, 0, ledsOn-1, resultAnim , RED, 1, false);
            if(ledsOn < LED_COUNT)
            {
              //all other leds off:
              ws2812fx.setSegment(1, ledsOn, LED_COUNT-1, FX_MODE_STATIC , BLACK, 1, false);
            }else{
              //all leds on:
              ws2812fx.setSegment(1, 0, LED_COUNT-1, resultAnim , RED, 1, false);
            }
          }else{
            //result is 0 leds on, but we want to show at least 1
            ws2812fx.setSegment(0, 0, 0, FX_MODE_STATIC , RED, 1, false);
            ws2812fx.setSegment(1, 1, LED_COUNT-1, FX_MODE_STATIC , BLACK, 1, false);
          }
          currentReading = 0;
          perC = 0;
          totalReading = 0;
          state = 3;
        }
      }
    break;
    case 3:
      if (currentMillis - previousMillis >= resultTimeOut) 
      {
        previousMillis = currentMillis;  
        //move state back to 1
        Serial.println("moving back to state 1");
        ws2812fx.setSegment(0, 0, LED_COUNT-1, idleAnim , RED, animSpeed, false);
        ws2812fx.setSegment(1, LED_COUNT+1, 1, FX_MODE_STATIC , BLACK, 1, false); //move segment 1 outside of active LEDs
        currentReading = 0;
        perC = 0;
        totalReading = 0;
        state = 1;
      }
    break;
  }
}
