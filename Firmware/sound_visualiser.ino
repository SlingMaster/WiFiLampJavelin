// sound_visualiser.ino
#ifdef USE_SOUND_VISUALISER
const byte BAR_DEF = 101U;
const byte BAR_SPECTR = 102U;
const byte BAR_SPECTR_V = 103U;
const byte BAR_4 = 104U;
const byte BAR_5 = 105U;

const byte SPECTR_DEF = 106U;
const byte SPECTR_SCROL = 107U;
const byte SPECTR_SCROL_V = 108U;
const byte SPECTR_4 = 109U;
const byte SPECTR_5 = 110U;

const byte ABSTRACT_1 = 111U;
const byte ABSTRACT_2 = 112U;
const byte ABSTRACT_3 = 113U;
const byte ABSTRACT_4 = 114U;
const byte ABSTRACT_DEV = 115U;
const byte ABSTRACT_5 = 117U;
// ======================================
String getIoTInfo() {
  // init sound data transfer -----------
  String json = "\"device\":{";
  // ip -----------
  json += "\"ip\":\"" + ipToString(WiFi.localIP()) + "\"";
  // name ---------
  json += ",\"name\":\"" + LAMP_NAME + "\"";
  json += ",\"w\":" +  String(WIDTH);
  json += ",\"h\":" + String(HEIGHT);
  json += "}";
  // ---------------------------
  return json;
}

// ======================================
void drawNote(CRGB color, bool isClear) {
  static const bool data[9][6] = {
    {0, 0, 0, 1, 0, 0 },
    {0, 0, 0, 1, 1, 0 },
    {0, 0, 0, 1, 0, 1 },
    {0, 0, 0, 1, 0, 1 },
    {0, 0, 0, 1, 0, 0 },
    {0, 0, 0, 1, 0, 0 },
    {0, 1, 1, 1, 0, 0 },
    {1, 1, 1, 1, 0, 0 },
    {0, 1, 1, 0, 0, 0 }
  };
  uint8_t posX = CENTER_X_MINOR - 2;
  uint8_t posY = CENTER_Y_MAJOR + 5;
  if (isClear) FastLED.clear();
  for (uint8_t y = 0U; y < 9U; y++) {
    for (uint8_t x = 0U; x < 6; x++) {
      drawPixelXY(posX + x, posY - y, (data[y][x]) ? color : CRGB(0, 0, 0));
    }
  }
  if (isClear) FastLED.show();
}

// ======================================
void SoundVisualiser(char *packetBuffer, int16_t packetSize, byte id_cmd) {

  // visualization type -----
  switch (id_cmd) {
    /* BARS */
    case BAR_DEF:
    case BAR_SPECTR:
    case BAR_SPECTR_V:
      visBars(id_cmd, packetBuffer);
      break;
    case BAR_4:
      visRings(id_cmd, packetBuffer);
      break;
    case BAR_5:
      ; visSymmetricWave(id_cmd, packetBuffer);
      break;

    /* SPECTR */
    case SPECTR_DEF:
    case SPECTR_SCROL:
      visSpectrum(id_cmd, packetBuffer);
      break;
    case SPECTR_SCROL_V:
      visSpectrumV(id_cmd, packetBuffer);
      break;
    case SPECTR_4:
      visSpectrumExplosion(id_cmd, packetBuffer);
      break;
    case SPECTR_5:
      visRadialBars(id_cmd, packetBuffer);
      break;

    /* ABSTRACT */
    case ABSTRACT_1:
      visHornyPeacock(id_cmd, packetBuffer);
      break;
    case ABSTRACT_2:
      visColoredBlots(id_cmd, packetBuffer);
      break;
    case ABSTRACT_3:
      visMagicEye(id_cmd, packetBuffer);
      break;
    case ABSTRACT_4:
      visCheerfulFire(id_cmd, packetBuffer);
      break;
    case ABSTRACT_5:
      visDragonBbreath(id_cmd, packetBuffer);
      break;
    case ABSTRACT_DEV:
      visTuningIndicator(id_cmd, packetBuffer);
      break;
    default:
      break;
  }
  FastLED.show();
  step++;
  // printf("ID_CMD • [ %3d ] %3d | %3d |\n", id_cmd, packetBuffer[4], packetBuffer[5]);
}

