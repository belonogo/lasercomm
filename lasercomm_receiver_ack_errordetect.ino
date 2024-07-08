#define RX_DATA_PIN             2     // Пин для получения данных
#define RX_CLOCK_PIN            3     // Пин для получения сигнала часов
#define ACK_PIN                 4     // Пин для отправки сигнала подтверждения (ACK)
#define NUMBER_BYTES_PER_BLOCK  24

volatile byte RX_Data[NUMBER_BYTES_PER_BLOCK];
volatile byte RX_Buffer;
volatile byte RX_Bit = 0b0;

volatile size_t Bit_PosId = 0;
volatile size_t Byte_PosId = 0;

// Функция для вычисления контрольного бита (четности) для байта данных
byte calculateParity(byte b) {
    byte parity = 0;
    for (int i = 0; i < 8; ++i) {
        parity ^= (b >> i) & 0x01;  // XOR всех битов байта
    }
    return parity;
}

// Функция, которая вызывается при каждом прерывании по фронту сигнала часов
void Append_Bit() {
    if (Byte_PosId == NUMBER_BYTES_PER_BLOCK) {
        Serial.println();
        Byte_PosId = 0;
    }
    if (Bit_PosId == 9) { // 8 бит данных + 1 бит паритета
        byte receivedData = RX_Buffer >> 1; // Получаем байт данных (8 бит)
        byte receivedParity = RX_Buffer & 0x01; // Получаем контрольный бит (1 бит)
        byte calculatedParity = calculateParity(receivedData);
        if (receivedParity == calculatedParity) {
            RX_Data[Byte_PosId] = receivedData;
            Serial.print((char) RX_Data[Byte_PosId]);

            // Отправляем сигнал подтверждения (ACK)
            digitalWrite(ACK_PIN, HIGH);
            delayMicroseconds(10);
            digitalWrite(ACK_PIN, LOW);

            Bit_PosId = 0;
            ++Byte_PosId;
        } else {
            Serial.println("Ошибка контрольного бита");
            Bit_PosId = 0;
        }
    } else {
        (RX_Bit == 0b0) ? RX_Buffer &= ~(1 << Bit_PosId) : RX_Buffer |= 1 << Bit_PosId;
        ++Bit_PosId;
    }
}

void setup() {
    Serial.begin(500000);
    pinMode(RX_DATA_PIN, INPUT_PULLUP);   // Устанавливаем пин для данных как вход с подтяжкой
    pinMode(RX_CLOCK_PIN, INPUT_PULLUP);  // Устанавливаем пин для часов как вход с подтяжкой
    pinMode(ACK_PIN, OUTPUT);             // Устанавливаем пин для ACK как выход

    attachInterrupt(digitalPinToInterrupt(RX_CLOCK_PIN), Append_Bit, RISING); // Настраиваем прерывание на фронт сигнала часов
}

void loop() {
    RX_Bit = !(PINE & (1 << PE4)); // Читаем бит данных
}
