/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */
#include <Button2.h>
#include<stdlib.h>
#include <TFT_eSPI.h>
#include <math.h>
#include "bufferTFT.h"
typedef struct{
  float x;
  u_char height;

} Pillar;
typedef struct{
  float y;
  float speed;

} Bird;

unsigned short buffer[32400]={0};
TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
Button2 btn1(35);
#define ScreenH (240)
#define ScreenW (135)
#define KongxiHeight (45)
#define PillarW (20)
#define MinSpeed (-10)
#define birdW (5)
#define birdX (ScreenW/2)

int PillarSpace=60;
Pillar pillars[3]={
  {0,50},
  {-50-PillarW,60},
  {-100-PillarW*2,70}
};
Bird bird ={ScreenH/2,0};
bool dead=false;
int score=0;

void initTFT(){
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);
  

  if (TFT_BL > 0)
  {                          // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
    pinMode(TFT_BL, OUTPUT); // Set backlight pin to output mode
    digitalWrite(TFT_BL, 1);
                             // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
  }
}


void setup()
{
    initTFT();
    btn1.setPressedHandler([](Button2 & b) {
      bird.speed-=5;
      if(bird.speed<MinSpeed)bird.speed=MinSpeed;
    });

    Serial.begin(115200);
    delay(10);
    
    // We start by connecting to a WiFi network

    Serial.println();

    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Starting", tft.width() / 2, tft.height() / 2);

 
}

void drawRectToBuffer(int x,int y,int w,int h,u_short color){
  int i,j;
  if(x<0){
    w=w+x;
    x=0;
  }
  if(y<0){
    h=h+y;
    y=0;
  }
  if(x+w>ScreenW){//修正超出范围
    w=ScreenW-x;
  }
  if(y+h>ScreenH){//修正超出范围
    h=ScreenH-y;
  }
  for(i=0;i<h;i++){
    for(j=0;j<w;j++){
      buffer[(j+x)*ScreenH+(i+y)]=color;

    }
  }

}
u_char lastPillar=0;
void judgeColid(){
  u_char curPillar=0;

  if(pillars[0].x+PillarW>=birdX&&pillars[0].x<=birdX||pillars[0].x+PillarW>=birdX+birdW&&pillars[0].x<=birdX+birdW){
    curPillar=1;
  }else if(pillars[1].x+PillarW>=birdX&&pillars[1].x<=birdX||pillars[1].x+PillarW>=birdX+birdW&&pillars[1].x<=birdX+birdW){
    curPillar=2;
  }else if(pillars[2].x+PillarW>=birdX&&pillars[2].x<=birdX||pillars[2].x+PillarW>=birdX+birdW&&pillars[2].x<=birdX+birdW){
    curPillar=3;
  }
  if(lastPillar&&(!curPillar)){
    score++;
  }
  lastPillar=curPillar;
  if(curPillar){
    if(bird.y<=ScreenH - pillars[curPillar-1].height -KongxiHeight){
      dead=true;
    }else if(bird.y+birdW>=ScreenH - pillars[curPillar-1].height){
      dead=true;
    }
  }else{
    if(bird.y<0||bird.y+birdW>=ScreenH){
      dead=true;
    }
  }
}
void updateBird(){
  bird.speed+=0.15f;
  bird.speed-=0.015f*(bird.speed*abs(bird.speed));
  bird.y+=bird.speed;
}
void updatePillars(){
  pillars[0].x+=0.6f;
  pillars[1].x+=0.6f;
  pillars[2].x+=0.6f;
  if(pillars[0].x>=ScreenW){
    pillars[0].x=pillars[2].x-PillarSpace-PillarW;
    pillars[0].height=rand()%160+20;
  }
  if(pillars[1].x>=ScreenW){
    pillars[1].x=pillars[0].x-PillarSpace-PillarW;
    pillars[1].height=rand()%160+20;
  }
  if(pillars[2].x>=ScreenW){
    pillars[2].x=pillars[1].x-PillarSpace-PillarW;
    pillars[2].height=rand()%160+20;
  }
}
void loop()
{
  btn1.loop();
  memset(buffer,0,sizeof(buffer));
  
  drawRectToBuffer(pillars[0].x,ScreenH-pillars[0].height,PillarW,pillars[0].height,0xffff);
  drawRectToBuffer(pillars[1].x,ScreenH-pillars[1].height,PillarW,pillars[1].height,0xffff);
  drawRectToBuffer(pillars[2].x,ScreenH-pillars[2].height,PillarW,pillars[2].height,0xffff);

  drawRectToBuffer(pillars[0].x,0,PillarW,ScreenH-pillars[0].height-KongxiHeight,0xffff);
  drawRectToBuffer(pillars[1].x,0,PillarW,ScreenH-pillars[1].height-KongxiHeight,0xffff);
  drawRectToBuffer(pillars[2].x,0,PillarW,ScreenH-pillars[2].height-KongxiHeight,0xffff);
  tft.drawString((String)"score "+score,0,0,4);
  if(!dead){
    drawRectToBuffer(birdX,(int)bird.y,birdW,birdW,0xFFE0);
    updatePillars();
    updateBird();
    judgeColid();

  }else{
    drawRectToBuffer(birdX,(int)bird.y,birdW,birdW,0xF800);
  }
  
  //drawRectToBuffer(x1-PillarSpace-20,ScreenH-PillarH,20,PillarH,0xffff);
  //drawRectToBuffer(x1-PillarSpace*2-40,ScreenH-PillarH,20,PillarH,0xffff);
  tft.pushImage(0,0,240,135,buffer);
    // xTaskCreate(

    //   getData, /* Task function. */

    //   "TaskOne", /* String with name of task. */

    //   10000, /* Stack size in bytes. */

    //   NULL, /* Parameter passed as input of the task */

    //   1, /* Priority of the task. */

    //   NULL); /* Task handle. */

}


