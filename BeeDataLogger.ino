#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <BDL.h>
#include <RTClib.h>

RTC_DS3231 rtc;
BDL bdl;

DateTime lastMidnight;  // Переменная для хранения времени последней полуночи
bool firstRun = true;  // Флаг для определения первого запуска
unsigned long lastRecordTime = 0;  // Время последней записи

String currentFilename;

void initSDCard(){
   if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void setup() {
  Serial.begin(115200);
  
  bdl.begin();
   if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  DateTime now = rtc.now();
  unsigned long currentTime = millis();
  initSDCard();

  // Проверяем, если наступил новый день (полночь)
  if (now.day() != lastMidnight.day()) {
    lastMidnight = DateTime(now.year(), now.month(), now.day(), 0, 0, 0);  // Обновляем время последней полуночи
    createNewLogFile();
  }

  String filename = currentFilename;

  // Открываем файл для добавления данных
  File file = SD.open(filename, FILE_APPEND);

  if (file) {
    String timestamp = now.timestamp();
    file.println(timestamp);
    file.close();
    Serial.println(timestamp);
    lastRecordTime = currentTime;
  } else {
    Serial.println("Failed to open '" + filename + "' for writing");
  }

  // Если прошло больше 5 секунд с момента последней записи, добавляем "Logging continued"
  if (currentTime - lastRecordTime > 5000) {
    addLoggingContinued();
    lastRecordTime = currentTime; // Обновляем время последней записи
  }

  delay(5000); // Задержка 5 секунд между записями
}

void createNewLogFile() {
  DateTime now = rtc.now();
  currentFilename = "/log" + String(now.day()) + String(now.month()) + String(now.year()) + ".txt";
  // Создаем новый файл
  File file = SD.open(currentFilename, FILE_APPEND);
  if (file) {
    file.println("Start Logging:");
    file.close();
    Serial.println("File '" + currentFilename + "' created.");
  } else {
    Serial.println("Failed to open '" + currentFilename + "' for writing");
  }
}

void addLoggingContinued() {
  String filename = currentFilename;

  // Открываем файл для добавления данных
  File file = SD.open(filename, FILE_APPEND);

  if (file) {
    if (firstRun) {
      file.println("Start Logging:");
      firstRun = false;
    } else {
      file.println("Logging continued");
    }
    file.close();
    Serial.println("Logging continued");
  } else {
    Serial.println("Failed to open '" + filename + "' for writing");
  }
}