// ======================================
// •      Bars Wave Visualization       •
// ======================================
void visBars(byte id_cmd, char *packetBuffer) {
  const byte PEAK = HEIGHT * 0.8;
  const float STEP = 160 / PEAK;     //PEAK / 8;
  byte cur_val;
  byte peak_delay = (id_cmd == BAR_SPECTR) ? 4U : 3U;
  // FastLED.clear();
  // dimAll(72U);
  for (uint8_t x = 0U; x < WIDTH / 2; x++) {
    byte val = (HEIGHT / 100.0) * packetBuffer[(x + 6)];
    // 0 bar [1] peak --------
    if (val >= noise3d[1][x][0]) {
      noise3d[1][x][0] = val;
    } else {
      if ((noise3d[1][x][0] > 0) & (step % peak_delay == 0)) {
        noise3d[1][x][0]--;
      }
    }
    // [0] level 1 peak ------
    if (val >= noise3d[0][x][0] - 1) {
      noise3d[0][x][0] = val;
    } else {
      if (noise3d[0][x][0] > 0) {
        noise3d[0][x][0]--;
      }
    }

    // bar draw ----
    cur_val = noise3d[0][x][0];
    for (uint8_t y = 0U; y < HEIGHT; y++) {
      switch (id_cmd) {
        /* BARS */
        case BAR_DEF:
          hue = 148U + y;                         // color bar
          hue2 = (y > cur_val) ? 0 : 220 + y;     // brightness 110 + y * STEP
          break;
        case BAR_SPECTR:
          hue = x * 255 / (WIDTH / 2) + abs(HEIGHT - y * 2) + step;
          hue2 = (y > cur_val) ? 0 : 160 + (y / 2) * STEP;
          break;
        case BAR_SPECTR_V:
          hue = 80 + y * STEP;
          hue2 = (y > cur_val - 2) ? 0 : 240;
          break;
        default:
          break;
      }

      leds[XY(x * 2, y)] = CHSV(hue, 255U, hue2);
      leds[XY(x * 2 + 1, y)] = CHSV(hue, 255U, hue2);
    }
    // -------------

    // peak draw ---
    cur_val = noise3d[1][x][0];
    deltaHue = cur_val > PEAK ? 255U : 0U;
    leds[XY(x * 2, cur_val)] = CHSV(240U, deltaHue, 255U);
    leds[XY(x * 2 + 1, cur_val)] = CHSV(240U, deltaHue, 255U);
    // -------------
  }
}

/* ======================================
           Rings | Кільця
   ==================================== */
void visRings(byte id_cmd, char *packetBuffer) {
  const byte PEAK = HEIGHT * 0.8;
  const float STEP = 160 / PEAK;     //PEAK / 8;
  byte cur_val;
  byte peak_delay = (id_cmd == BAR_SPECTR) ? 4U : 3U;
  fadeToBlackBy(leds, NUM_LEDS, 20);
  for (uint8_t x = 0U; x < WIDTH / 2; x++) {
    byte val = (HEIGHT / 100.0) * packetBuffer[(x + 6)]; // * 1.25;

    // 0 bar [1] peak --------
    if (val >= noise3d[1][x][0]) {
      noise3d[1][x][0] = val;
    } else {
      if ((noise3d[1][x][0] > 0) & (step % peak_delay == 0)) {
        noise3d[1][x][0]--;
      }
    }

    // [0] level 1 peak ------
    if (val >= noise3d[0][x][0] - 1) {
      noise3d[0][x][0] = val;
    } else {
      if (noise3d[0][x][0] > 0) {
        noise3d[0][x][0]--;
      }
    }

    // bar draw ----
    cur_val = noise3d[0][x][0];
    // -------------

    // peak draw ---
    cur_val = noise3d[1][x][0];
    deltaHue = cur_val > PEAK ? 255U : 255 - cur_val * 16U;
    hue = cur_val >= PEAK ? 240U : x * 32U;
    hue2 = cur_val >= PEAK ? 255U : 64 + cur_val * 8;
    CRGB color = CHSV(hue, deltaHue, hue2);
    DrawLine(0, cur_val + 1, WIDTH, cur_val + 1, CRGB::Black);
    DrawLine(0, cur_val, WIDTH, cur_val, color);
    // -------------
  }
}

