/*
Copyright (c) 2020, seivasan79@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
  
*/

#include "U8glib.h"
#include "avdweb_VirtualDelay.h"

U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE); // I2C / TWI

#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire_mandata(2);
OneWire oneWire_esterna(3);
DallasTemperature temp_mandata(&oneWire_mandata);
DallasTemperature temp_esterna(&oneWire_esterna);

VirtualDelay delay1;

bool b;
float tm = 0;
float te = 0;
int apertura = 6; 
int chiusura = 7; 
int pot = A0; //comfort
int potk1 = A1; //fattore k
float potmin = A2; //intervento anticondensa
int setpt;
int anticondensa;
int setpoint; //temperatura ambiente voluta
int mandata;  //temperatura mandata
float K1;   // fattore k
int deltaT1;  // differenza tra setpoint e temp ext
int deltaT2;  // differenza tra setpoint e temp interna
int Tvol;
int Tvol_1;
unsigned long time;
unsigned long lampeggio_time = 0;
int selettore = 13;
int selettore_lettura;
char *modo;
int setpoint_man;
char *valve;



void setup() {
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255, 255, 255);
  }

  temp_mandata.begin();
  temp_esterna.begin();
  pinMode(selettore, INPUT_PULLUP);
  pinMode(apertura, OUTPUT);
  pinMode(chiusura, OUTPUT);
  pinMode(pot, INPUT);
  pinMode(potk1, INPUT);
  pinMode(potmin, INPUT);
  digitalWrite(chiusura, HIGH);
  digitalWrite(apertura, HIGH);
  Serial.begin(9600);

}

void loop() {
  potk1 = analogRead(A1);
  potmin = analogRead(A2);
  K1 = (float)map(potk1, 0, 1023, 4000, 0) / 1000;
  anticondensa = map(potmin, 0, 1023, 60, 5);
  deltaT1 = (setpoint - te);
  deltaT2 = (setpoint - 6);
  Tvol = setpoint + (deltaT1 * K1) + (deltaT2 * K1);
  Tvol_1 = constrain(Tvol, 10, 90);
  temp_mandata.requestTemperatures();
  temp_esterna.requestTemperatures();
  tm = temp_mandata.getTempCByIndex(0);
  te = temp_esterna.getTempCByIndex(0);
  selettore_lettura = digitalRead(selettore);
  time = millis();
  setpt = analogRead(pot);
  setpoint = map(setpt, 0, 1023, 25, 11);
  setpoint_man = map(setpt, 0, 1023, 80, 4);

  if (selettore_lettura == HIGH) {
    Tvol_1 = Tvol_1;
    modo = "auto";
  }
  if (selettore_lettura == LOW) {
    Tvol_1 = setpoint_man;
    modo = "man";
  }

  if (tm > Tvol_1 + 3 or tm < anticondensa) {
    valve = "-";
    chiudi();
  } else {
    digitalWrite(chiusura, HIGH);
    valve = "0";
  }


  if (tm < Tvol_1 - 3 and tm > anticondensa) {
    valve = "+";
    apri();
  } else {
    digitalWrite(apertura, HIGH);
  }

  u8g.firstPage();
  do {
    draw();
  } while ( u8g.nextPage() );



  Serial.print("Setpoint  ");
  Serial.print(setpoint);
  Serial.print(" C tm  ");
  Serial.print(tm);
  Serial.print("    C te    ");
  Serial.print(te);
  Serial.print("    T Vol  ");
  Serial.print(Tvol  );
  Serial.print(" SetMan    ");
  Serial.print(setpoint_man);
  Serial.print("   mode  ");
  Serial.print(modo);
  Serial.print("   contrain  ");
  Serial.println(Tvol_1);


}



void draw(void) {
  u8g.setFont(u8g_font_ncenB08);
  u8g.setPrintPos(0, 10);
  u8g.print(" Seiv McDuell DIY");
  u8g.setFont(u8g_font_6x12);
  u8g.setPrintPos(0, 40);
  u8g.print("T.ext     C ");
  u8g.print(te);
  u8g.setPrintPos(0, 30);
  u8g.print("T.supply  C ");
  u8g.print(tm);
  u8g.setPrintPos(0, 20);
  u8g.print("Amb       C ");
  u8g.print(setpoint);
  u8g.setPrintPos(0, 50);
  u8g.print("Stpoint   C ");
  u8g.print(Tvol_1);
  u8g.setPrintPos(95, 60);
  u8g.print("Dir:");
  u8g.print(valve);
  u8g.setPrintPos(0, 60);
  u8g.print("K:");
  u8g.print(K1);
  u8g.setPrintPos(40, 60);
  u8g.print(" T_Min:");
  u8g.print(anticondensa);
  u8g.setFont(u8g_font_7x13O);
  u8g.setPrintPos(100, 20);
  u8g.print(modo);

}

void apri() {
  digitalWrite(chiusura, HIGH);
  delay1.start(1500);
  if (delay1.elapsed())
    digitalWrite(apertura, b = !b);
}

void chiudi() {
  digitalWrite(apertura, HIGH);
  delay1.start(1500);
  if (delay1.elapsed())
    digitalWrite(chiusura, b = !b);
}
