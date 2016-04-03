#include "util/atomic.h"

// This sketch is designed for an Arduino Uno with the DCC signal attached to digital pin 2

// *****  START USER CONFIGURATION

// DCC_PIN is the Arduino digital input corresponding to where the DCC signal is coming in.
// It must be an external interrupt for whatever platform you're using.
// Pin 2 is recommended for the Uno

#define DCC_PIN  2


// OPTION_RAW_PACKET_DISPLAY set to non-zero pumps packets through as hex strings
// 0 = Human readable decode
// 1 = Hex String in format P:XX XX XX\n

#define OPTION_RAW_PACKET_DISPLAY 1


// OPTION_SHOW_PREAMBLE_BITS set to 1 displays the number of preamble bits sent before the 
// command came through.  It will be displayed as nnn/ (where n = preamble bits) on the beginning
// of each decode line
// 0 = No preamble bit count display
// 1 = Preamble bit count displayed

#define OPTION_SHOW_PREAMBLE_BITS 0


// OPTION_SUPPRESS_IDLE_PKTS set to 1 suppresses the display of IDLE packets on the rails
// These can be highly annoying to sort through, so most folks are going to want this on
// To turn it off and display each idle packet, set this to 0

#define OPTION_SUPPRESS_IDLE_PKTS 1


// *****  END USER CONFIGURATION
// Normally, you won't need to touch anything beyond here

// The Timer0 prescaler is hard-coded in wiring.c 
#define TIMER_PRESCALER 64

// We will use a time period of 80us and not 87us as this gives us a bit more time to do other stuff 
#define DCC_BIT_SAMPLE_PERIOD (F_CPU * 70L / TIMER_PRESCALER / 1000000L)

#if (DCC_BIT_SAMPLE_PERIOD > 254)
#error DCC_BIT_SAMPLE_PERIOD too big, use either larger prescaler or slower processor
#endif
#if (DCC_BIT_SAMPLE_PERIOD < 8)
#error DCC_BIT_SAMPLE_PERIOD too small, use either smaller prescaler or faster processor
#endif


typedef enum
{
  WAIT_PREAMBLE = 0,
  WAIT_START_BIT,
  WAIT_DATA,
  WAIT_END_BIT
} DccRxWaitState;

#define MAX_DCC_MESSAGE_LEN 6    // including XOR-Byte

typedef struct
{
  uint8_t Size;
  uint8_t PreambleBits;
  uint8_t Flags;
  uint8_t Data[MAX_DCC_MESSAGE_LEN];
} DCCPkt ;

#define DCC_PKT_QUEUE_DEPTH 8

typedef struct
{
   volatile uint8_t headIdx;
   volatile uint8_t tailIdx;
   volatile uint8_t full;
   DCCPkt pktBufferArray[DCC_PKT_QUEUE_DEPTH];
} DCCPktQueue;

DCCPktQueue dccq;

// Interrupt handlers and such shamelessly borrowed from Alex Shepherd's NmraDcc library
void ExternalInterruptHandler(void)
{
  OCR0B = TCNT0 + DCC_BIT_SAMPLE_PERIOD ;
  
  TIMSK0 |= (1<<OCIE0B);  // Enable Timer0 Compare Match B Interrupt
  TIFR0  |= (1<<OCF0B);   // Clear  Timer0 Compare Match B Flag 
}