/* ======================================
     Symmetric Wave | Симетрична хвиля
   ==================================== */
void visSymmetricWave(byte id_cmd, char *packetBuffer) {
  const uint8_t STEP = 256 / HEIGHT;
  fadeToBlackBy(leds, NUM_LEDS, 25);
  for (byte x = 0; x < WIDTH; x++) {
    byte val = (HEIGHT / 100.0) * packetBuffer[(x + 6)];
    if ( val < 3) {
      val = val * 1.75;
    }
    hue = 64 + val * STEP * 0.8;
    for (uint8_t y = 0; y < val; y++) {
      drawPixelXY(x, y, CHSV(hue, 255, 255 - y * STEP));
      drawPixelXY(x, HEIGHT - y, CHSV(hue, 255, 255 - y * STEP));
    }
  }
}

// ======================================
// •    Spectrum Wave Visualization     •
// ======================================
void visSpectrum(byte id_cmd, char *packetBuffer) {
  dimAll(72U);
  if (id_cmd ==  SPECTR_SCROL) {
    deltaHue += 8;
  } else if (id_cmd ==  SPECTR_DEF) {
    deltaHue = 0U;
  }

  for (uint8_t x = 0U; x < WIDTH * 2; x++) {
    byte posX =  x / 2;
    leds[XY(x / 2, HEIGHT - 2)] = CHSV(0U, 0U, (posX % 2U) ? 96U : 0);
    leds[XY(x / 2, 1)] = CHSV(0U, 0U, (posX % 2U) ? 96U : 0);
    //float val = ((HEIGHT / 200.05) * packetBuffer[(x / 2 + 6)]) * 1.25;
    float val = ((HEIGHT / 200.05) * packetBuffer[(x / 2 + 6)]);
    /* color    */ hue = x * 255 / (WIDTH * 2) + deltaHue;
    /* saturate */ deltaHue2 = (val < 1.5) ? (220 + val * 2) : 255U;
    DrawLineF(x / 2.0, CENTER_Y_MINOR - val, x / 2.0, CENTER_Y_MINOR + val, CHSV(hue, deltaHue2, 255U));
  }
}


// ======================================
void visSpectrum2(byte id_cmd, char *packetBuffer) {
  dimAll(72U);
  if (id_cmd ==  SPECTR_SCROL) {
    deltaHue += 8;
  } else if (id_cmd ==  SPECTR_DEF) {
    deltaHue = 0U;
  }

  for (uint8_t x = 0U; x < WIDTH * 2; x++) {
    byte posX =  x / 2;
    leds[XY(x / 2, HEIGHT - 2)] = CHSV(0U, 0U, (posX % 2U) ? 96U : 0);
    leds[XY(x / 2, 1)] = CHSV(0U, 0U, (posX % 2U) ? 96U : 0);
    float val = ((HEIGHT / 200.05) * packetBuffer[(x / 2 + 6)]); // * 1.25;
    hue = x * 255 / (WIDTH * 2) + deltaHue;
    deltaHue2 = (val < 1.5) ? (220 + val * 2) : 255U;

    // Измененная часть кода
    if (posX % 4U == 0) {
      DrawLineF(x / 2.0, CENTER_Y_MINOR - val, x / 2.0, CENTER_Y_MINOR + val, CHSV(hue, deltaHue2, 255U));
    } else if (posX % 4U == 1) {
      DrawLineF(x / 2.0, CENTER_Y_MINOR + val, x / 2.0, CENTER_Y_MAJOR - val, CHSV(hue, deltaHue2, 255U));
    } else if (posX % 4U == 2) {
      DrawLineF(x / 2.0, CENTER_Y_MAJOR - val, x / 2.0, CENTER_Y_MAJOR + val, CHSV(hue, deltaHue2, 255U));
    } else {
      DrawLineF(x / 2.0, CENTER_Y_MAJOR + val, x / 2.0, CENTER_Y_MINOR - val, CHSV(hue, deltaHue2, 255U));
    }
  }
}