// 
// #include <SPI.h>
// #include "WiFi.h"
// #include <Wire.h>
// #include "esp_adc_cal.h"
// #include "esp_system.h"
// #include "esp_wifi.h"
// #include "esp_event_loop.h"
// #include "esp_log.h"
// #include "nvs_flash.h"
// #include "freertos/event_groups.h"
// #include <WiFi.h>

// #ifndef TFT_DISPOFF
// #define TFT_DISPOFF 0x28
// #endif

// #ifndef TFT_SLPIN
// #define TFT_SLPIN 0x10
// #endif

// #define TFT_MOSI 19
// #define TFT_SCLK 18
// #define TFT_CS 5
// #define TFT_DC 16
// #define TFT_RST 23

// #define TFT_BL 4 // Display backlight control pin
// #define ADC_EN 14
// #define ADC_PIN 34
// #define BUTTON_1 35
// #define BUTTON_2 0

// TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library

// char buff[512];
// int vref = 1100;
// int btnCick = false;

// //! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
// void delay(int ms)
// {
//   esp_sleep_enable_timer_wakeup(ms * 1000);
//   esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
//   esp_light_sleep_start();
// }


// void showVoltage()
// {
//   static uint64_t timeStamp = 0;
//   if (millis() - timeStamp > 1000)
//   {
//     timeStamp = millis();
//     uint16_t v = analogRead(ADC_PIN);
//     float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
//     String voltage = "Voltage :" + String(battery_voltage) + "V";
//     Serial.println(voltage);
//     //tft.fillScreen(TFT_BLACK);
//     tft.setTextDatum(MC_DATUM);
//     tft.drawString(voltage, tft.width() / 2, tft.height() / 2);
//   }
// }

// void taskOne(void *parameter)

// {

//   while (true)
//   {
    
//     Serial.println("sssssss" + portTICK_PERIOD_MS);
//     vTaskDelay(1000/portTICK_PERIOD_MS);
//   }

//   Serial.println("Ending task 1");

//   vTaskDelete(NULL);
// }

// void setup()
// {
//   delay(10000);
//   Serial.begin(115200);
//   Serial.println("Start");
//   Serial.print("Connecting to ");
//   Serial.println("peter");

//   Serial.print(WiFi.begin("peter", "13706737886"));
  

//   while (WiFi.status() != WL_CONNECTED) {
//       delay(500);
//       Serial.print(".");
//   }

//   Serial.println("");
//   Serial.println("WiFi connected");
//   Serial.println("IP address: ");
//   Serial.println(WiFi.localIP());

  
//   tft.init();
//   tft.setRotation(1);
//   tft.fillScreen(TFT_BLACK);
//   tft.setTextSize(2);
//   tft.setTextColor(TFT_WHITE);
//   tft.setCursor(0, 0);
//   tft.setTextDatum(MC_DATUM);
//   tft.setTextSize(1);
//   // xTaskCreate(

//   //     taskOne, /* Task function. */

//   //     "TaskOne", /* String with name of task. */

//   //     10000, /* Stack size in bytes. */

//   //     NULL, /* Parameter passed as input of the task */

//   //     1, /* Priority of the task. */

//   //     NULL); /* Task handle. */

//   if (TFT_BL > 0)
//   {                          // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
//     pinMode(TFT_BL, OUTPUT); // Set backlight pin to output mode
//     digitalWrite(TFT_BL, 1);
//                              // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
//   }
  


//   tft.setSwapBytes(true);
//   delay(5000);

//   tft.setRotation(0);
//   int i = 5;

//   while (true)
//   {
//     tft.fillScreen(TFT_RED);
//     showVoltage();
//     delay(1000);
//     tft.fillScreen(TFT_BLUE);
//     showVoltage();
//     delay(1000);
//     tft.fillScreen(TFT_GREEN);
//     showVoltage();
//     delay(1000);
//   }

//   esp_adc_cal_characteristics_t adc_chars;
//   esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC1_CHANNEL_6, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
//   //Check type of calibration value used to characterize ADC
//   if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
//   {
//     Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
//     vref = adc_chars.vref;
//   }
//   else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
//   {
//     Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
//   }
//   else
//   {
//     Serial.println("Default Vref: 1100mV");
//   }
// }

// void loop()
// {
//   delay(1000);
// }