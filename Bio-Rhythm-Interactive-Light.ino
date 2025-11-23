#include <Adafruit_NeoPixel.h>

#define microPin1 A0
#define microPin2 A1
#define microPin3 A2
#define microPin4 A3
#define microPin5 A4

#define touchPin 4
#define mosfetPin 3

#define LED_PIN_1 5  // Pin for the 1st WS2812B strip
#define LED_PIN_2 6  // Pin for the 2nd WS2812B strip
#define LED_PIN_3 7  // Pin for the 3rd WS2812B strip

#define getSoundTime 1000

Adafruit_NeoPixel ws2812b_1(1, LED_PIN_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ws2812b_2(1, LED_PIN_2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ws2812b_3(1, LED_PIN_3, NEO_GRB + NEO_KHZ800);

unsigned long startMillis = 0;

int signalMax1 = 0;
int signalMin1 = 4096;
int signalMax2 = 0;
int signalMin2 = 4096;
int signalMax3 = 0;
int signalMin3 = 4096;
int signalMax4 = 0;
int signalMin4 = 4096;
int signalMax5 = 0;
int signalMin5 = 4096;

int sample1, sample2, sample3, sample4, sample5;
long runCount[2] = { 0, 0 };
long triangleValue = 0;

int airflowValue = 0;
int thresholdValue;
bool getValue = true;
bool breatheDetected = false;

int buttonState;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 80;  // Touch module debounce delay

int ledBrightness = 0;
int targetBrightness = 0;
int currentStage = 0;  // Current stage
bool ledTouch = true;
bool ledOff = true;
bool stageReached = false;  // Flag to indicate if stage peak is reached
bool peakCheck = false;
bool getNoise = false;
bool buttonPress = false;
bool isPeakChecking = false; // Added flag
bool ifGetvalue = false;
bool stageWhile = false;
bool loopStage = false;

unsigned long previousMillis = 0;

// Finite State Machine setup, enum for stages
enum Mode { STAGE1, STAGE2, STAGE3, STAGE4};
Mode currentMode = STAGE1;

#define runMax 3
#define runMin 10
#define waitTime 1500  // 5 seconds wait time (adjusted via logic)

void setup() {
  Serial.begin(9600);
  pinMode(touchPin, INPUT_PULLUP);
  pinMode(mosfetPin, OUTPUT);
  analogWrite(mosfetPin, 0);

  ws2812b_1.begin();
  ws2812b_2.begin();
  ws2812b_3.begin();

  noiseRemove();
  setAllWhite();
  ws2812b_1.setBrightness(100);
  ws2812b_2.setBrightness(100);
  ws2812b_3.setBrightness(100);

}

void loop() {
  touch();
  peakChecking();
  getBreathingChange();
  updateLEDBrightness();
  updateIndicators();  // Update indicator lights
  delay(10);
}

/**
 * Remove background noise (Calibration)
 */
void noiseRemove() {
  startMillis = millis();

  while (millis() - startMillis < getSoundTime) {

    sample1 = analogRead(microPin1);
    sample2 = analogRead(microPin2);
    sample3 = analogRead(microPin3);
    sample4 = analogRead(microPin4);
    sample5 = analogRead(microPin5);

    if (sample1 > signalMax1) signalMax1 = sample1;
    if (sample1 < signalMin1) signalMin1 = sample1;

    if (sample2 > signalMax2) signalMax2 = sample2;
    if (sample2 < signalMin2) signalMin2 = sample2;

    if (sample3 > signalMax3) signalMax3 = sample3;
    if (sample3 < signalMin3) signalMin3 = sample3;

    if (sample4 > signalMax4) signalMax4 = sample4;
    if (sample4 < signalMin4) signalMin4 = sample4;

    if (sample5 > signalMax5) signalMax5 = sample5;
    if (sample5 < signalMin5) signalMin5 = sample5;
  }
}

/**
 * Read microphone breathing values and adjust brightness
 */
void getBreathingChange() {
  int sumSample1 = 0;
  int sumSample2 = 0;
  int sumSample3 = 0;
  int sumSample4 = 0;
  int sumSample5 = 0;

  for (int i = 0; i < 5; i++) {
    sumSample1 += analogRead(microPin1);
    sumSample2 += analogRead(microPin2);
    sumSample3 += analogRead(microPin3);
    sumSample4 += analogRead(microPin4);
    sumSample5 += analogRead(microPin5);
  }

  sample1 = sumSample1 / 5;
  sample2 = sumSample2 / 5;
  sample3 = sumSample3 / 5;
  sample4 = sumSample4 / 5;
  sample5 = sumSample5 / 5;

  int times = 1;
  int relativeSample1 = (sample1 > signalMax1) ? (sample1 - signalMax1) * times : (sample1 < signalMin1) ? abs(signalMin1 - sample1) * times
                                                                                                         : 0;
  int relativeSample2 = (sample2 > signalMax2) ? (sample2 - signalMax2) * times : (sample2 < signalMin2) ? abs(signalMin2 - sample2) * times
                                                                                                         : 0;
  int relativeSample3 = (sample3 > signalMax3) ? (sample3 - signalMax3) * times : (sample3 < signalMin3) ? abs(signalMin3 - sample3) * times
                                                                                                         : 0;
  int relativeSample4 = (sample4 > signalMax4) ? (sample4 - signalMax4) * times : (sample4 < signalMin4) ? abs(signalMin4 - sample4) * times
                                                                                                         : 0;
  int relativeSample5 = (sample5 > signalMax5) ? (sample5 - signalMax5) * times : (sample5 < signalMin5) ? abs(signalMin5 - sample5) * times
                                                                                                         : 0;

  int weightedSample = (relativeSample1 + relativeSample2 + relativeSample3 + relativeSample4 + relativeSample5) / 5;

  if (weightedSample > 0) {
    if (++runCount[0] > runMax) {
      runCount[1] = 0;
      triangleValue += weightedSample;
    }
  } else {
    if (++runCount[1] > runMin) {
      runCount[0] = 0;
      runCount[1] = runMin;
      triangleValue = 0;
    }
  }

  if (triangleValue != 0) {
    airflowValue = triangleValue / runCount[0];
    Serial.println(airflowValue);

    if (!breatheDetected && getValue == true && airflowValue > 0) {
      thresholdValue = 40; // Modifiable threshold
      Serial.print("Breath threshold detected: ");
      Serial.println(thresholdValue);
      getValue = false;  // Set flag to false after acquiring threshold
    }
    
    if(airflowValue >= thresholdValue){
      breatheDetected = true;
    }
    if(!getValue && breatheDetected){
    if (airflowValue >= 0 && !stageReached) {
      updateTargetBrightness();  // Adjust target brightness
      ledOff = false;            // LED On
    }
    }

  } else {
    airflowValue = 0;
    Serial.println(airflowValue);
  }

  // If airflow is below threshold or 0, and stage peak not reached, LED fades out
  if (airflowValue == 0 && !stageReached && !buttonPress) {
    if(ledBrightness >= 0 && ledBrightness < 85){ // Stage 1
    targetBrightness = 0;
    updateLEDBrightness();
    currentStage = 0;  // Reset Stage
    ledOff = true;     // Mark LED as off
    //getValue = true;   // Re-enable threshold acquisition
    }
    else if(ledBrightness >= 85 && ledBrightness < 170){ // Stage 2
    targetBrightness = 85;
    updateLEDBrightness();
    currentStage = 1;  // Reset Stage
    }
    else if(ledBrightness >= 170 && ledBrightness < 250){ // Stage 3
    targetBrightness = 170;
    updateLEDBrightness();
    currentStage = 2; 
    }
  }

  if (ledBrightness == 0 && !peakCheck || ledBrightness == 85 && !peakCheck || ledBrightness == 170 && !peakCheck) {
    peakCheck = true;
  }
}

/**
 * Adjust LED Brightness
 */
void updateLEDBrightness() {
  unsigned long currentMillis = millis();
  unsigned long interval = 33; // Light ON speed, modifiable
  unsigned long interval1 = 9; // Light OFF speed, modifiable

  if (currentMillis - previousMillis >= interval) { // LED stage light-up time approx 3 seconds
    previousMillis = currentMillis;

    if (ledBrightness < targetBrightness) {
      ledBrightness++;
    } 

    if (ledBrightness >= targetBrightness && !stageReached) {
      stageReached = true;  // Reached stage peak, set flag to true
    }

    analogWrite(mosfetPin, ledBrightness);
  }
  else if(currentMillis - previousMillis >= interval1){ // LED stage fade-out time approx 1.5 seconds
    if (ledBrightness > targetBrightness) {
      ledBrightness--;
    }

    if (ledBrightness >= targetBrightness && !stageReached) {
      stageReached = true;  
    }

    analogWrite(mosfetPin, ledBrightness);
  }
}

/**
 * Button Control Logic
 */
void touch() {
  int reading = digitalRead(touchPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        breatheDetected = false;
        Serial.println("Button Pressed");
        if (!getNoise) {
          while(airflowValue != 0){
          noiseRemove();
          noiseRemove();
          break;
          }
          getNoise = true;
        }
          Serial.println("Start Countdown");
          unsigned long countdownStart = millis();
          while (millis() - countdownStart < waitTime) {  // 5 seconds countdown
            Serial.print("Countdown: ");
            Serial.println((waitTime - (millis() - countdownStart)) / 1000);
            getBreathingChange();     // Non-blocking breath detection
            
            Serial.println("No breath detected, LED fading out");
            targetBrightness = 0;
            updateLEDBrightness();
            ledOff = true;  // Set LED OFF flag
            buttonPress = true;
            // Reset currentStage to 0 here
            currentStage = 0;
            stageReached = false;
          }
          getNoise = false;
        
      }
    }
  }
  lastButtonState = reading;
  if(ledBrightness == 0){
    buttonPress = false;
  }
}

