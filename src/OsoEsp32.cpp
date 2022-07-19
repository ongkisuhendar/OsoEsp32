#include "OsoEsp32.h"
#include "Arduino.h"

void begin(long baud){
Serial.begin(baud);	
Serial.println();
}

void oke(){
	Serial.println("oke mang");
	delay(1000);
}


void no(){
	Serial.println("no mang");
	delay(1000);
}