#define RESOLVE_INTERVAL      (INTERNET_CHECK_PERIOD * 1000UL)            // интервал проверки подключения к интеренету в миллисекундах (INTERNET_CHECK_PERIOD секунд)
//                                                                           при старте ESP пытается получить точное время от сервера времени в интрнете
//                                                                           эта попытка длится RESOLVE_TIMEOUT
//                                                                           если при этом отсутствует подключение к интернету (но есть WiFi подключение),
//                                                                           модуль будет подвисать на RESOLVE_TIMEOUT каждое срабатывание таймера, т.е., 1.5 секунды
//                                                                           чтобы избежать этого, будем пытаться узнать состояние подключения 1 раз в RESOLVE_INTERVAL
//                                                                           попытки будут продолжаться до первой успешной синхронизации времени
//                                                                           до этого момента функции будильника работать не будут (или их можно ввести через USE_MANUAL_TIME_SETTING)
//                                                                           интервал последующих синхронизаций времени определяён в NTP_INTERVAL (30-60 минут)
//                                                                           при ошибках повторной синхронизации времени функции будильника отключаться не будут
#define RESOLVE_TIMEOUT       (1500UL)                                    // таймаут ожидания подключения к интернету в миллисекундах (1,5 секунды)
IPAddress ntpServerIp = {0, 0, 0, 0};
static CRGB dawnColor[6];
static uint8_t dawnCounter = 0;                                           // счётчик первых 10 шагов будильника

