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
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  return root[name].as<String>();
}

// ------------- Чтение значения json int
int jsonReadtoInt(String &json, String name) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  return root[name];
}

// ------------- Запись значения json String
String jsonWrite(String &json, String name, String volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
  return json;
}

// ------------- Запись значения json int
String jsonWrite(String &json, String name, int volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
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

// --------------------------------------
String convertList(String srcList ) {
  const char enter = 13;
  String stringTwo = srcList;
  stringTwo.replace("{\"n\":\"", "");
  String stringTemp = stringTwo;
  stringTemp.replace("\",\"v\":[", ",");
  stringTwo = stringTemp;
  stringTwo.replace("]},", ";");
  stringTemp = stringTwo;
  stringTemp.replace("]}", ";");
  stringTwo = stringTemp;
  stringTwo.replace("[", "");
  stringTemp = stringTwo;
  stringTemp.replace("]", "");
  stringTwo = stringTemp;
  stringTwo.replace("\n\r", "|");
  return "List not found";
  // return stringTemp;
}
