/*************************************************************************
 * Lumiboard - RGB LED Lamp
 * 
 * Author: Jason Webb
 * Author website: http://jason-webb.info
 * Author e-mail: zen.webb@gmail.com
 * 
 * Project wiki: http://jason-webb.info/wiki/index.php?title=Lumiboard
 * Github repo: https://github.com/jasonwebb/Lumiboard
 **************************************************************************/
#include <SoftwareSerial.h>

// Pin definitions ======================
#define  LED1_R  11
#define  LED1_G  10
#define  LED1_B  9

#define  LED2_R  6
#define  LED2_G  5
#define  LED2_B  3

#define  DEBUG_LED  13

#define  BT_RX  7
#define  BT_TX  8

#define  POT1  A0
#define  POT2  A1
#define  POT3  A2
#define  POT_THRESHOLD  2

#define  SENSOR_RTN  2
#define  SENSOR_TEMPERATURE  A7
#define  SENSOR_LIGHT  A3
#define  SENSOR_AUDIO  A0  // A6

// Global arrays ===================================================================
float LED1_NEW_RGB[3] = {255,255,0};    // Next values for RGB LED 1 (buffer)
float LED2_NEW_RGB[3] = {255,255,0};    // Next values for RGB LED 2 (buffer)
unsigned int LED1_RGB[3] = {0,0,0};          // Current RGB values of RGB LED 1 (actual values)
unsigned int LED2_RGB[3] = {0,0,0};          // Current RGB values of RGB LED 2 (actual values)
unsigned int current_pot_values[3] = {0,0,0};  // Current RGB potentiometer values
unsigned int new_pot_values[3] = {0,0,0};      // Most recent RGB pot values

// Sensor variables ==========================================================
// Reading buffers
const unsigned int buffer_size = 10;           // Number of readings to take
unsigned int temperature_values[buffer_size];  // Buffer of temperature values
unsigned int light_values[buffer_size];        // Buffer of ambient light values
unsigned int audio_values[buffer_size];        // Buffer of audio values

// Rolling averages
unsigned int averageTemperature = 0;
unsigned int averageLight = 0;
unsigned int averageAudio = 0;

// General operational flags =======================================================
boolean enableLED1 = true;
boolean enableLED2 = true;
boolean enablePots = true;  // Only when pots are connected and you want to use them
boolean enableAudio = false;
boolean debug = true;
boolean sensorDebug = false;

// Specific variables used by various modes ========================================
// GENERAL
float brightness = 1;                // Brightness as a value between 0-1
int fadeSpeed = 10;                  // Milliseconds to wait between each clock cycle. Larger = slower
int counter = 0;                     // General purpose counter
int counterThreshold = 255;          // Tipping point to reset other periodic variables

// COLOR WASH MODE
int decChannel = 0, incChannel = 2;  // Initial channels to wash in "color wash mode"
boolean pickNewColors = true;        // Flag to pick new colors for "color wash mode"
int stepAmount = 1;                  // Amount to step in each cycle of "color wash mode"

// AUDIO REACTIVE MODE
int audioThreshold = 550;            // Minimum value to trigger brightness change

// NIGHTLIGHT MODE
byte nightlightColor[3] = {255,255,255};

// Behavior flow control variables ======================================================
const byte COLOR_WASH_MODE = 0;    // Color washing (blending between all RGB values)
const byte DIRECT_POT_MODE = 1;    // RGB pots directly control LEDs
const byte THERMOMETER_MODE = 2;   // Red = hot, blue = cold
const byte NIGHTLIGHT_MODE = 3;    // Brightness inversely proportional to ambient light
byte INPUT_MODE = DIRECT_POT_MODE;

void setup() {  
  // Take initial reading of pots
  checkPots();

  // Start up Serial connection
  Serial.begin(57600);
}

