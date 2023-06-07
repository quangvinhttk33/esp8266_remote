#include <Arduino.h>
#include "OneButton.h"
#include <IRremote.hpp>
#include <TM1637Display.h>
#include <arduino-timer.h>
// Khai báo các chân kết nối
#define receiverPin 16       // dùng để đọc tín hiệu mắt hồng ngoại
const int buttonEncoder = 0; // define SW of the encoder
#define ENC_A_PIN 13         // define clk of the encoder
#define ENC_B_PIN 12         // define DT of the encoder
#define CLK 5                // d1
#define DIO 4                // d2
// khai báo led báo
TM1637Display display(CLK, DIO);
// nút nhấn
OneButton button(buttonEncoder, true); // tạo đối tượng Button mới
auto timer = timer_create_default();   // tạo đối tượng Timer mới
// auto timer1 = timer_create_default();// tạo đối tượng Timer mới
int trangthai_led = LOW;
// Nhấn đơn, nhấn đôi, nhấn giữ
int dem1 = 0;
int dem2 = 0;
int dem3 = 0;
int speed = 0;          // số lần phim tốc độ nhấn
int countButtonOff = 0; // số lần phim tốc độ nhấn

// IR remote
IRrecv irrecv(receiverPin); // tạo đối tượng IRrecv mới
decode_results results;     // lưu giữ kết quả giải mã tín hiệu
int poutput;
volatile int enc_value = 0; // vòng xoay encoder
// các định nghĩa ban đầu chương trình
const int led = 2;
unsigned long previousMillis = 0;   // thời điểm của lần cập nhật trước đó
unsigned long interval = 500;       // khoảng thời gian (ms) của mỗi chu kỳ nhấp nháy
boolean statusProgram = false;      // kiểm tra trang thái thiết bị đã bật hay chưa? off, on
boolean statusProgramSetup = false; // kiểm tra trang thái thiết bị có vào chế độ cấu hình không? off, on
boolean statusProgramTime = false;  // kiểm tra trang thái thiết bị có vào chế độ hẹn giờ không không? off, on
unsigned long coutTimer = 0;
int feq = 1000;             // tần số ban đầu
volatile int feqDuty = 10; // duty ban đầu
void IRAM_ATTR isr_encoder()
{
    if (digitalRead(ENC_A_PIN) != poutput)
    {
        if (digitalRead(ENC_B_PIN) != poutput)
        {
            enc_value++;
            if (feqDuty < 100)
            {
                feqDuty += 1;
            }
            else
            {
                feqDuty = 100;
            }
        }
        else
        {
            enc_value--;
            if (feqDuty > 0)
            {
                feqDuty -= 1;
            }
            else
            {
                feqDuty = 0;
            }
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
        if (speed == 1)
        {
            feqDuty = 10;
        }
        if (speed == 2)
        {
            feqDuty = 20;
        }
        if (speed == 3)
        {
            feqDuty = 30;
        }
        if (speed == 4)
        {
            feqDuty = 40;
        }
        if (speed == 5)
        {
            feqDuty = 50;
        }
        if (speed == 6)
        {
            feqDuty = 60;
        }
        if (speed == 7)
        {
            feqDuty = 70;
        }
        if (speed == 8)
        {
            feqDuty = 80;
        }
        if (speed == 8)
        {
            feqDuty = 90;
        }
        if (speed == 10)
        {
            feqDuty = 100;
            speed = 0;
        }
    }
    else
    {
        statusProgram = true;
    }
    Serial.println("Toc do dang la feqDuty");
    Serial.println(feqDuty);
}

void nhan_double()
{
    dem2 += 1;
    Serial.print("Nhan 2 lan: tat mo tup nang");
    Serial.println(dem2);
}

void nhan_giu()
{
    dem3 += 1;
    Serial.print("Nhan va giu tat chuong trinh ");
    Serial.println(dem3);
    statusProgram = false;
}
bool turnOff(void *)
{
    statusProgram = false;
    return true; // repeat? true
}
// hienr thị led
const uint8_t SEG_OFF[] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_A | SEG_F | SEG_G | SEG_E,                 // F
    SEG_A | SEG_F | SEG_G | SEG_E                  // F
};
void setup()
{
    Serial.begin(115200);          // serial baudrate 9600
    IrReceiver.begin(receiverPin); // Start the receiver
    pinMode(led, OUTPUT);
    pinMode(ENC_A_PIN, INPUT_PULLUP);                                       // enable pull-up resistor for pin A
    pinMode(ENC_B_PIN, INPUT_PULLUP);                                       // enable pull-up resistor for pin B
    attachInterrupt(digitalPinToInterrupt(ENC_B_PIN), isr_encoder, CHANGE); // assign interrupt to pin B
    poutput = digitalRead(ENC_A_PIN);                                       // Read the initial state of the clock
    pinMode(buttonEncoder, INPUT);                                          // Cài đặt button encoder là INPUT
    button.attachDoubleClick(nhan_double);                                  // Kích hoạt lệnh khi nhấn liên tục 2 lần
    button.attachClick(nhan_don);                                           // Kích hoạt lệnh khi nhấn 1 lần rồi nhả
    button.attachLongPressStart(nhan_giu);                                  // Kích hoạt lệnh khi nhấn giữ 1s
    digitalWrite(led, trangthai_led);
    display.setBrightness(4);
    display.clear();
}
// đều chế xung
void gialapPWM()
{
    // unsigned long currentTime = micros();
    // unsigned long chuky = 1000000 / feq;
    // unsigned long tHight = feqDuty * chuky / 100;
    // unsigned long tLow = chuky - tHight;
    // while (micros() - currentTime < (unsigned long)tHight)
    // {
    //     digitalWrite(2, HIGH);
    // }
    // digitalWrite(2, LOW);
    // currentTime = micros();
    // while ((micros() - currentTime) < tLow) // 1000UL bằng 1000Hz
    // {
    //     digitalWrite(2, LOW);
    // }
    int dutyCycle = map(feqDuty, 0, 100, 0, 255); // Chuyển đổi giá trị feqDuty từ 0-100% thành 0-255
    analogWrite(pwm, dutyCycle);
}
void loop()
{
    if (IrReceiver.decode())
    {
        switch (IrReceiver.decodedIRData.decodedRawData)
        {
        case 0xFE017F80:
        case 0x40D:
        case 0xB946FF00: // phim auto
            statusProgram = !statusProgram;
            Serial.println("on/off chuong trinh");
            break;
        case 0xE51A7F80:
        case 0x4D:
        case 0xBB44FF00: // phim 3h
            if (statusProgram == true)
            {
                speed += 1;
                if (speed == 1)
                {
                    feqDuty = 10;
                }
                if (speed == 2)
                {
                    feqDuty = 20;
                }
                if (speed == 3)
                {
                    feqDuty = 30;
                }
                if (speed == 4)
                {
                    feqDuty = 40;
                }
                if (speed == 5)
                {
                    feqDuty = 50;
                }
                if (speed == 6)
                {
                    feqDuty = 60;
                }
                if (speed == 7)
                {
                    feqDuty = 70;
                }
                if (speed == 8)
                {
                    feqDuty = 80;
                }
                if (speed == 8)
                {
                    feqDuty = 90;
                }
                if (speed == 10)
                {
                    feqDuty = 100;
                    speed = 0;
                }
            }
            else
            {
                statusProgram = true;
            }
            Serial.println("Toc do dang la feqDuty");
            Serial.println(feqDuty);
            break;
        case 0xFD027F80:
        case 0xEA15FF00: // phim 5h
        case 0x8D:
            if (statusProgram == true)
            {
                timer.cancel();
                statusProgramTime = true;
                coutTimer += 1;
                if (coutTimer > 48)
                {
                    statusProgramTime = false;
                    coutTimer = 0;
                }
                else
                {
                    timer.every(coutTimer*1800000, turnOff);
                }
            }
            Serial.println("Nhan phim timer : ");
            Serial.println(coutTimer);
            break;
        case 0xFC037F80:
        case 0x10D:
        case 0xE916FF00: // phim 8h
            Serial.println("Nhan phim mode");
            break;
        case 0xF9067F80:
        case 0x20D:
        case 0xF708FF00: // phim đèn 100%
            Serial.println("Nhan phim swing");
            break;
        case 0xED127F80:
        case 0xBC43FF00: // phim on đen
            Serial.println("Nhan phim Up");
            break;
        case 0xE11E7F80:
        case 0xF20DFF00: // phim on đen
            Serial.println("Nhan phim Down");
            break;
        case 0xFA057F80:
        case 0xA55AFF00: // phim on đèn 50%
            Serial.println("Nhan phim Mo Den");
            break;
        }
        Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX); // Print "old" raw data
        delay(200);
        IrReceiver.resume();
    }
    button.tick(); // Kiểm tra trạng thái nút nhấn

    if (statusProgram == false)
    {
        display.setSegments(SEG_OFF, 3, 1);
        statusProgramTime = false;
        coutTimer = 0;
        timer.cancel();
    }
    else
    {
        gialapPWM();
        display.showNumberDec(feqDuty, false, 3, 1);
        if (statusProgramTime == true)        {
            timer.tick();
            Serial.println("da hen gio tat");
        }
        else
        {
            timer.cancel();
        }
    }
}