ISR(TIMER0_COMPB_vect)
{
  uint8_t DccBitVal = !digitalRead(DCC_PIN);
  static DccRxWaitState rxState; 
  static uint8_t bitCount = 0;
  static uint8_t tempByte = 0;
  static DCCPkt PacketBuf;
  

  // Disable Timer0 Compare Match B Interrupt
  TIMSK0 &= ~(1<<OCIE0B);

  bitCount++;

  switch( rxState )
  {
    case WAIT_PREAMBLE:
      if( DccBitVal )
      {
        if( bitCount > 10 )
          rxState = WAIT_START_BIT ;
      }
      else
        bitCount = 0 ;
  
      break;
  
    case WAIT_START_BIT:
      if( !DccBitVal )
      {
        rxState = WAIT_DATA ;
        PacketBuf.Size = 0;
        PacketBuf.PreambleBits = 0;
        for(uint8_t i = 0; i< MAX_DCC_MESSAGE_LEN; i++ )
          PacketBuf.Data[i] = 0;
  
        // We now have 1 too many PreambleBits so decrement before copying
        PacketBuf.PreambleBits = bitCount - 1 ;
  
        bitCount = 0 ;
        tempByte = 0 ;
      }
      break;
  
    case WAIT_DATA:
      tempByte <<= 1;
      if( DccBitVal )
        tempByte |= 1 ;
  
      if( bitCount == 8 )
      {
        if( PacketBuf.Size == MAX_DCC_MESSAGE_LEN ) // Packet is too long - abort
        {
          rxState = WAIT_PREAMBLE ;
          bitCount = 0 ;
        }
        else
        {
          rxState = WAIT_END_BIT ;
          PacketBuf.Data[ PacketBuf.Size++ ] = tempByte ;
        }
      }
      break;
  
    case WAIT_END_BIT:
      if( DccBitVal ) // End of packet?
      {
        rxState = WAIT_PREAMBLE ;
        dccPktPush(&dccq, &PacketBuf);
      }
      else  // Get next Byte
        rxState = WAIT_DATA ;
  
      bitCount = 0 ;
      tempByte = 0 ;
  }
}

void dccQueueInitialize(DCCPktQueue* q)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
       q->headIdx = q->tailIdx = 0;
       q->full = 0;
       memset(q->pktBufferArray, 0, DCC_PKT_QUEUE_DEPTH * sizeof(DCCPkt));
   }
}

uint8_t dccQueueDepth(DCCPktQueue* q)
{
   uint8_t result = 0;
   if(q->full)
       return(DCC_PKT_QUEUE_DEPTH);

   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
       result = (uint8_t)(q->headIdx - q->tailIdx) % DCC_PKT_QUEUE_DEPTH;
   }
   return(result);
}

uint8_t dccPktPush(DCCPktQueue* q, DCCPkt* pkt)
{
   uint8_t* pktPtr;
   // If full, bail with a false
   if (q->full)
       return(0);

   memcpy((uint8_t*)&q->pktBufferArray[q->headIdx], (uint8_t*)pkt, sizeof(DCCPkt));

   if( ++q->headIdx >= DCC_PKT_QUEUE_DEPTH )
       q->headIdx = 0;
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
       if (q->headIdx == q->tailIdx)
           q->full = 1;
   }
   return(1);
}

uint8_t dccPktPop(DCCPktQueue* q, DCCPkt* pkt)
{
  if (0 == dccQueueDepth(q))
  {
    memset(pkt, 0, sizeof(DCCPkt));
    return(0);
  }
  
  memcpy(pkt, &q->pktBufferArray[q->tailIdx], sizeof(DCCPkt));
  
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    if( ++q->tailIdx >= DCC_PKT_QUEUE_DEPTH )
      q->tailIdx = 0;
    q->full = 0;
  }

  return(1);
}



void setup() 
{
  // put your setup code here, to run once:
  dccQueueInitialize(&dccq);
  TCCR0A &= ~((1<<WGM01)|(1<<WGM00));
  attachInterrupt( digitalPinToInterrupt(DCC_PIN), ExternalInterruptHandler, RISING);
  Serial.begin(115200);
  Serial.print(F("DCC Packet Analyzer v0.1\n"));
  Serial.print(F("----------------------------\n"));
  Serial.print(F("Iowa Scaled Engineering\n"));  
  Serial.print(F("https://github.com/IowaScaledEngineering/DCCPacketAnalyzer\n"));
}

void rawPacketDecode(DCCPkt* pkt)
{
  uint8_t i;
  Serial.print("P:");
  for(i=0; i<pkt->Size; i++)
  {
    char buffer[4];
    sprintf(buffer, "%02X ", pkt->Data[i]);
    Serial.print(buffer);
  }
  Serial.print("\n");
}

