#define TX_DATA_PIN             44    // Пин для передачи данных
#define TX_CLOCK_PIN            48    // Пин для передачи сигнала часов
#define ACK_PIN                 52    // Пин для получения сигнала подтверждения (ACK)

#define NUMBER_BYTES_PER_BLOCK  25
#define MY_MESSAGE              "I Love Instructables !!!"

byte TX_Data[NUMBER_BYTES_PER_BLOCK];
char Input_Message[]                  = MY_MESSAGE;
size_t byteGroupId                    = 0;

// Функция для вычисления контрольного бита (четности) для байта данных
byte calculateParity(byte b) {
    byte parity = 0;
    for (int i = 0; i < 8; ++i) {
        parity ^= (b >> i) & 0x01;  // XOR всех битов байта
    }
    return parity;
}

// Функция для преобразования данных сообщения в массив байтов с контрольным битом
void ConvertDataToBits() {
    size_t byte_id = 0;
    for (size_t i = 0; i < strlen(Input_Message); ++i) {
        char character = Input_Message[i];
        byte bits = byte(character);
        bits = (bits << 1) | calculateParity(bits); // Добавляем контрольный бит к байту данных
        TX_Data[byte_id] = bits;
        ++byte_id;
    }
}

void setup() {
    Serial.begin(500000);
    pinMode(TX_DATA_PIN, OUTPUT);   // Устанавливаем пин для данных как выход
    pinMode(TX_CLOCK_PIN, OUTPUT);  // Устанавливаем пин для часов как выход
    pinMode(ACK_PIN, INPUT);        // Устанавливаем пин для ACK как вход

    digitalWrite(TX_DATA_PIN, LOW);
    digitalWrite(TX_CLOCK_PIN, LOW);

    ConvertDataToBits(); // Преобразуем данные сообщения в байты с контрольным битом
    delay(10);
}

void loop() {
    // Отправляем байты данных с контрольным битом по одному биту
    for (size_t bitId = 0; bitId < 9; ++bitId) { // 8 бит данных + 1 бит паритета
        if (bitRead(TX_Data[byteGroupId], bitId) == 1) {
            digitalWrite(TX_DATA_PIN, HIGH);
            delayMicroseconds(4);
            digitalWrite(TX_CLOCK_PIN, HIGH);
            delayMicroseconds(4);
            digitalWrite(TX_CLOCK_PIN, LOW);
            digitalWrite(TX_DATA_PIN, LOW);
        } else {
            digitalWrite(TX_DATA_PIN, LOW);
            delayMicroseconds(4);
            digitalWrite(TX_CLOCK_PIN, HIGH);
            delayMicroseconds(4);
            digitalWrite(TX_CLOCK_PIN, LOW);
        }
        delayMicroseconds(8);
    }

    // Ждем подтверждение от приемника
    while (digitalRead(ACK_PIN) == LOW) {
        delayMicroseconds(1);
    }

    if (byteGroupId != (NUMBER_BYTES_PER_BLOCK - 2)) {
        ++byteGroupId;  
    } else {
        byteGroupId = 0;
    }
}
