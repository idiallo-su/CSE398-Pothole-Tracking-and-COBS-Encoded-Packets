#include <Wire.h> 
#include <Adafruit_BNO055.h> 
#include <Adafruit_Sensor.h> 
#include <utility/imumaths.h> 
#include <PacketSerial.h> 
  
//using namespace LibSerial 
  
Adafruit_BNO055 bno = Adafruit_BNO055(55); 
  
float a_prev = 0; //Global because it is needed across loops 
  
//Indicies 0-19 will be constantly updated 
//Indicies 20-39 will be updates when we hit a bump 
float previousSecondAcc[40];  
float previousSecondForce[40]; 
  
PacketSerial myPacketSerial; 
  
struct send_packet { 
  char header; 
  float measurement[40]; 
  float orient_yaw; 
  float orient_pitch; 
  float orient_roll; 
  uint8_t checksum; 
}; 
  
/************ SETUP ************ SETUP ************ SETUP ************/ 
  
void setup() { 
   
  // put your setup code here, to run once: 
  Serial.begin(9600); 
  Serial1.begin(9600); 
  Serial.println("Setup called"); Serial1.println(""); 
  
  //Initialize the sensor 
  if(!bno.begin()) { 
    Serial.print("No BNO055 detected. Check wiring or I2C Addr."); 
	while(1); 
  } 
  
  delay(1000); 
  
  bno.setExtCrystalUse(true); 
  
  myPacketSerial.setStream(&Serial); 
  myPacketSerial.setStream(&Serial1); 
} 
  
/************ LOOP ************ LOOP ************ LOOP ************/ 
  
/*sending basic packet 
send_packet testpkt; 
testpkt.header = 'testeberger'; 
testpkt.checksum = 27; 
myPacketSerial.send((uint8_t*)&testpkt, sizeof(testpkt)); */ 
  
  
void loop() { 
  // put your main code here, to run repeatedly: 
  
   
  
  //Euler angle 
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER); 
  //Accelation vector 
  imu::Vector<3> acc = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL); 
  
  int sensorPin = A1; 
  int sensorValue = analogRead(sensorPin); 
  float F = pow((((1024*20000)/sensorValue) - 20000)/(pow(10, 5.88)),(1.0/-0.8)); 
  
  /* 
  // Orientation (euler angles) 
  Serial.print("X: "); 
  Serial.println(euler.x()); 
  Serial.print(" Y: "); 
  Serial.println(euler.y()); 
  Serial.print(" Z: "); 
  Serial.println(euler.z()); 
  
  // Acceleration 
  Serial.print("X: "); 
  Serial.println(acc.x()); 
  Serial.print(" Y: "); 
  Serial.println(acc.y()); 
  Serial.print(" Z: "); 
  Serial.println(acc.z()); 
  */ 
  
  //Combined Acceleration 
  float a_measured = sqrt( pow(acc.x(),2) + pow(acc.y(),2) + pow(acc.z(),2) ); 
  
  /* 
  //Filtered Acceleration 
  float a_fil = (0.86*a_prev) + (0.14*a_measured); 
  
   
  Serial.print("Measured Acceleration "); 
  Serial.println(a_measured); 
  */ 
   
  /* 
  Serial.print("Previous Acceleration "); 
  Serial.println(a_prev); 
  Serial.print("Filtered Acceleration "); 
  Serial.println(a_fil); 
  Serial.println(acc.z()); 
  Serial.println(""); 
  */ 
  
  //On every loop we need to update the array 
  for(int i=0; i<19; i++) {  
    previousSecondAcc[i] = previousSecondAcc[i+1]; 
    previousSecondForce[i] = previousSecondForce[i+1]; 
     
  } 
  previousSecondAcc[19] = a_measured; 
  previousSecondForce[19] = sensorValue; 
  
  // Button Pressed 
  if(abs(F) > 50){ 
  
    Serial.print("We touched the butt...on"); 
  
    send_packet pkt; 
    pkt.header = 'm';                	// button header 
    pkt.orient_yaw = euler.z(); 
    pkt.orient_pitch = euler.y(); 
    pkt.orient_roll = euler.x(); 
  
	for(int i=0; i<=19; i++) {  
      pkt.measurement[i] = previousSecondForce[i]; 
	} 
  
	//Print previous second array 
	for(int i=0; i<=19; i++) {  
      Serial.println(pkt.measurement[i]); 
      Serial.print(" "); 
	} 
    Serial.println(""); 
  
	//Future second into the packet 
	for(int i=20; i<40; i++) { 
      sensorValue = analogRead(sensorPin); 
  	F = pow((((1024*20000)/sensorValue) - 20000)/(pow(10, 5.88)),(1.0/-0.8)); 
      pkt.measurement[i] = F; 
  	delay(50); 
	} 
    Serial.println(""); 
    Serial.println("Done Recording"); 
  
    pkt.checksum = 0; 
	for (int i=0; i<40; i++){ 
      pkt.checksum += pkt.measurement[i]; 
	} 
    pkt.checksum += (int)(pkt.header); //Calculates the checksum on Arduino's side 
    myPacketSerial.send((uint8_t*)&pkt, sizeof(pkt)); 
  
  } 
  
  // Bump detected  
  if(abs(a_measured) > (0.25*9.81))   
  { 
    Serial.println("We hit a bump!!!"); 
  
    send_packet pkt; 
    pkt.header = 'z';                	// bump header 
    pkt.orient_yaw = euler.z(); 
    pkt.orient_pitch = euler.y(); 
    pkt.orient_roll = euler.x(); 
  
     
	//Previous second into the packet 
	for(int i=0; i<=19; i++) {  
      pkt.measurement[i] = previousSecondAcc[i]; 
	} 
     
	//Print previous second array 
	for(int i=0; i<=19; i++) {  
      Serial.println(pkt.measurement[i]); 
      Serial.print(" "); 
	} 
    Serial.println(""); 
     
	//Future second into the packet 
	for(int i=20; i<40; i++) { 
  
    	acc = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL); 
        a_measured = sqrt( pow(acc.x(),2) + pow(acc.y(),2) + pow(acc.z(),2) ); 
       
    	//Serial.print("Measured Acceleration "); 
    	//Serial.println(a_measured); 
  
    	// /* 
    	//Gather the data for the packet 
        pkt.measurement[i] = a_measured; 
    	//  
        Serial.print(pkt.measurement[i]); 
        Serial.print(" "); 
         
         
    	delay(50); 
	} 
    Serial.println(""); 
    Serial.println("Done Recording"); 
  
    pkt.checksum = 0; 
	for (int i=0; i<40; i++){ 
      pkt.checksum += pkt.measurement[i]; 
	} 
    pkt.checksum += (int)(pkt.header); //Calculates the checksum on Arduino's side 
    myPacketSerial.send((uint8_t*)&pkt, sizeof(pkt)); 
  } 
  
  //Update previous  
  //a_prev = a_fil;  
  
  //Writing to Waveforms Test 
  /* 
  while(1) { 
    Serial.write( 'A' ); 
    Serial.write( 'B' ); 
    Serial.write( 'C' ); 
    Serial.println(""); 
	delay(1000); 
  } 
  */ 
  
  //Reading from Waveforms Test 
  /* 
  if (Serial.available() > 0) { 
	// get incoming byte: 
    inByte = Serial.read(); 
    Serial.println(thirdSensor); 
  } 
  */ 
   
  delay(50); 
  
  
} 
