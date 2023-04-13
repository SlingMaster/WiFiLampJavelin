/* effectTicker.ino

  ====================================================================================================================================================
  основная причина изменения этого файла и перехода на 3 версию прошивки это изменение принципа вызова эффектов,
  чтобы существенно сократить код и максимально исключить ручной труд переложив эти функции на процессор
  массив эффектов представляет собой указатели на функции еффектов, таким способом не нужно присваивать эффектам номер,
  чтобы потом к ним обращаться в масиве они нумеруются автоматом
  [ !!! ] есть несколько ограничений :
  1. никогда не трогайте в списке эффекты «Бeлый cвeт», «Maтpицa», «Oгoнь» у них фиксированные места
    дальше можете раставлять эффекты на свое усмотрение до бегущей строки она всегда должна быть последней
  2. нужно соблюдать такую же последовательность в константе defaultSettings[][3] файл Constants.h
   и файлах effects1.json, effects2.json, effects3.json, effects4.json файлы предусматриваю поддержку 30 эффектов из списка общим числом 120
   формат записи:
   поля | n         | v
   "Название эффекта,min_скорость,max_скорость,min_масштаб,max_масштаб,выбор_ли_цвета_это(0-нет,1-да 2-совмещённый);"
   проще добавлять новые эффекты в файл effects4.json там есть свободное место это упростит добавление новых эффектов
   главное соблюдать последовательность во весех файлах поэтому чтобы не путаться должно обязательно указываться название эффекта
   теперь общее количество эффектов MODE_AMOUNT высчисляется автоматически на основе defaultSettings

   • для поклонников программы FireLamp нужно будет распределить содержимое четырех файлов равномерно в effects1.json, effects2.json, effects3.json
   список достигает предела и нужно эксперементировать пройдет весь список или обрежется сейчас загружаются только эффекты из 3 первых файлов
   «Акварель» последний на текущий момент котейка программу не обновляет, а это мешает двигаться дальше,
   темболее что в архиве есть альтернатива для всех платформ
  ====================================================================================================================================================
*/

uint32_t effTimer;