// ======================================
void visSpectrumV(byte id_cmd, char *packetBuffer) {
  dimAll(72U);
  deltaHue ++;
  for (uint8_t y = 0U; y < HEIGHT; y++) {
    byte posY = y;
    float val = ((WIDTH / 200.0) * packetBuffer[(y + 6)]);
    /* color    */ hue = 80 + y * 255 / (HEIGHT) - deltaHue;
    /* saturate */ deltaHue2 = 255U;
    DrawLineF(CENTER_X_MINOR - val, y, CENTER_X_MINOR + val, y,  CHSV(hue, deltaHue2, 255U));
  }
}

/* ======================================
   Spectrum Explosion | Вибух спектру
   ==================================== */
void visSpectrumExplosion(byte id_cmd, char *packetBuffer) {
  dimAll(100U);
  uint8_t numRays = HEIGHT;
  float angleIncrement = 360.0 / numRays;

  for (uint8_t ray = 0U; ray < numRays; ray++) {
    float angle = ray * angleIncrement;
    float angleRad = angle * PI / 180.0;
    float val = (WIDTH / 200.0) * packetBuffer[(ray + 6)];
    float radius = val * 1.95;

    emitterX = (CENTER_X_MINOR + radius * cos(angleRad));
    emitterY = (CENTER_Y_MINOR + radius * sin(angleRad));
    for (uint8_t distance = 0U; distance < min(WIDTH, HEIGHT) / 2; distance++) {
      hue = static_cast<uint8_t>(angle * 255 / 360 + deltaHue2);
      uint8_t brightness = distance * val;
      drawPixelXYF(emitterX, emitterY, CHSV(hue, 255, 255));
    }
  }
  deltaHue2++;
  drawNote(CHSV(step, 128U, 255U), false);
}

/* ======================================
   Radial Bars | Радіальні стрижні
   ==================================== */
void visRadialBars(byte id_cmd, char* packetBuffer) {
  const uint8_t numRays = HEIGHT;
  const float angleIncrement = 360.0 / numRays;
  const float valScale = WIDTH / 200.0;
  const float radiusScale = 1.95;

  dimAll(100U);
  for (uint8_t ray = 0U; ray < numRays; ray++) {
    // Calculate the angle for the current ray
    float angle = ray * angleIncrement;
    float angleRad = angle * PI / 180.0;

    // Extract the amplitude value from the packetBuffer and scale it
    float val = valScale * packetBuffer[ray + 6];

    // Calculate the radius based on the scaled amplitude value
    float radius = val * radiusScale;

    // Calculate the coordinates of the emitter point
    emitterX = CENTER_X_MAJOR + radius * cos(angleRad) * 1.25;
    emitterY = CENTER_Y_MINOR + radius * sin(angleRad) * 1.25;
    hue = static_cast<uint8_t>(angle * 255 / 360 + deltaHue2);
    uint8_t brightness = radius * numRays;
    // Draw the line from the emitter point with constant brightness
    DrawLineF(CENTER_X_MINOR, CENTER_Y_MINOR, emitterX, emitterY, CHSV( hue, 255, brightness));
  }
  deltaHue2++;
}

// ======================================
// •       Abstract Visualization       •
// ======================================

/* ======================================
   Horny Peacock | Сексуально збуджений павич
   ==================================== */
void visHornyPeacock(byte id_cmd, char *packetBuffer) {
  if (hue > 30) {
    hue = 0;
  }
  currentPalette = RainbowStripeColors_p;
  scale = map(packetBuffer[(hue + 6)], 1, 100, 1U, 30U) ;
  speed = map(packetBuffer[(hue + 7)], 1, 100, 1U, 15U) ;
  colorLoop = 0;
  hue++;
  fillNoiseLED();
}

/* ======================================
   Colored blots | Кольорові плями
   ==================================== */