void loop() {
  switch(INPUT_MODE) {

    // COLOR WASH MODE ================================================
    // Cycle through all RGB values
  case COLOR_WASH_MODE: {
      // Pick new channels to alter once in a while
      if(pickNewColors) {
        incChannel = random(3);

        do {
          decChannel = random(3);
        } while(decChannel == incChannel);

        pickNewColors = 0;
        counter = 0;

        if(debug) {
          Serial.print("PICKING NEW CHANNELS: INC = ");
          Serial.print(incChannel);
          Serial.print(", DEC = ");
          Serial.println(decChannel);
        }
      }

      // Decrement one channel
      if(LED1_RGB[decChannel] > 0) {
        LED1_NEW_RGB[decChannel] -= stepAmount;
        LED2_NEW_RGB[decChannel] -= stepAmount;
      }

      // Increment the other channel      
      if(LED1_RGB[incChannel] < 255) {
        LED1_NEW_RGB[incChannel] += stepAmount;
        LED2_NEW_RGB[incChannel] += stepAmount;
      }

      // Push new colors to LEDs
      //updateLEDs();
      delay(fadeSpeed);

      // Maintain counter, check for overflow
      if(counter >= counterThreshold) {
        pickNewColors = true;
        counter = 0;
      } else {
        counter++;
      }
      
      updateLEDs();

      break; 
    }

    // DIRECT POT MODE ================================================
    // RGB LED values directly controlled by RGB pots
  case DIRECT_POT_MODE: {
      // Check if pots have changed
      boolean potsChanged = checkPots();  

      // If so, update RGB LEDs
      if(potsChanged) {
        for(int i=0; i<3; i++) {
          LED1_NEW_RGB[i] = map(current_pot_values[i], 0, 1023, 0, 255);
          LED2_NEW_RGB[i] = map(current_pot_values[i], 0, 1023, 0, 255);
        }

        updateLEDs();

        // Output debug info via Serial
        if(debug) {
          Serial.print("POTS: ");
          Serial.print(potsChanged);
          Serial.print("\t");

          for(int i=0; i<3; i++) {
            Serial.print(current_pot_values[i]);
            Serial.print("\t");
          }
          Serial.println();
        }
      }        
    } 
    break;

    // THERMOMETER MODE ==============================================
    // Turn red for warm, blue for cool
  case THERMOMETER_MODE: {
      // Check that the sensor board is connected
      boolean sensorsConnected = checkSensors();

      if(sensorsConnected) {   
        float temperature = ((averageTemperature * 5 / 1023) - 0.5) / 0.01;

        if(debug) {
          Serial.print("TEMPERATURE = ");
          Serial.print(temperature);
          Serial.println(" F");
        }
      } else {
        Serial.println("Connect the sensor board!");
      }
    } 
    break;

    // NIGHTLIGHT MODE ====================================================
    // Vary brightness of static color inversely with ambient light
  case NIGHTLIGHT_MODE: {
      // Check that the sensor board is connected
      boolean sensorsConnected = checkSensors();

      if(sensorsConnected) {        
        // DO SOMETHING WITH AVERAGE

        if(debug) {
          Serial.print("AMBIENT LIGHT LEVEL = ");
          Serial.println(averageLight);
        }       
      } else {
        Serial.println("Connect the sensor board!");
      }      
    } 
    break;
  }

  // Audio reactive modulation
  if(enableAudio) {
    // Check sensors if they have not been checked yet
    if(INPUT_MODE == COLOR_WASH_MODE || INPUT_MODE == DIRECT_POT_MODE) {
      boolean sensorsConnected = checkSensors();

      if(sensorsConnected) {
        if(averageAudio >= audioThreshold)
          brightness = .99;
        else
          brightness = 1;

        updateLEDs();
      } else {
        Serial.println("Connect the sensor board!");
      }

      updateLEDs();
    }
  }
}

/*****************************************************
 * Check sensor board for readings
 ******************************************************/
boolean checkSensors() {
  // First make sure the board is connected
  //  byte rtnCheck = digitalRead(SENSOR_RTN);
  byte rtnCheck = 1;

  if(rtnCheck == 0) {
    return false; 
  } else {
    // Shift value buffers down
    for(int i=0; i<buffer_size-1; i++) {
      temperature_values[i] = temperature_values[i+1];
      light_values[i] = light_values[i+1];
      audio_values[i] = audio_values[i+1];
    }

    // Append most recent sensor readings to buffers
    temperature_values[buffer_size-1] = analogRead(SENSOR_TEMPERATURE);  
    light_values[buffer_size-1] = analogRead(SENSOR_LIGHT);
    audio_values[buffer_size-1] = analogRead(SENSOR_AUDIO);

    // Compute new rolling averages for each of the sensors and push to global variables
    int tt = 0, tl = 0, ta = 0;
    for(int i=0; i<buffer_size; i++) {
      tt += temperature_values[i];
      tl += light_values[i];
      ta += audio_values[i];
    }

    averageTemperature = tt/buffer_size;
    averageLight = tl/buffer_size;
    averageAudio = ta/buffer_size;

    if(sensorDebug) {
      Serial.print("TEMP: ");
      Serial.print(averageTemperature);
      Serial.print(", LIGHT: ");
      Serial.print(averageLight);
      Serial.print(", AUDIO: ");
      Serial.println(averageAudio);
    }

    return true;
  }  
}

/*********************************************
 * Check for activity on the RGB pots
 **********************************************/
boolean checkPots() {
  // Get latest readings from pots
  new_pot_values[0] = analogRead(POT1);
  new_pot_values[1] = analogRead(POT2);
  new_pot_values[2] = analogRead(POT3);

  // Update values if they vary significantly
  if((new_pot_values[0] > current_pot_values[0] + POT_THRESHOLD) || (new_pot_values[0] < current_pot_values[0] - POT_THRESHOLD) ||
    (new_pot_values[1] > current_pot_values[1] + POT_THRESHOLD) || (new_pot_values[1] < current_pot_values[1] - POT_THRESHOLD) ||
    (new_pot_values[2] > current_pot_values[2] + POT_THRESHOLD) || (new_pot_values[2] < current_pot_values[2] - POT_THRESHOLD)) {
    for(int i=0; i<3; i++)
      current_pot_values[i] = new_pot_values[i];

    return true;
  } else {
    return false;
  }
}

/***********************************************************************
 * Updates both of the RGB LEDs using the current values of the arrays
 ************************************************************************/
void updateLEDs() {
  updateBrightness();

  // Update LED 1
  if(enableLED1) {
    analogWrite(LED1_R, LED1_RGB[0]);
    analogWrite(LED1_G, LED1_RGB[1]);
    analogWrite(LED1_B, LED1_RGB[2]);
  }

  // Update LED 2
  if(enableLED2) {
    analogWrite(LED2_R, LED2_RGB[0]);
    analogWrite(LED2_G, LED2_RGB[1]);
    analogWrite(LED2_B, LED2_RGB[2]);
  }
}

/**********************************************
 * Updates the brightness of all LEDs
 * - Also pushes RGB buffer to actual values
 **********************************************/
void updateBrightness() {
  // Multiplies brightness scalar across RGB buffers, then pushes result to actual LED values
  for(int i=0; i<3; i++) {
    LED1_RGB[i] = LED1_NEW_RGB[i] * brightness;
    LED2_RGB[i] = LED2_NEW_RGB[i] * brightness;
    Serial.println(LED1_RGB[0]);
  } 
}
