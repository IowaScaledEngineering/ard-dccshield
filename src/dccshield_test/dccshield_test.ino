// Iowa Scaled Engineering
// This sketch is for our test harness for the ARD-DCCSHIELD v2.0
// This requires the INA219 library from here:
//  https://github.com/flav1972/ArduinoINA219


#define DIN_DCC_SIG_D7      6
#define DIN_DCC_SIG_D3      5
#define DIN_DCC_SIG_D2      4

#define DOUT_DCC_SIG_ACK    7
#define DOUT_DCC_POLARITY   3
#define DOUT_LOAD_RESISTOR  2

#define DOUT_I2C_SDA        A4
#define DOUT_I2C_SCL        A5

#define AIN_DCCSHIELD_PWR   A0
#define AIN_I2C_SDA         A2
#define AIN_I2C_SCL         A1
#define AIN_I2C_PWR         A3

#include <Wire.h>
#include <INA219.h>


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  pinMode(DIN_DCC_SIG_D2, INPUT_PULLUP);
  pinMode(DIN_DCC_SIG_D3, INPUT_PULLUP);
  pinMode(DIN_DCC_SIG_D7, INPUT_PULLUP);
  
  pinMode(DOUT_LOAD_RESISTOR, OUTPUT);
  digitalWrite(DOUT_LOAD_RESISTOR, false);
  pinMode(DOUT_DCC_POLARITY, OUTPUT);
  digitalWrite(DOUT_DCC_POLARITY, false);

  pinMode(DOUT_DCC_SIG_ACK, OUTPUT);
  digitalWrite(DOUT_DCC_SIG_ACK, false);
  
  Serial.print("\n\n");

}

void waitForKey()
{
  while(!Serial.available());
  while(Serial.available())
  {
    Serial.read();  // Flush the buffer
  }
}

