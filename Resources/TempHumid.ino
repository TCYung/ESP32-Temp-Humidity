#include <Arduino.h>
#include <SensirionI2cSht4x.h>
#include <Wire.h>

// macro definitions
// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

#define USE_HSPI_FOR_EPD
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_154_D67

#define GxEPD2_BW_IS_GxEPD2_BW true
#define GxEPD2_3C_IS_GxEPD2_3C true
#define GxEPD2_7C_IS_GxEPD2_7C true
#define GxEPD2_1248_IS_GxEPD2_1248 true
#define IS_GxEPD(c, x) (c##x)
#define IS_GxEPD2_BW(x) IS_GxEPD(GxEPD2_BW_IS_, x)
#define IS_GxEPD2_3C(x) IS_GxEPD(GxEPD2_3C_IS_, x)
#define IS_GxEPD2_7C(x) IS_GxEPD(GxEPD2_7C_IS_, x)
#define IS_GxEPD2_1248(x) IS_GxEPD(GxEPD2_1248_IS_, x)

#if defined(ESP32)
#define MAX_DISPLAY_BUFFER_SIZE 65536ul // e.g.
#if IS_GxEPD2_BW(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
#elif IS_GxEPD2_3C(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
#elif IS_GxEPD2_7C(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2))
#endif
// GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/ 15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25)); driver board
//GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/ 5, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // regular esp32
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/ 16, /*DC=*/ 17, /*RST=*/ 18, /*BUSY=*/ 8)); // pcb esp32-s3


#endif

// alternately you can copy the constructor from GxEPD2_display_selection.h of GxEPD_Example to here
// and adapt it to the ESP32 Driver wiring, e.g.
//GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(/*CS=*/ 15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25)); // GDEH0154D67

// comment out unused bitmaps to reduce code space used
#include "bitmaps/Bitmaps200x200.h" // 1.54" b/w
// uncomment 3 lines below if driver board
// #if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
// SPIClass hspi(HSPI);
//#endif


SensirionI2cSht4x sensor;

static char errorMessage[64];
static int16_t error;
const char Temperature[] = "Temp: ";
const char Humidity[] = "Humidity: ";
int sda = 6;
int scl = 5;

void setup() {
  // uncomment the 4 lines under if driver board
  // #if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
  // hspi.begin(13, 12, 14, 15); // remap hspi for EPD (swap pins)
  // display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  //#endif
    SPI.begin(15, -1, 7, 16); // remap spi (sck, miso, mosi, cs)

    // 4 lines below for serial output sensor testing
    // Serial.begin(115200);
    // while (!Serial) {
    //     delay(100);
    // }
    Wire.begin(sda, scl);
    sensor.begin(Wire, SHT40_I2C_ADDR_44);

    sensor.softReset();
    delay(10);
    uint32_t serialNumber = 0;
    error = sensor.serialNumber(serialNumber);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute serialNumber(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    // 3 lines below for serial output testing
    // Serial.print("serialNumber: ");
    // Serial.print(serialNumber);
    // Serial.println();

    display.init(115200);
    display.setRotation(1);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(2);
    int16_t tbx, tby; uint16_t tbw, tbh;
    // center HelloWorld
    // display.getTextBounds(Temperature, 0, 0, &tbx, &tby, &tbw, &tbh);
    // uint16_t Tempx = ((display.width() - tbw) / 2) - tbx;
    // uint16_t Tempy = ((display.height() - tbh) / 2) - tby;
    display.firstPage();
    do
    {
      display.fillScreen(GxEPD_WHITE);
      display.setCursor(0, 25); //middle was x = 44 y = 105
      display.print(Temperature);
      display.setCursor(0, 125);
      display.print(Humidity);
    }
    while (display.nextPage());
}

void loop() {
    float aTemperature = 0.0;
    float aHumidity = 0.0;
    float FTemp = 0.0;
    char FTempBuffer[10];
    char HumidBuffer[10];
    delay(20);

    // lines below for serial output sensor testing
    // int16_t tbx, tby; uint16_t tbw, tbh;
    // center HelloWorld
    // display.getTextBounds(Temperature, 0, 0, &tbx, &tby, &tbw, &tbh);
    // uint16_t Tempx = ((display.width() - tbw) / 2) - tbx;
    // uint16_t Tempy = ((display.height() - tbh) / 2) - tby;
    // Serial.print(Tempx);
    // Serial.print(Tempy);

    error = sensor.measureLowestPrecision(aTemperature, aHumidity);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute measureLowestPrecision(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    FTemp = ((aTemperature*9/5) + 32);
    dtostrf(FTemp, 4, 2, FTempBuffer);
    strcat(FTempBuffer, "F");
    const char* FTemp2 = FTempBuffer;
    dtostrf(aHumidity, 4, 2, HumidBuffer);
    strcat(HumidBuffer, "%");
    const char* Humid = HumidBuffer;
    
    // lines below for serial output sensor testing
    // Serial.print("aTemperature: ");
    // Serial.print(aTemperature);
    // Serial.print("\t");
    // Serial.print("FTemp: ");
    // Serial.print(FTemp);
    // Serial.print("\t");
    // Serial.print("aHumidity: ");
    // Serial.print(aHumidity);
    // Serial.println();
    //display.init(115200);

    TempPartial(FTemp2);
    HumidPartial(Humid);
    //delay(1000);
}

void TempPartial(const char* FTemp2)
{
  //Serial.println("helloFullScreenPartialMode");
  display.setPartialWindow(0, 40, 200, 55);
  // do this outside of the loop
  int16_t tbx, tby; uint16_t tbw, tbh;
  // center HelloWorld
  display.getTextBounds(FTemp2, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t hwx = ((display.width() - tbw) / 2) - tbx;
  uint16_t hwy = 75;
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(hwx, hwy);
    display.print(FTemp2);
  }
  while (display.nextPage());
  //Serial.println("helloFullScreenPartialMode done");
}

void HumidPartial(const char* Humid)
{
  //Serial.println("helloFullScreenPartialMode");
  display.setPartialWindow(0, 150, 200, 200);
  // do this outside of the loop
  int16_t tbx2, tby2; uint16_t tbw2, tbh2;
  // center HelloWorld
  display.getTextBounds(Humid, 0, 0, &tbx2, &tby2, &tbw2, &tbh2);
  uint16_t hwx2 = ((display.width() - tbw2) / 2) - tbx2;
  uint16_t hwy2 = 175;
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(hwx2, hwy2);
    display.print(Humid);
  }
  while (display.nextPage());
  //Serial.println("helloFullScreenPartialMode done");
}