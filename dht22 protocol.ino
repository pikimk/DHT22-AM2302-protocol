#define PIN 5 // note, this is pin 5 from port d


void setup(){
Serial.begin(9600);
}

uint8_t data[5] = {0}; //set all to 0 



void loop(){

  // data direction register (0x2B) set the 5th bit high ( output )
  DDRD |= ( 1 << PIN);

  // set pin 5 to port D (0x2A) high
  PORTD |= ( 1 << PIN );

  delay(100);

  //start of sequence first pull low for 5ms;
  PORTD &= ~( 1 << PIN );
  delay(1);
  PORTD |= ( 1 << PIN);
  delayMicroseconds(30);

  //start reading, by setting data direction bit 5 to low for input
  DDRD &= ~(1 << 5);
  delayMicroseconds(40);

  //read first bit
  bool bitLow = (PIND >> PIN) & 1;
  delayMicroseconds(80);

  //read the second bit
  bool bitHigh = (PIND >> PIN) & 1;
  delayMicroseconds(40);
  //sensor ack finished

  // now start reading bits
  int time50us = 0;
  int timeLowHigh =0;
  if(bitLow == false && bitHigh == true){ // sensor sending ready for data transmit
  
  //byte number loop
  for(int i = 0; i < 5; i++){

    //bit number loop
    for(int j = 7; j >= 0; j--){

      // Count 50us
       while (((PIND >> PIN) & 1) == false){ 
          time50us++;
        }

        // check timing for on/off state after the period of 50us
        while (((PIND >> PIN) & 1) == true){
          timeLowHigh++;
        }

        //if it is more then 50us, set bit high, else set bit low

        if(time50us < timeLowHigh){
          data[i] |= (1 << j);
        }else{
          data[i] &= ~(1 << j);
        }
        
        //reset timing
        time50us = timeLowHigh = 0;

    }

  }

  uint8_t checksum = data[0] + data[1]+ data[2] + data[3];
  if( checksum == data[4]){

      int16_t tempData = 0; // signed 16 bit cause we expect negative temp
      uint16_t humData = 0;
 

     //Shifting 2 bytes into one ex. 01100100 and 00010011 into 0110010000010011 - humidity 
     humData |= ((data[0] << 8) | data[1]);
     //Shifting 2 bytes into one ex. 01100100 and 00010011 into 0110010000010011 - humidity 
     tempData |= ((data[2] << 8) | data[3]);

    //handle negative temp, the 15t bit is 1 if it is negative, other 14 show the values
    if (tempData < 0){
      // the 15th bit is 1 means negative value

      tempData &= ~(1 << 15);
      //set it like positive, and multiply with -1
      tempData*= -1;
      
    }

     //we need to get decimal values since we have high and low byte ex. 239 / 10 = 23.9
     float temp = (float)tempData / 10;
     float hum = (float)humData / 10;
  

     //we got the values now handling
     Serial.print("Temperature - ");
     Serial.println(temp);
     Serial.print("Humidity - ");
     Serial.println(hum);

    }else{
    //Transmision error, bits dont add up
    Serial.println("Error Checksum");
    }
  
  }else{ // Sensor error
    Serial.println("Error");
  }

//sensor reset time
delay(1000);
 
}


