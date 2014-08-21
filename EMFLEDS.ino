#include "FastLED.h"

#define LEDS_PER_BLOCK 150
#define NUM_BLOCKS 6

CRGB leds[NUM_BLOCKS][LEDS_PER_BLOCK];

int valr,valg,valb;
int rate;
boolean up;
boolean long_command;

const int row_length = 30;
const int rows_per_block = 5;

#define IMAGE_BUFFER_SIZE 2700
#define BUFFER_SIZE 60

unsigned short image_buffer[IMAGE_BUFFER_SIZE];  // Final Buffer
unsigned short serial_buffer[60];   // Serial buffer
int image_buffer_pos;


void setup() {
  

  Serial.begin(9600);
  
  FastLED.addLeds<TM1809, 5>(leds[0], LEDS_PER_BLOCK ); 
  FastLED.addLeds<TM1809, 6>(leds[1], LEDS_PER_BLOCK );
  FastLED.addLeds<TM1809, 7>(leds[2], LEDS_PER_BLOCK ); 
  FastLED.addLeds<TM1809, 8>(leds[3], LEDS_PER_BLOCK ); 
  FastLED.addLeds<TM1809, 9>(leds[4], LEDS_PER_BLOCK ); 
  FastLED.addLeds<TM1809,10>(leds[5], LEDS_PER_BLOCK );
 
  
  pinMode(13, OUTPUT);   
  
  valr = 5 ;
  valg = 5;
  valb = 5;
  rate = 5;
  up = true;
  clear_image();
  clear_buffer();
  
  image_buffer_pos = 0;
 
}

void clear_buffer() {
  while(Serial.available()){
    Serial.read();
  }
}

void clear_image() {
  for (unsigned int j=0; j < NUM_BLOCKS; j++){
    for (unsigned int i=0; i < LEDS_PER_BLOCK; i++){
      leds[j][i] = CRGB::Black;
    }
  }
  FastLED.show();
}


void send_error_message(byte message){

  clear_buffer();
  Serial.write(message);
  byte ack = Serial.read();
  while(ack != 0x42)
    ack = Serial.read();
  
}


bool read_serial_terminator() {
  if (Serial.available() > 0){
    byte final = Serial.read();
    
    if (final != 0x96){
       send_error_message(0x96);
      return false;
    }else {
      Serial.write(0x99);
      return true;
    }
  }
  send_error_message(0x96);
  return false;
}

void read_serial_image(){
  int i = 0;
  
  if (Serial.available() > BUFFER_SIZE){
    for (i= 0; i < BUFFER_SIZE; i++){
      image_buffer[image_buffer_pos] = Serial.read();
      image_buffer_pos++;
    }
    if( read_serial_terminator() && image_buffer_pos == IMAGE_BUFFER_SIZE){
      set_image();
    
      image_buffer_pos = 0;
    }
  }
}

void read_serial_clear() {
 
  if (read_serial_terminator()){
    clear_image();
  }
}

void read_serial() {
   
    byte preamble,command,data,final;
  
    if (Serial.available() > 1) {
      
     
       preamble = Serial.read();
     
       if (preamble == 0x99){
         command = Serial.read();
         switch (command){
           case 0x69 :  
             read_serial_image();
             break;
            
            case 0x70:
              read_serial_clear();
              break; 
             
            case 0x71:
              for(int i=0; i < 500; i ++){
                test_func();
                delay(20);
              }
              clear_image();
              read_serial_terminator();
              break; 
            
            default:
              read_serial_terminator();
              send_error_message(0x97);
         }
       
       }else {
         send_error_message(0x95);
       }
     }
    
  
}


void set_image() {
  
  for(int k=0; k < NUM_BLOCKS; k++){ 
    for (int i=0; i < rows_per_block; i++){
      int bt = ((k * rows_per_block * row_length * 3) + (i * row_length * 3));
      
    // Even rows right to left
      if (i % 2 == 0) {
        for (int j=0; j < row_length; j++){
          leds[k][ (row_length * (i + 1)) -1 - j  ].setRGB( image_buffer[bt + j * 3], image_buffer[bt + j * 3 + 1], image_buffer[bt + j * 3 + 2]);
        }
        
      } else {
        for (int j=0; j < row_length; j++){
          leds[k][(row_length * i) + j  ].setRGB( image_buffer[bt + j * 3], image_buffer[bt + j * 3 + 1], image_buffer[bt + j * 3 + 2]);
        } 
      }
    }
  }
    
  FastLED.show();
}


void test_func() {
  
  for (unsigned int k=0; k < NUM_BLOCKS; k++){
    for (unsigned int i=0; i < LEDS_PER_BLOCK; i++){
      leds[k][i] = CRGB(valb,valr,valg);

    }
  }
  FastLED.show();
  
  delay(10);
  
  if (up){
    if (valr < 255) valr+=rate;
    else{
      if (valg < 255) valg+=rate;
      else {
        if (valb < 255) valb+=rate;
        else
          up = false;
      }
    }
  } else {
    if (valr > 0) valr-=rate;
    else{
      if (valg > 0) valg-=rate;
      else {
        if (valb > 0) valb-=rate;
        else
          up = true;
      }
    }
  }
}

// It appears that its not RGB so much as BRG! This happens with Chinese LEDS sometimes, apparently

void loop() { 
  
  //test_func();
  
  //delay(20);
 
  read_serial();
  
}
