#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>
#include <TouchScreen.h>

#define RX_DATA_PIN             2     // Пин для получения данных
#define RX_CLOCK_PIN            3     // Пин для получения сигнала часов
#define ACK_PIN                 4     // Пин для отправки сигнала подтверждения (ACK)

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define NUMBER_BYTES_PER_BLOCK  24

volatile byte RX_Data[NUMBER_BYTES_PER_BLOCK];
volatile byte RX_Buffer;
volatile byte RX_Bit = 0b0;

volatile size_t Bit_PosId = 0;
volatile size_t Byte_PosId = 0;

byte calculateParity(byte b) {
    byte parity = 0;
    for (int i = 0; i < 8; ++i) {
        parity ^= (b >> i) & 0x01;
    }
    return parity;
}

void Append_Bit() {
    if (Byte_PosId == NUMBER_BYTES_PER_BLOCK) {
        Byte_PosId = 0;
    }
    if (Bit_PosId == 9) { // 8 бит данных + 1 бит паритета
        byte receivedData = RX_Buffer >> 1; // Получаем байт данных (8 бит)
        byte receivedParity = RX_Buffer & 0x01; // Получаем контрольный бит (1 бит)
        byte calculatedParity = calculateParity(receivedData);
        if (receivedParity == calculatedParity) {
            RX_Data[Byte_PosId] = receivedData;

            // Отправляем сигнал подтверждения (ACK)
            digitalWrite(ACK_PIN, HIGH);
            delayMicroseconds(10);
            digitalWrite(ACK_PIN, LOW);

            tft.print((char) RX_Data[Byte_PosId]); // Отображаем полученный символ на TFT дисплее

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
    pinMode(RX_DATA_PIN, INPUT_PULLUP);
    pinMode(RX_CLOCK_PIN, INPUT_PULLUP);
    pinMode(ACK_PIN, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(RX_CLOCK_PIN), Append_Bit, RISING);

    tft.begin(0x9341); // Инициализация TFT дисплея (замените 0x9341 на ваш ID дисплея)
    tft.setRotation(1); // Устанавливаем ориентацию дисплея
    tft.fillScreen(BLACK); // Очищаем экран
    tft.setTextColor(WHITE); // Устанавливаем цвет текста
    tft.setTextSize(2); // Устанавливаем размер текста
    tft.setCursor(0, 0); // Устанавливаем курсор в начало экрана
}

void loop() {
    RX_Bit = !(PINE & (1 << PE4)); // Читаем бит данных
}
