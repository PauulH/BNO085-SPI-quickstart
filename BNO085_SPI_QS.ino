
// Quck start SPI comms with BNO085 IMU


#include <SPI.h>


// Define the pins
#define BNO08X_CS 17
#define BNO08X_INT 16
#define BNO08X_RESET 18
#define BNO08X_WAKE 0

// for storing the payloads
uint8_t cargo[500];

//create the product ID request and continous rotation vector request
uint8_t RequestProductID[] = { 6, 0, 2, 0, 0xF9, 0 };
uint8_t RequestRotationVectorReport[] = { 21, 0, 2, 0, 0xFD, 0x05, 0, 0, 0, 0b11000100, 0b00001001, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


void setup() {

  Serial.begin(115200);

// setup the digital pins
  pinMode(BNO08X_WAKE, OUTPUT);   
  pinMode(BNO08X_CS, OUTPUT);     
  pinMode(BNO08X_RESET, OUTPUT);  
  pinMode(BNO08X_INT, INPUT);  

  //set the chip select and wake pins    
  digitalWrite(BNO08X_CS, HIGH);
  digitalWrite(BNO08X_WAKE, HIGH);

// setup the  SPI comms. Max SPI clock frequence is 3MHz according to BNO085 datasheet
  SPI.begin();
  SPI.beginTransaction(SPISettings(3000000, MSBFIRST, SPI_MODE3));
 // delay(200);

// according to section 6.5.3 of the BNO085 datasheet, the reset pin should be pulled low for a minimum of 10ns before internal initialization will begin
// after the reset pin in pulled high there in an internal initialization period of minimum 90ms
  digitalWrite(BNO08X_RESET, LOW);
  delayMicroseconds(1);
  digitalWrite(BNO08X_RESET, HIGH);
  delay(200);

// Wait for the interrupt pin to be desserted. Once this occurs we must wait for internal configuration to occur. This will take about 4ms
// see section 6.5.3 of the datasheet for timing details
  while(!digitalRead(BNO08X_INT));
  delay(20);

//the interrupt pin has de-asserted and the internal configuration time is complete. We can now read the SHTP data.
  IMU_read_SHTP();

 
// wait for the interrupt pin to be asserted then read the reset complete message
  while (digitalRead(BNO08X_INT));
  IMU_read_RESET();

// wait for the interrupt pin to be asserted then read the reset complete message
  while (digitalRead(BNO08X_INT));
  IMU_read_INITIALIZATION_MESSAGE();

// we now have to wake the sensor up and send the product ID request
  IMU_send_PRODUCT_ID();

// we now have to wake the sensor up and read the product ID response number 1
  while (digitalRead(BNO08X_INT));
  IMU_read_PRODUCT_ID_1();

// read the product ID response number 2
  while (digitalRead(BNO08X_INT));
  IMU_read_PRODUCT_ID_2();

delay(5000);

// set up the continous sensor report
  IMU_send_ROTATION_VECTOR_REQUEST();
 //delay(5000);
// wait for the interrupt pin to be asserted then read the reset complete message


// setup the interrupt that will read the continous rotation vector report
  attachInterrupt(digitalPinToInterrupt(BNO08X_INT), IMU_read, FALLING);
}






void loop() {



delay(1000);
}



//--------SHTP advertisement read-----------------------
void IMU_read_SHTP()
{

int length;
unsigned int result = 0;
digitalWrite(BNO08X_CS, LOW); 

// read the first 4 bytes (SHTP header)
  for (int i=0; i<4; i++){
  result = SPI.transfer(0);
  cargo[i] = result;
  Serial.print(cargo[i],HEX);
  Serial.print(" , ");
  }

// determine the length of the payload
length = cargo[1] << 8;
length = length | cargo[0];
//Discard bit 15
length = length & 32767;


// read the rest of the payload
  for (int i=4; i<(length) ; i++){
  result = SPI.transfer(0);
  cargo[i] = result;
  Serial.print(cargo[i],HEX);
  Serial.print(" , ");
  }

Serial.println("....");
digitalWrite(BNO08X_CS, HIGH); 



//------------display a few values so we can see its working---------------------------------
Serial.print("Payload length-");
Serial.println(length);


//SHTP version...5.3 SHTP Advertisement
Serial.print("SHTP version - ");
Serial.write(cargo[13]);
Serial.write(cargo[14]);
Serial.write(cargo[15]);
Serial.write(cargo[16]);
Serial.write(cargo[17]);
Serial.println("..");

//application name
Serial.print("Application name - ");
Serial.write(cargo[37]);
Serial.write(cargo[38]);
Serial.write(cargo[39]);
Serial.write(cargo[40]);
Serial.println("..");


//channel name
Serial.print("Channel name - ");
Serial.write(cargo[47]);
Serial.write(cargo[48]);
Serial.write(cargo[49]);
Serial.write(cargo[50]);
Serial.write(cargo[51]);
Serial.write(cargo[52]);
Serial.write(cargo[53]);
Serial.println("..");
Serial.println("..");
Serial.println("..");

}



//--------SHTP advertisement read-----------------------

void IMU_read_RESET()
{


int length;
unsigned int result = 0;
digitalWrite(BNO08X_CS, LOW); 

// read the first 4 bytes (SHTP header)
  for (int i=0; i<4; i++){
  result = SPI.transfer(0);
  cargo[i] = result;
  }

// determine the length of the payload
length = cargo[1] << 8;
length = length | cargo[0];
//Discard bit 15
length = length & 32767;

// read the rest of the payload
  for (int i=4; i<(length) ; i++){
  result = SPI.transfer(0);
  cargo[i] = result;
  }


digitalWrite(BNO08X_CS, HIGH); 

// display some info
Serial.print("Reset complete message length - ");
Serial.println(length);

Serial.print("Reset result - ");

switch (cargo[4]) {
  case 0:
    Serial.println("reset error");
    break;
  case 1:
    Serial.println("reset complete");
    break;
  default:
    // statements
    break;
}

Serial.println("..");
Serial.println("..");


}


//--------------Initialization message read---------------------------

 void IMU_read_INITIALIZATION_MESSAGE(){

int length;
unsigned int result = 0;   

Serial.println("Initialization message"); 

digitalWrite(BNO08X_CS, LOW); 

// read the first 4 bytes (SHTP header)
  for (int i=0; i<4; i++){
  result = SPI.transfer(0);
  cargo[i] = result;
  Serial.print(cargo[i],HEX);
  Serial.print(" , ");
  }

// determine the length of the payload
length = cargo[1] << 8;
length = length | cargo[0];
//Discard bit 15
length = length & 32767;

// read the rest of the payload
  for (int i=4; i<(length) ; i++){
  result = SPI.transfer(0);
  cargo[i] = result;
  Serial.print(cargo[i],HEX);
  Serial.print(" , ");
  }

Serial.println("....");
digitalWrite(BNO08X_CS, HIGH); 

//display some info
Serial.print("Payload length- ");
Serial.println(length);
Serial.println("..");
Serial.println("..");

}


//--------------------send the product ID request---------------------------
void IMU_send_PRODUCT_ID(){

digitalWrite(BNO08X_WAKE, LOW);

while (digitalRead(BNO08X_INT));
digitalWrite(BNO08X_CS, LOW);
digitalWrite(BNO08X_WAKE, HIGH);

  for (int i = 0; i < 6; i++) {
    cargo[i] = SPI.transfer(RequestProductID[i]);
    Serial.print(cargo[i], HEX);
    Serial.print(" , ");
  }
Serial.println("---- Product ID request sent");
digitalWrite(BNO08X_CS, HIGH);

Serial.println("..");
Serial.println("..");

}


//----------------------read the product ID payload 1--------------------
void IMU_read_PRODUCT_ID_1(){

int length;
unsigned int result = 0;  

Serial.println("1st product ID report "); 

digitalWrite(BNO08X_CS, LOW); 

// read the first 4 bytes (SHTP header)
  for (int i=0; i<4; i++){
  result = SPI.transfer(0);
  cargo[i] = result;
  Serial.print(cargo[i],HEX);
  Serial.print(" , ");
  }

// determine the length of the payload
length = cargo[1] << 8;
length = length | cargo[0];
//Discard bit 15
length = length & 32767;

// read the rest of the payload
  for (int i=4; i<(length) ; i++){
  result = SPI.transfer(0);
  cargo[i] = result;
  Serial.print(cargo[i],HEX);
  Serial.print(" , ");
  }

Serial.println("....");
digitalWrite(BNO08X_CS, HIGH); 

//display some info
Serial.print("Payload length - ");
Serial.println(length);
Serial.println("..");
Serial.println("..");

}

//---------------------read the product ID payload 1--------------------
void IMU_read_PRODUCT_ID_2(){

int length;
unsigned int result = 0;  

Serial.println("2nd product ID report "); 

digitalWrite(BNO08X_CS, LOW); 

// read the first 4 bytes (SHTP header)
  for (int i=0; i<4; i++){
  result = SPI.transfer(0);
  cargo[i] = result;
  Serial.print(cargo[i],HEX);
  Serial.print(" , ");
  }

// determine the length of the payload
length = cargo[1] << 8;
length = length | cargo[0];
//Discard bit 15
length = length & 32767;

// read the rest of the payload
  for (int i=4; i<(length) ; i++){
  result = SPI.transfer(0);
  cargo[i] = result;
  Serial.print(cargo[i],HEX);
  Serial.print(" , ");
  }

Serial.println("....");
digitalWrite(BNO08X_CS, HIGH); 

//display some info
Serial.print("Payload length - ");
Serial.println(length);
Serial.println("..");
Serial.println("..");

}

//-----------send the continous rotation vector request-------------------------
void  IMU_send_ROTATION_VECTOR_REQUEST(){

digitalWrite(BNO08X_WAKE, LOW);

while (digitalRead(BNO08X_INT));
digitalWrite(BNO08X_CS, LOW);
digitalWrite(BNO08X_WAKE, HIGH);

  for (int i = 0; i < 21; i++){
  SPI.transfer(RequestRotationVectorReport[i]);
  Serial.print(cargo[i], HEX);
  Serial.print(" , ");}

Serial.println("---- Rotation vector request sent");
digitalWrite(BNO08X_CS, HIGH);


}

//----------read the continous rotation vector report---------------------
void IMU_read()
{
int length;
unsigned int result = 0;

digitalWrite(BNO08X_CS, LOW); 

// read the first 4 bytes (SHTP header)
  for (int i=0; i<4; i++){
  result = SPI.transfer(0);
  cargo[i] = result;
  Serial.print(cargo[i],HEX);
  Serial.print(" , ");
  }

// determine the length of the payload
length = cargo[1] << 8;
length = length | cargo[0];
// Discard bit 15
length = length & 32767;

// read the rest of the payload
  for (int i=4; i<(length) ; i++){
  result = SPI.transfer(0);
  cargo[i] = result;
  Serial.print(cargo[i],HEX);
  Serial.print(" , ");
  }
digitalWrite(BNO08X_CS, HIGH);
Serial.println("... ");

}