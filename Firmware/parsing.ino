/*---------------------------------------
  parsing.ino
  --------------------------------------- */
// ======================================
void parseUDP() {
  int32_t packetSize = Udp.parsePacket();
  if (packetSize) {
    int16_t n = Udp.read(packetBuffer, MAX_UDP_BUFFER_SIZE);
    packetBuffer[n] = '\0';
    strcpy(inputBuffer, packetBuffer);
    if (Udp.remoteIP() == WiFi.localIP())  {                // не реагировать на свои же пакеты
      return;
    }
#ifdef GENERAL_DEBUG
    // LOG.print(F("Inbound UDP packet: "));
    // LOG.println(inputBuffer);
#endif
    char reply[MAX_UDP_BUFFER_SIZE];
    commandDecode (inputBuffer, reply, true);
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(reply);
    Udp.endPacket();
  }
}



// ======================================
uint8_t commandDecode (char *inputBuffer, char *outputBuffer, bool generateOutput) {
  char buff[MAX_UDP_BUFFER_SIZE], *endToken = NULL;
  String BUFF = String(inputBuffer);

#ifdef USE_MULTIPLE_LAMPS_CONTROL
  /* multiple lamp control by UDP protocol - */
  if (!strncmp_P(inputBuffer, PSTR("MULTI"), 5)) {
    multipleLampParsing(inputBuffer);
    inputBuffer[0] = '\0';
    return 0;
  }
#endif

  /* lamp control by MQTT protocol -------- */
  String jsonStr = String(inputBuffer);
  uint8_t CMD = jsonReadtoInt(jsonStr, "cmd");
  uint8_t val = jsonReadtoInt(jsonStr, "val");
  String valStr = jsonRead(jsonStr, "valStr");
  char stringTime[6U];
  localTime(stringTime);
  // --------------------------------------
  //  LOG.printf_P(PSTR("| COMMAND • %03d | val • %03d | valStr |"), CMD, val);
  //  LOG.println(valStr);

  /* led lamp control by MQTT protocol ----- */

#if USE_MQTT
  if (CMD == CMD_DISCOVER) {
    sendJsonData(MqttManager::mqttBuffer, CMD, getIoT());
    MqttManager::publishState();
    inputBuffer[0] = '\0';
    return CMD;
  } else {
#ifdef USE_T_SENSOR
    if (CMD == 51U) {  /* IoT SENSOR TEMPERATURE */
      sendCurrentTemperature(inputBuffer);
    }
#endif
    if (CMD > 100U) {
#ifdef  USE_RELAYS
      /* relays control by MQTT protocol ------ */
      remoteRelay(CMD, val);
      // sendResponseRelaysState(CMD, stringTime, inputBuffer);
#endif
    } else {
      /* run command compatible http command */
      runCommand(CMD, val, valStr);
    }
  }
#else
  /* run command compatible http command
     UDP protocol only -------------- */
  /* send response for UDP request ----- */
#if USE_MQTT
  if (generateOutput == 1U) {
    sendJsonData(outputBuffer, 0, getLampState());
  }
#endif
  runCommand(CMD, val, valStr);
#endif

  /* ------------------------------------- */

  if (strlen(inputBuffer) > 0) {
    if (generateOutput) {                       // если запрошен вывод ответа выполнения команд, копируем его в исходящий буфер
      strcpy(outputBuffer, inputBuffer);
    }
    inputBuffer[0] = '\0';                      // очистка буфера, читобы не он не интерпретировался, как следующий входной пакет
  }
  return CMD;
}
#if USE_MQTT
// ======================================
void sendCurrent(char *outputBuffer) {
  sendJsonData(outputBuffer, 0, getLampState());
}

