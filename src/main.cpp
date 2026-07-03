#include <Arduino.h>      //Made with PlatformIO
#include <WiFi.h>
#include <ThingSpeak.h>

// Wi-Fi set up
const char* ssid = "********";                // Network SSID
const char* password = "************";        // Network password (WPA)

// ThingSpeak set up
unsigned long myChannelNumber = 0000000;          // Channel ID
const char * myWriteAPIKey = "****************";  // API Key

WiFiClient client;

// Send to ThingSpeak every minute
unsigned long lastSent = 0;
const unsigned long sendingInterval = 60000; // 60000 ms = 1 minuto

// Pin set up
const int pinU = 34;
const int pinI = 35;

// 100ms => 5x 50hz cycles or 6x 60hz cycles or 10x 100hz cycles 
const unsigned long tSampling = 100; 

// Initial average values (teoretical offset values)
float mVAdcU = 1.5;
float mVAdcI = 1.5;

void setup(){
  Serial.begin(115200);       // ESP32 Baudrate
  analogReadResolution(12);   // Set adc to 12 bit of resolution

  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  
  // Wait 500ms each time until it's connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi successfully connected!");
  
  // Initialize ThingSpeak
  ThingSpeak.begin(client);
}

void loop(){

  // Set up initial values for the variables used in the calculations 
  unsigned long beginTime = millis();
  int sampleNr = 0;                           // Number of samples taken in one sampling cycle

  float sumSquaredU = 0;                      // Sum of the squares of the voltage samples
  float sumSquaredI = 0;                      // Sum of the squares of the current samples
  float sumPower = 0;                         // Sum of the instantaneous power samples (voltage * current)
  
  float lastVoltage = 0;                      // Previous voltage sample, used for zero-crossing detection
  unsigned long lastCrossingTime = 0;         // Time of the last zero-crossing, used to calculate the period
  unsigned long accumulatedPeriod = 0;        // Accumulated period time for calculating the average period and frequency
  int crossing = 0;                           // Number of zero-crossings detected, used to calculate the average period and frequency

  float sumRawU = 0;                          // Sum of the raw voltage samples, used to dynamically adjust the voltage offset (mVAdcU) 
  float sumRawI = 0;                          // Sum of the raw current samples, used to dynamically adjust the current offset (mVAdcI)

  // Sampling loop
  while((millis() - beginTime) < tSampling){

    // Take milivolt readings from the ADC
    float vAdcU = analogReadMilliVolts(pinU) / 1000.0;
    float vAdcI = analogReadMilliVolts(pinI) / 1000.0;

    // Sum raw values to recalculate the DC offset
    sumRawU += vAdcU;
    sumRawI += vAdcI;

    // Calculate instantaneous values, removing offset and applying the transducer scaling factor
    float InstantVoltage = (vAdcU - mVAdcU) * 2.0 * 132.0; 
    float InstantCurrent = (vAdcI - mVAdcI) * 20.0;
    
    unsigned long currentTime = micros();

    // Zero crossing detection for frequency calculation
    if (lastVoltage < 0.0 && InstantVoltage >= 0.0){
      if (lastCrossingTime > 0){
        accumulatedPeriod += (currentTime - lastCrossingTime);
        crossing++;
      }
      lastCrossingTime = currentTime;
    }

    lastVoltage = InstantVoltage;

    // Squared sum of the samples
    sumSquaredU += (InstantVoltage * InstantVoltage);
    sumSquaredI += (InstantCurrent * InstantCurrent);
    sumPower += (InstantVoltage * InstantCurrent);

    // Increment the sample count
    sampleNr++;
  }

  // Check if we have taken any samples to avoid division by zero
  if (sampleNr > 0){

    // Update the DC offset values based on the average of the raw samples taken in this cycle
    mVAdcU = sumRawU / sampleNr;
    mVAdcI = sumRawI / sampleNr;

    // Average of the squared values
    float averageSquaredU = sumSquaredU / sampleNr;
    float averageSquaredI = sumSquaredI / sampleNr;

    // RMS calculations
    float RMSVoltage = sqrt(averageSquaredU);
    float RMSCurrent = sqrt(averageSquaredI);
    float apparentPower = RMSVoltage * RMSCurrent;
    float activePower = (sumPower / sampleNr); 
    
    // Power factor
    float powerFactor = 0;
    if (apparentPower > 0.01) {
      powerFactor = activePower / apparentPower;
    }

    // Calculate signal frequency
    float frequency = 0;

    if (crossing > 0){
      float periodoMedioSecs = (accumulatedPeriod / (float)crossing) / 1000000.0;
      frequency = 1.0 / periodoMedioSecs;
    }

    // Data serial output
    Serial.println("------------------------");
    Serial.print("Sample Number: "); Serial.println(sampleNr);
    Serial.print("Frequency: "); Serial.print(frequency); Serial.println(" Hz");
    Serial.print("=>U rms = "); Serial.print(RMSVoltage); Serial.println(" V");
    Serial.print("=>I rms = "); Serial.print(RMSCurrent); Serial.println(" A");
    Serial.print("Active Power: "); Serial.print(activePower); Serial.println(" W");
    Serial.print("Apparent Power: "); Serial.print(apparentPower); Serial.println(" VA");
    Serial.print("Power Factor: "); Serial.println(powerFactor);
    Serial.println("*************************");
  
    // Send data to THINGSPEAK
    // Only send if we are connected to Wi-Fi and if the defined interval has passed since the last sending
    if ((millis() - lastSent >= sendingInterval) && (WiFi.status() == WL_CONNECTED)) {
      
      // Set the fields with the values to be sent
      ThingSpeak.setField(1, RMSVoltage);
      ThingSpeak.setField(2, RMSCurrent);
      ThingSpeak.setField(3, activePower);
      ThingSpeak.setField(4, apparentPower);
      ThingSpeak.setField(5, powerFactor);
      ThingSpeak.setField(6, frequency);

      // Make the HTTP POST request
      Serial.println("Sending data to THINGSPEAK...");
      int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
      
      if(httpCode == 200){
        Serial.println("Data saved in ThingSpeak successfully!");
      } else {
        Serial.print("Problem sending. HTTP error code: ");
        Serial.println(httpCode);
      }
      
      // Reset the timer for the next sending
      lastSent = millis();
    }
  }

  delay(200);
}