void decodeMultifunction(DCCPkt* pkt, uint8_t* data, uint8_t dataSz)
{
  uint8_t d1 = *data;
  uint8_t d2 = (dataSz>1)?(*(data+1)):0;
  uint8_t c;
  uint8_t instructionType = (d1>>5) & 0x07;
  char buffer[32];

  switch (instructionType)
  {
    case 0: // Decoder and Consist Control Instructions
      
      if (0 == (d1 & 0x10))
      {
        // Decoder Control
        switch((d1>>1) & 0x07)
        {
          case 0: // Decoder reset
            if (d1 & 0x01)
              Serial.print(F("HARD RESET\n"));
            else
              Serial.print(F("RESET\n"));
            break;
          
          case 1: // Factory Test
            if (d1 & 0x01)
              Serial.print(F("Factory test ON\n"));
            else
              Serial.print(F("Factory test OFF\n"));
            break;

          case 3: // Set decoder flags
            if (dataSz >= 2)
            {
              switch((d2>>4) & 0x0F)
              {
                case 0:
                  Serial.print(F("CV Access DISABLE\n"));
                  break;
                case 4:
                  Serial.print(F("Decoder Ack DISABLE\n"));
                  break;
                case 5:
                  if (d1 & 0x01)
                    Serial.print(F("& CONSIST "));
                  Serial.print(F("BiDi ENABLE\n"));
                  break;
                case 8:
                  Serial.print(F("BiDi Set Sub="));
                  sprintf(buffer, "%d\n", d2 & 0x07);
                  Serial.print(buffer);
                  break;
                case 9:
                  Serial.print(F("CV Access ENABLE Sub=\n"));
                  sprintf(buffer, "%d\n", d2 & 0x07);
                  Serial.print(buffer);
                  break;
                case 15:
                  Serial.print(F("CV Access ENABLE\n"));
                  break;
                default:
                  rawPacketDecode(pkt);             
                  break;              
              }
            } else {
              Serial.print(F("Malformed Decoder Flag Packet\n"));
            }
            break;

          case 5: // Set advanced addressing
            if (d1 & 0x01)
              Serial.print(F("CV29 Addr EN\n"));
            else
              Serial.print(F("CV29 Addr DIS\n"));
            break;

          case 7: // Set Decoder Ack
            if (d1 & 0x01)
              Serial.print(F("Dcdr Ack EN\n"));
            else
              Serial.print(F("Dcdr Ack DIS\n"));
            break;
          default:
            rawPacketDecode(pkt);                    
            break;
        }
      } else {
        // Consist Control
        if (dataSz >= 2)
        {
          if (d2 !=0)
            Serial.print(F("ADD to "));
          else
            Serial.print(F("RMVD from "));
          sprintf(buffer, "cnst %03d in %s\n", d2 & 0x7F, (d1 & 0x01)?"REV":"FWD");
          Serial.print(buffer);
          
        } else {
          Serial.print(F("Malformed Consist Packet\n"));
        }
      }
      break;

    case 1:  //Advanced operations instructions
      switch(d1 & 0x1F)
      {
         case 31: // 128 step speed
           if (dataSz >=2)
           {
             if (0 == (d2 & 0x7f))
               sprintf(buffer, "%c-STP\n", (d2 & 0x80)?'F':'R');
             else if (1 == (d2 & 0x7F))
               sprintf(buffer, "%c-EST\n", (d2 & 0x80)?'F':'R');
             else
               sprintf(buffer, "%c-%03d\n", (d2 & 0x80)?'F':'R', d2 & 0x7F);
             Serial.print(buffer);
           } else {
            Serial.print(F("Malformed 128-step speed"));
           }
           break;

         case 30: // Restricted speed
           if (dataSz >= 2)
           {
             if (d2 & 80)
              sprintf(buffer, "EN restrd spd - %03d\n", d2 & 0x3F);
             else
              sprintf(buffer, "DIS restrd spd\n");
             Serial.print(buffer);

           } else {
             Serial.print(F("Malformed restricted speed\n"));            
           }

         case 29: // Analog Function Group
           if (dataSz >= 3)
           {
             sprintf(buffer, "AnalogF %03d v=%03d\n", d2, *(data+2));
             Serial.print(buffer);
           }
           else
             Serial.print(F("Malformed analog function\n"));
           break;

         default:
          Serial.print(F("Unknown Advanced Op Pkt\n"));
          break;
      }
      break;
    
    case 2: // Reverse
    case 3: // Forward
      sprintf(buffer, "%c-", (d1 & 0x20)?'F':'R');
      Serial.print(buffer);
  
      if (0 == (d1 & 0x0F))
      {
        sprintf(buffer, "STP FL=%d\n", (d1 & 0x10)?1:0);
      } 
      else if (1 == (d1 & 0x0F))
      {
        sprintf(buffer, "ESP FL=%d\n", (d1 & 0x10)?1:0);
      }
      else
      {
        sprintf(buffer, "%02d/28 %02d/14 FL=%d\n", ((d1&0x0F)<<1) + ((d1&0x10)?1:0), (d1&0x0F), ((d1&0x10)?1:0));
      }
      Serial.print(buffer);
      break;
  
    case 4: // Function Group 1
      Serial.print("FL:");
      Serial.print((d1 & 0x10)?"1":"0");
  
      Serial.print(" F1-F4:");
      for(c=0x01; c<0x10; c<<=1)
      {
        Serial.print((c & d1)?"1":"0");
      }
      Serial.print("\n");
      break;
  
    case 5: // Function Group 2 / 3
      if (d1 & 0x10)
      {
        Serial.print("F5-F8:");
      } else {
        Serial.print("F9-F12:");
      }
      for(c=0x01; c<0x10; c=c<<1)
      {
        Serial.print((c & d1)?"1":"0");
      }
      Serial.print("\n");
      break;
  
    default:
      rawPacketDecode(pkt);
      break;
  }
}