// -------------------------------------
// массив указателей на функции эффектов
// -------------------------------------
void (*FuncEff[MODE_AMOUNT])(void) = {
  /* 1.............................................. */
  colorsRoutine2,    				  // Cмeнa цвeтa
  madnessNoiseRoutine,			  // Бeзyмиe
  cloudsNoiseRoutine,  			  // Oблaкa
  lavaNoiseRoutine, 				  // Лaвa
  plasmaNoiseRoutine,  			  // Плaзмa
  rainbowNoiseRoutine,  			// Paдyгa 3D
  rainbowStripeNoiseRoutine,  // Пaвлин
  zebraNoiseRoutine,				  // 3eбpa
  forestNoiseRoutine,				  // Лec
  oceanNoiseRoutine,          // Океан
  BBallsRoutine,              // Mячики
  bounceRoutine,              // Mячики бeз гpaниц
  popcornRoutine,             // Пoпкopн
  spiroRoutine,               // Cпиpaли
  PrismataRoutine,            // Пpизмaтa
  smokeballsRoutine,          // Дымoвыe шaшки
  execStringsFlame,           // Плaмя
  Fire2021Routine,            // Oгoнь 2021
  pacificRoutine,             // Tиxий oкeaн
  shadowsRoutine,             // Teни
  DNARoutine,                 // ДHK
  flock,                      // Cтaя
  flockAndPredator,           // Cтaя и xищник
  butterflys,                 // Moтыльки
  lampWithButterflys,         // Лaмпa c мoтылькaми
  snakesRoutine,              // 3мeйки
  nexusRoutine,               // Nexus
  spheresRoutine,             // Шapы
  Sinusoid3Routine,           // Cинycoид
  MetaBallsRoutine,           // Meтaбoлз
  /* 2.............................................. */
  polarRoutine,               // Ceвepнoe cияниe
  spiderRoutine,              // Плaзмeннaя лaмпa
  LavaLampRoutine,            // Лaвoвaя лaмпa
  LiquidLamp,                 // Жидкaя лaмпa
  LiquidLampAuto,             // Жидкaя лaмпa (auto)
  newMatrixRoutine,           // Kaпли нa cтeклe
  StrobeAndDiffusion,         // Строб.Хаос.Дифузия
  fire2012again,              // Oгoнь 2012
  Fire2018_2,                 // Oгoнь 2018
  fire2020Routine2,           // Oгoнь 2020
  whirl,                      // Bиxpи плaмeни
  whirlColor,                 // Paзнoцвeтныe виxpи
  magmaRoutine,               // Maгмa
  LLandRoutine,               // Kипeниe
  fire2012WithPalette,        // Boдoпaд
  fire2012WithPalette4in1,    // Boдoпaд 4 в 1
  poolRoutine,                // Бacceйн
  pulse2,                     // Пyльc
  pulse4,                     // Paдyжный пyльc
  pulse8,                     // Бeлый пyльc
  oscillatingRoutine,         // Ocциллятop
  starfield2Routine,          // Иcтoчник
  fairyRoutine,               // Фeя
  RainbowCometRoutine,        // Koмeтa
  ColorCometRoutine,          // Oднoцвeтнaя кoмeтa
  MultipleStream,             // Двe кoмeты
  MultipleStream2,            // Тpи кoмeты
  attractRoutine,             // Пpитяжeниe
  MultipleStream3,            // Пapящий oгoнь
  MultipleStream5,            // Bepxoвoй oгoнь
  /* 3.............................................. */
  MultipleStream8,            // Paдyжный змeй
  sparklesRoutine,            // Koнфeтти
  twinklesRoutine,            // Mepцaниe
  Smoke,                      // Дым
  SmokeColor,                 // Paзнoцвeтный дым
  picassoSelector,            // Пикacco
  WaveRoutine,                // Boлны
  sandRoutine,                // Цвeтныe дpaжe
  ringsRoutine,               // Koдoвый зaмoк
  Mirage,                     // Міраж
  // cube2dRoutine,              // Kyбик Pyбикa
  simpleRain,                 // Tyчкa в бaнкe
  stormyRain,                 // Гроза в банке
  coloredRain,                // Ocaдки
  RainRoutine,                // Paзнoцвeтный дoждь
  Snowfall,                   // Cнeгoпaд
  stormRoutine2,              // 3вeздoпaд / Meтeль
  LeapersRoutine,             // Пpыгyны
  lightersRoutine,            // Cвeтлячки
  ballsRoutine,               // Cвeтлячки co шлeйфoм
  lumenjerRoutine,            // Люмeньep
  lightBallsRoutine,          // Пeйнтбoл
  rainbowRoutine,             // Paдyгa
  colorsWine,                 // Вино
  Swirl,                      // Завиток
  Ukraine,                    // Моя краïна Украïна
  Spermatozoa,                // Генофонд Українців
  OilPaints,                  // Масляные Краски
  Watercolor,                 // Акварель
  BotswanaRivers,             // Реки Ботсваны
  FeatherCandleRoutine,       // Свеча
  Hourglass,                  // Песочные Часы
  /* 4.............................................. */
  Contacts,                   // Kонтакти
  RadialWave,                 // Радіальна хвиля
  FireSparks,                 // Вогонь з іскрами
  Spectrum,                   // Spectrum
  LotusFlower,                // Цветок Лотоса
  ChristmasTree,              // Новогодняя Елка
  ByEffect,                   // Побочный Эффект
  Firework,                   // Фейерверк
  PlanetEarth,                // Планета Земля
  WebTools,                   // Мечта Дизайнера
  ColorFrizzles,              // Цветные Кудри
  DropInWater,                // Краплі на воді
  MagicLantern,               // Чарівний Ліхтар
  FlowerRuta,                 // Червона Рута
  HandFan,                    // Опахало
  PlasmaWaves,                // Плазмові Хвилі
  TixyLand,                   // Tixy Land
  EffectStars,                // Зірки
  LightFilter,                // Cвітлофільтр
  Bamboo,                     // Бамбук
  NewYearsCard,               // Новорічна листівка
  TasteHoney,                 // Смак Меду
  Tornado,                    // Райдужний Торнадо
  CreativeWatch,              // Креативний Годинник
  HeatNetworks,               // Теплові Мережі
  /* • самое удобное место для добавления нового эффекта • */
  //  ballRoutine,                // Блуждающий кубик
  /* • ------------------------------------------------- • */
  /* • ------------------------------------------------- •
     !!! последние пять эффектов имеют постоянную прописку
    никогда не перемещайте их по списку, остальные эффекты
    можно размещать в любой последовательности. */
  matrixRoutine,              // Maтpицa ......... (EFF_MATRIX)
  fireRoutine,                // Oгoнь ........... (EFF_MATRIX + 1)
  whiteColorStripeRoutine,    // Бeлый cвeт ...... (MODE_AMOUNT - 2)
  colorRoutine,               // Цвeт
#ifdef USE_TIME_EFFECT
  clockRoutine,               // Чacы
#endif
  text_running                // Бeгyщaя cтpoкa ........... (MODE_AMOUNT - 1)
};

