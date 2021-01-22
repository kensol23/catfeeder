// @author Kener Solorzano Farrier

#include <Servo.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <AnalogMultiButton.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <extEEPROM.h>
#include <RTClib.h>

#define SERVOIN 9
#define TECLADO A0
#define RTC1 A5
#define RTC2 A4
#define BUZZER A2
#define AUX A3
#define LCDRS 7
#define LCDE 6
#define LCDD4 5
#define LCDD5 4
#define LCDD6 3
#define LCDD7 2
#define I2C24C32Add  0x50

const int BOTONES_PIN = A0;
const int BOTONES_TOTAL = 4;
const int BOTONES_VALORES[BOTONES_TOTAL] = {0, 343, 526, 638};

const int BOTON_ATRAS = 0;
const int BOTON_ADELANTE = 1;
const int BOTON_ARRIBA = 2;
const int BOTON_ABAJO = 3;

AnalogMultiButton teclado(BOTONES_PIN, BOTONES_TOTAL, BOTONES_VALORES);

Servo miservo;
bool lid_closed = true;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  //pinMode(A0,INPUT);// inicia el pin A0
  
  pinMode(2,INPUT_PULLUP);
  miservo.attach(9);
  miservo.write(0);
}

void loop() {
  teclado.update();
 
  int boton = analogRead(A0);

  if(teclado.isPressed(BOTON_ATRAS))
  {
    Serial.println("ATRAS!!");
  }

  if(teclado.isPressed(BOTON_ADELANTE))
  {
    Serial.println("ADELANTE!!");
  }

  if(teclado.isPressed(BOTON_ARRIBA))
  {
    Serial.println("ARRIBA!!");
  }
  
  if(teclado.isPressed(BOTON_ABAJO))
  {
    Serial.println("ABAJO!!");
  }

  //Serial.println(boton);
  /*if(boton == 0)
  {
    miservo.write(180);
    //delay(300);
  } else {
    miservo.write(0);
  }*/
  //delay(2000);
  //delay(2000);
}