void visColoredBlots(byte id_cmd, char *packetBuffer) {
  if (hue > 30) {
    hue = 0;
  }
  currentPalette = PartyColors_p;
  currentPalette[8] = CRGB::Black;
  currentPalette[12] = CRGB::Black;
  scale = packetBuffer[(hue + 6)];
  speed = packetBuffer[(hue + 7)] / 2;
  colorLoop = 1;
  hue++;
  fillNoiseLED();
  ihue += 1;
}


/* ======================================
   Magic Eye | Чарівне око
   ==================================== */
void visMagicEye(byte id_cmd, char* packetBuffer) {
  const uint8_t STEP = 255 / CENTER_X_MINOR;
  const uint8_t OFFSET_Y = CENTER_Y_MINOR * 0.4;
  uint8_t soundValue = packetBuffer[6];

  // Calculate the radius based on the sound value --
  float radius = soundValue / 100.0 * max(CENTER_X_MINOR, CENTER_Y_MINOR);

  // Loop through all matrix points -----------------
  for (uint8_t x = 0; x < WIDTH; x++) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      // Calculate the distance from the center to the current point
      float distance = sqrt(pow(x - CENTER_X_MINOR - 1, 2) + pow(y - OFFSET_Y, 2));
      deltaHue = 255 - abs(CENTER_X_MINOR - x) * STEP;
      hue = 112U + distance * radius;
      // Check if the point is inside the radius ----
      if (distance <= radius) {
        deltaHue = 80 + STEP * distance;
        leds[XY(x, y)] = CHSV(hue, 255 - distance, 255);
      } else {
        leds[XY(x, y)] = CHSV(hue, 255, 200 - deltaHue * 0.5);
      }
    }
  }
}

/* ======================================
   Cheerful Fire | Веселий Вогонь
   ==================================== */
void visCheerfulFire(byte id_cmd, char *packetBuffer) {
  if (pcnt >= 30) {
    shiftUpFlame();
    generateFlameLine(packetBuffer);
    pcnt = 0;
  }
  drawFlame(pcnt, packetBuffer);
  pcnt += 25;
}

// ----------------------------------------
void generateFlameLine(char *packetBuffer) {
  for (uint8_t x = 0U; x < WIDTH; x++) {
    line[x] = map( packetBuffer[(x + 6)], 0, 75, 127, 255U);// формування нижнього рядка полум'я на базі даних звуку в діапазоні 127 - 255
  }
}

// ----------------------------------------
void shiftUpFlame() {                                       // scroll up
  for (uint8_t y = HEIGHT - 1U; y > 0U; y--) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      uint8_t newX = x % 16U;
      if (y > 8U) continue;
      matrixValue[y][newX] = matrixValue[y - 1U][newX];     //смещение пламени (только для зоны очага)
    }
  }
  for (uint8_t x = 0U; x < WIDTH; x++) {                    // прорисовка новой нижней линии
    uint8_t newX = x % 16U;                                 // сократил формулу без доп. проверок
    matrixValue[0U][newX] = line[newX];
  }
}