// ======================================
void sendJsonData(char *outputBuffer, uint8_t cmd, String jsonStr) {
  String json = "{";
  // cmd -------------- •
  json += "\"cmd\":" + String(cmd);
  // chip id ------------
  json += ",\"id\":\"" + getChipIdToStr() + "\"";
#ifdef USE_NTP
  char stringTime[6U];
  localTime(stringTime);
  char timeBuf[9];
  getFormattedTime(timeBuf);
  // time sync ----------
  json += ",\"sync\":\"" + String(timeBuf) + "\"";
#endif
  // json data ----------
  if (jsonStr != "") {
    json += jsonStr;
  }
  json += "}";

  char jsonData[json.length() + 1];
  json.toCharArray(jsonData, json.length() + 1);
  // -------------------------------------
  sprintf_P(outputBuffer, PSTR("%s"), jsonData );
}

// ======================================
String getIoT() {
  char buf_s[strlen_P(MqttClientIdPrefix) + 1];
  strcpy_P(buf_s, MqttClientIdPrefix);
  String type = String(buf_s);
  // ---------------------------
  String json = ",\"device\":{";
  // ip -----------
  json += "\"ip\":\"" + ipToString(WiFi.localIP()) + "\"";
  // port ------- •
  json += ",\"udp_port\":" + String(ESP_UDP_PORT);
  // name ---------
  json += ",\"name\":\"" + LAMP_NAME + "\"";
  // type ---------
  json += getTypeIoT(); // ",\"type\":\"" + type.substring(0, type.length() - 1) + "\"";
  json += "}";
  // ---------------------------
  return json;
}

// ======================================
String getTypeIoT() {
  char buf_s[strlen_P(MqttClientIdPrefix) + 1];
  strcpy_P(buf_s, MqttClientIdPrefix);
  String type = String(buf_s);
  String json = ",\"type\":\"" + type.substring(0, type.length() - 1) + "\"";
  return json;
}

// ======================================
String getLampState() {
  char buf_s[strlen_P(MqttClientIdPrefix) + 1];
  strcpy_P(buf_s, MqttClientIdPrefix);
  String type = String(buf_s);
  // ---------------------------
  String json = ",\"lamp_state\":{";
  // power ------ •
  json += "\"power\":" + String(ONflag);
  // eff -------- •
  json += ",\"eff\":" + String(currentMode);
  // brightness - •
  json += ",\"brightness\":" + String(modes[currentMode].Brightness);
  // speed ------ •
  json += ",\"speed\":" + String(modes[currentMode].Speed);
  // scale ------ •
  json += ",\"scale\":" + String(modes[currentMode].Scale);
  // max_eff ---- •
  json += ",\"max_eff\":" + String(MODE_AMOUNT);
  json += "}";
  // ---------------------------
  return json;
}

// ======================================
String getChipIdToStr() {
  uint32_t chipId = ESP.getChipId();
  String id = "00000000" + String(ESP.getChipId(), HEX);
  id.remove(0, id.length() - 8);
  id.toUpperCase();
  return id;
}
#endif

// ======================================
// ======================================
void setFPS() {
  /* set effects frame delay ----- */
  if (pgm_read_byte(&defaultSettings[currentMode][3]) == DYNAMIC) {
    FPSdelay = 256U - modes[currentMode].Speed;
  } else {
    if (pgm_read_byte(&defaultSettings[currentMode][3]) != SOFT_DELAY) {
      FPSdelay = pgm_read_byte(&defaultSettings[currentMode][3]);
    }
  }
#ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR(" Effect • %03d | FPSdelay • %d\n\r"), currentMode, FPSdelay);
#endif
}

// ======================================
void updateSets() {
  loadingFlag = true;
  settChanged = true;
  setFPS();
  eepromTimeout = millis();

#if (USE_MQTT)
  if (espMode == 1U) {
    MqttManager::needToPublish = true;
  }
#endif
}

// ======================================
void sendAlarms(char *outputBuffer) {
  strcpy_P(outputBuffer, PSTR("ALMS"));

  for (byte i = 0; i < 7; i++) {
    sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, (uint8_t)alarms[i].State);
  }

  for (byte i = 0; i < 7; i++) {
    sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, alarms[i].Time);
  }
  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, dawnMode + 1);
}
