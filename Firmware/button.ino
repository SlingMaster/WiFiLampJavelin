
// ---------------------------------------
#define WARNING_BRIGHTNESS    (10U)                         // яркость вспышки
void showWarning(
  // мигающий цвет
  // используется для отображения краткосрочного предупреждения; блокирующий код!)
  CRGB color,                                               /* цвет вспышки                                                 */
  uint32_t duration,                                        /* продолжительность отображения предупреждения (общее время)   */
  uint16_t blinkHalfPeriod)                                 /* продолжительность одной вспышки в миллисекундах (полупериод) */
{
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время вывода текста
  digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif
  uint32_t blinkTimer = millis();
  enum BlinkState { OFF = 0, ON = 1 } blinkState = BlinkState::OFF;
  FastLED.setBrightness(WARNING_BRIGHTNESS);                // установка яркости для предупреждения
  FastLED.clear();
  FastLED.delay(2);
  fillAll(color);
  uint32_t startTime = millis();
  while (millis() - startTime <= (duration + 5)) {          // блокировка дальнейшего выполнения циклом на время отображения предупреждения
    if (millis() - blinkTimer >= blinkHalfPeriod) {          // переключение вспышка/темнота
      blinkTimer = millis();
      blinkState = (BlinkState)!blinkState;
      FastLED.setBrightness(blinkState == BlinkState::OFF ? 0 : WARNING_BRIGHTNESS);
      FastLED.delay(1);
    }
    delay(50);
  }

  FastLED.clear();
  FastLED.setBrightness(ONflag ? modes[currentMode].Brightness : 0);  // установка яркости, которая была выставлена до вызова предупреждения
  FastLED.delay(1);

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы или будильника
  digitalWrite(MOSFET_PIN, ONflag || (dawnFlag && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
#endif
  loadingFlag = true;                                       // принудительное отображение текущего эффекта (того, что был активен перед предупреждением)
}

// ---------------------------------------
void runEffect() {
  if (gb) {
    global_br = jsonReadtoInt(configSetup, "global_br");
    FastLED.setBrightness(map(modes[currentMode].Brightness + global_br, 1, 511, 1U, 250U));
  } else {
    FastLED.setBrightness(modes[currentMode].Brightness);
  }
  updateSets();
  if (random_on && FavoritesManager::FavoritesRunning) {
    selectedSettings = 1U;
  }
#if (USE_MQTT)
  if (espMode == 1U) MqttManager::needToPublish = true;
#endif


#ifdef USE_MULTIPLE_LAMPS_CONTROL
  multipleLampControl();
#endif  //USE_MULTIPLE_LAMPS_CONTROL 
  lendLease = false;
}

// ---------------------------------------
void prevEffect() {
  if (Favorit_only) {
    uint8_t lastMode = currentMode;
    do {
      if (++currentMode >= MODE_AMOUNT) currentMode = 0U;
    } while (FavoritesManager::FavoriteModes[currentMode] == 0U && currentMode != lastMode);
    if (currentMode == lastMode)                        // если ни один режим не добавлен в избранное, всё равно куда-нибудь переключимся
      if (++currentMode >= MODE_AMOUNT) currentMode = 0U;
  } else {
    if (++currentMode >= MODE_AMOUNT) currentMode = 0U;
  }
  runEffect();
}

// ---------------------------------------
void nextEffect() {
  if (Favorit_only) {
    uint8_t lastMode = currentMode;
    do {
      if (--currentMode >= MODE_AMOUNT) currentMode = MODE_AMOUNT - 1;
    } while (FavoritesManager::FavoriteModes[currentMode] == 0 && currentMode != lastMode);
    if (currentMode == lastMode)                      // если ни один режим не добавлен в избранное, всё равно куда-нибудь переключимся
      if (--currentMode >= MODE_AMOUNT) currentMode = MODE_AMOUNT - 1;
  } else {
    if (--currentMode >= MODE_AMOUNT) currentMode = MODE_AMOUNT - 1;
  }
  runEffect();
}

// ---------------------------------------
void cycleEffect() {
  // FavoritesManager::FavoritesRunning = (FavoritesManager::FavoritesRunning == 1) ? 0 : 1;
  eff_auto = (eff_auto == 1) ? 0 : 1;
  if (eff_auto) {
    FavoritesManager::FavoritesRunning = 0;
  }
  warnDinamicColor(eff_auto);
}

// ---------------------------------------
void autoSwapEff() {
  //  if (ONflag && eff_auto && (FavoritesManager::FavoritesRunning == 0)) {
  if (ONflag && eff_auto) {
    if (eff_rnd) {
      currentMode = random8(EFF_MATRIX + 1);
      runEffect();
    } else {
      if (currentMode > (EFF_MATRIX)) {
        currentMode = 0;
      }
      prevEffect();
    }
#ifdef GENERAL_DEBUG
    LOG.println(LAMP_NAME);
    LOG.printf_P(PSTR("Auto Swap Effects %02d |\n"), currentMode);
#endif
  }
}

// ---------------------------------------
void changeBrightness(bool Direction) {
  uint8_t delta = modes[currentMode].Brightness < 10U // определение шага изменения яркости: при яркости [1..10] шаг = 1, при [11..16] шаг = 3, при [17..255] шаг = 15
                  ? 1U
                  : 5U;
  if (gb) {
    global_br =  constrain(Direction
                           ? global_br + delta
                           : global_br - delta,
                           1, 127);
    uint16_t tempBri =  modes[currentMode].Brightness + global_br;
    if (tempBri > 255) {
      tempBri = 255;
    }
    FastLED.setBrightness(tempBri);
  } else {
    modes[currentMode].Brightness =
      constrain(Direction
                ? modes[currentMode].Brightness + delta
                : modes[currentMode].Brightness - delta,
                1, 255);
    FastLED.setBrightness(modes[currentMode].Brightness);
  }

#ifdef USE_MULTIPLE_LAMPS_CONTROL
  multipleLampControl();
#endif  //USE_MULTIPLE_LAMPS_CONTROL

#ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR("Новое значение яркости: %d\n\r"), modes[currentMode].Brightness);
#endif
}

// ---------------------------------------
void runOTA() {
#ifdef OTA
  if (otaManager.RequestOtaUpdate()) {
    ONflag = true;
    currentMode = EFF_MATRIX;                             // принудительное включение режима "Матрица" для индикации перехода в режим обновления по воздуху
    changePower();
#ifdef  JAVELIN
    digitalWrite(OTA_PIN, LOW);
    leds[NUM_LEDS + ROUND_MATRIX] = CHSV{96U, 255U, 255U};
    leds[NUM_LEDS + ROUND_MATRIX + 2] = CHSV{96U, 255U, 255U};
#ifdef BACKLIGHT_PIN
    digitalWrite(BACKLIGHT_PIN, LOW);
#endif
#endif
  }
#endif
}

// ---------------------------------------
void showIP() {
  if (espMode == 1U) {
    loadingFlag = true;

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время вывода текста
    digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif
    while (!fillString(WiFi.localIP().toString().c_str(), CRGB::White, false)) {
      delay(1);
      ESP.wdtFeed();
    }

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы или будильника
    digitalWrite(MOSFET_PIN, ONflag || (dawnFlag && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
#endif
    loadingFlag = true;
  }
}


// ---------------------------------------

void smartLampOff(uint8_t timeout ) {
#ifdef BUTTON_CAN_SET_SLEEP_TIMER
  showWarning(CRGB::Red, 1000, 250U);                        // мигание синим цветом 1 секунду подтверждение включения
  ONflag = true;
  changePower();
  settChanged = true;
  eepromTimeout = millis();


  TimerManager::TimeToFire = millis() + timeout * 60UL * 1000UL;
  TimerManager::TimerRunning = true;
#endif //BUTTON_CAN_SET_SLEEP_TIMER
}

// =====================================
#ifdef ESP_USE_BUTTON
bool brightDirection;
static bool startButtonHolding = false;                     // флаг: кнопка удерживается для изменения яркости/скорости/масштаба лампы кнопкой
static bool Button_Holding = false;
// --------------------------------------
void buttonTick() {
  if (!buttonEnabled) {                                     // события кнопки не обрабатываются, если она заблокирована
    return;
  }

  touch.tick();
  uint8_t clickCount = touch.hasClicks() ? touch.getClicks() : 0U;
  if (clickCount >= 1U) {
    LOG.printf_P(PSTR("Click : %d | Speed : %d | Scale : %d \n\r"), clickCount, modes[currentMode].Speed, modes[currentMode].Scale);
  }
  // однократное нажатие =======
  if (clickCount == 1U) {
    if (dawnFlag) {
      manualOff = true;
      dawnFlag = false;
      FastLED.setBrightness(modes[currentMode].Brightness);
      changePower();
    } else {
      ONflag = !ONflag;
      changePower();
    }
    updateSets();

#if (USE_MQTT)
    if (espMode == 1U) {
      MqttManager::needToPublish = true;
    }
#endif

#ifdef USE_MULTIPLE_LAMPS_CONTROL
    multipleLampControl();
#endif  //USE_MULTIPLE_LAMPS_CONTROL
  }

  // двухкратное нажатие =======
  if (ONflag && clickCount == 2U) {
    prevEffect();
  }

  // трёхкратное нажатие =======
  if (ONflag && clickCount == 3U) {
    nextEffect();
  }

  // четырёхкратное нажатие =======
  if (clickCount == 4U) {
    //                                                        // нa выбор -------------
    // runOTA();                                              // редко используемый режим проще и удобней включить из приложения заменен на любимый эффект
    cycleEffect();                                            // или включение показа эффектов в цикле
  }

  //  пятикратное нажатие =======
  //  включить эффект огонь
  if (clickCount == 5U) {                                     // нa выбор -------------
    // showIP();                                              // вывод IP на лампу
    currentMode = EFF_MATRIX + 1; runEffect();                // включить эффект огонь

  }

  // шестикратное нажатие =======
  if (clickCount == 6U) {                                     // нa выбор -------------
    // printTime(thisTime, true, ONflag);                     // вывод текущего времени бегущей строкой
    // currentMode = EFF_FAV; runEffect();                    // или любимый эффект
    smartLampOff(BUTTON_SET_SLEEP_TIMER1);                    // включение дампы на 5 минут (не зависимо от того включена она или выключена)
  }

  // семикратное нажатие =======
  if (clickCount == 7U) {                                     // смена рабочего режима лампы: с WiFi точки доступа на WiFi клиент или наоборот

#ifdef RESET_WIFI_ON_ESP_MODE_CHANGE
    if (espMode) wifiManager.resetSettings();                 // сброс сохранённых SSID и пароля (сброс настроек подключения к роутеру)
#endif
    espMode = (espMode == 0U) ? 1U : 0U;
    jsonWrite(configSetup, "ESP_mode", (int)espMode);
    saveConfig();

#ifdef GENERAL_DEBUG
    LOG.printf_P(PSTR("Рабочий режим лампы изменён и сохранён в энергонезависимую память\n\rНовый рабочий режим: ESP_MODE = %d, %s\n\rРестарт...\n\r"),
                 espMode, espMode == 0U ? F("WiFi точка доступа") : F("WiFi клиент (подключение к роутеру)"));
    delay(1000);
#endif

    showWarning(CRGB::Red, 3000U, 500U);                      // мигание красным цветом 3 секунды - смена рабочего режима лампы, перезагрузка
    ESP.restart();
  }
#ifdef GENERAL_DEBUG
  if (clickCount > 0U) {
    LOG.printf_P(PSTR("Button Click Count: %d | %d ms\n\r"), clickCount, BUTTON_SET_DEBOUNCE);
  }
#endif

  // кнопка только начала удерживаться
  // if (ONflag && touch.isHolded())
  if (touch.isHolded()) {                                     // пускай для выключенной лампы удержание кнопки включает белую лампу
    brightDirection = !brightDirection;
    startButtonHolding = true;
  }

  // кнопка нажата и удерживается
  //  if (ONflag && touch.isStep())
  if (touch.isStep()) {
    if (ONflag && !Button_Holding) {
      int8_t but = touch.getHoldClicks();

      switch (but ) {
        case 0U: {                                               // просто удержание (до удержания кнопки кликов не было) - изменение яркости
            changeBrightness(brightDirection);
#ifdef PROPERTIES_LEVEL_INDICATOR
            properties_level = 1;
#endif
            break;
          }

        case 1U: {                                               // удержание после одного клика - изменение скорости
            modes[currentMode].Speed = constrain(brightDirection ? modes[currentMode].Speed + 1 : modes[currentMode].Speed - 1, 1, 255);
#ifdef PROPERTIES_LEVEL_INDICATOR
            properties_level = 2;
#endif

#ifdef USE_MULTIPLE_LAMPS_CONTROL
            multipleLampControl();
#endif  //USE_MULTIPLE_LAMPS_CONTROL
#ifdef GENERAL_DEBUG
            LOG.printf_P(PSTR("Новое значение скорости: %d\n\r"), modes[currentMode].Speed);
#endif
            updateSets();
            break;
          }

        case 2U: {                                             // удержание после двух кликов - изменение масштаба
            modes[currentMode].Scale = constrain(brightDirection ? modes[currentMode].Scale + 1 : modes[currentMode].Scale - 1, 1, 100);
#ifdef PROPERTIES_LEVEL_INDICATOR
            properties_level = 3;
#endif

#ifdef USE_MULTIPLE_LAMPS_CONTROL
            multipleLampControl();
#endif  //USE_MULTIPLE_LAMPS_CONTROL

#ifdef GENERAL_DEBUG
            LOG.printf_P(PSTR("Новое значение масштаба: %d\n\r"), modes[currentMode].Scale);
#endif
            updateSets();
            break;
          }

        default:
          break;
      }

      settChanged = true;
      eepromTimeout = millis();
    } else {
      if (!Button_Holding) {
        Button_Holding = true;
        currentMode = EFF_WHITE_COLOR;
        ONflag = true;
        changePower();
        settChanged = true;
        eepromTimeout = millis();
      }
    }
  }
  // кнопка отпущена после удерживания
  if (ONflag && !touch.isHold() && startButtonHolding) {     // кнопка отпущена после удерживания, нужно отправить MQTT сообщение об изменении яркости лампы
    properties_level = 0;
    startButtonHolding = false;
    Button_Holding = false;
    loadingFlag = true;

#if (USE_MQTT)
    if (espMode == 1U) MqttManager::needToPublish = true;
#endif
  }
}
#endif
