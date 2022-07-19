#include "OsoEsp32.h"
#include "Arduino.h"

void oso::init(long baud){
Serial.begin(baud);	
Serial.println();
ini();
}

void oke(){
	Serial.println("oke mang");
	delay(1000);
}


void no(){
	Serial.println("no mang");
	delay(1000);
}
void oso::ini(){	
oke();
no();
return;
}