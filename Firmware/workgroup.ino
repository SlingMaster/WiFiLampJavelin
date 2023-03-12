// workgroup.ino

#ifdef USE_MULTIPLE_LAMPS_CONTROL
// ======================================
// ========== WORKGROUP CONTROL =========
// ======================================
void initWorkGroup(String lampList) {
  WORKGROUP = 1;
  jsonWrite(configSetup, "workgroup", WORKGROUP);
  LAMP_LIST = lampList;
  jsonWrite(configSetup, "lamp_list", LAMP_LIST);
  saveConfig();
#ifdef GENERAL_DEBUG
  LOG.println("\n• INIT WORKGROUP • ");
  LOG.println("• Workgroup Lamp : " + LAMP_LIST);
#endif
}

// ======================================
void resetWorkGroup() {
  WORKGROUP = 0;
  jsonWrite(configSetup, "workgroup", WORKGROUP);
  saveConfig();
#ifdef GENERAL_DEBUG
  LOG.println("\n- RESET WORKGROUP - ");
#endif
}

// ======================================
String getValueIP(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// ======================================
void redirectCommandToGroup(String ip) {
  char Host[16];
  char outputBuffer[24];
  ip.toCharArray(Host, ip.length() + 1);
  sprintf_P(outputBuffer, PSTR("MULTI,%u,%u,%u,%u,%u"),
            ONflag,
            currentMode,
            modes[currentMode].Brightness,
            modes[currentMode].Speed,
            modes[currentMode].Scale);
  Udp.beginPacket(Host, localPort);
  Udp.write(outputBuffer);
  Udp.endPacket();
#ifdef GENERAL_DEBUG
  LOG.print ("• SEND CMD TO WORKGROUP | IP ");
  LOG.print (Host);
  LOG.print (" • ");
  LOG.println (outputBuffer);
#endif
}

// ======================================
void multipleLampControl() {
  int index = 0;
  String ip = "none";
  if ( connect) {
    if ( WORKGROUP > 0 ) {
      while (index < 8) {
        ip = getValueIP(LAMP_LIST, ',', index);
        if (ip != "") {
          redirectCommandToGroup(ip);
        } else {
          index = 8;
        }
        index++;
      }
    }
  }
}

// ======================================
void multipleLampParsing(char *inputBuffer) {
  uint8_t valid = 0, i = 0;
  while (inputBuffer[i])   {   //пакет должен иметь вид MULTI,%U,%U,%U,%U,%U соответственно ON/OFF,№эффекта,яркость,скорость,масштаб
    if (inputBuffer[i] == ',')  {
      valid++;  //Проверка на правильность пакета (по количеству запятых)
    }
    i++;
  }
  if (valid == 5)   {                                 //Если пакет правильный выделяем лексемы,разделённые запятыми, и присваиваем параметрам эффектов
    char *tmp = strtok (inputBuffer, ",");            //Первая лексема MULTI пропускается
    tmp = strtok (NULL, ",");
    // ?????
    eff_auto = 0;                                     // сброс управления циклом | чтобы управление шло только от активной лампы
    if (ONflag != atoi(tmp))   {
      ONflag = atoi( tmp);
      changePower();                                  // Активацмя состояния ON/OFF
    }
    tmp = strtok (NULL, ",");
    if (currentMode != atoi(tmp))   {
      if (atoi (tmp) < MODE_AMOUNT)   {
        currentMode = atoi (tmp);
        tmp = strtok (NULL, ",");
        modes[currentMode].Brightness = atoi (tmp);
        tmp = strtok (NULL, ",");
        modes[currentMode].Speed = atoi (tmp);
        tmp = strtok (NULL, ",");
        modes[currentMode].Scale = atoi (tmp);
        loadingFlag = true; // Перезапуск эффекта
        FastLED.setBrightness(modes[currentMode].Brightness); //Применение яркости
      } else {
        currentMode = MODE_AMOUNT - 3;  //Если полученный номер эффекта больше , чем количество эффектов в лампе,включаем последний "адекватный" эффект
        loadingFlag = true; // Перезапуск эффекта
        FastLED.setBrightness(modes[currentMode].Brightness); //Применение яркости
      }
    } else {
      tmp = strtok (NULL, ",");
      if (modes[currentMode].Brightness != atoi(tmp))   {
        modes[currentMode].Brightness = atoi (tmp);
        FastLED.setBrightness(modes[currentMode].Brightness); //Применение яркости
      }
      tmp = strtok (NULL, ",");
      if (modes[currentMode].Speed != atoi(tmp))   {
        modes[currentMode].Speed = atoi (tmp);
        loadingFlag = true; // Перезапуск эффекта
      }
      tmp = strtok (NULL, ",");
      if (modes[currentMode].Scale != atoi(tmp))   {
        modes[currentMode].Scale = atoi (tmp);
        loadingFlag = true; // Перезапуск эффекта
      }
    }
    setFPS();
#ifdef GENERAL_DEBUG
    String pwr = ((ONflag == 1) ? " • " : "off");
    LOG.println ("\n  ====== INBOUND WORKGROUP ===== |  SET  |");
    LOG.println   (" | POWER | EFF | BRI | SPD | SCL | DELAY |");
    LOG.print (" |  " + pwr);
    LOG.printf_P(PSTR("  | %03d | %03d | %03d | %03d | %05d |\n"), currentMode, modes[currentMode].Brightness, modes[currentMode].Speed, modes[currentMode].Scale, FPSdelay);
#endif //GENERAL_DEBUG
  }
  inputBuffer[0] = '\0';
}

#endif //USE_MULTIPLE_LAMPS_CONTROL