// -------------------------------------
void effectsTick() {
  if (isJavelinMode()) {
    if (ONflag && (millis() - effTimer >= FPSdelay)) {
      effTimer = millis();
      if (extCtrl) {
        /* extend control by UDP protocol transfer data to led matrix */
        return;
      }
      if (lendLease) {
        /* javelin eff mode --------------- */
        Javelin();
      } else {
        /* standard start effects --------- */
        (*FuncEff[currentMode])();
      }
      /* state level indicator */
#ifdef JAVELIN
      StateLampIndicator();
#else
#ifdef PROPERTIES_LEVEL_INDICATOR
      stateIndicator();               // индикатор уровня текущщих значений яркости, скорости, масштаба (только в момент нажатия на сенсорную кнопку)
#endif
#endif


#ifdef WARNING_IF_NO_TIME_ON_EFFECTS_TOO
      if (!timeSynched) {
        noTimeWarning();
      } else {
        FastLED.clear();
      }
#endif
      FastLED.show();
    }


#ifdef WARNING_IF_NO_TIME
    else if (!timeSynched && !ONflag && !((uint8_t)millis())) {
      noTimeWarningShow();
    }
#endif
  }
}

//--------------------------------------
#ifdef PROPERTIES_LEVEL_INDICATOR
void stateIndicator() {

  // ================= • ========   bar color |    br   |  speed  |  scale  |
  static const uint32_t color[4] PROGMEM = { 0, 0x0500FF, 0x1FFF00, 0xFF007F};
  uint8_t val;
  uint8_t dir = HEIGHT;                     // для горизонтального бара WIDTH
  uint8_t posX = floor(WIDTH * 0.25);
  if (properties_level > 0) {
    switch (properties_level ) {
      case 1:
        val = floor(modes[currentMode].Brightness * dir / 255);
        break;
      case 2:
        val = floor(modes[currentMode].Speed * dir / 255);
        break;
      case 3:
        val = floor(modes[currentMode].Scale * dir / 100);
        break;
      default:
        val = 0;
        break;
    }
    //  LOG.printf_P(PSTR("Val %03d | Level %01d |\n"), val, properties_level);
    // vertical bar ----
    drawRec(1, 0, floor(WIDTH * 0.5), HEIGHT, 0x000000);
    DrawLine(posX, 0, posX, val, color[properties_level]);
    // horizontal bar --
    // drawRec(0, 0, WIDTH, 6, 0x000000);
    // DrawLine(0, 0, val, 0, color[properties_level]);
  }
}
#endif

// --------------------------------------
void changePower() {
  if (ONflag) {
    loadingFlag = true;
    effectsTick();
    for (uint8_t i = 0U; i < modes[currentMode].Brightness; i = constrain(i + 8, 0, modes[currentMode].Brightness)) {
      FastLED.setBrightness(i);
      FastLED.delay(1);
    }
    FastLED.setBrightness(modes[currentMode].Brightness);
    FastLED.delay(1);
  } else {
    effectsTick();
    for (uint8_t i = modes[currentMode].Brightness; i > 0; i = constrain(i - 8, 0, modes[currentMode].Brightness)) {
      FastLED.setBrightness(i);
      FastLED.delay(1);
    }
    FastLED.clear();
    FastLED.delay(2);

  }
#ifdef BACKLIGHT_PIN
  digitalWrite(BACKLIGHT_PIN, (ONflag ? HIGH : LOW));
#endif
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)         // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы
  digitalWrite(MOSFET_PIN, ONflag ? MOSFET_LEVEL : !MOSFET_LEVEL);
#endif

  TimerManager::TimerRunning = false;
  TimerManager::TimerHasFired = false;
  TimerManager::TimeToFire = 0ULL;
#ifdef AUTOMATIC_OFF_TIME
  if (ONflag) {
    TimerManager::TimerRunning = true;
    TimerManager::TimeToFire = millis() + AUTOMATIC_OFF_TIME;
  }
#endif

  if (FavoritesManager::UseSavedFavoritesRunning == 0U) {    // если выбрана опция Сохранять состояние (вкл/выкл) "избранного", то ни выключение модуля, ни выключение матрицы не сбрасывают текущее состояние (вкл/выкл) "избранного"
    FavoritesManager::TurnFavoritesOff();
  }

#if (USE_MQTT)
  if (espMode == 1U) {
    MqttManager::needToPublish = true;
  }
#endif
}

#ifdef WARNING_IF_NO_TIME

// --------------------------------------
void noTimeWarning() {
  // зеленая анимация режим работы с роутером
  // синяя анимация режим работы в режиме точки доступа
#ifdef JAVELIN
  javelinConnect(espMode ? 90U : 160U);
#else
  espModeState(espMode ? 112U : 176U);
#endif
}

// --------------------------------------
void noTimeWarningShow() {
  noTimeWarning();
  FastLED.setBrightness(WARNING_IF_NO_TIME);
  FastLED.delay(1);
}

// --------------------------------------
void noTimeClear() {
  if (!timeSynched) {
#ifdef JAVELIN
    DrawLevel(0, ROUND_MATRIX - 1, ROUND_MATRIX, CHSV {200, 255, 250});
#else
    FastLED.clear();
    FastLED.delay(2);
#endif
  }
}
#endif //WARNING_IF_NO_TIME