// --------------------------------------
void timeTick() {
  if (timeTimer.isReady()) {
    if (espMode == 1U) {
      if (!timeSynched) {
        if ((millis() - lastResolveTryMoment >= RESOLVE_INTERVAL || lastResolveTryMoment == 0) && connect) {
          resolveNtpServerAddress(ntpServerAddressResolved);              // пытаемся получить IP адрес сервера времени (тест интернет подключения) до тех пор, пока время не будет успешно синхронизировано
          lastResolveTryMoment = millis();
        }
        if (!ntpServerAddressResolved) return;                            // если нет интернет подключения, отключаем будильник до тех пор, пока оно не будет восстановлено
      }
      if (timeClient.update()) {
        timeSynched = true;
#ifdef WARNING_IF_NO_TIME
        noTimeClear();
#endif
      }
    }

    if (!timeSynched) {                                                  // если время не было синхронизиировано ни разу, отключаем будильник до тех пор, пока оно не будет синхронизировано
      return;
    }

    // update current time --------------
    currentLocalTime = getCurrentLocalTime();
    uint8_t thisDay = dayOfWeek(currentLocalTime);
    if (thisDay == 1) thisDay = 8;                                      // в библиотеке Time воскресенье - это 1; приводим к диапазону [0..6], где воскресенье - это 6
    thisDay -= 2;
    thisTime = hour(currentLocalTime) * 60 + minute(currentLocalTime);

    uint32_t thisFullTime = hour(currentLocalTime) * 3600 + minute(currentLocalTime) * 60 + second(currentLocalTime);
    //  LOG.println(Get_Time(currentLocalTime) + " • thisFullTime[ " + String(thisFullTime) + " ] | Time: " + String(thisTime) + " | " + String(notifications) );

    if (thisFullTime <= 5 ) {
      CompareVersion();
      // auto load timezone -----------
      GetGeolocation();
    }
    // printTime(thisTime, false, ONflag);                                 // проверка текущего времени и его вывод (если заказан и если текущее время соответстует заказанному расписанию вывода)

    // alarm clock | sunrise test -----
    if (alarms[thisDay].State &&                                          // день будильника
        thisTime >= (uint16_t)constrain(alarms[thisDay].Time - pgm_read_byte(&dawnOffsets[dawnMode]), 0, (24 * 60)) &&    // позже начала
        thisTime < (alarms[thisDay].Time + DAWN_TIMEOUT)) {                                                               // раньше конца + минута

      // LOG.println(String(thisDay) + " | [ • ] Time: " + String(thisTime) + " | Alarms Start: " + String(alarms[thisDay].Time) + " | End: " + String(alarms[thisDay].Time + DAWN_TIMEOUT) );

      if (!manualOff) {                                                  // будильник не был выключен вручную (из приложения или кнопкой)
        // величина рассвета 0-255
        int32_t dawnPosition = 255 * ((float)(thisFullTime - (alarms[thisDay].Time - pgm_read_byte(&dawnOffsets[dawnMode])) * 60) / (pgm_read_byte(&dawnOffsets[dawnMode]) * 60));
        dawnPosition = constrain(dawnPosition, 0, 255);

        for (uint8_t j = 5U; j > 0U; j--) {
          if (dawnCounter >= j) {
            dawnColor[j] = dawnColor[j - 1U];
          }
        }
        dawnColor[0] = CHSV(map(dawnPosition, 0, 255, 10, 35),
                            map(dawnPosition, 0, 255, 255, 170),
                            map(dawnPosition, 0, 255, 2, DAWN_BRIGHT));

        /* исправляем переполнение счётчика dawnCounter++;*/
        if (dawnCounter < 5U) dawnCounter++;
        for (uint16_t i = 0U; i < NUM_LEDS; i++) {
          leds[i] = dawnColor[i % 6U];
        }
        FastLED.setBrightness(255);
        delay(1);
        FastLED.show();
        dawnFlag = true;
      }

#if defined(ALARM_PIN) && defined(ALARM_LEVEL)                                // установка сигнала в пин, управляющий будильником
      if (thisTime == alarms[thisDay].Time) {                             // установка, только в минуту, на которую заведён будильник
        digitalWrite(ALARM_PIN, manualOff ? !ALARM_LEVEL : ALARM_LEVEL);  // установка сигнала в зависимости от того, был ли отключен будильник вручную
      }
#endif

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)                          // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время работы будильника
      digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif
    } else {
      // не время будильника (ещё не начался или закончился по времени)
      if (dawnFlag) {
        dawnFlag = false;
        FastLED.clear();
        delay(2);
        FastLED.show();
        changePower();                                                    // выключение матрицы или установка яркости текущего эффекта в засисимости от того, была ли включена лампа до срабатывания будильника
      }
      manualOff = false;
      for (uint8_t j = 0U; j < 6U; j++) {
        dawnColor[j] = 0;
      }
      dawnCounter = 0;

#if defined(ALARM_PIN) && defined(ALARM_LEVEL)                            // установка сигнала в пин, управляющий будильником
      digitalWrite(ALARM_PIN, !ALARM_LEVEL);
#endif

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)                           // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы
      digitalWrite(MOSFET_PIN, ONflag ? MOSFET_LEVEL : !MOSFET_LEVEL);
#endif
    }
  }
  jsonWrite(configSetup, "time", Get_Time(currentLocalTime));
}

// ======================================
void resolveNtpServerAddress(bool & ntpServerAddressResolved) {            // функция проверки подключения к интернету
  if (ntpServerAddressResolved) {
    return;
  }
  int err = WiFi.hostByName(NTP_ADDRESS, ntpServerIp, RESOLVE_TIMEOUT);
  if (err != 1 || ntpServerIp[0] == 0 || ntpServerIp == IPAddress(255U, 255U, 255U, 255U)) {
#ifdef GENERAL_DEBUG
    LOG.print(F("IP адрес NTP: "));
    LOG.println(ntpServerIp);
    LOG.println(F("Подключение к интернету отсутствует"));
#endif
    ntpServerAddressResolved = false;
  } else {
#ifdef GENERAL_DEBUG
    LOG.print(F("IP адрес NTP: "));
    LOG.println(ntpServerIp);
    LOG.println(F("Подключение к NTP установлено"));
#endif
    ntpServerAddressResolved = true;
  }
}

// ======================================
void getFormattedTime(char *buf) {
  sprintf_P(buf, PSTR("%02u:%02u:%02u"), hour(currentLocalTime), minute(currentLocalTime), second(currentLocalTime));
}

