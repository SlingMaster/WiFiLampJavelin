/*---------------------------------------
  parsing.ino
  --------------------------------------- */
// ======================================
void parseUDP() {
  const byte BIT08 = 99U;
  const byte TRANSFER_START = 254U;
  const byte TRANSFER_END = 255U;

  const byte TRANSFER_SOUND_INIT = 125U;
  const byte TRANSFER_SOUND_PAUSE = 126U;
  const byte TRANSFER_SOUND_STOP = 127U;

  int16_t packetSize = Udp.parsePacket();
  const int16_t MAX_PACKET = HEIGHT * 3U;
  bool generateOutput;
  if (packetSize) {
    int16_t n = Udp.read(packetBuffer, MAX_FRAME_BUFER);
    packetBuffer[n] = '\0';
    // crop color data if packetBuffer long ---
    if (packetSize > MAX_PACKET) {
      strncpy(inputBuffer, packetBuffer, 128U);
    } else {
      strcpy(inputBuffer, packetBuffer);
    }
    // ----------------------------------------

    if (Udp.remoteIP() == WiFi.localIP())  {                // не реагировать на свои же пакеты
      return;
    }

#ifdef GENERAL_DEBUG
    // LOG.print(F("Inbound UDP packet: "));
    // printf("COL • [ %3d ] %3d | %3d |\n", packetBuffer[3], packetBuffer[4], packetBuffer[5]);
#endif
    char reply[MAX_UDP_BUFFER_SIZE];

    // SOUND VISUALISER ============
#ifdef USE_SOUND_VISUALISER
    String inf = getIoTInfo();
#endif
    /* external sound data ------------------ */
    if (!strncmp_P(inputBuffer, PSTR("SND"), 3)) {
#ifdef USE_SOUND_VISUALISER
      byte id_cmd = packetBuffer[3];
      char jsonData[inf.length() + 1];
      inf.toCharArray(jsonData, inf.length() + 1);
#ifdef JAVELIN
      if (!extCtrl) DrawLevel(0, 35, 35, CHSV {160, 255, 255});
#endif
      switch (id_cmd) {
        case TRANSFER_SOUND_INIT:
          /* init and send info data -------- */
          extCtrl = true;
          FastLED.setBrightness(30U);
          drawNote(CRGB::Blue);
          FPSdelay = HIGH_DELAY;
          sprintf_P(reply, PSTR("{\"cmd\":%u,%s}"), CMD_DISCOVER, jsonData);
          break;
        case TRANSFER_SOUND_PAUSE:
          drawNote(CRGB::Magenta);
          break;
        case TRANSFER_SOUND_STOP:
          /* stop analizator ---------------- */
          sprintf_P(reply, PSTR("{\"cmd\":%u,%s}"), id_cmd, "{}");
          setFPS();
          FastLED.clear();
          FastLED.delay(2);
          extCtrl = false;
#ifdef JAVELIN
          JavelinLight(0x000000, 0x000000, 0x000000);
          DrawLevel(0, 35, 35, CHSV {0, 255, 0});
#endif
          runEffect();
          break;
        default:
          /* sound visualisetion ------------ */
          extCtrl = true;
          sprintf_P(reply, PSTR("{\"cmd\":%u,%s}"), 0, "{}");
          SoundVisualiser(packetBuffer, packetSize, id_cmd);
          break;
      }
#endif
    }
    // =============================

    /* external gif animate ------- */
    else if (!strncmp_P(inputBuffer, PSTR("FRM"), 3)) {
#ifdef JAVELIN
      if (!extCtrl) DrawLevel(0, 35, 35, CHSV {210, 255, 255});
#endif

      extCtrl = true;
      // x command or column --------
      byte x = packetBuffer[3];
      // printf("ID • [ %3d ] \n", x);

      if (x == BIT08) {
        //  full frame | rgb • 332 | 8 bit ---
        //  printf("COL • [ %3d ] %3d | %3d |\n", x, packetBuffer[4], packetBuffer[5]);
        sprintf_P(reply, PSTR("RES,%u"), x);
        crateFrame8bitColor(packetBuffer);
      } else {
        if (x < WIDTH) {
          // only column ---------------------
          // rgb • 888 | 24 bit color
          sprintf_P(reply, PSTR("RES,%u"), x);
          crateColumn24bitColor(packetBuffer, x);
        }
        /*if (x == 160U) {
          // rgb • 565 | 16 bit color -------
          printf("COL • [ %3d ] %3d | %3d |\n", x, packetBuffer[4], packetBuffer[5]);
          crateFrame16bitColor(packetBuffer);
          sprintf_P(reply, PSTR("RES,%u"), x);
          //printf("COL • [ %3d ] %3d | %3d |\n", x, packetBuffer[4], packetBuffer[5]);
          // LOG.printf_P(PSTR("%5d | COL • %3d | %3d | %3d |\n\r"), packetSize, x, packetBuffer[4], packetBuffer[5]);
          }*/
        if (x == TRANSFER_START) {
          // send matrix size -------
          extCtrl = true;
          sprintf_P(reply, PSTR("INIT,%u,%u"), WIDTH, HEIGHT);
          FastLED.setBrightness(20U);
        }
        if (x == TRANSFER_END) {
          // response end -----------
          sprintf_P(reply, PSTR("END,%u,%u"), WIDTH, HEIGHT);
          loadingFlag = true;
          delay(500);
          extCtrl = false;

#ifdef JAVELIN
          JavelinLight(0x000000, 0x000000, 0x000000);
          DrawLevel(0, 35, 35, CHSV {0, 255, 0});
#endif
          runEffect();
        }
      }

    } else {
      commandDecode (inputBuffer, reply, true);
    }

    packetBuffer[0] = '\0';
    inputBuffer[0] = '\0';
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


  /* led lamp control by MQTT protocol -- */
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
      /* relays control by MQTT protocol ---- */
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
  if (generateOutput == 1U) {
    sendJsonData(outputBuffer, 0, getLampState());
  }

  runCommand(CMD, val, valStr);
  return CMD;
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


#if USE_MQTT
// ======================================
void sendCurrent(char *outputBuffer) {
  sendJsonData(outputBuffer, 0, getLampState());
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
#endif

// ======================================
String getLampState() {

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

// ======================================
// Projector | Show Frame Function
// ======================================
void crateFrame8bitColor(char *Buffer) {
  const byte OFFSET = 4U;    // FRM [x]
  uint16_t pixIndex;

  // draw frame ------------------------
  for (uint8_t x = 0U; x < WIDTH; x++) {
    for (uint8_t y = 0U; y < HEIGHT; y++) {
      /* convert rgb332 to rgb888 ----------*/
      pixIndex = OFFSET + x * HEIGHT + y;
      leds[XY(x, y)] = rgb332ToCRGB(Buffer[pixIndex]);
    }
  }
  FastLED.show();
}

/*
  // ======================================
  void crateFrame16bitColor(char *Buffer) {
  const byte OFFSET = 4U;    // FRM [x]
  uint32_t pixIndex;
  // draw frame ------------------------
  for (uint8_t x = 0U; x < WIDTH; x++) {
    for (uint8_t y = 0U; y < HEIGHT; y++) {
      // convert rgb565 to rgb888 -----------
      //  masc rgb565  0xF800 | 0x07E0 | 0x001F
      pixIndex = OFFSET + (x * HEIGHT + y) * 2;
      uint8_t r = (Buffer[pixIndex + 1] & 0xF8);
      uint8_t g = ((Buffer[pixIndex + 1] & 0x07) << 5) + ((Buffer[pixIndex] & 0xE0) << 5);
      uint8_t b = (Buffer[pixIndex] & 0x1F) << 3;
      // LOG.printf_P(PSTR("%6d | COL • %2d %2d || %3d | %3d | %3d |\n\r"), pixIndex, x, y, r, g, b);
      leds[XY(x, y)] = CRGB(r, g, b);
    }
  }
  FastLED.show();
  }
*/

// ======================================
void crateColumn24bitColor(char *Buffer, byte x) {
  const byte OFFSET = 4U;    // FRM [x]
  // draw frame ------------------------
  if (x < WIDTH) {
    for (uint8_t y = 0U; y < HEIGHT; y++) {
      // set color from matrix col -----
      uint8_t r = Buffer[y * 3 + OFFSET];
      uint8_t g = Buffer[y * 3 + 1 + OFFSET];
      uint8_t b = Buffer[y * 3 + 2 + OFFSET];
      // printf("COL [ %2d ] • %3d | %3d | %3d | ", x, r, g, b);
      leds[XY(x, y)] = CRGB(r, g, b);
    }
    // printf("%s \n\r", "*");if (x%4 == 0 WIDTH - 1) {
    if (x >= WIDTH - 1) {
      FastLED.show();
    }
  }
}
