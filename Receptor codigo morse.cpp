#include <Wire.h>			// libreria de comunicacion por I2C
#include <LCD.h>			// libreria para funciones de LCD
#include <LiquidCrystal_I2C.h>		// libreria para LCD por I2C
#include <SPI.h>  //librerias para modulo inalambrico
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

//Modulo inalambrico
const int CE = 8;
const int CSN = 53;

RF24 radio (CE, CSN);

const uint64_t canal = 0xE8E8F0F0E1LL;
int valores=0;
//pantalla
LiquidCrystal_I2C lcd (0x27, 2, 1, 0, 4, 5, 6, 7); // DIR, E, RW, RS, D4, D5, D6, D7
//seccion de morse
int tonePin = 2;
int toneFreq = 1000;
int ledPin = 13;
int buttonPin = 3;
int debounceDelay = 30;

int dotLength = 240;

  int dotSpace = dotLength;
  int dashLength = dotLength*3;
  int letterSpace = dotLength*3;
  int wordSpace = dotLength*7; 
  float wpm = 1200./dotLength;
  
int t1, t2, onTime, gap;
bool newLetter, newWord, letterFound, keyboardText;
int lineLength = 0;
int maxLineLength = 20; 

char* letters[] = 
{
".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", // A-I
".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", // J-R 
"...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.." // S-Z
};

char* numbers[] = 
{
"-----", ".----", "..---", "...--", "....-", //0-4
".....", "-....", "--...", "---..", "----." //5-9
};

String dashSeq = "";
char keyLetter, ch;
int i, index;
//fin de seccion codigo morse

//prueba recepcion
int LED = 6;

