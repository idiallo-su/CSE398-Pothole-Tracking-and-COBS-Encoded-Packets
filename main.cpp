//Project 2 main

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <libserial/SerialPort.h>
#include <libserial/SerialStream.h>
#include "COBSdecoder.cpp"

using namespace std;

struct recieve_packet {
  char header;
  float measurment[40];
  float orient_yaw;
  float orient_pitch;
  float orient_roll;
  uint8_t checksum;
};

//Our checksum will be created by adding the values of our header and 40 measurements
uint8_t calculateChecksum(char header, float measurments[]) {
  uint_t checkSum = 0.0;
  int headerInt = (int)(header);
  
  checkSum = checkSum + headerInt;
  
  for(int i=0; i<40; i++) { checkSum += measurments[i]; }
  
  return checkSum;
}

int main() {
  
  cout << "Hi1" << endl;
  libSerial::SerialPort arduino;
  arduino.Open( "/dev/ttyS0" );
  cout << "Hi2" << endl;
  arduino.SetBaudRate( LibSerial::BaudRate::BAUD_9600 );
  uint8_t c;
  receive_packet pkt;
  cobSerial myCOB;
  float RPiChecksum;
  
  time_t currentTime1;
  tm * currentTime2;
  char StringDate[100];
  char stringTime[100];
  std::ofstream TestFile;
  std::ofstream AccelFile;
  std::ofstream ForceFile;
  
  ctime(&currentTime);
  currentTime2 = localtime(&currentTime1);
  
  strftime(StringDate, 50, "Is is the %B %d %y", currentTime2);
  strftime(Stringtime, 50, "The time is %T", currentTime2);
  
  cout << StringDate << endl;
  cout << StringTime << endl;
  
  //Checking if opened properly
  if(!arduino.IsOpen()) {
    cout << "Serial Port is not open." << endl;
  }
  
  //Writing to Waveforms Demo
  /*
  my_serial_port.WriteByte( 'A' ); //41
  usleep(10000);
  my_serial_port.WriteByte( 'B' ); //42
  usleep(10000);
  my_serial_port.WriteByte( 'C' ); //43
  usleep(10000);
  my_serial_port.WriteByte( 'X' ); //58
  usleep(10000);
  my_serial_port.WriteByte( 'Y' ); //59
  usleep(10000);
  my_serial_port.WriteByte( 'Z' ); //5A
  usleep(10000);
  */
  
  //Reading from Waveforms
  /*
  while(1) {
    char next_char = 'B';
    int timeout_ms = 25000;
    
    while(my_serial_port.IsDataAvailable()) {
      my_serial_port.ReadByte(next_char, 0);
      usleep(1000);
      cout << next_char << endl;
      usleep(1000);
    }
  }
  */
