// json_utils.h
// Функции для работы с json файлами ===
// developer alvikskor =================
#include <ArduinoJson.h>        //Установить из менеджера библиотек версию 5.13.5 !!!. https://arduinojson.org/
#ifdef USE_LittleFS
#include <LittleFS.h>
#define SPIFFS LittleFS
#endif

// ------------- Чтение значения json String
String jsonRead(String &json, String name) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  return doc[name].as<String>();
}

// ------------- Чтение значения json int
int jsonReadtoInt(String &json, String name) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  return doc[name];
}

// ------------- Запись значения json String
String jsonWrite(String &json, String name, String volume) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  doc[name] = volume;
  json = "";
  serializeJson(doc, json);
  return json;
}

// ------------- Запись значения json int
String jsonWrite(String &json, String name, int volume) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);

  doc[name] = volume;
  json = "";
  serializeJson(doc, json);
  return json;
}

// ------------- Запись строки в файл
String writeFile(String fileName, String strings ) {
  File configFile = SPIFFS.open("/" + fileName, "w");
  if (!configFile) {
    return "Failed to open config file";
  }
  configFile.print(strings);
  //strings.printTo(configFile);
  configFile.close();
  return "Write sucsses";
}

void saveConfig () {
  writeFile("config.json", configSetup );
}

// ------------- Чтение файла в строку
String readFile(String fileName, size_t len ) {
  File configFile = SPIFFS.open("/" + fileName, "r");
  if (!configFile) {
    return "Failed";
  }
  size_t size = configFile.size();
  if (size > len) {
    configFile.close();
    return "Large";
  }
  String temp = configFile.readString();
  configFile.close();
  return temp;
}
