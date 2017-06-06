#include <NmraDcc.h>
#include <Wire.h>
#include <Relay16.h>
// This Example shows how to use the library as a DCC Accessory Decoder to drive 8 Pulsed Turnouts


// You can print every DCC packet by un-commenting the line below
//#define NOTIFY_DCC_MSG

// You can print every notifyDccAccTurnoutOutput call-back by un-commenting the line below
#define NOTIFY_TURNOUT_MSG

// You can also print other Debug Messages uncommenting the line below
#define DEBUG_MSG

// Un-Comment the line below to force CVs to be written to the Factory Default values
// defined in the FactoryDefaultCVs below on Start-Up
#define FORCE_RESET_FACTORY_DEFAULT_CV

// Un-Comment the line below to Enable DCC ACK for Service Mode Programming Read CV Capablilty 
#define ENABLE_DCC_ACK  15  // This is A1 on the Iowa Scaled Engineering ARD-DCCSHIELD DCC Shield

#define NUM_TURNOUTS 16              // Set Number of Turnouts (Pairs of Pins)
#define ACTIVE_OUTPUT_STATE HIGH			// Set the ACTIVE State of the output to Drive the Turnout motor electronics HIGH or LOW 

#define DCC_DECODER_VERSION_NUM 11  // Set the Decoder Version - Used by JMRI to Identify the decoder

#define RELAY16_J5  LOW  // J5 address jumper on relay16
#define RELAY16_J6  LOW  // J6 address jumper on relay16
#define RELAY16_J7  LOW  // J7 address jumper on relay16


struct CVPair
{
  uint16_t  CV;
  uint8_t   Value;
};

#define CV_ACCESSORY_DECODER_OUTPUT_PULSE_TIME 2  // CV for the Output Pulse ON ms
#define CV_ACCESSORY_DECODER_CDU_RECHARGE_TIME 3  // CV for the delay in ms to allow a CDU to recharge
#define CV_ACCESSORY_DECODER_ACTIVE_STATE      4  // CV to define the ON Output State 

// To set the Turnout Addresses for this board you need to change the CV values for CV1 (CV_ACCESSORY_DECODER_ADDRESS_LSB) and 
// CV9 (CV_ACCESSORY_DECODER_ADDRESS_MSB) in the FactoryDefaultCVs structure below. The Turnout Addresses are defined as: 
// Base Turnout Address is: ((((CV9 * 64) + CV1) - 1) * 4) + 1 
// With NUM_TURNOUTS 8 (above) a CV1 = 1 and CV9 = 0, the Turnout Addresses will be 1..8, for CV1 = 2 the Turnout Address is 5..12

CVPair FactoryDefaultCVs [] =
{
  {CV_ACCESSORY_DECODER_ADDRESS_LSB,        1},		// CV 1 Board Address (lower 6 bits) 
  {CV_ACCESSORY_DECODER_ADDRESS_MSB,        0},		// CV 9 Board Address (Upper 3 bits)
  {CV_ACCESSORY_DECODER_OUTPUT_PULSE_TIME, 50},   // x 10mS for the output pulse duration
  {CV_ACCESSORY_DECODER_CDU_RECHARGE_TIME, 30},   // x 10mS for the CDU recharge delay time
  {CV_ACCESSORY_DECODER_ACTIVE_STATE,    ACTIVE_OUTPUT_STATE},
};

uint8_t FactoryDefaultCVIndex = 0;

NmraDcc  Dcc ;
DCC_MSG  Packet ;
Relay16 relayBoard;
uint16_t BaseTurnoutAddress;

// This function is called whenever a normal DCC Turnout Packet is received
void notifyDccAccTurnoutOutput( uint16_t Addr, uint8_t Direction, uint8_t OutputPower )
{
#ifdef  NOTIFY_TURNOUT_MSG
  Serial.print("notifyDccAccTurnoutOutput: Turnout: ") ;
  Serial.print(Addr,DEC) ;
  Serial.print(" Direction: ");
  Serial.print(Direction ? "Closed" : "Thrown") ;
  Serial.print(" Output: ");
  Serial.print(OutputPower ? "On" : "Off") ;
#endif
  if(( Addr >= BaseTurnoutAddress ) && ( Addr < (BaseTurnoutAddress + NUM_TURNOUTS )) && OutputPower )
  {
    uint16_t relayNumber = (Addr - BaseTurnoutAddress) + 1; // Relays are numbered 1-16 in the Relay16 library
    if (0 == Direction)
      relayBoard.relayOff(relayNumber);
    else
      relayBoard.relayOn(relayNumber);

#ifdef  NOTIFY_TURNOUT_MSG
    Serial.print(" Relay: ");
    Serial.print(relayNumber,DEC);
    Serial.print(" State: ");
    Serial.print((0==Direction)?"OFF":"ON");
#endif
  }
#ifdef  NOTIFY_TURNOUT_MSG
  Serial.println();
#endif
}

