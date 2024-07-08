#define RX_DATA_PIN             2
#define RX_CLOCK_PIN            3
#define NUMBER_BYTES_PER_BLOCK  24

volatile byte   RX_Data[NUMBER_BYTES_PER_BLOCK];
volatile byte   RX_Buffer;
volatile byte   RX_Bit            = 0b0;

volatile size_t   Bit_PosId       = 0;
volatile size_t   Byte_PosId      = 0;


// Method triggered by rising clock interruption, append the value of RX_Bit to 8-bit Buffer RX_Buffer and
// then copies to RX_Data, also controls the indexes.
void Append_Bit()
{
  //RX_Bit = !digitalRead( RX_DATA_PIN );

  if ( Byte_PosId == NUMBER_BYTES_PER_BLOCK )
  {
    Serial.println();
    Byte_PosId = 0;
  }

  if ( Bit_PosId == 8 )
  {
    RX_Data[Byte_PosId] = RX_Buffer;
    Serial.print( (char) RX_Data[Byte_PosId] );

    Bit_PosId = 0;
    ++Byte_PosId;
  }

  // Ternary Operator, Correctly append the bit to the its right position in the Buffer
  // bitWrite( RX_Buffer, Bit_Pos, RX_Bit );
  
  (RX_Bit == 0b0) ? RX_Buffer &= ~(1 << Bit_PosId) : RX_Buffer |= 1 << Bit_PosId;

  ++Bit_PosId;
}

void setup()
{
  Serial.begin( 500000 );

  pinMode( RX_DATA_PIN,  INPUT_PULLUP );
  //DDRE &= ~(0 << PE4);
  pinMode( RX_CLOCK_PIN, INPUT_PULLUP );
  //DDRE &= ~(0 << PE5);

  attachInterrupt( digitalPinToInterrupt( RX_CLOCK_PIN ), Append_Bit, RISING );
}

void loop() 
{
  // Constantly reads the RX_DATA_PIN, since the transmitter assigns the bit before making a clock (rising edge)
  // would not be a problem, but watch out for falling edge
  RX_Bit = !( PINE & ( 1 << PE4 ) );
}