void loop() 
{
  // put your main code here, to run repeatedly:
  if (dccQueueDepth(&dccq))
  {
    DCCPkt pkt;
    char buffer[32];    
    uint8_t i;
    uint8_t crc;
    dccPktPop(&dccq, &pkt);

    // First, check the CRC
    crc = 0;
    for (i=0; i<pkt.Size-1; i++)
    {
        crc ^= pkt.Data[i];
    }

    if (crc != pkt.Data[pkt.Size-1])
    {
      if (OPTION_SHOW_PREAMBLE_BITS != 0)
      {
        sprintf(buffer, "%03d/", pkt.PreambleBits);
        Serial.print(buffer);
      }


      sprintf(buffer, "E%02d: CRC ", pkt.PreambleBits);
      Serial.print(buffer);
      for(i=0; i<pkt.Size; i++)
      {
        char buffer[4];
        sprintf(buffer, "%02X ", pkt.Data[i]);
        Serial.print(buffer);
      }
      Serial.print("\n");

    }
    else if (0xFF == pkt.Data[0] && 0 == pkt.Data[1])
    {
      // Do nothing
      if(! OPTION_SUPPRESS_IDLE_PKTS)
      {
        if (OPTION_SHOW_PREAMBLE_BITS != 0)
        {
          sprintf(buffer, "%03d/", pkt.PreambleBits);
          Serial.print(buffer);
        }

        switch(OPTION_RAW_PACKET_DISPLAY)
        {
          case 0:
            Serial.print(F("IDLE"));
            break;
          case 1:
            rawPacketDecode(&pkt);
            break;
          default:
            break;
        }
      }

      
    }
    else
    {
      switch(OPTION_RAW_PACKET_DISPLAY)
      {
        case 1:
          rawPacketDecode(&pkt);
          break;        
        default:

          if (OPTION_SHOW_PREAMBLE_BITS != 0)
          {
            sprintf(buffer, "%03d/", pkt.PreambleBits);
            Serial.print(buffer);
          }
    
          if (0 == pkt.Data[0])
          {
            // Multi-function broadcast address
            Serial.print(F("L:*ALL* "));
          }
          else if (0xFF == pkt.Data[0])
          {
              // Idles should have been handled above - this one's malformed
            Serial.print(F("E:Malformed IDLE"));
          }
          else if (1 <= pkt.Data[0] && pkt.Data[0] <= 127)
          {
             // Multi-function (locomotive) decoder, 7 bit address
            sprintf(buffer, "L:%03d ", pkt.Data[0]);
            Serial.print(buffer);          
            decodeMultifunction(&pkt, pkt.Data + 1, pkt.Size - 1);
          }
          else if (192 <= pkt.Data[0] && pkt.Data[0] <= 231)
          {
            uint16_t addr = (uint16_t)pkt.Data[0] & 0x003F;
            addr *= 256;
            addr += pkt.Data[1];
            
            sprintf(buffer, "L:%04d ", addr);
            Serial.print(buffer);
            decodeMultifunction(&pkt, pkt.Data + 2, pkt.Size - 2);
            
          }
          break;
      }
    }
  }
}