void initRelayboard()
{
  Wire.begin();
  //  If you're using a standard Iowa Scaled shield to connect the I2C lines to the Arduino,
  //  the /IORST line is likely tied to Digital IO line 4.  Use the initializer below.
  relayBoard.begin(RELAY16_J5, RELAY16_J6, RELAY16_J7, -1);
  relayBoard.allOff();

  BaseTurnoutAddress = (((Dcc.getCV(CV_ACCESSORY_DECODER_ADDRESS_MSB) * 64) + Dcc.getCV(CV_ACCESSORY_DECODER_ADDRESS_LSB) - 1) * 4) + 1  ;

}

void setup()
{
  Serial.begin(115200);

  // Setup which External Interrupt, the Pin it's associated with that we're using and enable the Pull-Up 
  Dcc.pin(0, 2, 1);
  
  // Call the main DCC Init function to enable the DCC Receiver
  Dcc.init( MAN_ID_DIY, DCC_DECODER_VERSION_NUM, CV29_ACCESSORY_DECODER, 0 );

#ifdef DEBUG_MSG
  Serial.print("\nNMRA DCC 16-Relay Accessory Decoder. Ver: "); Serial.println(DCC_DECODER_VERSION_NUM,DEC);
#endif  

#ifdef FORCE_RESET_FACTORY_DEFAULT_CV
  Serial.println("Resetting CVs to Factory Defaults");
  notifyCVResetFactoryDefault(); 
#endif

  initRelayboard();  

}

void loop()
{
  // You MUST call the NmraDcc.process() method frequently from the Arduino loop() function for correct library operation
  Dcc.process();

  if( FactoryDefaultCVIndex && Dcc.isSetCVReady())
  {
    FactoryDefaultCVIndex--; // Decrement first as initially it is the size of the array
    uint16_t cv = FactoryDefaultCVs[FactoryDefaultCVIndex].CV;
    uint8_t val = FactoryDefaultCVs[FactoryDefaultCVIndex].Value;
#ifdef DEBUG_MSG
    Serial.print("loop: Write Default CV: "); Serial.print(cv,DEC); Serial.print(" Value: "); Serial.println(val,DEC);
#endif     
    Dcc.setCV( cv, val );

    initRelayboard();
  }
}

void notifyCVChange(uint16_t CV, uint8_t Value)
{
#ifdef DEBUG_MSG
  Serial.print("notifyCVChange: CV: ") ;
  Serial.print(CV,DEC) ;
  Serial.print(" Value: ") ;
  Serial.println(Value, DEC) ;
#endif  

  Value = Value;  // Silence Compiler Warnings...
}

void notifyCVResetFactoryDefault()
{
  // Make FactoryDefaultCVIndex non-zero and equal to num CV's to be reset 
  // to flag to the loop() function that a reset to Factory Defaults needs to be done
  FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs)/sizeof(CVPair);
};

// This function is called by the NmraDcc library when a DCC ACK needs to be sent
// Calling this function should cause an increased 60ma current drain on the power supply for 6ms to ACK a CV Read 
#ifdef  ENABLE_DCC_ACK
void notifyCVAck(void)
{
#ifdef DEBUG_MSG
  Serial.println("notifyCVAck") ;
#endif
  // Configure the DCC CV Programing ACK pin for an output
  pinMode( ENABLE_DCC_ACK, OUTPUT );

  // Generate the DCC ACK 60mA pulse
  digitalWrite( ENABLE_DCC_ACK, HIGH );
  delay( 10 );  // The DCC Spec says 6ms but 10 makes sure... ;)
  digitalWrite( ENABLE_DCC_ACK, LOW );
}
#endif


#ifdef  NOTIFY_DCC_MSG
void notifyDccMsg( DCC_MSG * Msg)
{
  Serial.print("notifyDccMsg: ") ;
  
  for(uint8_t i = 0; i < Msg->Size; i++)
  {
    Serial.print(Msg->Data[i], HEX);
    Serial.write(' ');
  }
  Serial.println();
}
#endif
