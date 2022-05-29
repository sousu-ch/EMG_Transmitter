//#define M5_LCD
#define M5GFX_LCD

#if defined(M5GFX_LCD)
#include <M5GFX.h>
M5GFX display;
#endif

#define M5STICKCPLUSSETTING
#ifdef M5STICKCPLUSSETTING
#include <M5StickCPlus.h> // M5StickCPlusの読み込み
#else
//#include <M5StickC.h> // M5StickCの読み込み
#endif

#include "crc8.h"
#include <Wire.h>

#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

//グローバル
uint8_t uart_buf_for_send[64];
uint8_t switch_count = 0;
uint32_t count10ms = 0;

#if defined(M5GFX_LCD)
static int prev_x[4];
static uint16_t colors[4]{
    TFT_RED,
    TFT_GREEN,
    TFT_CYAN,
    TFT_ORANGE};
#define BAR_STAT 82
#endif

union sensor_data
{
  uint8_t u8_sdata[2];
  uint16_t u16_sdata;
};
union sensor_data sdata[4];

//メッセージID
#define EMG_DATA 0xA0
#define ERR_MSG 0

void xSetSleep(void) // version M5stick-C plus
{
  M5.Axp.Write1Byte(0x31, M5.Axp.Read8bit(0x31) | (1 << 3));
  M5.Axp.Write1Byte(0x90, M5.Axp.Read8bit(0x90) | 0x07);
  M5.Axp.Write1Byte(0x12, M5.Axp.Read8bit(0x12) & ~(1 << 1) & ~(1 << 2) & ~(1 << 3));
}

const int ADC_PIN = GPIO_NUM_26;

// UARTに送信するフレームを生成し、データを詰め込んでPCに送信する
void send_uart_message(uint8_t data[], uint8_t size)
{
  //    uint8_t uart_buf_for_send[256];
  uart_buf_for_send[0] = 0xAA;
  uart_buf_for_send[1] = 0xAA;
  uart_buf_for_send[2] = 0xAA;
  uart_buf_for_send[3] = size;
  for (uint8_t i = 0; i < size; i++)
  {
    uart_buf_for_send[i + 4] = data[i];
  }
  uart_buf_for_send[size + 4] = GetCRC8_TABLE(uart_buf_for_send, size + 4);
  Serial.write(uart_buf_for_send, size + 5);
  SerialBT.write(uart_buf_for_send, size + 5);
}

long start_t, last_t;

void setup()
{
#if defined(M5_LCD)
  M5.begin();
  delay(50);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setRotation(3);
  M5.Axp.ScreenBreath(10);
  setCpuFrequencyMhz(80);
#endif

#if defined(M5GFX_LCD)
  display.init();
  display.startWrite();
  display.fillScreen(TFT_BLACK);
  display.setRotation(3);
  display.setTextSize(display.width() / 80);

#endif

  // ADC端子初期化

  /*
   * G36/G25は同じポートを利用しています。
   * そのため、片方のピンを利用しているとき、
   * もう片方のピンはフローティング入力（プルアップもプルダウンもしない状態）にする必要があります。
   */
  pinMode(36, INPUT);
  gpio_pulldown_dis(GPIO_NUM_25); // Disable pull-down on GPIO.
  gpio_pullup_dis(GPIO_NUM_25);   // Disable pull-up on GPIO.

  // 1番センサー
  pinMode(26, INPUT);
  // 1番センサー
  pinMode(32, INPUT);
  // 1番センサー
  pinMode(33, INPUT);

  // BluetoothSerial
  SerialBT.begin("ESP32");

  start_t = last_t = millis();
}