void loop() 
{
  bool d2, d3, d7, pass;
  bool allPass = true;
  char buffer[128];
  
  // put your main code here, to run repeatedly:
  Serial.print(F("\nSet all dip switches ON and \"POWER FROM DCC\"\n"));
  Serial.print(F("\n** Press any key to start test **\n"));
  waitForKey();

  // Turn off DCC reversing relay to minimize voltage droop
  digitalWrite(DOUT_DCC_POLARITY, true);
  delay(100);  

  {
    Wire.begin();
    INA219 v12mon;
    v12mon.begin();
    float busV = 0.0;

    do
    {
      busV = v12mon.busVoltage();

      Serial.print(F("12V Supply:   "));
      Serial.print(busV, 4);
      Serial.println(" V");
      if(busV < 11.5)
      {
        Serial.print(F("ERROR: Eanble 12V Supply"));
        delay(1000);
      }
    } while (busV < 11.5);
    Wire.end();
  }  

  // Supply voltage is on a 1/3 divider
  float supplyVoltage = 3.0 * analogRead(AIN_DCCSHIELD_PWR) * (5.0 / 1023.0);
  pass = false;
  if (supplyVoltage > 4.75 && supplyVoltage < 5.25)
    pass = true;
  
  sprintf(buffer, "Onboard supply voltage (unloaded) %dmV ... %s\n", (int)(supplyVoltage * 1000), pass?"PASS":"!!FAIL!!");
  Serial.print(buffer);

  if (!pass)
  {
    Serial.print(F("\nOnboard supply voltage check has failed\nPress any key to restart, will not continue\n"));
    waitForKey();
    return;    
  }

  {
    Wire.begin();
    INA219 v12mon;
    v12mon.begin();
    float busI = v12mon.shuntCurrent() * 1000.0;

    Serial.print(F("12V Supply Current (unloaded)   "));
    Serial.print(busI, 4);
    Serial.print(F(" mA"));
    Wire.end();

    if (busI < 20.0)
    {
      Serial.println(F(" PASS"));
    } else {
      Serial.println(F(" FAIL!"));
      Serial.print(F("\nIdle supply check has failed\nPress any key to restart, will not continue\n"));
      waitForKey();
      return;    
    }
    
  }  

  Serial.print("Loading onboard supply\n");
  digitalWrite(DOUT_LOAD_RESISTOR, true);
  delay(500);
  // Supply voltage is on a 1/3 divider
  supplyVoltage = 3.0 * analogRead(AIN_DCCSHIELD_PWR) * (5.0 / 1023.0);
  pass = false;
  if (supplyVoltage > 4.7 && supplyVoltage < 5.3)
    pass = true;

  {
    Wire.begin();
    INA219 v12mon;
    v12mon.begin();
    float busI = v12mon.shuntCurrent() * 1000.0;

    Serial.print(F("12V Supply Current (loaded)   "));
    Serial.print(busI, 4);
    Serial.print(" mA");
    Wire.end();

    if (busI > 150.0 && busI < 186.0)
    {
      Serial.println(F(" PASS"));
    } else {
      Serial.println(F(" FAIL!"));
      Serial.print(F("\nLoaded supply current check has failed\nPress any key to restart, will not continue\n"));
      waitForKey();
      return;    
    }

  
  }  

  digitalWrite(DOUT_LOAD_RESISTOR, false);
  Serial.print("Unloading onboard supply\n");
  
  sprintf(buffer, "Onboard supply voltage (loaded) %dmV ... %s\n", (int)(supplyVoltage * 1000), pass?"PASS":"!!FAIL!!");
  Serial.print(buffer);

  if (!pass)
  {
    Serial.print(F("\nOnboard supply voltage check has failed\nPress any key to restart, will not continue\n"));
    waitForKey();
    return;    
  }


  // Qwiic rail check is a divider between power and ground to test both - should be 3.3V / 2
  supplyVoltage = 2.0 * analogRead(AIN_I2C_PWR) * (5.0 / 1023.0);
  pass = false;
  if (supplyVoltage > 3.1 && supplyVoltage < 3.5)
    pass = true;
  
  sprintf(buffer, "Qwiic supply voltage %dmV ... %s\n", (int)(supplyVoltage * 1000), pass?"PASS":"!!FAIL!!");
  Serial.print(buffer);

  if (!pass)
  {
    Serial.print(F("\nOnboard supply voltage check has failed\nPress any key to restart, will not continue\n"));
    waitForKey();
    return;    
  }

  digitalWrite(DOUT_I2C_SDA, true);
  digitalWrite(DOUT_I2C_SCL, true);
  
  pinMode(DOUT_I2C_SDA,OUTPUT); // setup A4 and A5 as outputs
  pinMode(DOUT_I2C_SCL,OUTPUT);
  digitalWrite(DOUT_I2C_SDA, true);
  digitalWrite(DOUT_I2C_SCL, true);

  sprintf(buffer, "Testing Qwiic data lines\n");
  Serial.print(buffer);

  delay(100);
  
  // Qwiic rail check is a divider between power and ground to test both - should be 3.3V / 2
  supplyVoltage = analogRead(AIN_I2C_SDA) * (5.0 / 1023.0);
  pass = false;
  if (supplyVoltage > 3.1 && supplyVoltage < 3.5)
    pass = true;

  allPass = allPass && pass;
  
  sprintf(buffer, "Qwiic SDA high voltage %dmV ... %s\n", (int)(supplyVoltage * 1000), pass?"PASS":"!!FAIL!!");
  Serial.print(buffer);

  digitalWrite(DOUT_I2C_SDA, false);
  delay(100);

  // Qwiic rail check is a divider between power and ground to test both - should be 3.3V / 2
  supplyVoltage = analogRead(AIN_I2C_SDA) * (5.0 / 1023.0);
  pass = false;
  if (supplyVoltage < 0.4)
    pass = true;

  allPass = allPass && pass;
  
  sprintf(buffer, "Qwiic SDA low voltage %dmV ... %s\n", (int)(supplyVoltage * 1000), pass?"PASS":"!!FAIL!!");
  Serial.print(buffer);

  // Qwiic rail check is a divider between power and ground to test both - should be 3.3V / 2
  supplyVoltage = analogRead(AIN_I2C_SCL) * (5.0 / 1023.0);
  pass = false;
  if (supplyVoltage > 3.1 && supplyVoltage < 3.5)
    pass = true;

  allPass = allPass && pass;
  
  sprintf(buffer, "Qwiic SCL high voltage while SDA low %dmV ... %s\n", (int)(supplyVoltage * 1000), pass?"PASS":"!!FAIL!!");
  Serial.print(buffer);


  digitalWrite(DOUT_I2C_SDA, true);
  delay(100);
  
  // Qwiic rail check is a divider between power and ground to test both - should be 3.3V / 2
  supplyVoltage = analogRead(AIN_I2C_SCL) * (5.0 / 1023.0);
  pass = false;
  if (supplyVoltage > 3.1 && supplyVoltage < 3.5)
    pass = true;

  allPass = allPass && pass;
  
  sprintf(buffer, "Qwiic SCL high voltage %dmV ... %s\n", (int)(supplyVoltage * 1000), pass?"PASS":"!!FAIL!!");
  Serial.print(buffer);

  digitalWrite(DOUT_I2C_SCL, false);
  delay(100);


  // Qwiic rail check is a divider between power and ground to test both - should be 3.3V / 2
  supplyVoltage = analogRead(AIN_I2C_SCL) * (5.0 / 1023.0);
  pass = false;
  if (supplyVoltage < 0.4)
    pass = true;

  allPass = allPass && pass;
  
  sprintf(buffer, "Qwiic SCL low voltage %dmV ... %s\n", (int)(supplyVoltage * 1000), pass?"PASS":"!!FAIL!!");
  Serial.print(buffer);

  // Qwiic rail check is a divider between power and ground to test both - should be 3.3V / 2
  supplyVoltage = analogRead(AIN_I2C_SDA) * (5.0 / 1023.0);
  pass = false;
  if (supplyVoltage > 3.1 && supplyVoltage < 3.5)
    pass = true;

  allPass = allPass && pass;
  
  sprintf(buffer, "Qwiic SDA high voltage while SCL low %dmV ... %s\n", (int)(supplyVoltage * 1000), pass?"PASS":"!!FAIL!!");
  Serial.print(buffer);



  digitalWrite(DOUT_I2C_SCL, true);


  if (!allPass)
  {
    Serial.print(F("\nQwiic Level Shifter voltage tests failed\nPress any key to restart, will not continue\n"));
    waitForKey();
    return;    
  }

  pinMode(DOUT_I2C_SDA,INPUT_PULLUP); // setup A4 and A5 as outputs
  pinMode(DOUT_I2C_SCL,INPUT_PULLUP);

  Serial.print(F("Starting DCC Polarity A test\n"));

  do
  {
    digitalWrite(DOUT_DCC_POLARITY, false);
    delay(100);
    d2 = digitalRead(DIN_DCC_SIG_D2);
    d3 = digitalRead(DIN_DCC_SIG_D3);
    d7 = digitalRead(DIN_DCC_SIG_D7);
  
  
    pass = d2 && d3 && d7;
    sprintf(buffer, "DCC Signal Inputs HIGH %s  [D2=%s D3=%s D7=%s]\n", (pass)?"PASS":"!!FAIL!!", d2?"HIGH":"LOW", d3?"HIGH":"LOW", d7?"HIGH":"LOW");
    Serial.print(buffer);

    if (!pass)
      delay(500);
    
  } while (!pass);
  
  Serial.print("Check D1 (amber) is ON and press key to continue\n");
  waitForKey();

  do
  {  
    Serial.print("\nStarting DCC Polarity B test\n");
    digitalWrite(DOUT_DCC_POLARITY, true);
    delay(100);
    d2 = digitalRead(DIN_DCC_SIG_D2);
    d3 = digitalRead(DIN_DCC_SIG_D3);
    d7 = digitalRead(DIN_DCC_SIG_D7);
  
    pass = !d2 && !d3 && !d7;
  
    sprintf(buffer, "DCC Signal Inputs LOW %s  [D2=%s D3=%s D7=%s]\n", pass?"PASS":"!!FAIL!!", d2?"HIGH":"LOW", d3?"HIGH":"LOW", d7?"HIGH":"LOW");
    Serial.print(buffer);
    if (!pass)
      delay(500);
    
  } while (!pass);
  
  sprintf(buffer, "Switch off D7");
  Serial.print(buffer);

  do {
     sprintf(buffer, ".");
     Serial.print(buffer);
     delay(250);
    
  } while(!digitalRead(DIN_DCC_SIG_D7));
  sprintf(buffer, "PASS!\n");
  Serial.print(buffer);

  sprintf(buffer, "Switch off D3");
  Serial.print(buffer);

  do {
     sprintf(buffer, ".");
     Serial.print(buffer);
     delay(250);
    
  } while(!digitalRead(DIN_DCC_SIG_D3));
  sprintf(buffer, "PASS!\n");
  Serial.print(buffer);

  sprintf(buffer, "Testing that D2 is still switched on");
  Serial.print(buffer);

  do {
     sprintf(buffer, ".");
     Serial.print(buffer);
     delay(250);
    
  } while(digitalRead(DIN_DCC_SIG_D2));
  sprintf(buffer, "PASS!\n");
  Serial.print(buffer);

  {
    Wire.begin();
    INA219 v12mon;
    v12mon.begin();
    float busIstart = v12mon.shuntCurrent() * 1000.0;


    Serial.println(F("Testing DCC ACK generator"));

    digitalWrite(DOUT_DCC_SIG_ACK, true);
    delay(100);
    float busIend = v12mon.shuntCurrent() * 1000.0;
    digitalWrite(DOUT_DCC_SIG_ACK, false);

    Serial.print(F("ACK current   "));
    Serial.print(busIend - busIstart, 4);
    Serial.print(" mA");
    
    Wire.end();

    busIend -= busIstart;

    if (busIend > 60.0 && busIend < 72.0)
    {
      Serial.println(F(" PASS"));
    } else {
      Serial.println(F(" FAIL!"));
      Serial.print(F("\nLDCC ACK current check has failed\nPress any key to restart, will not continue\n"));
      waitForKey();
      return;    
    }
  }



  Serial.print("******************************************************\n");
  Serial.print("** TEST COMPLETE - BOARD PASSES                     **\n");
  Serial.print("******************************************************\n\n\n");
}