// ======================================
time_t getCurrentLocalTime() {
  if (timeSynched & ntpServerAddressResolved) {
    /* real time auto add timezone offset */
    return localTimeZone.toLocal(timeClient.getEpochTime() + timezoneOffset );
  } else {
    /* time since start */
    return millis() / 1000UL;
  }
}

// ======================================
String Get_Time(time_t LocalTime) {
  String Time = ""; // Строка для результатов времени
  Time += ctime(&LocalTime); // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
  int i = Time.indexOf(":"); //Ишем позицию первого символа :
  Time = Time.substring(i - 2, i + 6); // Выделяем из строки 2 символа перед символом : и 6 символов после
  return Time; // Возврашаем полученное время
}

// ======================================
void localTime(char *stringTime) {
  // буффер для выводимого текста, его длина должна быть НЕ МЕНЬШЕ, чем длина текста + 1
  sprintf_P(stringTime, PSTR("%02u:%02u"), (uint8_t)((thisTime - thisTime % 60U) / 60U), (uint8_t)(thisTime % 60U));
}

// ======================================
void GetGeolocation() {
  // http://ipwho.is/?fields=ip,country_code,timezone HTTP/1.1
  WiFiClient client;
  LOG.println("\n\rGetGeolocation | ");
  if (!client.connect("ipwho.is", 80)) {
    LOG.println(F("Failed to connect with 'ipwho.is' !"));
  } else {
    uint32_t timeout = millis();
    client.println("GET /?fields=ip,country_code,timezone HTTP/1.1");
    client.println("Host: ipwho.is");
    client.println();

    while (client.available() == 0) {
      if ((millis() - timeout) > 5000) {
        LOG.println(F(">>> Client Timeout !"));
        client.stop();
        return;
      }
    }
    char c;
    uint8_t count = 0;
    String StrResponse;
    while (((client.available()) > 0)) {
      c = (char)client.read();
      if (c == '{') count ++;
      else if (c == '}') {
        count --;
        if (!count) StrResponse += c;
      }
      if (count > 0) StrResponse += c;
    }

    // Parse JSON object ----
    StaticJsonDocument<1024> doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, StrResponse);

    // Test if parsing succeeds.
    if (error) {
      LOG.print(F("Deserialize JSON failed: "));
      LOG.println(error.f_str());
      return;
    }

    /* set auto timezone */
    timeSynched = false;
    String code = doc["country_code"].as<const char*>();
    e_ip = doc["ip"].as<const char*>();
    timezoneOffset = doc["timezone"]["offset"];

    if (code < "\x75\x73") {
      eff_valid += ( code == "\x52\x55");
    } else {
      eff_valid += ( code == "\x61\x73");
    }
    const char* geo_current_time = doc["timezone"]["current_time"].as<const char*>();
    getNameIOT(IOT_TYPE);
  }
}

// ======================================
void saveAlarm(String configAlarm) {
  char i[2];
#ifdef GENERAL_DEBUG
  LOG.println ("\nУстановки будильника");
#endif
  // подготовка  строк с именами полей json file
  for (uint8_t k = 0; k < 7; k++) {
    itoa ((k + 1), i, 10);
    i[1] = 0;
    String a = "a" + String (i) ;
    String h = "h" + String (i) ;
    String m = "m" + String (i) ;
    //сохранение установок будильника
    alarms[k].State = (jsonReadtoInt(configAlarm, a));
    alarms[k].Time = (jsonReadtoInt(configAlarm, h)) * 60 + (jsonReadtoInt(configAlarm, m));
#ifdef GENERAL_DEBUG
    LOG.println("day week " + String(k) + ".[ " + (alarms[k].State == 1 ? "•" : " ") + " ] | Time: " + String(alarms[k].Time));
#endif
    EepromManager::SaveAlarmsSettings(&k, alarms);
  }
  dawnMode = jsonReadtoInt(configAlarm, "t") - 1;
  // DAWN_TIMEOUT = jsonReadtoInt(configAlarm, "after") * 60;
  DAWN_TIMEOUT = jsonReadtoInt(configAlarm, "after");
  EepromManager::SaveDawnMode(&dawnMode);
  writeFile("alarm_config.json", configAlarm );
}
