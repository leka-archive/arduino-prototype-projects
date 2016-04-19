#include <Arduino.h>
#include <Wire.h>

// #include "Sensors.h"
#include "DriveSystem.h"
#include "Moti.h"
#include <Adafruit_NeoPixel.h>
#include <ctype.h> 

#define PIN 8
# define Serio Serial

#define FIMU_ACC_ADDR ADXL345_ADDR_ALT_LOW

ADXL345 acc;

int DTC;
byte range;
#define NB_LEDS 3
#define	LED_ANGLE 360/NB_LEDS
#define THRESHOLD 120

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);


int lastR = 0;
int lastG = 0;
int lastB = 0;

/* Read leds data send by the Qt app */

/********* NOT DEFINITIVE ***********/

boolean isValidNumber(String str){
	boolean res = true;
	for(int i=0;i<str.length();i++) {
		if(!(isDigit(str.charAt(i)) || str.charAt(i) == ' ')) res = false;
	}
	return res;
} 

void read(){
	String inputString ="";
	while(Serial.available()){

		char inChar = (char) Serial.read();
			
		inputString += inChar;

		if(inChar == '\n' && inputString[0] == '\t'){
			int SpaceIndex = inputString.indexOf(' ');
			//  Search for the next Space just after the first
			int secondSpaceIndex = inputString.indexOf(' ', SpaceIndex+1);
			//  Search for the next Space just after the first
			int thirdSpaceIndex = inputString.indexOf(' ', secondSpaceIndex+1);
			int endIndex = inputString.indexOf(' ', thirdSpaceIndex+1);

			String firstValue = inputString.substring(1, SpaceIndex);
			String secondValue = inputString.substring(SpaceIndex+1, secondSpaceIndex);
			String thirdValue = inputString.substring(secondSpaceIndex+1,thirdSpaceIndex);
			String quadValue = inputString.substring(thirdSpaceIndex+1,endIndex);

			// to restablish the original values 
			int r = firstValue.toInt()-1;
			int g = secondValue.toInt()-1;
			int b = thirdValue.toInt()-1;
			int alpha = quadValue.toInt()-1;

			if(!(lastR == r && lastG == g && lastB == b)){
				if(r>=0 && r<=255 && g>=0 && g<=255 && b>=0 && b<=255 && alpha < 360 && alpha >= 0 && isValidNumber(firstValue) && isValidNumber(secondValue) && isValidNumber(thirdValue) && isValidNumber(quadValue)){
					// String complete
					for(int i =0;i<NB_LEDS;i++){
						int angle_led = LED_ANGLE*i;
						// we light up leds according to the distance between them and the angle measured by the giro, the closer the led
 						// the more shiny 
						float coeff = 1.0; // max = 11
						
//conditions had to be written to resolve issues due to angles near 0° and 360°
						if(alpha < THRESHOLD){
							if(angle_led >= alpha+360-THRESHOLD || angle_led < alpha + THRESHOLD){
								if(angle_led >= alpha+360-THRESHOLD){
									coeff += 10*abs(alpha-(angle_led-360))/THRESHOLD;
								}else{
									coeff += 10*abs(alpha-angle_led)/THRESHOLD;
								}
								strip.setPixelColor(i, (int) r/coeff, (int) g/coeff, (int) b/coeff);
							}else{
								strip.setPixelColor(i, 0, 0, 0);
							}
						} else if(alpha >= 360 - THRESHOLD){
							if(angle_led <= alpha-360+THRESHOLD || angle_led > alpha - THRESHOLD){
								if(angle_led <= alpha-360+THRESHOLD){
									coeff += 10*abs(alpha-(angle_led+360))/THRESHOLD;
								}else{
									coeff += 10*abs(alpha-angle_led)/THRESHOLD;
								}
								strip.setPixelColor(i, (int) r/coeff, (int) g/coeff, (int) b/coeff);
							}else{
								strip.setPixelColor(i, 0, 0, 0);
							}
						} else {
							if(angle_led >= alpha-THRESHOLD && angle_led <= alpha+THRESHOLD){
								coeff += 10*abs(alpha-angle_led)/THRESHOLD;
								strip.setPixelColor(i, (int) r/coeff, (int) g/coeff, (int) b/coeff);
							}else{
								strip.setPixelColor(i, 0, 0, 0);
							}
						}
						
					}
					lastR = r;
					lastG = g;
					lastB = b;
					strip.show();
				}
			}
			inputString="";
		}		
	}
}
void chSetup() {

	acc.init(FIMU_ACC_ADDR);
	int setRange = 4;
	acc.setRangeSetting(setRange);
	delay(1000);
	acc.getRangeSetting(&range);

	int xyz[3];
	float xyz_gyr[3];

	while (TRUE) {
		/* Send data to Qt app */
		acc.readAccel(&xyz[0], &xyz[1], &xyz[2]);
		Serial.print("A ");
		Serial.print(xyz[0]);
		Serial.print(" ");
		Serial.print(xyz[1]);
		Serial.print(" ");
		Serial.print(xyz[2]);
		Serial.print(" ");

		acc.get_Gxyz(xyz_gyr);
		Serial.print(xyz_gyr[0]);
		Serial.print(" ");
		Serial.print(xyz_gyr[1]);
		Serial.print(" ");
		Serial.print(xyz_gyr[2]);

		/* Read leds data */
		read();

		delay(100);
	 }
}


void setup() {
	Serio.begin(115200);
	while (!Serio);
  	strip.begin();
  	strip.show(); // Initialize all pixels to 'off'

	Wire.begin();
	delay(500);

	chBegin(chSetup);

	while (TRUE);
}

void loop() { }