void setup()
{
  //declaraciones de pantalla
    lcd.setBacklightPin(3,POSITIVE);	// puerto P3 de PCF8574 como positivo
    lcd.setBacklight(HIGH);		// habilita iluminacion posterior de LCD
    lcd.begin(16, 2);			// 16 columnas por 2 lineas para LCD 1602A
    lcd.clear();			// limpia 
    




  //declaraciones para morse
  delay(500);
  pinMode(ledPin, OUTPUT);
  pinMode(tonePin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println();
  Serial.println("-------------------------------");
  Serial.println("Morse Code decoder/encoder");
  Serial.print("Speed=");
  Serial.print(wpm);
  Serial.print("wpm, ");
  Serial.print("dot=");
  Serial.print(dotLength);
  Serial.println("ms");   
  
// Test the LED and tone
  tone(tonePin, toneFreq);
  digitalWrite(ledPin, HIGH);
  delay(2000);
  digitalWrite(ledPin, LOW);
  noTone(tonePin);
  delay(600);

//flash to demonstrate the expected key speed
//A
   Serial.print("A .-  ");
   index = 'A'-65;
   flashSequence(letters[index]);
   delay(wordSpace);
//B
   Serial.print("B -...  ");
   index = 'B'-65;
   flashSequence(letters[index]);
   delay(wordSpace);
//C
   Serial.print("C -.-.  ");
   index = 'C'-65;
   flashSequence(letters[index]);
   delay(wordSpace);

  Serial.println();
  Serial.println("-------------------------------");
  Serial.println("Click field in Serial Monitor,");
  Serial.println("type text and press Enter, or");
  Serial.println("Key in Morse Code to decode:");
  Serial.println("-------------------------------");
      
  newLetter = false; //if false, do NOT check for end of letter gap
  newWord = false;  //if false, do NOT check for end of word gap
  keyboardText = false; 
  //fin declaraciones morse
//SECCION MODULO INALAMBRICO
radio.begin();
  radio.openReadingPipe(1,canal);
  radio.startListening();

  pinMode(LED , OUTPUT);//para probar recepcion

}

void loop()
{
     
      
  //seccion de pantalla
    //lcd.setCursor(0, 0);		// ubica cursor en columna 0 y linea 0
    //lcd.print("Hola, han pasado");	// escribe el texto
    /*
    lcd.setCursor(0, 1);		// ubica cursor en columna 0 y linea 1
    lcd.print(millis() / 1000);		// funcion millis() / 1000 para segundos transcurridos
    lcd.print(" seg.");			// escribe seg.    
*/
//seccion para morse
// Check to see if something has been entered on the keyboard
  if (Serial.available() > 0)
  {
    if (keyboardText == false) 
    {
      Serial.println();
      Serial.println("-------------------------------");
    }
    keyboardText = true;
    ch = Serial.read();
    if (ch >= 'a' && ch <= 'z')
    { ch = ch-32; }
    
    if (ch >= 'A' && ch <= 'Z')
    {
      Serial.print(ch); 
      Serial.print(" ");
      Serial.println(letters[ch-'A']);
      flashSequence(letters[ch-'A']);
      delay(letterSpace);
    }
    if (ch >= '0' && ch <= '9')
    {
      Serial.print(ch);
      Serial.print(" ");
      Serial.println(numbers[ch-'0']);
      flashSequence(numbers[ch-'0']);
      delay(letterSpace);
    }
    if (ch == ' ')
    {
      Serial.println("_");
      delay(wordSpace);    
    } 

// Print a header after last keyboard text    
     if (Serial.available() <= 0) 
     {
      Serial.println();
      Serial.println("Enter text or Key in:");
      Serial.println("-------------------------------");
      keyboardText = false;
     }
  }
 //----------------------------------------------------------------------------------------entrada lectura--------------------------------
  if(radio.available())
      {radio.read(&valores,sizeof(valores));

  if (valores==1 ) //button is pressed
  {
    digitalWrite(LED,HIGH);
    newLetter = true; 
    newWord = true;
    t1=millis(); //time at button press
    digitalWrite(ledPin, HIGH); //turn on LED and tone
    tone(tonePin, toneFreq);
    delay(debounceDelay);
    radio.read(&valores,sizeof(valores));     
    while (valores==1) // wait for button release
      {delay(debounceDelay);
      radio.read(&valores,sizeof(valores));}//agregue esta parte
      delay(debounceDelay);
    t2 = millis();  //time at button release
    onTime=t2-t1;  //length of dot or dash keyed in
    digitalWrite(ledPin, LOW); //torn off LED and tone
    noTone(tonePin); 
  // radio.read(&valores,sizeof(valores));//tercer punto prueba
//check if dot or dash 

    if (onTime <= dotLength*1.5) //allow for 50% longer 
      {dashSeq = dashSeq + ".";} //build dot/dash sequence
    else 
      {dashSeq = dashSeq + "-";}
  }  //end button press section
      }//cierre if radio
// look for a gap >= letterSpace to signal end letter
// end of letter when gap >= letterSpace

  gap=millis()-t2; 
  if (newLetter == true && gap>=letterSpace)  
  { 
    
//check through letter sequences to find matching dash sequence

    letterFound = false; keyLetter = 63; //char 63 is "?"
    for (i=0; i<=25; i++)
    {
      if (dashSeq == letters[i]) 
      {
        keyLetter = i+65;
        letterFound = true;   
        break ;    //don't keep checking if letter found  
      }
    }
    if(letterFound == false) //now check for numbers
    {
      for (i=0; i<=10; i++)
      {
      if (dashSeq == numbers[i]) 
        {
          keyLetter = i+48;
          letterFound = true;   
          break ;    //don't keep checking if number found  
        }
      }
    }    
    Serial.print(keyLetter);
    lcd.print(keyLetter);//------------------------------------------------prueba----------
    if(letterFound == false) //buzz for unknown key sequence
    {
      tone(tonePin, 100, 500);
    }  
    newLetter = false; //reset
    dashSeq = "";
    lineLength=lineLength+1;
  }  
  
// keyed letter has been identified and printed

// when gap is >= wordSpace, insert space between words
// lengthen the word space by 50% to allow for variation

  if (newWord == true && gap>=wordSpace*1.5)
    { 
     newWord = false; 
     Serial.print("_");
     lcd.print("_");//----------------------------------------prueba
     lineLength=lineLength+1;
     
// flash to indicate new word

    digitalWrite(ledPin, HIGH);
    delay(25);
    digitalWrite(ledPin, LOW);       
    } 

// insert linebreaks

  if (lineLength >= maxLineLength) 
    {
      Serial.println();
      lineLength = 0;
    }      
} 

void flashSequence(char* sequence)
{
  int i = 0;
  while (sequence[i] == '.' || sequence[i] == '-')
  {
    flashDotOrDash(sequence[i]);
    i++;
  }
}

void flashDotOrDash(char dotOrDash)
{
  digitalWrite(ledPin, HIGH);
  tone(tonePin, toneFreq);
  if (dotOrDash == '.')
   { delay(dotLength); }
     else
   { delay(dashLength); }

  digitalWrite(ledPin, LOW);
  noTone(tonePin);
  delay(dotLength); 

}