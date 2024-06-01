#include <SPI.h>
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

const int CE = 9;
const int CSN = 10;

//variable receptor
boolean off=false;

RF24 radio (CE, CSN);

const uint64_t canal = 0xE8E8F0F0E1LL;
int valor = 0;
int lectura_actual = 0;
int estado_anterior = 0;
int estado = 0;
void setup() {
  // put your setup code here, to run once:
  radio.begin();
  radio.openWritingPipe(canal);
  pinMode(5, INPUT_PULLUP);

Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  //delay(500);
  off= digitalRead(5);
  
  if(off){
    valor=0;
  }else{
    valor=1;
   /* Serial.print("presiono boton ");
    delay(500);
    */
  }
  radio.write(&valor,sizeof(valor)); 
  Serial.print(valor);
  delay(20);
  //exit(0);
}