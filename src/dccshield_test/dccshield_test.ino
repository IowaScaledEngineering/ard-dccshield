/*************************************************************************
Title:    ARD-DCCSHIELD test
Authors:  Michael Petersen <railfan@drgw.net>
File:     $Id: $
License:  GNU General Public License v3

LICENSE:
    Copyright (C) 2016 Nathan D. Holmes & Michael D. Petersen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

*************************************************************************/

/*  Test Jig Setup:
 *  12V DC in, connected to the DCC IN terminals.  Red LED across a 30ohm resistor
 *  in series with power.  This LED will light during an ACK pulse.
 *  
 *  RJ11 with 10k resistors from all pins to GND, except IORST.
 */

void setup()
{
	Serial.begin(9600);

  pinMode(2, INPUT_PULLUP);  // DCC Input
  pinMode(3, INPUT_PULLUP);  // DCC Input
  pinMode(7, INPUT_PULLUP);  // DCC Input
	pinMode(A1, OUTPUT); // ACK Output
  digitalWrite(A1, LOW);  // Turn off ACK

  pinMode(4, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  pinMode(A0, INPUT_PULLUP);

  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
}

void printDigitalIOs(void)
{
  Serial.print("Digital IOs: ");
  Serial.print(digitalRead(4));
  Serial.print(digitalRead(8));
  Serial.print(digitalRead(9));
  Serial.print(digitalRead(10));
  Serial.print(digitalRead(11));
  Serial.print(digitalRead(12));
  Serial.print(digitalRead(13));
  Serial.print(digitalRead(A0));
  Serial.print("\n");
}

void loop()
{
  Serial.print("Connect 12V to DCC IN... Check for D1 on\n");
  Serial.print("Swap 12V polarity... Check for D1 off\n");
  while(!Serial.available());
  while(Serial.available())
  {
    Serial.read();  // Flush the buffer
  }

  Serial.print("Checking for DCC on D2 (install JP4 right)... ");
  while(digitalRead(2));
  Serial.print("Done!\n");

  Serial.print("Checking for DCC on D3 (install JP4 left)... ");
  while(digitalRead(3));
  Serial.print("Done!\n");

  Serial.print("Checking for DCC on D7 (install JP2 right)... ");
  while(digitalRead(7));
  Serial.print("Done!\n");

  Serial.print("\nInstall JP3 (both), JP1 (both), JP2 (right)...\n");
  while(!Serial.available());
  while(Serial.available())
  {
    Serial.read();  // Flush the buffer
  }
  Serial.print("Checking I2C connector not connected...");
  while(!(digitalRead(5) && digitalRead(6)));
  Serial.print(" Done!\n");
  
  Serial.print("\nConnect I2C...\n\n");
  Serial.print("Checking D5 & D6 low...");
  while(digitalRead(5) || digitalRead(6));
  Serial.print(" Done!\n");

  Serial.print("Checking SDA and SCL...\n");
  int pass1 = 0, pass2 = 0;
  while(!pass1 || !pass2)
  {
    int adc;
    Serial.print("SDA: ");
    adc = analogRead(4);
    Serial.print(adc);
    if((adc > 400) && (adc < 600))
    {
      Serial.print(" Pass!\n");
      pass1 = 1;
    }
    else
    {
      Serial.print(" *** Fail ***\n");
      pass1 = 0;
    }
    Serial.print("SCL: ");
    adc = analogRead(5);
    Serial.print(adc);
    if((adc > 400) && (adc < 600))
    {
      Serial.print(" Pass!\n");
      pass2 = 1;
    }
    else
    {
      Serial.print(" *** Fail ***\n");
      pass2 = 0;
    }
    Serial.print("--------------------\n");
    delay(500);
  }  

  Serial.print("\nInstall JP7 (both)...\n");
  while(!Serial.available());
  while(Serial.available())
  {
    Serial.read();  // Flush the buffer
  }

  Serial.print("DCC ACK (install JP7 jumpers).  Check blinking LED...\n");
  while(!Serial.available())
  {
    digitalWrite(A1, HIGH);
    printDigitalIOs();
    delay(500);
    digitalWrite(A1, LOW);
    printDigitalIOs();
    delay(500);
  }

  Serial.print("Install JP5, disconnect USB, measure 5V output...\n");
  Serial.print("Bye bye!\n");
  while(1);
}