// ----------------------------------------
void drawFlame(uint8_t pcnt, char *packetBuffer) {
  int32_t nextv;
  uint8_t baseSat = 255U;


  //first row interpolates with the "next" line
  deltaHue = (packetBuffer[random(6U, 32U)] < 25) ? constrain (shiftHue[0] + random(0U, 2U) - random(0U, 2U), 15U, 17U) : shiftHue[0];
  shiftHue[0] = deltaHue;                                   // заносим это значение в стэк

  deltaValue = (packetBuffer[random(6U, 32U)] < 45) ? constrain (shiftValue[0] + random(0U, 2U) - random(0U, 2U), 15U, 17U) : shiftValue[0]; // random(0U, 3U)= скорость смещения очага чем больше 3U - тем медленнее
  // 15U, 17U - амплитуда качания -1...+1 относительно 16U
  shiftValue[0] = deltaValue;


  for (uint8_t x = 0U; x < WIDTH; x++) {                                          // прорисовка нижней строки (сначала делаем ее, так как потом будем пользоваться ее значением смещения)
    uint8_t newX = x % 16;                                                        // сократил формулу без доп. проверок
    nextv =                                                                       // расчет значения яркости относительно valueMask и нижерасположенной строки.
      (((100.0 - pcnt) * matrixValue[0][newX] + pcnt * line[newX]) / 100.0) -
      pgm_read_byte(&valueMask[0][(x + deltaValue) % 16U]);
    nextv -= (100 - packetBuffer[x + 6U]) / 5;                                    // корекція яскравості полум'я з урахуванням амплітуди звуку

    if (packetBuffer[x + 6U] < 20U) nextv -= (100 - packetBuffer[x + 6U]) / 5;
    CRGB color = CHSV(                                                            // вычисление цвета и яркости пикселя
                   /* H */  pgm_read_byte(&hueMask[0][(x + deltaHue) % 16U]),
                   /* S */  baseSat,
                   /* V */  (uint8_t)max(0, nextv) );
    leds[XY(x, 1)] = color;
    leds[XY(x, 0)] = color;
  }

  //each row interpolates with the one before it
  for (uint8_t y = HEIGHT - 1U; y > 0U; y--) {                                      // прорисовка остальных строк с учетом значения низлежащих
    deltaHue = shiftHue[y];                                                         // извлекаем положение
    shiftHue[y] = shiftHue[y - 1];                                                  // подготавлеваем значение смешения для следующего кадра основываясь на предыдущем
    deltaValue = shiftValue[y];                                                     // извлекаем положение
    shiftValue[y] = shiftValue[y - 1];                                              // подготавлеваем значение смешения для следующего кадра основываясь на предыдущем

    if (y > 9U) {                                                                   // цикл стирания текущей строоки для искр
      for (uint8_t _x = 0U; _x < WIDTH; _x++) {                                     // стираем строчку с искрами (очень не оптимально)
        drawPixelXY(_x, y, 0U);
      }
    }
    for (uint8_t x = 0U; x < WIDTH; x++) {                                          // пересчет координаты x для текущей строки
      uint8_t newX = x % 16U;                                                       // функция поиска позиции значения яркости для матрицы valueMask
      if (y < 8U) {                                                                 // если строка представляет очаг
        nextv =                                                                     // расчет значения яркости относительно valueMask и нижерасположенной строки.
          (((100.0 - pcnt) * matrixValue[y][newX] +
            pcnt * matrixValue[y - 1][newX]) / 100.0) -
          pgm_read_byte(&valueMask[y][(x + deltaValue) % 16U]);
        nextv -= (100 - packetBuffer[x + 6U]) / 4;                                  // корекція яскравості полум'я з урахуванням амплітуди звуку
        CRGB color = CHSV(
                       /* H */  pgm_read_byte(&hueMask[y][(x + deltaHue) % 16U ]),
                       /* S */  baseSat,
                       /* V */  (uint8_t)max(0, nextv) );
        leds[XY(x, y + 1)] = color;
        // if (y == 0) leds[XY(x, y)] = color;
      } else if (y == 9U && SPARKLES) {                                               // если это самая нижняя строка искр - формитуем искорку из пламени
        if (random(0, 20) == 0 && getPixColorXY(x, y - 1U) != 0U) drawPixelXY(x, y, getPixColorXY(x, y - 2U));  // 20 = обратная величина количества искр
        else drawPixelXY(x, y, 0U);
      } else if (SPARKLES) {                                                          // если это не самая нижняя строка искр - перемещаем искорку выше
        // старая версия для яркости
        newX = (random(0, 4)) ? x : (x + WIDTH + random(0U, 2U) - random(0U, 2U)) % WIDTH ;   // с вероятностью 1/3 смещаем искорку влево или вправо
        if (getPixColorXY(x, y - 1U) > 0U) drawPixelXY(newX, y, getPixColorXY(x, y - 1U));    // рисуем искорку на новой строчке
      }
    }
  }
}


/* ======================================
   Tuning Indicator
  ====================================== */
