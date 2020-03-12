#include <SoftwareSerial.h>
#include "MPU9250.h"

const int GPS_DATA_BUFFER_SIZE = 256;
int GPSDataBufferIndex = 0;
char GPSDataBuffer[GPS_DATA_BUFFER_SIZE] = {0x00};
MPU9250 mpu;

SoftwareSerial sdCard(11 ,12);
SoftwareSerial GPS(8, 9);
void setup() {
  Wire.begin();
  delay(2000);
  mpu.setup();
  mpu.setMagneticDeclination(-7);
  mpu.calibrateMag();
  mpu.update();
  sdCard.begin(115200);
  GPS.begin(9600);
  GPS.listen();
  
  sdCard.println("X acceleration, Y acceleration, Z acceleration, X magnet, Y magnet, Z magnet, TMP Temperature(C), Humidity, Pressure, Latitude, Longitude, Altitude, Time");
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
}

void loop() {
  sdCard.print(.01227 * analogRead(A0) - 4.8); //X
  sdCard.print(", ");
  sdCard.print(.012345679 * analogRead(A1) - 4.86);//Y
  sdCard.print(", ");
  sdCard.print(.01282 * analogRead(A2) - 5.166667);//Z
  sdCard.print(", ");
  sdCard.print(mpu.getMag(0)); //MPU mag X
  sdCard.print(", ");
  sdCard.print(mpu.getMag(1)); //MPU mag Y
  sdCard.print(", ");
  sdCard.print(mpu.getMag(2)); //MPU mag Z
  sdCard.print(", ");
  sdCard.print(208.0 * analogRead(A3) * 5.0/1023.0 - 136.2);//TMP
  sdCard.print(", ");
  sdCard.print(47.37 * analogRead(A4)* 5.0/1023.0 -39.4);//Humidity
  sdCard.print(", ");
  sdCard.print(25.5 * analogRead(A6) * 5.0/1023.0);//Pressure
  sdCard.print(", ");
  


  

  long startTime = millis();  
  while( (millis() < startTime + GPS_DATA_BUFFER_SIZE + 2) && (GPSDataBufferIndex !=
  GPS_DATA_BUFFER_SIZE - 1)) { 
    if(GPS.available()) {
      GPSDataBuffer[GPSDataBufferIndex] = GPS.read();
      GPSDataBufferIndex++;
    }
  }
 
  GPS.end();
  // END RX ONLYi

  // BEGIN TX ONLY
  char* gpgga = ptr_to_first_gpgga();
  int dist_to_gpgga = distance_to_first_gpgga(gpgga);
  String GPGGA = "";
  String latitude = "";
  String longitude = "";
  String altitude = "";
  int count = 0;
  int dist = 1;

  if(dist_to_gpgga != -1) { // if we found a $GPGGA string
    for(int i = dist_to_gpgga; i <= distance_to_first_newline_after_gpgga(gpgga) + dist_to_gpgga; ++i){
      GPGGA = GPSDataBuffer[i];
      count = 0;
      for (int i = 0; i < GPGGA.length(); i++){
        if (GPGGA[i] == ','){
          count++;
        }
        if (count == 2){
          while (GPGGA[i + dist] != ','){
            latitude += GPGGA[i + dist];
            dist++;  
          }
          i = i + dist + 1;
          count++;
          dist = 1;
        }
        if (count == 4){
          while (GPGGA[i + dist] != ','){
            longitude += GPGGA[i + dist];
            dist++;  
          }
          i = i + dist + 1;
          count++;
          dist = 1;
        }
        if (count == 9){
          while (GPGGA[i + dist] != ','){
            altitude += GPGGA[i + dist];
            dist++;  
          }
          i = i + dist + 1;
          count++;
          dist = 1;
        }
      }
      
    }
  }
  
  // END TX ONLY
  
  reset_GPSDataBuffer_and_index();
  mpu.update(); 



  sdCard.print(latitude);
  sdCard.print(", ");
  sdCard.print(longitude);
  sdCard.print(", ");
  sdCard.print(altitude);
  sdCard.print(", ");
  sdCard.println(millis());
  delay(20);
  

}









void reset_GPSDataBuffer_and_index() {
  for (int i = 0; i < GPS_DATA_BUFFER_SIZE; i++){
    GPSDataBuffer[i] = '\0';
  }
  GPSDataBufferIndex = 0;
}

/*
 * REQUIRES: nothing
 * MODIFIES: nothing
 * EFFECTS: returns a pointer to a character ('$') inside of the GPSDataBuffer
 * (the beginning of the GPGGA string)
 */
char* ptr_to_first_gpgga() {
  return strstr(GPSDataBuffer, "$GPGGA");
}

/*
 * REQUIRES: a pointer, gpgga, to the '$' of a GPGGA string
 * MODIFIES: nothing
 * EFFECTS: returns -1 if the gpgga pointer is null, otherwise returns the
 * distance from the beginning of the GPSDataBuffer to the '$'
 */
int distance_to_first_gpgga(const char* gpgga) {
  if (!gpgga){
    return -1;
  }
  return gpgga - GPSDataBuffer;
}
/*
 * REQUIRES: a pointer, gpgga, to the '$' of a GPGGA string
 * MODIFIES: nothing
 * EFFECTS: returns -1 if the gpgga pointer is null or if no newline is found
 * after the GPGGA string, otherwise returns the distance from the beginning of
 * the GPSDataBuffer to the first newline following the GPGGA string
 */
int distance_to_first_newline_after_gpgga(const char* gpgga) {
   if (!gpgga){
    return -1;
   }
   if (!strchr(gpgga, '\n')){
      return -1;
   }
   return strchr(gpgga, '\n') - gpgga;
}
