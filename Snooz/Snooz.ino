#include <WiFi.h>

// ESP alexa library
//#define ESPALEXA_ASYNC
//#define ESPALEXA_DEBUG
//#define ESPALEXA_MAXDEVICES 3
#include <Espalexa.h>

// Configuration file
#include "config.h"

// LEDC channels for PWM pins
const int ledChannel_MTR = 0;
const int ledChannel_LED = 1;

// Callbacks and function prototype
void Callback_Snooz(EspalexaDevice* dev);

// Other global variables
Espalexa espalexa;


/*************************** Sketch Code ************************************/
void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Setup start");

  // Configure motor and LED
  // ledChannel, freq, resolution
  ledcSetup(ledChannel_MTR, 10000, 8);
  ledcSetup(ledChannel_LED, 10000, 8);
  
  // Atatch timer to pins
  // ledPin, ledChannel
  ledcAttachPin(16, ledChannel_MTR); // Motor
  ledcAttachPin(4, ledChannel_LED); // LEDs

  // Turn LED on
  ledcWrite(ledChannel_MTR, 0);
  ledcWrite(ledChannel_LED, 255);
  
  // Connect to WiFi access point.
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.printf("Connecting to %s: .", WLAN_SSID);

  // Connect to wifi with timeout
  unsigned long conn_timeout = millis();
  WiFi.setSleep(false);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  ledcWrite(ledChannel_LED, 0);
  while ( WiFi.status() != WL_CONNECTED ) 
  {
    delay(500);
    Serial.print(".");

    // Pulse the LED
    ledcWrite(ledChannel_LED, 50);
    delay(100);
    ledcWrite(ledChannel_LED, 0);

    // Timeout?
    if ( (millis() - conn_timeout) > (30 * 1000) )
    {
      Serial.println();
      Serial.printf("Connect timeout. Status: %d; Rebooting \n", WiFi.status());             
      ESP.restart();
    }
  }
  Serial.println();
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // Add alexa devices
  espalexa.addDevice("White Noise", Callback_Snooz, EspalexaDeviceType::onoff, 0);
  espalexa.begin();
  
  Serial.println("Setup end");

  for (int i=0; i<5; i++)
  {
    ledcWrite(ledChannel_LED, 255);
    delay(100);
    ledcWrite(ledChannel_LED, 0);
    delay(100);
  }
}


// Main loop is running on core 1 by default
void loop() 
{ 
  espalexa.loop();
  
  // Short delay
  delay(1);
}


// ================================================================
// Alexa Snooz Callback
void Callback_Snooz(EspalexaDevice* d) 
{   
  Serial.printf("Callback. Device:%s; Bright:%d, Red:%d, Green:%d, Blue:%d \n", d->getName(), d->getValue(), d->getR(), d->getG(), d->getB());

  // Stop the motor if command is less than 25%
  if (d->getValue() <= 65)
  {
    ledcWrite(ledChannel_MTR, 0);

    // Show it on LED
    ledcWrite(ledChannel_LED, 255);
    delay(500);
    ledcWrite(ledChannel_LED, 0);    
  }
  else
  {
    // Just update PWM value
    ledcWrite(ledChannel_MTR, d->getValue());

    // Blink LEDs to confirm the command
    for (int i=0; i<2; i++)
    {
      ledcWrite(ledChannel_LED, 255);
      delay(100);
      ledcWrite(ledChannel_LED, 0);
      delay(100);
    }  
  }


}