/**
 * Set Target Brightness based on Current Stage
 */
void updateTargetBrightness() { // Brightness levels modifiable
  switch (currentStage) {
    case 0:
      targetBrightness = 85;  // Stage 1 Peak
      break;
    case 1:
      targetBrightness = 170;  // Stage 2 Peak
      break;
    case 2:
      targetBrightness = 250;  // Stage 3 Peak
      break;
    case 3:
      targetBrightness = 250;  // Higher brightness if needed
      break;
  }
}

/**
 * Update indicator colors to reflect current stage
 */
void updateIndicators() { // Indicator RGB colors fixed
  // Set indicator colors based on current brightness stage
  if (ledBrightness >= 249) { // OK
    setIndicatorColor(ws2812b_3, 0, 255, 0);  // Green, Stage 3 indicator ON
    currentStage = 3;
  }
  if (ledBrightness >= 170 && ledBrightness < 249) { // OK
    setIndicatorColor(ws2812b_2, 0, 255, 0);      // Green, Stage 2 indicator ON
    setIndicatorColor(ws2812b_3, 255, 255, 255);  // White, Stage 3 indicator OFF
    currentStage = 2;
  }
  if (ledBrightness >= 85 && ledBrightness < 170) { // OK
    setIndicatorColor(ws2812b_1, 0, 255, 0);      // Green, Stage 1 indicator ON
    setIndicatorColor(ws2812b_2, 255, 255, 255);  // White, Stage 2 indicator OFF
    currentStage = 1;
  }

  // Fade out logic, ensuring indicators turn off in correct order
  if (ledBrightness < 85 && currentStage == 1) { // OK
    setIndicatorColor(ws2812b_1, 255, 255, 255);  // White, Stage 1 indicator OFF
    currentStage = 0;
  }
  if (ledBrightness < 170 && currentStage == 2) {// OK
    setIndicatorColor(ws2812b_2, 255, 255, 255);  // White, Stage 2 indicator OFF
    currentStage = 1;
  }
  if (ledBrightness < 249 && currentStage == 3) {// OK
    setIndicatorColor(ws2812b_3, 255, 255, 255);  // White, Stage 3 indicator OFF
    currentStage = 2;
  }

  // Ensure all indicators are off when LED is fully off
  if (ledBrightness == 0) {
    setIndicatorColor(ws2812b_1, 255, 255, 255);  // White, Indicator 1 OFF
    setIndicatorColor(ws2812b_2, 255, 255, 255);  // White, Indicator 2 OFF
    setIndicatorColor(ws2812b_3, 255, 255, 255);  // White, Indicator 3 OFF
  }
}



