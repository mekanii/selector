#include <Arduino.h>
#include "monofontoReg20.h"
#include "monofontoReg28.h"
#include "monofontoReg96.h"
#include "ArduinoJson.h"
#include "FS.h"
#include "SPIFFS.h"
#include "Encoder.h"
#include <RTClib.h>
#include "HX711.h"
#include "tone.h"
#include "TFT_eSPI.h"

#define MONOFONTO20 monofonto20
#define MONOFONTO28 monofonto28
#define MONOFONTO96 monofonto96

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define FORMAT_SPIFFS_IF_FAILED true
#define CREATE_NEW_PARTLIST true

#define RW_SCALE_FACTOR false
#define INIT_CALIBRATION true
#define TEST_SCALE_CALIBRATION false

#define HYSTERESIS 5.0f
#define HYSTERESIS_STABLE_CHECK 1.0f
#define STABLE_READING_REQUIRED 30

float lastWeight = 0.0f;
int stableReadingsCount = 0;

// Define pins for the rotary encoder
#define ENCODER_PIN_A 9
#define ENCODER_PIN_B 14
#define ENCODER_BUTTON_PIN 46

// Define pins for the Load Cell
#define SCALE_DT_PIN 48
#define SCALE_SCK_PIN 45

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);
RTC_DS3231 rtc;
HX711 scale;
Tone beep;

DynamicJsonDocument doc(1024);
JsonArray partList;
int numOfPart;

bool RTC_STATUS = false;

int selectorIndex = 0;
int lastSelectorIndex = -1;

int page1Name = -1;
int page2Name = -1;
int page3Name = -1;

String partName = "";
float partStd = 0;

int CHECK_STATUS = 0;

const char* optsMenu[2] = {
  "START",
  "SETTINGS"
};

const char* optsOption[3] = {
  "CONTINUE",
  "TARE",
  "EXIT"
};

const char* optsExitDialog[2] = {
  "No, CANCEL",
  "Yes, EXIT",
};