void loop()
{
  uint8_t msg[16];

  //    Lcd.fillScreen(WHITE);
  last_t = millis();
  if ((last_t - start_t) > 10)
  {
    start_t = last_t;
    count10ms++;
  }

  //データ測定および送信は50ms周期とした。
  //もっと細かい周期でデータ取得して移動平均してから送信してもよかったかも。
  //あまり高頻度で送信すると、SPPが飽和してしまって動作しなくなるようなので注意
  if ((count10ms % 5) == 0)
  {
    sdata[0].u16_sdata = analogRead(GPIO_NUM_36); // read the input pin
    sdata[1].u16_sdata = analogRead(GPIO_NUM_26); // read the input pin
    sdata[2].u16_sdata = analogRead(GPIO_NUM_32); // read the input pin
    sdata[3].u16_sdata = analogRead(GPIO_NUM_33); // read the input pin
    msg[0] = EMG_DATA;                            //メッセージIDをつける

    //センサーのデータを送信用バッファに格納する
    for (int i = 0; i < 4; i++)
    {
      msg[i * 2 + 1] = sdata[i].u8_sdata[0];
      msg[i * 2 + 2] = sdata[i].u8_sdata[1];
    }
    send_uart_message(msg, 9);
  }

  //表示周期は50ms周期とした
  if ((count10ms % 5) == 0)
  {
#if defined(M5_LCD)
    M5.Lcd.setTextSize(3);
    // sprintf(s,"%4ld %4ld %4ld %4ld", sdata[0].u16_sdata,sdata[1].u16_sdata,sdata[2].u16_sdata,sdata[3].u16_sdata);
    // M5.Lcd.print(s);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.fillRect(82, 0, sdata[0].u16_sdata / 22, 33, RED); // 3500[EMG max]/160[pix]≒22
    // M5.Lcd.setTextColor(BLACK, WHITE);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.printf("%02.1f", (float)sdata[0].u16_sdata / 35);
    M5.Lcd.fillRect(82, 32, sdata[1].u16_sdata / 22, 33, GREEN);
    M5.Lcd.setCursor(10, 32 + 10);
    M5.Lcd.printf("%02.1f", (float)sdata[1].u16_sdata / 35);
    M5.Lcd.fillRect(82, 65, sdata[2].u16_sdata / 22, 33, CYAN);
    M5.Lcd.setCursor(10, 65 + 10);
    M5.Lcd.printf("%02.1f", (float)sdata[2].u16_sdata / 35);
    M5.Lcd.fillRect(82, 98, sdata[3].u16_sdata / 22, 35, ORANGE);
    M5.Lcd.setCursor(10, 98 + 10);
    M5.Lcd.printf("%02.1f", (float)sdata[3].u16_sdata / 35);

    //電池残量
    float _vbat = M5.Axp.GetBatVoltage();
    float percent = (_vbat - 3) / 1.03;
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(200, 10);
    M5.Lcd.printf("%2.3f", percent);
#endif
    // 135x 240
#if defined(M5GFX_LCD)
    display.setTextSize(2);
    // display.fillRect(10, 10, 82, 33 * 4 - 1, TFT_BLACK);
    for (int ch = 0; ch < 4; ch++)
    {
      uint16_t x = sdata[ch].u16_sdata / 22;
      if (prev_x[ch] > x)
      {
        display.fillRect(BAR_STAT + x, ch * 33 + 10, prev_x[ch] - x, 15, TFT_BLACK);
      }
      else
      {
        display.fillRect(BAR_STAT + prev_x[ch], ch * 33 + 10, x - prev_x[ch], 15, colors[ch]);
      }
      prev_x[ch] = x;

      display.setCursor(10, ch * 33 + 10);
      display.printf("%4.1f", (float)sdata[ch].u16_sdata / 35);
    }

    //電池残量
    float _vbat = M5.Axp.GetBatVoltage();
    float percent = (_vbat - 3) / 1.03 * 100; // 3[V]を0%、3+1.03==4.03[V]を100%としている。ここはデバイスごとに微調整がいるかも
    display.setTextSize(1);
    display.setCursor(200, 125);
    display.printf("%5.1f", percent);
    display.display();
#endif
  }

  //ボタンAの回数によってモードを変更します
  // 1回目：LCDを限界まで輝度を下げる
  // 2回目：Sleepに入れます。（電源OFFのイメージ）
  //もう一回押す：Sleepから復帰して、高輝度で動作します
  M5.update();
  if (M5.BtnA.wasReleasefor(100))
  {
    switch_count++;
  }

  //１回目はLCDの輝度を下げる
  if (switch_count == 1)
  {
    delay(1000);
    M5.Axp.ScreenBreath(7);
  }

  //２回目はSleepに入れる
  if (switch_count == 2)
  {
    delay(1000);
    //--- AXP192::DeepSleep(uint64)t time_in_us)　の必要部分を抜き出し
    xSetSleep(); // SetSleep
    pinMode(GPIO_NUM_37, INPUT_PULLUP);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, LOW);
    esp_sleep_enable_timer_wakeup(1000000 * 3600 * 24 * 30);
    esp_deep_sleep(1000000 * 3600 * 24 * 30);
  }
}
