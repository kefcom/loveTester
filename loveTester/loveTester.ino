#include <WS2812FX.h> //requires the WS2812FX library



#define LED_COUNT 10 //how many leds on the strip
#define LED_COUNT2 9 //how many leds on the strip
#define LED_PIN D5 // ledstrip pin
#define LED_PIN2 D6 // ledstrip pin



int minVal = 2; // 'minimum' value on analog read (gets set automatically in state 0)
int maxVal = 1024; // 'maximum' value on analog read
int booster = 500; // booster value (between 0-1024), higher booster = higher results
int readings = 20; // how many readings should be done to determine average value
int resultTimeOut = 5000; // how long should the result be shown?
int baselineReadings = 20; // how many readings should be done to determine baseline value?
int currentReading = 0; //counter
int totalReading = 0; //counter
const long delayTime = 200; //polling rate
unsigned long previousMillis = 0; //counter
int ledsOn = 0; //variable to keep track of how many LED's should be on
int state = 0; //0: gather baseline data, 1: show idle animation, 2: show measure animation and measure, 3: show measurement

int animSpeed = 3000; //effect animation speed
int startupAnim = 0; //animation on startup
int idleAnim = 55; //animation to show when idle (not reading values)
int measureAnim = 13; //animation to show while measuring
int resultAnim = 0; //animation to show when showing result

int inVal; //reading variable
int perC; //percentage variable

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
WS2812FX ws2812fx2 = WS2812FX(LED_COUNT2, LED_PIN2, NEO_GRB + NEO_KHZ800);

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

  ws2812fx2.init();
  ws2812fx2.setBrightness(100);
  ws2812fx2.setSpeed(animSpeed);
  ws2812fx2.setColor(0xff0000);
  ws2812fx2.setMode(11);
  ws2812fx2.start();
}

void loop() {
  ws2812fx.service();
  ws2812fx2.service();
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
          perC = map(inVal, minVal, (maxVal - booster), 0,100);
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
      perC = map(inVal, minVal, (maxVal - booster), 0,100);
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
          Serial.print("MEASUREMENT: ");
          Serial.println(inVal);
          perC = map(inVal, minVal, (maxVal - booster), 0,100);
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