void rtcInit() {
  // rtc.adjust(DateTime(2023, 2, 11, 18, 48, 0));
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
//    while (1) delay(10);
  } else {
    RTC_STATUS = true;
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

String dateTimeStart() {
  if (RTC_STATUS) {
    DateTime dateTime = rtc.now();
    return "START " + String(dateTime.year()) + "-" + String(dateTime.month()) + "-" + String(dateTime.day()) + " " + String(dateTime.hour()) + ":" + String(dateTime.minute());
  } else {
    return "RTC ERR: 0000-00-00 00:00";
  }
}

String dateTimeNow() {
  if (RTC_STATUS) {
    DateTime dateTime = rtc.now();
    return String(dateTime.year()) + "-" + String(dateTime.month()) + "-" + String(dateTime.day()) + " " + String(dateTime.hour()) + ":" + String(dateTime.minute());
  } else {
    return "RTC ERR: 0000-00-00 00:00";
  }
}

float readScale() {
  static float filteredWeight = 0.0; // Initialize the filtered weight
  const float alpha = 0.2; // Smoothing factor (0 < alpha <= 1)
  
  float weight = 0.00;
  if (scale.wait_ready_timeout(1000)) {
    weight = scale.get_units();
    filteredWeight = alpha * weight + (1 - alpha) * filteredWeight; // Apply low-pass filter
//    if (std_weight != 0) {
//      float remain_weight = floatMod(weight, atof(std_weight));
//      if (remain_weight < hysteresis || remain_weight > (weight - hysteresis)) {
//        beep.OK();
//        while (weight > hysteresis) {
//          if (scale.wait_ready_timeout(1000)) {
//            weight = scale.get_units();
//            TFT.display_weight(weight);
//          } else {
//            Serial.println("HX711 not found.");
//          }
//
//          delay(500);
//        }
//      }
//    }
  } else {
    Serial.println("HX711 not found.");
  }
  return filteredWeight;
}

bool checkStableState(float wt) {
  if (wt >= wt - HYSTERESIS_STABLE_CHECK && wt <= wt + HYSTERESIS_STABLE_CHECK && abs(wt - lastWeight) <= HYSTERESIS_STABLE_CHECK) {
  // if (abs(wt - lastWeight) < HYSTERESIS_STABLE_CHECK) {
    stableReadingsCount++;
  } else {
    stableReadingsCount = 0;
  }
  lastWeight = wt;
    
  return stableReadingsCount >= STABLE_READING_REQUIRED;
}

void displayMenu() {
  spr.loadFont(MONOFONTO28);
  for (int i = 0; i < ARRAY_SIZE(optsMenu); i++) {
    tft.setCursor(30, 10 + i * spr.fontHeight());
    if (i == selectorIndex) {
      spr.setTextColor(TFT_ORANGE, TFT_BLACK);
      spr.printToSprite(String(" > ") + optsMenu[i]);
    } else {
      spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      spr.printToSprite(String("    ") + optsMenu[i]);
    }
  }
  spr.unloadFont();
}

void dispalyMenuDisabled() {
  spr.loadFont(MONOFONTO28);
  for (int i = 0; i < ARRAY_SIZE(optsMenu); i++) {
    tft.setCursor(30, 10 + i * spr.fontHeight());
    if (i == selectorIndex) {
      spr.setTextColor(TFT_BROWN, TFT_BLACK);
      spr.printToSprite(String(" > ") + optsMenu[i]);
    } else {
      spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
      spr.printToSprite(String("    ") + optsMenu[i]);
    }
  }
  spr.unloadFont();
}

void displayPart() {
  spr.loadFont(MONOFONTO28);
  tft.setCursor(200, 10);
  if (selectorIndex == -1) {
    spr.setTextColor(TFT_ORANGE, TFT_BLACK);
    spr.printToSprite(String(" > back"));
  } else {
    spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    spr.printToSprite(String("    back"));
  }

  // Serial.println(partList.size());
  for (int i = 0; i < partList.size(); i++) {
    // Serial.println(partList[i]["name"].as<String>());
    tft.setCursor(200, 10 + ((i + 1) * spr.fontHeight()));
    if (i == selectorIndex) {
      spr.setTextColor(TFT_ORANGE, TFT_BLACK);
      spr.printToSprite(String(" > ") + partList[i]["name"].as<String>());
    } else {
      spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      spr.printToSprite(String("    ") + partList[i]["name"].as<String>());
    }
  }
  spr.unloadFont();
}

void displayMainFrame() {
  spr.loadFont(MONOFONTO20);
  spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setCursor(10, 10);
  spr.printToSprite(String(dateTimeStart()));
  
  spr.unloadFont();
  spr.loadFont(MONOFONTO28);
  spr.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setCursor(10, 10 + (1 * spr.fontHeight()));
  spr.printToSprite(partName);
  tft.setCursor(10, 10 + (2 * spr.fontHeight()));
  spr.printToSprite(String(partStd, 2));
  spr.unloadFont();
  
  spr.loadFont(MONOFONTO96);
  spr.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setCursor(372, 120);
  spr.printToSprite("gr");
  spr.deleteSprite();
  spr.unloadFont();
}

void displayMain(float sc) {
  spr.loadFont(MONOFONTO96);
  spr.createSprite(372, 96);
  spr.setTextDatum(TR_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.fillSprite(TFT_BLACK);
  spr.drawFloat(sc, 2, 360, 0);
  spr.pushSprite(0, 120);
  spr.unloadFont();
}

void displayCheckStatus() {
  spr.loadFont(MONOFONTO96);
  spr.createSprite(108, 96);
  spr.setTextDatum(TL_DATUM);
  if (CHECK_STATUS != 0) {
    spr.fillSprite(TFT_BLACK);
    if (CHECK_STATUS == 1) {
      spr.setTextColor(TFT_GREEN, TFT_BLACK);
      spr.drawString("OK", 0, 0);
    } else if (CHECK_STATUS == 2) {
      spr.setTextColor(TFT_RED, TFT_BLACK);
      spr.drawString("NG", 0, 0);
    }
    spr.pushSprite(372, 216);
    delay(2000);
  } else {
    spr.fillSprite(TFT_BLACK);
    spr.pushSprite(372, 216);
  }
  spr.deleteSprite();
  spr.unloadFont();
}

void displayHeader(String text) {
  spr.loadFont(MONOFONTO28);
  spr.createSprite(480, spr.fontHeight());
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.fillSprite(TFT_BLACK);
  spr.drawString(text, 240, 0);
  spr.pushSprite(0, 40);
  tft.drawFastHLine(0, 80, 480, TFT_WHITE);
  spr.deleteSprite();
  spr.unloadFont();
}

void displayOption() {
  spr.loadFont(MONOFONTO28);
  spr.createSprite(480, spr.fontHeight());
  spr.setTextDatum(TC_DATUM);
  for (int i = 0; i < ARRAY_SIZE(optsOption); i++) {
    if (i == selectorIndex) {
      spr.setTextColor(TFT_ORANGE, TFT_BLACK);
      spr.fillSprite(TFT_BLACK);
      spr.drawString(optsOption[i], 240, 0);
      spr.pushSprite(0, 100 + (i * 40));
    } else {
      spr.setTextColor(TFT_WHITE, TFT_BLACK);
      spr.fillSprite(TFT_BLACK);
      spr.drawString(optsOption[i], 240, 0);
      spr.pushSprite(0, 100 + (i * 40));
    }
  }
  spr.deleteSprite();
  spr.unloadFont();
}

void displayExitDialog() {
  spr.loadFont(MONOFONTO28);
  spr.createSprite(480, spr.fontHeight());
  spr.setTextDatum(TC_DATUM);
  for (int i = 0; i < ARRAY_SIZE(optsExitDialog); i++) {
    if (i == selectorIndex) {
      spr.setTextColor(TFT_ORANGE, TFT_BLACK);
      spr.fillSprite(TFT_BLACK);
      spr.drawString(optsExitDialog[i], 240, 0);
      spr.pushSprite(0, 100 + (i * 40));
    } else {
      spr.setTextColor(TFT_WHITE, TFT_BLACK);
      spr.fillSprite(TFT_BLACK);
      spr.drawString(optsExitDialog[i], 240, 0);
      spr.pushSprite(0, 100 + (i * 40));
    }
  }
  spr.deleteSprite();
  spr.unloadFont();
}

bool constrainer(int *newPosition, int min, int arrSize) {
  bool state = false;
  selectorIndex = constrain(*newPosition, min, arrSize - 1);
  if (selectorIndex != lastSelectorIndex) {
    lastSelectorIndex = selectorIndex;
    state = true;
  } else if (selectorIndex == arrSize - 1) {
    *newPosition = selectorIndex;
    encoder.write(selectorIndex * 2);
  } else if (selectorIndex == min) {
    *newPosition = 0;
    encoder.write(0);
  }
  return state;
}

void rotarySelector() {
  int newPosition = encoder.read() / 2;
  if (newPosition != selectorIndex) {
    switch(page1Name) {
      case 0: // menu
        switch(page2Name) {
          default:  // menu selection
            if (constrainer(&newPosition, 0, ARRAY_SIZE(optsMenu))) {
              displayMenu();
            }
            break;
          case 0:   // start
            switch(page3Name) {
              default:  // part selection
                if (constrainer(&newPosition, -1, partList.size())) {
                  displayPart();
                }
                break;
              case 0:   //main
                displayMain(readScale());
                break;
              case 1:   // options
                if (constrainer(&newPosition, 0, ARRAY_SIZE(optsOption))) {
                  displayOption();
                }
                break;
              case 2:   // exit dialog
                if (constrainer(&newPosition, 0, ARRAY_SIZE(optsExitDialog))) {
                  displayExitDialog();
                }
                break;
            }
            break;
          case 1:   // settings
            break;
        }
        break;
    }
  }
  else if (page1Name == 0 && page2Name == 0 && page3Name == 0) {
    float weight = readScale();
    displayMain(weight);

    if (checkStableState(weight)) {
      // if (weight >= 0 - HYSTERESIS && weight <= 0 + HYSTERESIS) {
      if (weight <= partStd * 0.1f) {
        CHECK_STATUS = 0;
      } else if (weight >= partStd - HYSTERESIS && weight <= partStd + HYSTERESIS && CHECK_STATUS == 0) {
        CHECK_STATUS = 1;
        beep.OK();
      } else if (CHECK_STATUS == 0) {
        CHECK_STATUS = 2;
        beep.NG();
      }
      displayCheckStatus();
    }
  }
}

void rotaryButton() {
  if (digitalRead(ENCODER_BUTTON_PIN) == LOW) {
    encoder.write(0);
    switch(page1Name) {
      case 0: // menu
        switch(page2Name) {
          default:  // menu selection
            switch (selectorIndex){
              case 0:
                selectorIndex = 0;
                dispalyMenuDisabled();
                displayPart();
                page2Name = 0;
                break;
              case 1:
                break;
            }
            break;
          case 0:   // start
            switch(page3Name) {
              default:  // part selection
                if (selectorIndex == -1) {
                  selectorIndex = 0;
                  tft.fillScreen(TFT_BLACK);
                  displayMenu();
                  page2Name = -1;
                } else {
                  tft.fillScreen(TFT_BLACK);
                  partName = partList[selectorIndex]["name"].as<String>();
                  partStd = partList[selectorIndex]["std"].as<float>();
                  selectorIndex = 0;
                  displayMainFrame();
                  displayMain(readScale());
                  page3Name = 0;
                }
                break;
              case 0:   //main
                selectorIndex = 0;
                tft.fillScreen(TFT_BLACK);
                displayHeader("OPTIONS");
                displayOption();
                page3Name = 1;
                break;
              case 1:   // options
                switch (selectorIndex) {
                  case 0:   // continue
                    selectorIndex = 0;
                    tft.fillScreen(TFT_BLACK);
                    displayMainFrame();
                    displayMain(readScale());
                    page3Name = 0;
                    break;
                  case 1:   // tare
                    scale.tare();
                    selectorIndex = 0;
                    tft.fillScreen(TFT_BLACK);
                    displayMainFrame();
                    displayMain(readScale());
                    page3Name = 0;
                    break;
                  case 2:   // exit
                    selectorIndex = 0;
                    tft.fillScreen(TFT_BLACK);
                    displayHeader("Are you sure?");
                    displayExitDialog();
                    page3Name = 2;
                    break;
                }
                break;
              case 2:   // exit dialog
                switch (selectorIndex) {
                  case 0:   // continue
                    selectorIndex = 0;
                    tft.fillScreen(TFT_BLACK);
                    displayMainFrame();
                    displayMain(readScale());
                    page3Name = 0;
                    break;
                  case 1:   // exit
                    selectorIndex = 0;
                    tft.fillScreen(TFT_BLACK);
                    displayMenu();
                    page1Name = 0;
                    page2Name = -1;
                    page3Name = -1;
                    break;
                }
                break;
            }
            break;
          case 1:   // settings
            break;
        }
        break;
    }
    selectorIndex = 0;
    lastSelectorIndex = -1;
    delay(500);
  }
}

void writePart() {
  const char* pl[9] = {
    "NORMAL",
    "PART 01",
    "PART 02",
    "PART 03",
    "PART 04",
    "PART 05",
    "PART 06",
    "PART 07",
    "PART 08"
  };

  const float psl[9] = {
    0,
    0.10,
    0.25,
    400.00,
    1000.00,
    2.50,
    5.00,
    7.25,
    10.00
  };

  const size_t capacity = JSON_ARRAY_SIZE(9) + JSON_OBJECT_SIZE(9) + 400;
  DynamicJsonDocument doc(capacity);

  JsonArray partListArray = doc.createNestedArray("partList");
  for (int i = 0; i < 9; i++) {
    JsonObject part = partListArray.createNestedObject();
    part["name"] = pl[i];
    part["std"] = psl[i];
  }

  File file = SPIFFS.open("/partList.json", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  serializeJson(doc, file);
  file.close();
}

void appendPart(String name, String std) {
  const size_t capacity = JSON_ARRAY_SIZE(9) + JSON_OBJECT_SIZE(9) + 400;
  DynamicJsonDocument doc(capacity);

  // Read the existing file
  File file = SPIFFS.open("/partList.json", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) {
    Serial.println("Failed to read file, using empty JSON");
  }

  // Append new part to the array
  JsonArray partListArray = doc["partList"].as<JsonArray>();
  JsonObject newPart = partListArray.createNestedObject();
  newPart["name"] = name;
  newPart["std"] = std.toFloat();

  // Write the updated file
  file = SPIFFS.open("/partList.json", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  serializeJson(doc, file);
  file.close();
}

DynamicJsonDocument getPartList() {
  const size_t capacity = JSON_ARRAY_SIZE(9) + JSON_OBJECT_SIZE(9) + 400;
  doc.clear(); // Clear the document before use

  // Read the existing file
  File file = SPIFFS.open("/partList.json", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return DynamicJsonDocument(0);
  }
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) {
    Serial.println("Failed to read file, returning empty JSON document");
    return DynamicJsonDocument(0);
  }

  // Print the part list to Serial Monitor
  JsonArray partListArray = doc["partList"].as<JsonArray>();
  for (JsonObject part : partListArray) {
    Serial.print("name: ");
    Serial.print(part["name"].as<String>());
    Serial.print(", std: ");
    Serial.println(part["std"].as<float>(), 2); // Format to 2 decimal places
  }

  // Return the document containing the part list array
  return doc;
}

void writeScaleFactor(float sc = 0) {
  if (sc == 0) {
    sc = 2280.0f;
  }
  File file = SPIFFS.open("/scaleFactor.txt", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.print(sc, 2); // Write the scale factor to the file with 2 decimal places
  file.close();
  Serial.println("Scale factor written to SPIFFS");
}

float readScaleFactor() {
  File file = SPIFFS.open("/scaleFactor.txt", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return 2280.0f;
  }
  float sc = file.parseFloat();
  file.close();
  return sc;
}

void calibrateScale(float sc = 0) {
  Serial.println("Starting scale calibration...");

  scale.set_scale();
  scale.tare();

  if (sc == 0) {
    Serial.println("Place a known weight on the scale.");
    delay(5000);
    sc = scale.get_units(10);
    float wt = 1000.0f;
    sc = sc / wt;
  }
  scale.set_scale(sc);

  Serial.print("Calibration complete. Scale factor: ");
  Serial.println(scale.get_scale(), 2);

  // Save the scale factor to SPIFFS
  writeScaleFactor(scale.get_scale());
}


void setup() {
  Serial.begin(115200);

  rtcInit();
  
  scale.begin(SCALE_DT_PIN, SCALE_SCK_PIN);

  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
  
  delay(1000);
  
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  if(CREATE_NEW_PARTLIST) {
    writePart();
  }

  doc = getPartList();
  partList = doc["partList"].as<JsonArray>();
  // numOfPart = partList.size();

  if (RW_SCALE_FACTOR) {
    writeScaleFactor();
    readScaleFactor();
  }
  
  if (INIT_CALIBRATION) {
    calibrateScale(readScaleFactor());
  }

  if (TEST_SCALE_CALIBRATION) {
    calibrateScale();
  }

  tft.init();
  tft.setRotation(1);
  spr.setColorDepth(16);

  page1Name = 0;
  tft.fillScreen(TFT_BLACK);
  displayMenu();
}

void loop() {
  rotarySelector();
  rotaryButton();
}