void peakChecking() {// Modifiable, brightness levels modifiable
  if(!buttonPress){
  if (peakCheck) {
    if((ledBrightness == 85 && !loopStage) || (ledBrightness == 170 && !loopStage)) {
      loopStage = true;
      unsigned long lastTime = millis();
          while(millis() - lastTime < 10){
            Serial.println((10 - (millis() - lastTime)) / 10);
            }
          breatheDetected = false;
    }
    delay(70);
    if((ledBrightness == 85 && airflowValue == 0 ) || (ledBrightness == 170 && airflowValue == 0)){
      stageWhile = true;
    }

    if ((stageReached && stageWhile) || ledBrightness == 0) {  // Enter next stage only if peak reached
      Serial.println("Start Countdown");
      unsigned long countdownStart = millis();
      bool detectedBreath = false;

      while (millis() - countdownStart < 20000 && !buttonPress && stageReached && !isPeakChecking) {  // 20s countdown
        touch();
        Serial.print("Countdown: ");
        Serial.println((20000 - (millis() - countdownStart)) / 1000);
        getBreathingChange();      // Non-blocking breath detection
        if (airflowValue >= 0 && !isPeakChecking && breatheDetected) {  // Adjust threshold for stage 3, true once is enough
          detectedBreath = true;
          isPeakChecking = true;
          break;  // Immediately enter next stage
        }
      }

      //ifGetvalue = false;

      if (detectedBreath) {// If detectedBreath is repeatedly true, this function triggers repeatedly
        if (currentStage < 2) {
          currentStage++;
        } else if (currentStage == 2 && !stageReached) {
          currentStage = 3;  // Enter Stage 3
          stageReached = true;
        }
        //getValue = true;           // Re-enable threshold acquisition
        stageReached = false;      // Reset flag for next stage
        updateTargetBrightness();  // Set target brightness for next stage
        //Serial.println("Breath detected, entering next stage");
      }/*else {
            Serial.println("No breath detected, LED fading out");
            targetBrightness = 0;
            updateLEDBrightness();
            ledOff = true;  // Set LED OFF flag

            // Reset currentStage to 0 here
            currentStage = 0;
            stageReached = false;
          }*/
    }
    peakCheck = false;
    isPeakChecking = false; // Reset flag
    stageWhile = false;
    loopStage = false;
  }
  }
}

/**
 * Set Indicator Color
 */
void setIndicatorColor(Adafruit_NeoPixel &strip, int red, int green, int blue) {
  strip.setPixelColor(0, strip.Color(red, green, blue));
  strip.show();
}

/**
 * Set All Indicators to White
 */
void setAllWhite() {
  setIndicatorColor(ws2812b_1, 255, 255, 255);  // Indicator 1 White
  setIndicatorColor(ws2812b_2, 255, 255, 255);  // Indicator 2 White
  setIndicatorColor(ws2812b_3, 255, 255, 255);  // Indicator 3 White
}