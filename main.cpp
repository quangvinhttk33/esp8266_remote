#include <Arduino.h>
#include "OneButton.h"
#include <IRremote.hpp>
#include <arduino-timer.h>
#include <TM1637Display.h>
#include "EEPROM.h"
// Khai báo các chân kết nối
#define receiverPin 16           // d0(esp) - dùng để đọc tín hiệu mắt hồng ngoại
const uint8_t buttonEncoder = 0; // define SW of the encoder
#define ENC_A_PIN 13             // d6(esp) - clk of the encoder
#define ENC_B_PIN 12             // d7(esp) - data define DT of the encoder
#define CLK 5                    // d1(esp)
#define DIO 4                    // d2(esp)
TM1637Display display(CLK, DIO);
// nút nhấn
OneButton button(buttonEncoder, true); // tạo đối tượng Button mới
auto timer = timer_create_default();   // tạo đối tượng Timer mới
auto timer1 = timer_create_default();  // tạo đối tượng Timer mới
int trangthai_led = HIGH;
// Nhấn đơn, nhấn đôi, nhấn giữ
uint8_t dem2 = 0;
uint8_t speed = 0; // số lần phim tốc độ nhấn
// IR remote
IRrecv irrecv(receiverPin); // tạo đối tượng IRrecv mới
decode_results results;     // lưu giữ kết quả giải mã tín hiệu
int poutput;
// các định nghĩa ban đầu chương trình
const uint8_t led = 2;
const uint8_t pwm = 14;
boolean statusProgram = false;     // kiểm tra trang thái thiết bị đã bật hay chưa? off, on
boolean statusProgramTime = false; // kiểm tra trang thái thiết bị có vào chế độ hẹn giờ không không? off, on
unsigned long coutTimer = 0;
int feq = 1000;       // tần số ban đầu
volatile int feqDuty; // duty ban đầu
const uint8_t SEG_OFF[] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_A | SEG_F | SEG_G | SEG_E,                 // F
    SEG_A | SEG_F | SEG_G | SEG_E                  // F
};
void saveTocDoDongCo(int number, int sp)
{
    String str = String(number);
    String esspSpeed = String(sp);
    EEPROM.begin(512);
    for (unsigned int i = 0; i < 5; ++i)
    {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    for (unsigned int i = 0; i < str.length(); ++i)
    {
        EEPROM.write(i, str[i]);
    }
    for (unsigned int i = 0; i < esspSpeed.length(); ++i)
    {
        EEPROM.write(3, esspSpeed[i]);
    }
    EEPROM.commit();
    EEPROM.end();
}
void getTocDoDongCo()
{
    EEPROM.begin(512);
    String eepFeqDuty = "";
    String eepSpeed = "";
    for (int i = 0; i < 3; ++i)
    {
        eepFeqDuty += char(EEPROM.read(i));
    }
    for (int i = 3; i < 5; ++i)
    {
        eepSpeed += char(EEPROM.read(i));
    }
    if (eepFeqDuty.toInt() > 0)
    {
        feqDuty = eepFeqDuty.toInt();
        speed = eepSpeed.toInt();
    }
    else
    {
        feqDuty = 10;
        speed = 0;
    }
    Serial.println("feqDuty \n ");
    Serial.println(feqDuty);
    Serial.println("speed \n ");
    Serial.println(speed);
    EEPROM.end();
}
void hienthiLedNumber()
{
    if (statusProgram)
    {
        display.showNumberDec(feqDuty, false, 3, 1);
        saveTocDoDongCo(feqDuty, speed);
    }
    else
    {
        display.setSegments(SEG_OFF, 3, 1);
    }
}
void resetEEPROM()
{
    EEPROM.begin(512);
    Serial.println("clearing eeprom");
    for (unsigned int i = 0; i < 130; ++i)
    {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    delay(100);
    Serial.println("da xoa bo nho");
    delay(500);
}
void IRAM_ATTR isr_encoder()
{
    if (digitalRead(ENC_A_PIN) != poutput)
    {
        if (digitalRead(ENC_B_PIN) != poutput)
        {
            // cùng chiều kim đồng hồ
            if (statusProgram)
            {
                if (feqDuty < 100)
                {
                    feqDuty += 1;
                }
                else
                {
                    feqDuty = 100;
                }
            }
            hienthiLedNumber();
        }
        else
        {
            // ngược chiều kim đồng hồ
            if (statusProgram)
            {
                if (feqDuty > 0)
                {
                    feqDuty -= 1;
                }
                else
                {
                    feqDuty = 0;
                }
            }

            hienthiLedNumber();
        }
        Serial.println("Toc do dang la feqDuty");
        Serial.println(feqDuty);
    }
    poutput = digitalRead(ENC_A_PIN);
}
void nhan_don()
{
    if (statusProgram == true)
    {
        speed += 1;
        switch (speed)
        {
        case 0:
            feqDuty = 10;
            break;
        case 1:
            feqDuty = 20;
            break;
        case 2:
            feqDuty = 30;
            break;
        case 3:
            feqDuty = 40;
            break;
        case 4:
            feqDuty = 50;
            break;
        case 5:
            feqDuty = 60;
            break;
        case 6:
            feqDuty = 70;
            break;
        case 7:
            feqDuty = 80;
            break;
        case 8:
            feqDuty = 90;
            break;
        case 9:
            feqDuty = 100;
            speed = 0;
            break;
        case 10:
            feqDuty = 10;
            speed = 0;
            break;
        }
    }
    else
    {
        statusProgram = true;
    }
    hienthiLedNumber();
}
void nhan_double()
{
    dem2 += 1;
    Serial.print("Nhan 2 lan: tat mo tup nang");
    Serial.println(dem2);
}

bool turnOff(void *)
{
    statusProgram = false;
    display.setSegments(SEG_OFF, 3, 1);
    digitalWrite(led, HIGH);
    digitalWrite(pwm, LOW);
    statusProgramTime = false;
    coutTimer = 0;
    timer.cancel();
    return false; // repeat? true
}
void nhan_giu()
{
    statusProgram = false;
    display.setSegments(SEG_OFF, 3, 1);
    digitalWrite(led, HIGH);
    digitalWrite(pwm, LOW);
    statusProgramTime = false;
    coutTimer = 0;
    timer.cancel();
}
// đều chế xung
void gialapPWM()
{
    int dutyCycle = map(feqDuty, 0, 100, 0, 255); // Chuyển đổi giá trị feqDuty từ 0-100% thành 0-255
    analogWrite(pwm, dutyCycle);
}
String translateIR(uint32_t code)
{
    String keyBuff = "";
    switch (code)
    {
    case 0x40D:
        keyBuff = "off";
        break;
    case 0x4D:
        keyBuff = "speed";
        break;
    case 0xFD027F80:
    case 0x8D:
        keyBuff = "timer";
        break;
    case 0x10D:
        keyBuff = "mode";
        break;
    case 0x20D:
        keyBuff = "swing";
        break;
    default:
        return keyBuff;
    }
    Serial.println(keyBuff);
    return keyBuff;
}
void setup()
{
    Serial.begin(115200); // serial baudrate 9600
    delay(1000);
    getTocDoDongCo();
    display.setBrightness(4);
    display.clear();
    IrReceiver.begin(receiverPin); // Start the receiver
    pinMode(led, OUTPUT);
    pinMode(pwm, OUTPUT);
    pinMode(ENC_A_PIN, INPUT_PULLUP);                                       // enable pull-up resistor for pin A
    pinMode(ENC_B_PIN, INPUT_PULLUP);                                       // enable pull-up resistor for pin B
    attachInterrupt(digitalPinToInterrupt(ENC_B_PIN), isr_encoder, CHANGE); // assign interrupt to pin B
    poutput = digitalRead(ENC_A_PIN);                                       // Read the initial state of the clock
    pinMode(buttonEncoder, INPUT);                                          // Cài đặt button encoder là INPUT
    button.attachDoubleClick(nhan_double);                                  // Kích hoạt lệnh khi nhấn liên tục 2 lần
    button.attachClick(nhan_don);                                           // Kích hoạt lệnh khi nhấn 1 lần rồi nhả
    button.attachLongPressStart(nhan_giu);                                  // Kích hoạt lệnh khi nhấn giữ 1s
    display.setSegments(SEG_OFF, 3, 1);
}
void loop()
{
    if (IrReceiver.decode())
    {
        uint32_t IRcode = IrReceiver.decodedIRData.decodedRawData;
        String keyBuf = translateIR(IRcode);
        if (keyBuf == "off")
        {
            nhan_giu();
        }
        else if (keyBuf == "speed")
        {
            nhan_don();
        }
        else if (keyBuf == "time")
        {
        }
        else if (keyBuf == "mode")
        {
        }
        else if (keyBuf == "swing")
        {
        }
        delay(100);
        irrecv.resume(); // nhận giá trị tiếp theo
        hienthiLedNumber();
    }
    button.tick(); // Kiểm tra trạng thái nút nhấn
    if (statusProgram)
    {
        gialapPWM();
        if (statusProgramTime == true)
        {
            timer.tick();
        }
        else
        {
            timer.cancel();
        }
    }
}