void visTuningIndicator(byte id_cmd, char* packetBuffer) {
  CRGB color;
  const uint8_t PEAK =  HEIGHT * 0.75;
  const uint8_t STEP = 160 / HEIGHT;
  const uint8_t CENTER = CENTER_X_MINOR + 2;

  dimAll(200U);
  if (step > WIDTH - 1) step = 0;
  float soundValue = packetBuffer[6 + step];

  for (uint8_t y = 0U; y < HEIGHT; y++) {
    for (uint8_t x = 0U; x < WIDTH; x++) {

      // draw peak ------
      byte val = (HEIGHT / 100.0) * packetBuffer[6 + y];
      if (val > deltaValue) {
        deltaValue = val;
      } else {
        if ((deltaValue > 0) & (step % 2 == 0)) {
          deltaValue--;
        }
      }

      // draw spectre ---
      hue = 96 + y / 8;
      emitterX = static_cast<float> (log10(y / 100.0 * soundValue + 1) * 2.25 + 1.5 );
      deltaHue = 74 + y * STEP;

      if ((y > CENTER_Y_MINOR) & (soundValue > 25)) {
        drawPixelXYF(CENTER - emitterX - 1, y, CHSV(hue, 255, deltaHue));
        drawPixelXYF(CENTER + emitterX - 1, y, CHSV(hue, 255, deltaHue));
      } else {
        drawPixelXY(CENTER - emitterX, y, CHSV(hue, 255, deltaHue));
        drawPixelXY(CENTER + emitterX - 1, y, CHSV(hue, 255, deltaHue));
      }
      if (soundValue > 50) {
        drawPixelXYF(0.5, y, CHSV(hue, 255, 255 - deltaHue));
        drawPixelXYF(WIDTH - 1.5, y, CHSV(hue, 255, 255 - deltaHue));
      }

      // -------------
      deltaHue = (deltaValue >= PEAK) ? 255U : 0U;
      color = CHSV(240, deltaHue, 255);
      if (deltaValue > y) drawPixelXY(CENTER - 1, deltaValue, color);
    }
  }
  step++;
}

/* ======================================
   Dragon Bbreath | Дихання дракона
  ====================================== */
void visDragonBbreath(byte id_cmd, char *packetBuffer) {
  // https://github.com/FastLED/FastLED/wiki/Pixel-reference
  const uint8_t amount = 128;

  for (uint8_t y = HEIGHT - 1U; y > 0U; y--) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      uint8_t newX = (random(0, 4)) ? x : (x + WIDTH + random(0U, 2U) - random(0U, 2U)) % WIDTH;
      CRGB rgbColor = getPixColorXY(x, y - 1U);


      if ((getPixColorXY(x, y - 1U) > 0U) & (y > floor(HEIGHT * 0.75))) {
        // Convert color to CHSV
        CHSV hsvColor = rgb2hsv_approximate(rgbColor);
        // Set saturation to 50%
        hsvColor.saturation = 100;
        // Increase the brightness by a certain amount
        hsvColor.value = qadd8(hsvColor.value, 16);
        drawPixelXY(newX, y, hsvColor);
      } else {
        drawPixelXY(newX, y, rgbColor);
      }
    }
  }

  dimAll(160U);

  uint8_t numRays = HEIGHT;
  float angleIncrement = 360.0 / numRays;

  for (uint8_t ray = 0U; ray < numRays; ray++) {
    float angle = ray * angleIncrement;
    float angleRad = angle * PI / 180.0;
    uint8_t soundLevel = packetBuffer[(ray + 6)];
    float val = (WIDTH / 200.0) * soundLevel;
    float radius = val * 1.5;

    emitterX = (CENTER_X_MINOR + radius * cos(angleRad));
    emitterY = (CENTER_Y_MINOR / 2 + radius * sin(angleRad));
    for (uint8_t distance = 0U; distance < min(WIDTH, HEIGHT) / 2; distance++) {
      hue = static_cast<uint8_t>(angle * 255 / 360 + deltaHue2);
      uint8_t brightness = distance * val;
      if (soundLevel > 2U) drawPixelXYF(emitterX, emitterY, CHSV(hue, 255 - radius * 2,  constrain (soundLevel, 96U, 255U)));
    }
  }
  deltaHue2++;
}
#endif
