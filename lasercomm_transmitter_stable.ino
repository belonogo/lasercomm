#define TX_DATA_PIN             44
#define TX_CLOCK_PIN            48

#define NUMBER_BYTES_PER_BLOCK  25
#define MY_MESSAGE              "I Love Instructables !!!"
//#define MY_MESSAGE              "k"


byte TX_Data[NUMBER_BYTES_PER_BLOCK];
char Input_Message[]                  = MY_MESSAGE;
size_t byteGroupId                    = 0;


// Converts the given message to binary representation
void ConvertDataToBits()
{
  size_t byte_id = 0;
  
  for( size_t i = 0; i < strlen(Input_Message); ++i )
  {
     char character = Input_Message[ i ];
 
     for( size_t k = 8; k != 0; --k )
     {
        byte bits = byte( character );

        TX_Data[byte_id] = bits;
     }
     
     ++byte_id;
   }
}


void setup() 
{
  Serial.begin( 500000 );

  //pinMode( TX_DATA_PIN  , OUTPUT);
  //pinMode( TX_CLOCK_PIN , OUTPUT);

  DDRL |=  (1 << PL5); 
  DDRL |=  (1 << PL1); 

  digitalWrite(TX_DATA_PIN, LOW);
  digitalWrite(TX_CLOCK_PIN, LOW);

  ConvertDataToBits();
  
  delay(10);
}

// Responsible for sending the bits through the channel by switching pin states between 
// HIGH and LOW at a observated bit.
// The Data Rates are related to the delayMicroseconds values, it is already set to the
// highest possible (hardware limitation).
void loop() 
{
  for( size_t bitId = 0; bitId < 8; ++bitId )
  {
    if( bitRead( TX_Data[byteGroupId], bitId ) == 1 )
    {
      digitalWrite( TX_DATA_PIN , HIGH );
      delayMicroseconds(4);
      digitalWrite( TX_CLOCK_PIN, HIGH );
      
      delayMicroseconds(4);
      
      digitalWrite( TX_CLOCK_PIN , LOW );
      digitalWrite( TX_DATA_PIN ,  LOW );
    }
    else
    {
      digitalWrite( TX_DATA_PIN , LOW );
      delayMicroseconds(4);
      digitalWrite( TX_CLOCK_PIN, HIGH );
      
      delayMicroseconds(4);

      digitalWrite( TX_CLOCK_PIN , LOW );
    }
    delayMicroseconds(8);
  }


  if( byteGroupId != (NUMBER_BYTES_PER_BLOCK - 2) )
  {
    ++byteGroupId;  
  }
  else
  {
    byteGroupId = 0;
  }
}
