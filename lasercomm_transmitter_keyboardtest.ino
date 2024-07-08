#include <PS2Keyboard.h>

#define TX_DATA_PIN             44    // Пин для передачи данных
#define TX_CLOCK_PIN            48    // Пин для передачи сигнала часов
#define ACK_PIN                 52    // Пин для получения сигнала подтверждения (ACK)
#define KEYBOARD_DATA_PIN       2     // Пин для данных клавиатуры
#define KEYBOARD_CLOCK_PIN      3     // Пин для часов клавиатуры

PS2Keyboard keyboard;

#define NUMBER_BYTES_PER_BLOCK  25
char Input_Message[NUMBER_BYTES_PER_BLOCK];
size_t byteGroupId = 0;
size_t messageLength = 0;

byte calculateParity(byte b) {
    byte parity = 0;
    for (int i = 0; i < 8; ++i) {
        parity ^= (b >> i) & 0x01;
    }
    return parity;
}

void ConvertDataToBits() {
    for (size_t i = 0; i < messageLength; ++i) {
        char character = Input_Message[i];
        byte bits = byte(character);
        bits = (bits << 1) | calculateParity(bits); // Добавляем контрольный бит к байту данных
        Input_Message[i] = bits;
    }
}

void setup() {
    Serial.begin(500000);
    pinMode(TX_DATA_PIN, OUTPUT);
    pinMode(TX_CLOCK_PIN, OUTPUT);
    pinMode(ACK_PIN, INPUT);
    
    keyboard.begin(KEYBOARD_DATA_PIN, KEYBOARD_CLOCK_PIN); // Инициализация клавиатуры
    
    digitalWrite(TX_DATA_PIN, LOW);
    digitalWrite(TX_CLOCK_PIN, LOW);
}

void loop() {
    // Чтение ввода с клавиатуры
    if (keyboard.available()) {
        char c = keyboard.read();
        if (c == PS2_ENTER) {  // Когда нажата клавиша Enter, начинаем передачу
            messageLength = strlen(Input_Message);
            ConvertDataToBits();
            transmitMessage();
            memset(Input_Message, 0, sizeof(Input_Message)); // Очищаем буфер после передачи
            byteGroupId = 0;
        } else if (messageLength < NUMBER_BYTES_PER_BLOCK - 1) {
            Input_Message[messageLength++] = c;
        }
    }
}

void transmitMessage() {
    for (size_t i = 0; i < messageLength; ++i) {
        for (size_t bitId = 0; bitId < 9; ++bitId) { // 8 бит данных + 1 бит паритета
            if (bitRead(Input_Message[i], bitId) == 1) {
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
    }
}
