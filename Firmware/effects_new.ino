/* выделяем в памяти масив для загружаемых бинарных изображений
  header= 16 | заголовок
  w=16, h=16 | ширина высота
  color  = 2 | байт на цвет
  frames = 5 | количество кадров
  масив на 5 кадров 16x16 | размером w * h * frames * color + header = 2 576
  размер можно увеличивать по мере надобности, постоянно занимает место в памяти
  возможно в будущем будет сделано динамическим */

uint8_t const exp_gamma[256] = {
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,
  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
  1,   2,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,
  4,   4,   4,   4,   4,   5,   5,   5,   5,   5,   6,   6,   6,   7,   7,
  7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,  11,  11,  12,  12,
  12,  13,  13,  14,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,
  19,  20,  20,  21,  21,  22,  23,  23,  24,  24,  25,  26,  26,  27,  28,
  28,  29,  30,  30,  31,  32,  32,  33,  34,  35,  35,  36,  37,  38,  39,
  39,  40,  41,  42,  43,  44,  44,  45,  46,  47,  48,  49,  50,  51,  52,
  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,
  68,  70,  71,  72,  73,  74,  75,  77,  78,  79,  80,  82,  83,  84,  85,
  87,  89,  91,  92,  93,  95,  96,  98,  99,  100, 101, 102, 105, 106, 108,
  109, 111, 112, 114, 115, 117, 118, 120, 121, 123, 125, 126, 128, 130, 131,
  133, 135, 136, 138, 140, 142, 143, 145, 147, 149, 151, 152, 154, 156, 158,
  160, 162, 164, 165, 167, 169, 171, 173, 175, 177, 179, 181, 183, 185, 187,
  190, 192, 194, 196, 198, 200, 202, 204, 207, 209, 211, 213, 216, 218, 220,
  222, 225, 227, 229, 232, 234, 236, 239, 241, 244, 246, 249, 251, 253, 254,
  255
};
/* binImage bufer для бінарних img size вибраний по розміру підгружаємих картинок */
byte binImage[2336];

// ======================================
// espModeStat default lamp start effect
// ======================================
void  espModeState(uint8_t color) {
  if (loadingFlag) {
    loadingFlag = false;
    step = deltaValue;
    deltaValue = 1;
    hue2 = 0;
    deltaHue2 = 1;
    DrawLine(CENTER_X_MINOR, CENTER_Y_MINOR, CENTER_X_MAJOR + 1, CENTER_Y_MINOR, CHSV(color, 255, 210));
    DrawLine(CENTER_X_MINOR, CENTER_Y_MINOR - 1, CENTER_X_MAJOR + 1, CENTER_Y_MINOR - 1, CHSV(color, 255, 210));
    // setModeSettings(128U, 128U);
    pcnt = 1;
    FastLED.clear();
  }
  if (pcnt > 0 & pcnt < 200) {
    if (pcnt != 0) {
      pcnt++;
    }

    // animation esp state ===========
    dimAll(108);
    //    if (step % 2 == 0) {
    uint8_t w = validMinMax(hue2, 0, floor(WIDTH / 2) - 1);
    uint8_t posY = validMinMax(CENTER_Y_MINOR + deltaHue2, 0, HEIGHT - 1);
    DrawLine(CENTER_X_MINOR - w, posY, CENTER_X_MAJOR + w, posY, CHSV(color, 255, (210 - deltaHue2)));
    posY = validMinMax(CENTER_Y_MINOR - 1 - deltaHue2, 1, HEIGHT - 1);
    DrawLine(CENTER_X_MINOR - w, posY, CENTER_X_MAJOR + w, posY, CHSV(color, 255, (210 - deltaHue2)));

    if (deltaHue2 == 0) {
      deltaHue2 = 1;
    }
    hue2++;
    deltaHue2 = deltaHue2 << 1;
    if (deltaHue2 == 2) {
      deltaHue2 = deltaHue2 << 1;
    }
    if (CENTER_Y_MINOR + deltaHue2 > HEIGHT) {
      deltaHue2 = 0;
      hue2 = 0;
    }
    // LOG.printf_P(PSTR("espModeState | pcnt = %05d | deltaHue2 = %03d | step %03d | ONflag • %s\n"), pcnt, deltaHue2, step, (ONflag ? "TRUE" : "FALSE"));
  } else {

#ifdef USE_NTP
    // error ntp ------------------
    color = 255;        // если при включенном NTP время не получено, будем красным цветом мигать
#else
    color = 176U;       // иначе скромно синим - нормальная ситуация при отсутствии NTP
#endif //USE_NTP
    // animtion no time -----------
    leds[XY(CENTER_X_MINOR , 0U)] = CHSV( color, 255, (step % 4 == 0) ? 200 : 128);

  }
  // clear led lamp ---------------
  if ( pcnt >= 100) {
    pcnt = 0;
    //    FastLED.clear();
    //    FastLED.delay(2);
    FastLED.clear();
    delay(2);
    FastLED.show();
    loadingFlag = false;
  }
  step++;
}

//---------------------------------------
bool isJavelinMode() {
  if (eff_valid == CMD_NEXT_EFF) {
    currentMode = MODE_AMOUNT - 1;
  }
  if (eff_valid == 0) {
    currentMode = 84;
  }
  return !dawnFlag;
}

//---------------------------------------
// Global Function
//---------------------------------------
void drawRec(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint32_t color) {
  for (uint8_t y = startY; y < endY; y++) {
    for (uint8_t x = startX; x < endX; x++) {
      drawPixelXY(x, y, color);
    }
  }
}

//---------------------------------------
void drawRecCHSV(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, CHSV color) {
  for (uint8_t y = startY; y < endY; y++) {
    for (uint8_t x = startX; x < endX; x++) {
      drawPixelXY(x, y, color);
    }
  }
}

//--------------------------------------
uint8_t validMinMax(float val, uint8_t minV, uint32_t maxV) {
  uint8_t result;
  if (val <= minV) {
    result = minV;
  } else if (val >= maxV) {
    result = maxV;
  } else {
    result = ceil(val);
  }
  //  LOG.printf_P(PSTR( "result: %f | val: %f \n\r"), result, val);
  return result;
}

//--------------------------------------
void gradientHorizontal(int startX, int startY, int endX, int endY, uint8_t start_color, uint8_t end_color, uint8_t start_br, uint8_t end_br, uint8_t saturate) {
  static float step_color = 1.;
  static float step_br = 1.;
  static int color = start_color;
  static int br = start_br;
  uint8_t temp_step;
  if ((startX < 0) | ( startY < 0)) {
    return;
  }
  if (startX == endX) {
    if (startX >= WIDTH) {
      startX = WIDTH - 1 ;
    } else {
      endX++;
    }
  }
  if (startY == endY) {
    if (startY >= HEIGHT) {
      startY = HEIGHT - 1 ;
    } else {
      endY++;
    }
  }
  temp_step = abs(startX - endX);
  if (temp_step == 0) temp_step = 1;
  step_color = float(abs(end_color - start_color) / temp_step);
  step_br = float(abs(end_br - start_br) / temp_step);

  if (start_color > end_color) {
    step_color = -1. * step_color;
    color = end_color;
  } else {
    color = start_color;
  }
  br = start_br;
  if (start_br > end_br) {
    step_br = -1. * step_br;
  }

  for (uint8_t x = startX; x < endX; x++) {
    CHSV thisColor = CHSV(color + floor((x - startX) * step_color), saturate, br + floor((x - startX) * step_br));
    for (uint8_t y = startY; y < endY; y++) {
      leds[XY(x, y)] = thisColor;
    }
  }
}

//--------------------------------------
void gradientVertical(int startX, int startY, int endX, int endY, uint8_t start_color, uint8_t end_color, uint8_t start_br, uint8_t end_br, uint8_t saturate) {
  static float step_color = 1.;
  static float step_br = 1.;
  static int color = start_color;
  static int br = start_br;
  uint8_t temp_step;

  if ( (startX < 0) | ( startY < 0) | (startX > (WIDTH - 1)) | ( startY > (HEIGHT - 1))) {
    return;
  }
  if (startX == endX) {
    if (startX >= WIDTH) {
      startX = WIDTH - 1 ;
    } else {
      endX++;
    }
  }
  if (startY == endY) {
    if (startY >= HEIGHT) {
      startY = HEIGHT - 1 ;
    } else {
      endY++;
    }
  }
  temp_step = abs(startY - endY);
  if (temp_step == 0) temp_step = 1;

  step_color = float(abs(end_color - start_color) / temp_step);
  step_br = float(abs(end_br - start_br) / temp_step);

  if (start_color > end_color) {
    step_color = -1. * step_color;
    color = end_color;
  } else {
    color = start_color;
  }
  br = start_br;
  if (start_br > end_br) {
    step_br = -1. * step_br;
  }

  for (uint8_t y = startY; y < endY; y++) {
    CHSV thisColor = CHSV(color + floor((y - startY) * step_color), saturate, br + floor((y - startY) * step_br));
    for (uint8_t x = startX; x < endX; x++) {
      leds[XY(x, y)] = thisColor;
    }
  }
}

// ======================================
// функции для работы с бинарными файлами
// ======================================

// --------------------------------------
/* функция чтения бинарного файла изображения
    из файловой системы лампы */
void readBinFile(String fileName, size_t len ) {

  File binFile = SPIFFS.open("/" + fileName, "r");
  if (!binFile) {
    LOG.println("File not found");
    printMSG("Bin File not found", true);
    return;
  }
  size_t size = binFile.size();
  if (size > len) {
    binFile.close();
    LOG.println("Large File");
    return;
  }

  byte buffer[size];
  uint16_t amount;

  if (binFile == NULL) exit (1);
  binFile.seek(0);

  while (binFile.available()) {
    amount = binFile.read(buffer, size);
  }

#ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR("File size • %08d bytes\n"), amount);
#endif

  // binImage = malloc(amount);
  // byte *by = malloc(1024);
  // memset(binImage, 66, 1552);
  // byte *by = new byte[size];
  memcpy(binImage, buffer, amount);
  binFile.close();
}

// --------------------------------------
/* функция получения размера изображения
   из заголовка файла
*/
uint16_t getSizeValue(byte* buffer, byte b ) {
  return  (buffer[b + 1] << 8) + buffer[b];
}

// --------------------------------------
/* функция скрола изображения по оси X */
void scrollImage(uint16_t imgW, uint16_t imgH, uint16_t start_row) {
  const byte HEADER = 16;
  const uint16_t BYTES_PER_PIXEL = 2U;
  // const uint16_t imgSize = imgW * imgH * BYTES_PER_PIXEL + HEADER;
  uint8_t r, g, b;
  uint8_t padding = floor((HEIGHT - imgH) / 2);
  uint8_t topPos = HEIGHT - padding - 1;
  uint16_t pixIndex;
  uint8_t delta = 0;

  for (uint16_t x = 0; x < WIDTH; x++) {
    for (uint16_t y = 0; y < (imgH - 1); y++) {
      if ((start_row + x) > WIDTH) {
        delta = 1;
      }
      pixIndex = HEADER + (start_row + x + y * imgW) * BYTES_PER_PIXEL;

      /* convert rgb565 to rgb888 -----------
        masc rgb565  0xF800 | 0x07E0 | 0x001F */
      r = (binImage[pixIndex + 1] & 0xF8);
      g = ((binImage[pixIndex + 1] & 0x07) << 5) + ((binImage[pixIndex] & 0xE0) << 5);
      b = (binImage[pixIndex] & 0x1F) << 3;
      /* // вариант с изменением яркости ----
        hue = abs(16 - start_row) * 4;
        leds[XY(x, topPos - y - delta)] = CRGB(constrain(r - hue, 0, 255), constrain(g - hue, 0, 255), constrain(b - hue, 0, 255));
        // ------------------------------------
      */
      //      if (y == (imgH -2)) {
      //        LOG.printf_P(PSTR("Scroll | %04d | top • %01d | x: %02d y: %02d | %03d\n\r"), pixIndex, topPos, x, y, b);
      //      }
      leds[XY(x, topPos - y - delta )] = CRGB(r, g, b);
      /* draw background */
      if ((start_row == 0) && (y == 0) && (padding > 0)) {
        // fillAll(getPixColorXY(0, topPos));
        drawRec(0, HEIGHT - padding, WIDTH, HEIGHT, getPixColorXY(0, topPos));
        drawRec(0, 0, WIDTH, padding, getPixColorXY(0, topPos));
      }
    } /* end for y */
  }
}

// ======================================
// New Effects
// ======================================
uint32_t colorDimm(uint32_t colorValue, long lenght, long pixel) {

  uint8_t red = (colorValue & 0x00FF0000) >> 16;
  uint8_t green = (colorValue & 0x0000FF00) >> 8;
  uint8_t blue = (colorValue & 0x000000FF);

  double prozent = 100 / lenght;

  red = red - red * ((prozent * pixel) / 100);
  green = green - green * ((prozent * pixel) / 100);
  blue = blue - blue * ((prozent * pixel) / 100);

  // colorValue = strip.Color(red,green,blue);
  colorValue = red;
  colorValue = (colorValue << 8) + green;
  colorValue = (colorValue << 8) + blue;
  return colorValue;
}

// =============== Wine ================
//    © SlingMaster | by Alex Dovby
//               EFF_WINE
//--------------------------------------

void colorsWine() {
  uint8_t divider;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(20U + random8(200U), 200U);
    }
#endif
    loadingFlag = false;
    fillAll(CHSV(55U, 255U, 65U));
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    // minspeed 230 maxspeed 250 ============
    // minscale  40 maxscale  75 ============
    // красное вино hue > 0 & <=10
    // розовое вино hue > 10 & <=20
    // белое вино   hue > 20U & <= 40
    // шампанское   hue > 40U & <= 60

    deltaHue2 = 0U;                         // count для замедления смены цвета
    step = deltaValue;                      // чтообы при старте эффекта сразу покрасить лампу
    deltaHue = 1U;                          // direction | 0 hue-- | 1 hue++ |
    hue = 55U;                              // Start Color
    hue2 = 65U;                             // Brightness
    pcnt = 0;
  }

  deltaHue2++;
  // маштаб задает скорость изменения цвета 5 уровней
  divider = 5 - floor((modes[currentMode].Scale - 1) / 20);

  // возвращаем яркость для перехода к белому
  if (hue >= 10 && hue2 < 100U) {
    hue2++;
  }
  // уменьшаем яркость для красного вина
  if (hue < 10 && hue2 > 40U) {
    hue2--;
  }

  // изменение цвета вина -----
  if (deltaHue == 1U) {
    if (deltaHue2 % divider == 0) {
      hue++;
    }
  } else {
    if (deltaHue2 % divider == 0) {
      hue--;
    }
  }
  // --------

  // LOG.printf_P(PSTR("Wine | hue = %03d | Dir = %d | Brightness %03d | deltaHue2 %03d | divider %d | %d\n"), hue, deltaHue, hue2, deltaHue2, divider, step);

  // сдвигаем всё вверх -----------
  for (uint8_t x = 0U; x < WIDTH; x++) {
    for (uint8_t y = HEIGHT; y > 0U; y--) {
      drawPixelXY(x, y, getPixColorXY(x, y - 1U));
    }
  }

  if (hue > 40U) {
    // добавляем перляж для шампанского
    pcnt = random(0, WIDTH);
  } else {
    pcnt = 0;
  }

  // заполняем нижнюю строку с учетом перляжа
  for (uint8_t x = 0U; x < WIDTH; x++) {
    if ((x == pcnt) && (pcnt > 0)) {
      // с перляжем ------
      drawPixelXY(x, 0U, CHSV(hue, 150U, hue2 + 20U + random(0, 50U)));
    } else {
      drawPixelXY(x, 0U, CHSV(hue, 255U, hue2));
    }
  }

  // меняем направление изменения цвета вина от красного к шампанскому и обратно
  // в диапазоне шкалы HUE |0-60|
  if  (hue == 0U) {
    deltaHue = 1U;
  }
  if (hue == 60U) {
    deltaHue = 0U;
  }
  step++;
}

// ============== Swirl ================
//    © SlingMaster | by Alex Dovby
//              EFF_SWIRL
//--------------------------------------
void Swirl() {
  static uint8_t divider;
  uint8_t lastHue;
  static const uint32_t colors[6][6] PROGMEM = {
    {CRGB::Blue, CRGB::DarkRed, CRGB::Aqua, CRGB::Magenta, CRGB::Gold, CRGB::Green },
    {CRGB::Yellow, CRGB::LemonChiffon, CRGB::LightYellow, CRGB::Gold, CRGB::Chocolate, CRGB::Goldenrod},
    {CRGB::Green, CRGB::DarkGreen, CRGB::LawnGreen, CRGB::SpringGreen, CRGB::Cyan, CRGB::Black },
    {CRGB::Blue, CRGB::DarkBlue, CRGB::MidnightBlue, CRGB::MediumSeaGreen, CRGB::MediumBlue, CRGB:: DeepSkyBlue },
    {CRGB::Magenta, CRGB::Red, CRGB::DarkMagenta, CRGB::IndianRed, CRGB::Gold, CRGB::MediumVioletRed },
    {CRGB::Blue, CRGB::DarkRed, CRGB::Aqua, CRGB::Magenta, CRGB::Gold, CRGB::Green }
  };
  uint32_t color;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(50U + random8(190U), 250U);
    }
#endif
    loadingFlag = false;
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    divider = floor((modes[currentMode].Scale - 1) / 20); // маштаб задает смену палитры
    deltaHue2 = 0U;                         // count для замедления смены цвета
    deltaHue = 0U;                          // direction | 0 hue-- | 1 hue++ |
    hue2 = 0U;                              // x
    hue = 0;
    FastLED.clear();
  }

  // задаем цвет и рисуем завиток --------
  color = colors[divider][hue];
  if (deltaHue2 < HEIGHT - 3) {
    drawPixelXY(hue2, deltaHue2 + 2, color);
  }
  drawPixelXY(hue2, deltaHue2, color);
  drawPixelXY(WIDTH - hue2, HEIGHT - deltaHue2, colors[divider + 1][hue]);
  // -------------------------------------

  hue2++;                     // x
  // два варианта custom_eff задается в сетапе лампы ----
  if (custom_eff == 1) {
    if (hue2 % 2 == 0) {
      deltaHue2++;            // y
    }
  } else {
    deltaHue2++;              // y
  }
  // -------------------------------------

  if  (hue2 > WIDTH) {
    hue2 = 0U;
  }

  if (deltaHue2 >= HEIGHT) {
    deltaHue2 = 0U;
    // new swirl ------------
    hue2 = random8(WIDTH - 2);
    // hue2 = hue2 + 2;
    // select new color -----
    hue = random8(6);

    if (lastHue == hue) {
      hue++;
      if (hue >= 6) {
        hue = 0;
      }
    }
    lastHue = hue;
  }
  blurScreen(5U + random8(5));
  step++;
}

// -------------------------------------------
// for effect Ukraine
// -------------------------------------------
void drawCrest() {
  static const bool data[9][5] = {
    {0, 0, 1, 0, 0 },
    {1, 0, 1, 0, 1 },
    {1, 0, 1, 0, 1 },
    {1, 0, 1, 0, 1 },
    {1, 0, 1, 0, 1 },
    {1, 1, 1, 1, 1 },
    {1, 0, 1, 0, 1 },
    {0, 1, 1, 1, 0 },
    {0, 0, 1, 0, 0 }
  };
  uint8_t posX = floor(WIDTH * 0.5) - 3;
  uint8_t posY = constrain(floor(HEIGHT * 0.4) + 5, 9, HEIGHT);
  uint32_t color;
  FastLED.clear();
  for (uint8_t y = 0U; y < 9; y++) {
    for (uint8_t x = 0U; x < 5; x++) {
      drawPixelXY(posX + x, posY - y, (data[y][x]) ? CRGB(255, 0xD7, 0) : CRGB(0, 0, 0));
    }
  }
}

// ============== Ukraine ==============
//      © SlingMaster | by Alex Dovby
//              EFF_UKRAINE
//--------------------------------------
void Ukraine() {
  uint8_t divider;
  uint32_t color;
  static const uint16_t MAX_TIME = 500;
  uint16_t tMAX = 100;
  static const uint8_t timeout = 100;
  static const uint8_t blur_step = HEIGHT / 2;
  static const uint32_t colors[2][5] = {
    {CRGB::Blue, CRGB::MediumBlue, 0x0F004F, 0x02002F, 0x1F2FFF },
    {CRGB::Yellow, CRGB::Gold, 0x4E4000, 0xFF6F00, 0xFFFF2F }
  };

  // Initialization =========================
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(250U), 200U + random8(50U));
    }
#endif
    loadingFlag = false;
    drawCrest();
    // minspeed 200 maxspeed 250 ============
    // minscale   0 maxscale 100 ============
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = deltaValue;                        // чтообы при старте эффекта сразу покрасить лампу
    deltaHue2 = 0U;                           // count для замедления смены цвета
    deltaHue = 0U;                            // direction | 0 hue-- | 1 hue++ |
    hue2 = 0U;                                // Brightness
    ff_x = 1U;                                // counter
    tMAX = 100U;                              // timeout
  }
  divider = floor((modes[currentMode].Scale - 1) / 10); // маштаб задает режим рестарта
  tMAX = timeout + 100 * divider;

  if ((ff_x > timeout - 10) && (ff_x < timeout)) { // таймаут блокировки отрисовки флага
    if (ff_x < timeout - 5) {                  // размытие тризуба
      blurScreen(beatsin8(5U, 60U, 5U));
    } else {
      blurScreen(230U - ff_x);
    }
  }

  if (ff_x > tMAX) {
    if (divider == 0U) {                       // отрисовка тризуба только раз
      ff_x = 0U;
      tMAX += 20;
    } else {
      if (ff_x > tMAX + 100U * divider) {      // рестар эффект
        drawCrest();
        ff_x = 1U;
      }
    }
  }
  if ((ff_x != 0U) || (divider > 0)) {
    ff_x++;
  }

  // Flag Draw =============================
  if ((ff_x > timeout) || (ff_x == 0U))  {     // отрисовка флага
    if (step >= deltaValue) {
      step = 0U;
      hue2 = random8(WIDTH - 2);               // случайное смещение мазка по оси Y
      hue = random8(5);                        // flag color
      // blurScreen(dim8_raw(beatsin8(3, 64, 100)));
      // blurScreen(beatsin8(5U, 60U, 5U));
      // dimAll(200U);
    }
    if (step % blur_step == 0 && modes[currentMode].Speed > 230) {
      blurScreen(beatsin8(5U, 5U, 72U));
    }
    hue2++;                                    // x
    deltaHue2++;                               // y

    if (hue2 >= WIDTH) {
      if (deltaHue2 > HEIGHT - 2 ) {           // если матрица высокая дорисовываем остальные мазки
        deltaHue2 = random8(5);                // изменяем положение по Y только отрисовав весь флаг
      }
      if (step % 2 == 0) {
        hue2 = 0U;
      } else {
        hue2 = random8(WIDTH);                 // смещение первого мазка по оси X
      }
    }

    if (deltaHue2 >= HEIGHT) {
      deltaHue2 = 0U;
      if (deltaValue > 200U) {
        hue = random8(5);                      // если низкая скорость меняем цвет после каждого витка
      }
    }

    if (deltaHue2 > CENTER_Y_MAJOR - random8(2)) {    // меняем цвет для разных частей флага
      color = colors[0][hue];
    } else {
      color = colors[1][hue];
    }

    // LOG.printf_P(PSTR("color = %08d | hue2 = %d | speed = %03d | custom_eff = %d\n"), color, hue2, deltaValue, custom_eff);
    drawPixelXY(hue2, deltaHue2, color);
    // ----------------------------------
    step++;
  }
}

// ============ Oil Paints ==============
//      © SlingMaster | by Alex Dovby
//              EFF_PAINT
//           Масляные Краски
//---------------------------------------
void OilPaints() {

  uint8_t divider;
  uint8_t entry_point;
  uint16_t value;
  uint16_t max_val;
  if (loadingFlag) {

#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                          scale | speed 210
      setModeSettings(1U + random8(252U), 1 + random8(219U));
    }
#endif
    loadingFlag = false;
    FastLED.clear();
    // blurScreen(beatsin8(5U, 50U, 5U));
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = deltaValue;                    // чтообы при старте эффекта сразу покрасить лампу
    hue = floor(21.25 * (random8(11) + 1)); // next color
    deltaHue = hue - 22;                  // last color
    deltaHue2 = 80;                       // min bright
    max_val = pow(2, WIDTH);
    //    for ( int i = WIDTH; i < (NUM_LEDS - WIDTH); i++) {
    //      leds[i] = CHSV(120U, 24U, 64U);
    //    }
  }

  if (step >= deltaValue) {
    step = 0U;
    // LOG.printf_P(PSTR("%03d | log: %f | val: %03d\n\r"), modes[currentMode].Brightness, log(modes[currentMode].Brightness), deltaHue2);
  }

  // Create Oil Paints --------------
  // выбираем краски  ---------------
  if (step % CENTER_Y_MINOR == 0) {
    divider = floor((modes[currentMode].Scale - 1) / 10);             // маштаб задает диапазон изменения цвета
    deltaHue = hue;                                                   // set last color
    hue += 6 * divider;                                               // new color
    hue2 = 255;                                                       // restore brightness
    deltaHue2 = 80 - floor(log(modes[currentMode].Brightness) * 6);   // min bright
    entry_point = random8(WIDTH);                                     // start X position
    trackingObjectHue[entry_point] = hue;                             // set start position
    drawPixelXY(entry_point,  HEIGHT - 2, CHSV(hue, 255U, 255U));
    // !!! ********
    if (custom_eff == 1) {
      drawPixelXY(entry_point + 1,  HEIGHT - 3, CHSV(hue + 30, 255U, 255U));
    }
    // ************
    // LOG.printf_P(PSTR("BR %03d | SP %03d | SC %03d | hue %03d\n\r"), modes[currentMode].Brightness, modes[currentMode].Speed, modes[currentMode].Scale, hue);
  }

  // формируем форму краски, плавно расширяя струю ----
  if (random8(3) == 1) {
    // LOG.println("<--");
    for (uint8_t x = 1U; x < WIDTH; x++) {
      if (trackingObjectHue[x] == hue) {
        trackingObjectHue[x - 1] = hue;
        break;
      }
    }
  } else {
    // LOG.println("-->");
    for (uint8_t x = WIDTH - 1; x > 0U ; x--) {
      if (trackingObjectHue[x] == hue) {
        trackingObjectHue[x + 1] = hue;
        break;
      }
      // LOG.printf_P(PSTR("x = %02d | value = %03d | hue = %03d \n\r"), x, trackingObjectHue[x], hue);
    }
  }
  // LOG.println("------------------------------------");

  // выводим сформированную строку --------------------- максимально яркую в момент смены цвета
  for (uint8_t x = 0U; x < WIDTH; x++) {
    //                                                                                set color  next |    last  |
    drawPixelXY(x,  HEIGHT - 1, CHSV(trackingObjectHue[x], 255U, (trackingObjectHue[x] == hue) ? hue2 : deltaHue2));
  }
  //  LOG.println("");
  // уменьшаем яркость для следующих строк
  if ( hue2 > (deltaHue2 + 16)) {
    hue2 -= 16U;
  }
  // сдвигаем неравномерно поток вниз ---
  value = random16(max_val);
  //LOG.printf_P(PSTR("value = %06d | "), value);
  for (uint8_t x = 0U; x < WIDTH; x++) {
    if ( bitRead(value, x ) == 0) {
      //LOG.print (" X");
      for (uint8_t y = 0U; y < HEIGHT - 1; y++) {
        drawPixelXY(x, y, getPixColorXY(x, y + 1U));
      }
    }
  }
  // LOG.printf_P(PSTR("%02d | hue2 = %03d | min = %03d \n\r"), step, hue2, deltaHue2);
  // -------------------------------------

  step++;
}


// ========== Botswana Rivers ===========
//      © SlingMaster | by Alex Dovby
//              EFF_RIVERS
//            Реки Ботсваны

//---------------------------------------
void flora() {
  if (WIDTH < 10) return;
  static const uint8_t POS_X = (CENTER_X_MINOR - 6);
  static const  uint32_t LEAF_COLOR = 0x1F2F00;
  static const bool data[5][5] = {
    { 0, 0, 0, 0, 1 },
    { 0, 0, 1, 1, 1 },
    { 0, 1, 1, 1, 1 },
    { 0, 1, 1, 1, 0 },
    { 1, 0, 0, 0, 0 }
  };

  uint8_t h =  random8(10U, 15U);
  uint8_t posX = 3; // floor(WIDTH * 0.5) - 3;
  byte deltaY = random8(2U);

  gradientVertical(POS_X - 1, 0, POS_X, 9U, 70, 75, 65U, 255U, 255U);
  gradientVertical(POS_X + 1, 0, POS_X + 2, 15U, 70, 75, 65U, 255U, 255U);
  drawPixelXY(POS_X + 2, h - random8(floor(h * 0.5)), random8(2U) == 1 ? 0xFF00E0 :  random8(2U) == 1 ? 0xFFFF00 : 0x00FF00);
  drawPixelXY(POS_X + 1, h - random8(floor(h * 0.25)), random8(2U) == 1 ? 0xFF00E0 : 0xFFFF00);
  if (random8(2U) == 1) {
    drawPixelXY(posX + 1, 5U, random8(2U) == 1 ? 0xEF001F :  0x9FFF00);
  }
  h =  floor(h * 0.65);
  drawPixelXY(POS_X, h - random8(5, h - 2), random8(2U) == 1 ? 0xFF00E0 : 0xFFFF00);

  // draw leafs -------------------
  for (uint8_t y = 0; y < 5; y++) {
    for (uint8_t x = 0; x < 5; x++) {
      if (data[y][x]) {
        leds[XY(POS_X + x, 7 + deltaY - y)] = LEAF_COLOR;
        if (WIDTH > 16) {
          leds[XY(POS_X - x, 15 - deltaY - y)] = LEAF_COLOR;
        }
      }
    }
  }
}

//---------------------------------------
void animeBobbles() {
  const uint32_t color = 0xF0F7FF;
  // сдвигаем Bobbles вверх ----
  for (uint8_t x = CENTER_X_MINOR; x < (CENTER_X_MINOR + 4); x++) {
    for (uint8_t y = HEIGHT; y > 0U; y--) {
      if (getPixColorXY(x, y - 1) == color) {
        drawPixelXY(x, y, color);
        drawPixelXY(x, y - 1, getPixColorXY(WIDTH - 1, y - 1));
      }
    }
  }
  // ----------------------
  if ( step % 4 == 0) {
    drawPixelXY(random8(CENTER_X_MINOR, CENTER_X_MINOR + 4), 0U, color);
    if ( step % 11 == 0) {
      drawPixelXY(random8(CENTER_X_MINOR, CENTER_X_MINOR + 4), 1U, color);
    }
  }
}

//---------------------------------------
void createScene(uint8_t idx) {
  switch (idx) {
    case 0:     // blue green ------
      gradientVertical(0, HEIGHT * 0.25, WIDTH, HEIGHT, 96, 160, 64, 255, 255U);
      gradientVertical(0, 0, WIDTH, HEIGHT * 0.25, 96, 96, 255, 64, 255U);
      break; //CENTER_Y_MINOR
    case 1:     // aquamarine green
      gradientVertical(0, 0, WIDTH, HEIGHT, 96, 130, 48, 255, 255U);
      break;
    case 2:     // blue aquamarine -
      gradientVertical(0, CENTER_Y_MINOR, WIDTH, HEIGHT, 170, 160, 100, 200, 255U);
      gradientVertical(0, 0, WIDTH, CENTER_Y_MINOR, 100, 170, 255, 100, 255U);
      break;
    case 3:     // yellow green ----
      gradientVertical(0, CENTER_Y_MINOR, WIDTH, HEIGHT, 100, 80, 60, 160, 255U);
      gradientVertical(0, 0, WIDTH, CENTER_Y_MINOR, 96, 100, 205, 60, 255U);
      break;
    case 4:     // sea green -------
      gradientVertical(0, floor(HEIGHT  * 0.3), WIDTH, HEIGHT, 120, 160, 64, 200, 255U);
      gradientVertical(0, 0, WIDTH, floor(HEIGHT  * 0.3), 120, 120, 225, 64, 255U);
      break;
    default:
      drawRec(0, 0, WIDTH, HEIGHT, 0x000050);
      break;
  }
  flora();
}

//---------------------------------------
void BotswanaRivers() {
  uint8_t divider;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                          scale | speed 210
      setModeSettings(1U + random8(252U), 20 + random8(180U));
    }
#endif
    loadingFlag = false;
    step = 0U;
    divider = floor((modes[currentMode].Scale - 1) / 20);       // маштаб задает смену палитры воды
    createScene(divider);
  }

  // restore scene after power on ---------
  if (getPixColorXY(0U, HEIGHT - 2) == CRGB::Black) {
    createScene(divider);
  }

  // LOG.printf_P(PSTR("%02d | hue2 = %03d | min = %03d \n\r"), step, hue2, deltaHue2);
  // -------------------------------------
  animeBobbles();
  step++;
}


// ============ Watercolor ==============
//      © SlingMaster | by Alex Dovby
//            EFF_WATERCOLOR
//               Акварель
//---------------------------------------
void SmearPaint(uint8_t obj[trackingOBJECT_MAX_COUNT]) {
  uint8_t divider;
  int temp;
  static const uint32_t colors[6][8] PROGMEM = {
    {0x2F0000,  0xFF4040, 0x6F0000, 0xAF0000, 0xff5f00, CRGB::Red, 0x480000, 0xFF0030},
    {0x002F00, CRGB::LawnGreen, 0x006F00, 0x00AF00, CRGB::DarkMagenta, 0x00FF00, 0x004800, 0x00FF30},
    {0x002F1F, CRGB::DarkCyan, 0x00FF7F, 0x007FFF, 0x20FF5F, CRGB::Cyan, 0x004848, 0x7FCFCF },
    {0x00002F, 0x5030FF, 0x00006F, 0x0000AF, CRGB::DarkCyan, 0x0000FF, 0x000048, 0x5F5FFF},
    {0x2F002F, 0xFF4040, 0x6F004A, 0xFF0030, CRGB::DarkMagenta, CRGB::Magenta, 0x480048, 0x3F00FF},
    {CRGB::Blue, CRGB::Red, CRGB::Gold, CRGB::Green, CRGB::DarkCyan, CRGB::DarkMagenta, 0x000000, 0xFF7F00 }
  };
  if (trackingObjectHue[5] == 1) {  // direction >>>
    obj[1]++;
    if (obj[1] >= obj[2]) {
      trackingObjectHue[5] = 0;     // swap direction
      obj[3]--;                     // new line
      if (step % 2 == 0) {
        obj[1]++;
      } else {
        obj[1]--;
      }

      obj[0]--;
    }
  } else {                          // direction <<<
    obj[1]--;
    if (obj[1] <= (obj[2] - obj[0])) {
      trackingObjectHue[5] = 1;     // swap direction
      obj[3]--;                     // new line
      if (obj[0] >= 1) {
        temp = obj[0] - 1;
        if (temp < 0) {
          temp = 0;
        }
        obj[0] = temp;
        obj[1]++;
      }
    }
  }

  if (obj[3] == 255) {
    deltaHue = 255;
  }

  divider = floor((modes[currentMode].Scale - 1) / 16.7);
  if ( (obj[1] >= WIDTH) || (obj[3] == obj[4]) ) {
    // deltaHue value == 255 activate -------
    // set new parameter for new smear ------
    deltaHue = 255;
  }
  drawPixelXY(obj[1], obj[3], colors[divider][hue]);

  // alternative variant without dimmer effect
  // uint8_t h = obj[3] - obj[4];
  // uint8_t br = 266 - 12 * h;
  // if (h > 0) {
  // drawPixelXY(obj[1], obj[3], makeDarker(colors[divider][hue], br));
  // } else {
  // drawPixelXY(obj[1], obj[3], makeDarker(colors[divider][hue], 240));
  // }
}



//---------------------------------------
void Watercolor() {
  // #define DIMSPEED (254U - 500U / WIDTH / HEIGHT)
  uint8_t divider;
  if (loadingFlag) {

#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                          scale | speed 250
      setModeSettings(1U + random8(252U), 1 + random8(250U));
    }
#endif
    loadingFlag = false;
    FastLED.clear();
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = deltaValue;                    // чтообы при старте эффекта сразу покрасить лампу
    hue = 0;
    deltaHue = 255;                       // last color
    trackingObjectHue[1] = floor(WIDTH * 0.25);
    trackingObjectHue[3] = floor(HEIGHT * 0.25);
  }

  if (step >= deltaValue) {
    step = 0U;
    // LOG.printf_P(PSTR("%03d | log: %f | val: %03d | divider: %d \n\r"), modes[currentMode].Brightness, log(modes[currentMode].Brightness), deltaHue2, divider);
  }

  // ******************************
  // set random parameter for smear
  // ******************************
  if (deltaHue == 255) {

    trackingObjectHue[0] = 4 + random8(floor(WIDTH * 0.25));                // width
    trackingObjectHue[1] = random8(WIDTH - trackingObjectHue[0]);           // x
    int temp =  trackingObjectHue[1] + trackingObjectHue[0];
    if (temp >= (WIDTH - 1)) {
      temp = WIDTH - 1;
      if (trackingObjectHue[1] > 1) {
        trackingObjectHue[1]--;
      } else {
        trackingObjectHue[1]++;
      }
    }
    trackingObjectHue[2] = temp;                                            // x end
    trackingObjectHue[3] = 3 + random8(HEIGHT - 4);                         // y
    temp = trackingObjectHue[3] - random8(3) - 3;
    if (temp <= 0) {
      temp = 0;
    }
    trackingObjectHue[4] = temp;                                            // y end
    trackingObjectHue[5] = 1;
    divider = floor((modes[currentMode].Scale - 1) / 16.7);                 // маштаб задает смену палитры
    hue = random8(8);
    //    if (step % 127 == 0) {
    //      LOG.printf_P(PSTR("BR %03d | SP %03d | SC %03d | divider %d | [ %d ]\n\r"), modes[currentMode].Brightness, modes[currentMode].Speed, modes[currentMode].Scale, divider, hue);
    //    }
    hue2 = 255;
    deltaHue = 0;
  }
  // ******************************
  SmearPaint(trackingObjectHue);

  // LOG.printf_P(PSTR("%02d | hue2 = %03d | min = %03d \n\r"), step, hue2, deltaHue2);
  // -------------------------------------
  //  if (custom_eff == 1) {
  // dimAll(DIMSPEED);
  if (step % 4 == 0) {
    blurScreen(beatsin8(1U, 1U, 6U));
    // blurRows(leds, WIDTH, 3U, 10U);
  }
  step++;
}

// =========== FeatherCandle ============
//         адаптация © SottNick
//    github.com/mnemocron/FeatherCandle
//      modify & design © SlingMaster
//           EFF_FEATHER_CANDLE
//                Свеча
//---------------------------------------
// FeatherCandle animation data
const uint8_t  level = 160;
const uint8_t  low_level = 110;
const uint8_t *ptr  = anim;                     // Current pointer into animation data
const uint8_t  w    = 7;                        // image width
const uint8_t  h    = 15;                       // image height
uint8_t        img[w * h];                      // Buffer for rendering image
uint8_t        deltaX = CENTER_X_MAJOR - 4;     // position img
uint8_t last_brightness;
uint8_t posY = (HEIGHT < 18) ? 0 : constrain(HEIGHT / 5, 3, HEIGHT);
void FeatherCandleRoutine() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    // brightness | scale | speed
    // { 21, 220,  40}
    setModeSettings(1U + random8(99U), 190U + random8(65U));
  }
#endif
  if (loadingFlag) {
    FastLED.clear();
    hue = 0;
    trackingObjectState[0] = low_level;
    trackingObjectState[1] = low_level;
    trackingObjectState[2] = low_level;
    trackingObjectState[4] = CENTER_X_MINOR;
    loadingFlag = false;
  }

  uint8_t a = pgm_read_byte(ptr++);     // New frame X1/Y1
  if (a >= 0x90) {                      // EOD marker? (valid X1 never exceeds 8)
    ptr = anim;                         // Reset animation data pointer to start
    a   = pgm_read_byte(ptr++);         // and take first value
  }
  uint8_t x1 = a >> 4;                  // X1 = high 4 bits
  uint8_t y1 = a & 0x0F;                // Y1 = low 4 bits
  a  = pgm_read_byte(ptr++);            // New frame X2/Y2
  uint8_t x2 = a >> 4;                  // X2 = high 4 bits
  uint8_t y2 = a & 0x0F;                // Y2 = low 4 bits

  // Read rectangle of data from anim[] into portion of img[] buffer
  for (uint8_t y = y1; y <= y2; y++)
    for (uint8_t x = x1; x <= x2; x++) {
      img[y * w + x] = pgm_read_byte(ptr++);
    }
  int i = 0;
  uint8_t color = (modes[currentMode].Scale - 1U) * 2.57;



  // draw flame -------------------
  for (uint8_t y = 1; y < h; y++) {
    if ((HEIGHT < 15) || (WIDTH < 9)) {
      // for small matrix -----
      if (y % 2 == 0) {
        leds[XY(CENTER_X_MINOR - 1, 7 + posY)] = CHSV(color, 255U, 55 + random8(200));
        leds[XY(CENTER_X_MINOR, 6 + posY)] = CHSV(color, 255U, 160 + random8(90));
        leds[XY(CENTER_X_MINOR + 1, 6 + posY)] = CHSV(color, 255U, 205 + random8(50));
        leds[XY(CENTER_X_MINOR - 1, 5 + posY)] = CHSV(color, 255U, 155 + random8(100));
        leds[XY(CENTER_X_MINOR, 5 + posY)] = CHSV(color - 10U , 255U, 120 + random8(130));
        leds[XY(CENTER_X_MINOR, 4 + posY)] = CHSV(color - 10U , 255U, 100 + random8(120));
      }
    } else {
      for (uint8_t x = 0; x < w; x++) {
        uint8_t brightness = img[i];
        leds[XY(deltaX + x, y + posY)] = CHSV(brightness > 240 ? color : color - 10U , 255U, brightness);
        i++;
      }
    }
    // blur2d(leds, WIDTH, HEIGHT, 25U);
    // draw body FeatherCandle ------

    if (y <= posY) {
      if (y % 2 == 0) {
        gradientVertical(0, 0, WIDTH, 2 + posY, color, color, 48, 112, 10U);
      }
    }

    // drops of wax move -------------
    switch (hue ) {
      case 0:
        if (trackingObjectState[0] < level) {
          trackingObjectState[0]++;
        }
        break;
      case 1:
        if (trackingObjectState[0] > low_level) {
          trackingObjectState[0] --;
        }
        if (trackingObjectState[1] < level) {
          trackingObjectState[1] ++;
        }
        break;
      case 2:
        if (trackingObjectState[1] > low_level) {
          trackingObjectState[1] --;
        }
        if (trackingObjectState[2] < level) {
          trackingObjectState[2] ++;
        }
        break;
      case 3:
        if (trackingObjectState[2] > low_level) {
          trackingObjectState[2] --;
        } else {
          hue++;
          // set random position drop of wax
          trackingObjectState[4] = CENTER_X_MINOR - 3 + random8(6);
        }
        break;
    }

    if (hue > 3) {
      hue++;
    } else {
      // LOG.printf_P(PSTR("[0] = %03d | [1] = %03d | [2] = %03d \n\r"), trackingObjectState[0], trackingObjectState[1], trackingObjectState[2]);
      if (hue < 2) {
        leds[XY(trackingObjectState[4], posY + 1)] = CHSV(50U, 30U, trackingObjectState[0]);
      }
      if ((hue == 1) || (hue == 2)) {
        leds[XY(trackingObjectState[4], posY)] = CHSV(50U, 15U, trackingObjectState[1]); // - 10;
      }
      if (hue > 1) {
        leds[XY(trackingObjectState[4], posY - 1)] = CHSV(50U, 5U, trackingObjectState[2]); // - 20;
      }
    }
  }

  // next -----------------
  if ((trackingObjectState[0] == level) || (trackingObjectState[1] == level) || (trackingObjectState[2] == level)) {
    hue++;
  }
}

// ============= Hourglass ==============
//             © SlingMaster
//             EFF_HOURGLASS
//             Песочные Часы
//---------------------------------------
void Hourglass() {
  const float SIZE = 0.4;
  const uint8_t h = floor(SIZE * HEIGHT);
  uint8_t posX = 0;
  const uint8_t topPos  = HEIGHT - h;
  const uint8_t route = HEIGHT - h - 1;
  const uint8_t STEP = 18U;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                          scale | speed 210
      setModeSettings(15U + random8(225U), random8(255U));
    }
#endif
    loadingFlag = false;
    pcnt = 0;
    deltaHue2 = 0;
    hue2 = 0;

    FastLED.clear();
    hue = modes[currentMode].Scale * 2.55;
    for (uint8_t x = 0U; x < ((WIDTH / 2)); x++) {
      for (uint8_t y = 0U; y < h; y++) {
        drawPixelXY(CENTER_X_MINOR - x, HEIGHT - y - 1, CHSV(hue, 255, 255 - x * STEP));
        drawPixelXY(CENTER_X_MAJOR + x, HEIGHT - y - 1, CHSV(hue, 255, 255 - x * STEP));
      }
    }
  }

  if (hue2 == 0) {
    posX = floor(pcnt / 2);
    uint8_t posY = HEIGHT - h - pcnt;
    // LOG.printf_P(PSTR("• [%03d] | posX %03d | deltaHue2 %03d | \n"), step, posX, deltaHue2);

    /* move sand -------- */
    if ((posY < (HEIGHT - h - 2)) && (posY > deltaHue2)) {
      drawPixelXY(CENTER_X_MAJOR, posY, CHSV(hue, 255, 255));
      drawPixelXY(CENTER_X_MAJOR, posY - 2, CHSV(hue, 255, 255));
      drawPixelXY(CENTER_X_MAJOR, posY - 4, CHSV(hue, 255, 255));

      if (posY < (HEIGHT - h - 3)) {
        drawPixelXY(CENTER_X_MAJOR, posY + 1, CHSV(hue, 255, 0 ));
      }
    }

    /* draw body hourglass */
    if (pcnt % 2 == 0) {
      drawPixelXY(CENTER_X_MAJOR - posX, HEIGHT - deltaHue2 - 1, CHSV(hue, 255, 0));
      drawPixelXY(CENTER_X_MAJOR - posX, deltaHue2, CHSV(hue, 255, 255 - posX * STEP));
    } else {
      drawPixelXY(CENTER_X_MAJOR + posX, HEIGHT - deltaHue2 - 1, CHSV(hue, 255, 0));
      drawPixelXY(CENTER_X_MAJOR + posX, deltaHue2, CHSV(hue, 255, 255 - posX * STEP));
    }

    if (pcnt > WIDTH - 1) {
      deltaHue2++;
      pcnt = 0;
      if (modes[currentMode].Scale > 95) {
        hue += 4U;
      }
    }

    pcnt++;
    if (deltaHue2 > h) {
      deltaHue2 = 0;
      hue2 = 1;
    }
  }
  // имитация переворота песочных часов
  if (hue2 > 0) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      for (uint8_t y = HEIGHT; y > 0U; y--) {
        drawPixelXY(x, y, getPixColorXY(x, y - 1U));
        drawPixelXY(x, y - 1, 0x000000);
      }
    }
    hue2++;
    if (hue2 > route) {
      hue2 = 0;
    }
  }
}

// ============== Spectrum ==============
//             © SlingMaster
//         source code © kostyamat
//                Spectrum
//---------------------------------------
void  Spectrum() {
  static const byte COLOR_RANGE = 32;
  static uint8_t customHue;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random8(1, 100U), random8(215, 255U) );
    }
#endif // #if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    ff_y = map(WIDTH, 8, 64, 310, 63);
    ff_z = ff_y;
    speedfactor = map(modes[currentMode].Speed, 1, 255, 32, 4); // _speed = map(speed, 1, 255, 128, 16);
    customHue = floor( modes[currentMode].Scale - 1U) * 2.55;
    FastLED.clear();
  }
  uint8_t color = customHue + hue;
  if (modes[currentMode].Scale == 100) {
    if (hue2++ & 0x01 && deltaHue++ & 0x01 && deltaHue2++ & 0x01) hue += 8;
    fillMyPal16_2(customHue + hue, modes[currentMode].Scale & 0x01);
  } else {
    color = customHue;
    fillMyPal16_2(customHue + AURORA_COLOR_RANGE - beatsin8(AURORA_COLOR_PERIOD, 0U, AURORA_COLOR_RANGE * 2), modes[currentMode].Scale & 0x01);
  }

  for (byte x = 0; x < WIDTH; x++) {
    if (x % 2 == 0) {
      leds[XY(x, 0)] = CHSV( color, 255U, 128U);
    }

    emitterX = ((random8(2) == 0U) ? 545. : 390.) / HEIGHT;
    for (byte y = 2; y < HEIGHT - 1; y++) {
      polarTimer++;
      leds[XY(x, y)] =
        ColorFromPalette(myPal,
                         qsub8(
                           inoise8(polarTimer % 2 + x * ff_z,
                                   y * 16 + polarTimer % 16,
                                   polarTimer / speedfactor
                                  ),
                           fabs((float)HEIGHT / 2 - (float)y) * emitterX
                         )
                        ) ;
    }
  }
}

// ============ Lotus Flower ============
//             © SlingMaster
//               EFF_LOTUS
//             Цветок Лотоса
//---------------------------------------
void  Flower() {
  uint8_t br;
  if (step < 128) {
    br = 255 - step;  // 255 >> 128
  } else {
    br = step;        // 128 >> 255
  }
  if (modes[currentMode].Scale > 10) {
    dimAll(90);
    hue = floor(modes[currentMode].Scale * 1.9) + floor(br / 4);
  } else {
    FastLED.clear();
    hue = step;
  }
  if (step > 190) {
    hue2 = validMinMax(hue - 64 + floor(br / 4), 190, 250);
  } else {
    hue2 = hue + 64 - floor(br / 4);
  }

  for (uint8_t x = 0U ; x < WIDTH ; x++) {
    if (x % 6 == 0) {
      gradientVertical( x - deltaValue, 2U, x + 1 - deltaValue, HEIGHT * 0.8 - floor((255 - br) / 24) - random8(2), hue, hue2, 255, floor(br * 0.5), 255U);
      gradientVertical( x + 3U - deltaValue, 0U, x + 4U - deltaValue, HEIGHT * 0.8 - floor(br / 24) + random8(3), hue, hue2, 255, floor((255 - br * 0.5)), 255U);
      drawPixelXY(x - deltaValue, 0, 0x005F00);
      if (x % 2 == 0) {
        drawPixelXY(x - deltaValue, 1, 0x007F00);
      }
    }
  }
}

//---------------------------------------
void LotusFlower() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), 50U + random8(190U));
    }
#endif
    loadingFlag = false;
    deltaValue = 0;
    step = deltaValue;
    deltaValue = 0;
    hue = 120;
    hue2 = 0;
    deltaHue = 0;
    deltaHue2 = 0;
    FastLED.clear();
  }

  Flower();
  if (deltaHue == 0) {               // rotation
    deltaValue--;
    if (deltaValue <= 0) {
      deltaValue = 3;
    }
  } else {
    deltaValue++;
    if (deltaValue >= 3) {
      deltaValue = 0;
    }
  }
  deltaHue2++;
  if (deltaHue2 >= 18) {           // swap direction rotate
    deltaHue2 = 0;
    deltaHue = (deltaHue == 0) ? 1 : 0;
  }
  step++;
}

// =========== Christmas Tree ===========
//             © SlingMaster
//           EFF_CHRISTMAS_TREE
//            Новогодняя Елка
//---------------------------------------
void clearNoiseArr() {
  for (uint8_t x = 0U; x < WIDTH; x++) {
    for (uint8_t y = 0U; y < HEIGHT; y++) {
      noise3d[0][x][y] = 0;
      noise3d[1][x][y] = 0;
    }
  }
}

//---------------------------------------
void VirtualSnow(byte snow_type) {
  uint8_t posX = random8(WIDTH - 1);
  const uint8_t maxX = WIDTH - 1;
  static int deltaPos;
  byte delta = (snow_type == 3) ? 0 : 1;
  for (uint8_t x = delta; x < WIDTH - delta; x++) {

    // заполняем случайно верхнюю строку
    if ((noise3d[0][x][HEIGHT - 2] == 0U) &&  (posX == x) && (random8(0, 2) == 0U)) {
      noise3d[0][x][HEIGHT] = 1;
    } else {
      noise3d[0][x][HEIGHT] = 0;
    }

    for (uint8_t y = 0U; y < HEIGHT; y++) {
      switch (snow_type) {
        case 0:
          noise3d[0][x][y] = noise3d[0][x][y + 1];
          deltaPos = 0;
          break;
        case 1:
        case 2:
          noise3d[0][x][y] = noise3d[0][x][y + 1];
          deltaPos = 1 - random8(2);
          break;
        default:
          deltaPos = -1;
          if ((x == 0 ) & (y == 0 ) & (random8(2) == 0U)) {
            noise3d[0][WIDTH - 1][random8(CENTER_Y_MAJOR / 2, HEIGHT - CENTER_Y_MAJOR / 4)] = 1;
          }
          if (x > WIDTH - 2) {
            noise3d[0][WIDTH - 1][y] = 0;
          }
          if (x < 1)  {
            noise3d[0][x][y] = noise3d[0][x][y + 1];
          } else {
            noise3d[0][x - 1][y] = noise3d[0][x][y + 1];
          }
          break;
      }

      if (noise3d[0][x][y] > 0) {
        if (snow_type < 3) {
          if (y % 2 == 0U) {
            leds[XY(x - ((x > 0) ? deltaPos : 0), y)] = CHSV(160, 5U, random8(200U, 240U));
          } else {
            leds[XY(x + deltaPos, y)] = CHSV(160, 5U,  random8(200U, 240U));
          }
        } else {
          leds[XY(x, y)] = CHSV(160, 5U,  random8(200U, 240U));
        }
      }
    }
  }
}

//---------------------------------------
void GreenTree(uint8_t tree_h) {
  hue = floor(step / 32) * 32U;

  for (uint8_t x = 0U; x < WIDTH + 1 ; x++) {
    if (x % 8 == 0) {
      if (modes[currentMode].Scale < 60) {
        // nature -----
        DrawLine(x - 1U - deltaValue, floor(tree_h * 0.70), x + 1U - deltaValue, floor(tree_h * 0.70), 0x002F00);
        DrawLine(x - 1U - deltaValue, floor(tree_h * 0.55), x + 1U - deltaValue, floor(tree_h * 0.55), 0x004F00);
        DrawLine(x - 2U - deltaValue, floor(tree_h * 0.35), x + 2U - deltaValue, floor(tree_h * 0.35), 0x005F00);
        DrawLine(x - 2U - deltaValue, floor(tree_h * 0.15), x + 2U - deltaValue, floor(tree_h * 0.15), 0x007F00);

        drawPixelXY(x - 3U - deltaValue, floor(tree_h * 0.15), 0x001F00);
        drawPixelXY(x + 3U - deltaValue, floor(tree_h * 0.15), 0x001F00);
        if ((x - deltaValue ) >= 0) {
          gradientVertical( x - deltaValue, 0U, x - deltaValue, tree_h, 90U, 90U, 190U, 64U, 255U);
        }
      } else {
        // holiday -----
        drawPixelXY(x - 1 - deltaValue, floor(tree_h * 0.6), CHSV(step, 255U, 128 + random8(128)));
        drawPixelXY(x + 1 - deltaValue, floor(tree_h * 0.6), CHSV(step, 255U, 128 + random8(128)));

        drawPixelXY(x - deltaValue, floor(tree_h * 0.4), CHSV(step, 255U, 200U));

        drawPixelXY(x - deltaValue, floor(tree_h * 0.2), CHSV(step, 255U, 190 + random8(65)));
        drawPixelXY(x - 2 - deltaValue, floor(tree_h * 0.25), CHSV(step, 255U, 96 + random8(128)));
        drawPixelXY(x + 2 - deltaValue, floor(tree_h * 0.25), CHSV(step, 255U, 96 + random8(128)));

        drawPixelXY(x - 2 - deltaValue, 1U, CHSV(step, 255U, 200U));
        drawPixelXY(x - deltaValue, 0U, CHSV(step, 255U, 250U));
        drawPixelXY(x + 2 - deltaValue, 1U, CHSV(step, 255U, 200U));
        if ((x - deltaValue) >= 0) {
          gradientVertical( x - deltaValue, floor(tree_h * 0.75), x - deltaValue, tree_h,  hue, hue, 250U, 0U, 128U);
        }
      }
    }
  }
}

//---------------------------------------
void ChristmasTree() {
  static uint8_t tree_h = HEIGHT;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), 10U + random8(128));
    }
#endif
    loadingFlag = false;
    clearNoiseArr();
    deltaValue = 0;
    step = deltaValue;
    FastLED.clear();

    if (HEIGHT > 16) {
      tree_h = 16;
    }
  }

  if (HEIGHT > 16) {
    if (modes[currentMode].Scale < 60) {
      gradientVertical(0, 0, WIDTH, HEIGHT, 160, 160, 64, 128, 255U);
    } else {
      FastLED.clear();
    }
  } else {
    FastLED.clear();
  }
  GreenTree(tree_h);

  if (modes[currentMode].Scale < 60) {
    VirtualSnow(1);
  }
  if (modes[currentMode].Scale > 30) {
    deltaValue++;
  }
  if (deltaValue >= 8) {
    deltaValue = 0;
  }
  step++;
}

// ============== ByEffect ==============
//             © SlingMaster
//             EFF_BY_EFFECT
//            Побочный Эффект
// --------------------------------------
void ByEffect() {
  uint8_t saturation;
  uint8_t delta;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed 210
      setModeSettings(random8(100U), random8(200U));
    }
#endif
    loadingFlag = false;
    deltaValue = 0;
    step = deltaValue;
    FastLED.clear();
  }

  hue = floor(step / 32) * 32U;
  dimAll(180);
  // ------
  saturation = 255U;
  delta = 0;
  for (uint8_t x = 0U; x < WIDTH + 1 ; x++) {
    if (x % 8 == 0) {
      gradientVertical( x - deltaValue, floor(HEIGHT * 0.75), x + 1U - deltaValue, HEIGHT,  hue, hue + 2, 250U, 0U, 255U);
      if (modes[currentMode].Scale > 50) {
        delta = random8(200U);
      }
      drawPixelXY(x - 2 - deltaValue, floor(HEIGHT * 0.7), CHSV(step, saturation - delta, 128 + random8(128)));
      drawPixelXY(x + 2 - deltaValue, floor(HEIGHT * 0.7), CHSV(step, saturation, 128 + random8(128)));

      drawPixelXY(x - deltaValue, floor(HEIGHT * 0.6), CHSV(hue, 255U, 190 + random8(65)));
      if (modes[currentMode].Scale > 50) {
        delta = random8(200U);
      }
      drawPixelXY(x - 1 - deltaValue, CENTER_Y_MINOR, CHSV(step, saturation, 128 + random8(128)));
      drawPixelXY(x + 1 - deltaValue, CENTER_Y_MINOR, CHSV(step, saturation - delta, 128 + random8(128)));

      drawPixelXY(x - deltaValue, floor(HEIGHT * 0.4), CHSV(hue, 255U, 200U));
      if (modes[currentMode].Scale > 50) {
        delta = random8(200U);
      }
      drawPixelXY(x - 2 - deltaValue, floor(HEIGHT * 0.3), CHSV(step, saturation - delta, 96 + random8(128)));
      drawPixelXY(x + 2 - deltaValue, floor(HEIGHT * 0.3), CHSV(step, saturation, 96 + random8(128)));

      gradientVertical( x - deltaValue, 0U, x + 1U - deltaValue, floor(HEIGHT * 0.25),  hue + 2, hue, 0U, 250U, 255U);

      if (modes[currentMode].Scale > 50) {
        drawPixelXY(x + 3 - deltaValue, HEIGHT - 3U, CHSV(step, 255U, 255U));
        drawPixelXY(x - 3 - deltaValue, CENTER_Y_MINOR, CHSV(step, 255U, 255U));
        drawPixelXY(x + 3 - deltaValue, 2U, CHSV(step, 255U, 255U));
      }
    }
  }
  // ------
  deltaValue++;
  if (deltaValue >= 8) {
    deltaValue = 0;
  }
  step++;
}


// =====================================
//            Строб Хаос Дифузия
//          Strobe Haos Diffusion
//             © SlingMaster
// =====================================
/*должен быть перед эффектом Матрицf бегунок Скорость не регулирует задержку между кадрами,
  но меняет частоту строба*/
void StrobeAndDiffusion() {
  const uint8_t SIZE = 3U - custom_eff;
  const uint8_t DELTA = 1U;         // центровка по вертикали
  uint8_t STEP = 2U;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(1U + random8(100U), 1U + random8(150U));
    }
#endif
    loadingFlag = false;
    FPSdelay = 25U; // LOW_DELAY;
    hue2 = 1;
    clearNoiseArr();
    FastLED.clear();
  }

  STEP = floor((255 - modes[currentMode].Speed) / 64) + 1U; // for strob
  if (modes[currentMode].Scale > 50) {
    // diffusion ---
    blurScreen(beatsin8(3, 64, 80));
    FPSdelay = LOW_DELAY;
    STEP = 1U;
    if (modes[currentMode].Scale < 75) {
      // chaos ---
      FPSdelay = 30;
      VirtualSnow(0);
    }

  } else {
    // strob -------
    if (modes[currentMode].Scale > 25) {
      dimAll(200);
      FPSdelay = 30;
    } else {
      dimAll(240);
      FPSdelay = 40;
    }
  }

  const uint8_t rows = (HEIGHT + 1) / SIZE;
  deltaHue = floor(modes[currentMode].Speed / 64) * 64;
  bool dir = false;
  for (uint8_t y = 0; y < rows; y++) {
    if (dir) {
      if ((step % STEP) == 0) {   // small layers
        drawPixelXY(WIDTH - 1, y * SIZE + DELTA, CHSV(step, 255U, 255U ));
      } else {
        drawPixelXY(WIDTH - 1, y * SIZE + DELTA, CHSV(170U, 255U, 1U));
      }
    } else {
      if ((step % STEP) == 0) {   // big layers
        drawPixelXY(0, y * SIZE + DELTA, CHSV((step + deltaHue), 255U, 255U));
      } else {
        drawPixelXY(0, y * SIZE + DELTA, CHSV(0U, 255U, 0U));
      }
    }

    // сдвигаем слои  ------------------
    for (uint8_t x = 0U ; x < WIDTH; x++) {
      if (dir) {  // <==
        drawPixelXY(x - 1, y * SIZE + DELTA, getPixColorXY(x, y * SIZE + DELTA));
      } else {    // ==>
        drawPixelXY(WIDTH - x, y * SIZE + DELTA, getPixColorXY(WIDTH - x - 1, y * SIZE + DELTA));
      }
    }
    dir = !dir;
  }

  if (hue2 == 1) {
    step ++;
    if (step >= 254) hue2 = 0;
  } else {
    step --;
    if (step < 1) hue2 = 1;
  }
}

// =====================================
//               Фейерверк
//                Firework
//             © SlingMaster
// =====================================
void VirtualExplosion(uint8_t f_type, int8_t timeline) {
  const uint8_t DELAY_SECOND_EXPLOSION = HEIGHT * 0.25;
  uint8_t horizont = 1U; // HEIGHT * 0.2;
  const int8_t STEP = 255 / HEIGHT;
  uint8_t firstColor = random8(255);
  uint8_t secondColor = 0;
  uint8_t saturation = 255U;
  switch (f_type) {
    case 0:
      secondColor =  random(50U, 255U);
      saturation = random(245U, 255U);
      break;
    case 1: /* сакура */
      firstColor = random(210U, 230U);
      secondColor = random(65U, 85U);
      saturation = 255U;
      break;
    case 2: /* день Независимости */
      firstColor = random(160U, 170U);
      secondColor = random(25U, 50U);
      saturation = 255U;
      break;
    default: /* фризантемы */
      firstColor = random(30U, 40U);
      secondColor = random(25U, 50U);
      saturation = random(128U, 255U);
      break;
  }
  if ((timeline > HEIGHT - 1 ) & (timeline < HEIGHT * 1.75)) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      for (uint8_t y =  horizont; y < HEIGHT - 1; y++) {
        noise3d[0][x][y] = noise3d[0][x][y + 1];
        uint8_t bri = y * STEP;
        if (noise3d[0][x][y] > 0) {
          if (timeline > (HEIGHT + DELAY_SECOND_EXPLOSION) ) {
            /* second explosion */
            drawPixelXY((x - 2 + random8(4)), y - 1, CHSV(secondColor + random8(16), saturation, bri));
          }
          if (timeline < ((HEIGHT - DELAY_SECOND_EXPLOSION) * 1.75) ) {
            /* first explosion */
            drawPixelXY(x, y, CHSV(firstColor, 255U, bri));
          }
        } else {
          // drawPixelXY(x, y, CHSV(175, 255U, floor((255 - bri) / 4)));
        }
      }
    }
    uint8_t posX = random8(WIDTH);
    for (uint8_t x = 0U; x < WIDTH; x++) {
      // заполняем случайно верхнюю строку
      if (posX == x) {
        if (step % 2 == 0) {
          noise3d[0][x][HEIGHT - 1U] = 1;
        } else {
          noise3d[0][x][HEIGHT - 1U]  = 0;
        }
      } else {
        noise3d[0][x][HEIGHT - 1U]  = 0;
      }
    }
  }
}

// --------------------------------------
void Firework() {
  const uint8_t MAX_BRIGHTNESS = 40U;            /* sky brightness */
  const uint8_t DOT_EXPLOSION = HEIGHT * 0.95;
  const uint8_t HORIZONT = HEIGHT * 0.25;
  const uint8_t DELTA = 1U;                      /* центровка по вертикали */
  const float stepH = HEIGHT / 128.0;
  const uint8_t FPS_DELAY = 20U;
  const uint8_t STEP = 3U;
  const uint8_t skyColor = 156U;
  uint8_t sizeH;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(1U + random8(100U), 1U + random8(250U));
    }
#endif
    loadingFlag = false;
    deltaHue2 = 0;
    FPSdelay = 255U;
    clearNoiseArr();
    FastLED.clear();
    step = 0U;
    deltaHue2 = floor(modes[currentMode].Scale / 26);
    hue = 48U;            // skyBright
    sizeH = HEIGHT;
    if (modes[currentMode].Speed > 85U) {
      sizeH = HORIZONT;
      FPSdelay = FPS_DELAY;
    }
    if (modes[currentMode].Speed <= 85U) {
      gradientVertical(0, 0, WIDTH, HEIGHT,  skyColor,  skyColor, 96U, 0U, 255U);
    }
  }
  if (FPSdelay == 240U) {
    FPSdelay = FPS_DELAY;
  }
  if (FPSdelay > 230U) {
    //  if (FPSdelay > 128U) {
    /* вечерело */
    FPSdelay--;
    sizeH = (FPSdelay - 128U) * stepH;

    if (modes[currentMode].Speed <= 85U) {
      dimAll(225U);
      return;
    }
    if (sizeH > HORIZONT)  {
      dimAll(200);
      return;
    }
    if (sizeH == HORIZONT )  FPSdelay = FPS_DELAY;
  }

  if (step > DOT_EXPLOSION ) {
    blurScreen(beatsin8(3, 64, 80));
    //    FastLED.setBrightness(250);
  }
  if (step == DOT_EXPLOSION - 1) {
    /* включаем фазу затухания */
    FPSdelay = 70;
  }
  if (step > CENTER_Y_MAJOR) {
    dimAll(140);
  } else {
    dimAll(100);
  }


  /* ============ draw sky =========== */
  if ((modes[currentMode].Speed > 85U) & (modes[currentMode].Speed < 180U)) {
    gradientVertical(0, 0, WIDTH, HORIZONT, skyColor, skyColor, 48U, 0U, 255U);
  }

  /* deltaHue2 - Firework type */
  VirtualExplosion(deltaHue2, step);

  if ((step > DOT_EXPLOSION ) & (step < HEIGHT * 1.5)) {
    /* фаза взрыва */
    FPSdelay += 5U;
  }
  const uint8_t rows = (HEIGHT + 1) / 3U;
  deltaHue = floor(modes[currentMode].Speed / 64) * 64;
  if (step > CENTER_Y_MAJOR) {
    bool dir = false;
    for (uint8_t y = 0; y < rows; y++) {
      /* сдвигаем слои / эмитация разлета */
      for (uint8_t x = 0U ; x < WIDTH; x++) {
        if (dir) {  // <==
          drawPixelXY(x - 1, y * 3 + DELTA, getPixColorXY(x, y * 3 + DELTA));
        } else {    // ==>
          drawPixelXY(WIDTH - x, y * 3 + DELTA, getPixColorXY(WIDTH - x - 1, y * 3 + DELTA));
        }
      }
      dir = !dir;
      /* --------------------------------- */
    }
  }

  /* ========== фаза полета ========== */
  if (step < DOT_EXPLOSION ) {
    FPSdelay ++;
    if (HEIGHT < 20) {
      FPSdelay ++;
    }
    /* закоментируйте следующие две строки если плоская лампа
      подсветка заднего фона */
    if (custom_eff == 1) {
      DrawLine(0U, 0U, 0U, HEIGHT - step, CHSV(skyColor, 255U, 32U));
      DrawLine(WIDTH - 1, 0U, WIDTH - 1U, HEIGHT - step, CHSV(skyColor, 255U, 32U));
    }
    /* ------------------------------------------------------ */

    uint8_t saturation = (step > (DOT_EXPLOSION - 2U)) ? 192U : 20U;
    uint8_t rndPos = 3U * deltaHue2 * 0.5;
    drawPixelXY(CENTER_X_MINOR + rndPos, step,  CHSV(50U, saturation, 80U));                 // first
    drawPixelXY(CENTER_X_MAJOR + 1 - rndPos, step - HORIZONT,  CHSV(50U, saturation, 80U));  // second
    if (rndPos > 1) {
      drawPixelXY(CENTER_X_MAJOR + 4 - rndPos, step - HORIZONT + 2,  CHSV(50U, saturation, 80U));// three
    }
    /* sky brightness */
    if (hue > 2U) {
      hue -= 1U;
    }
  }
  if (step > HEIGHT * 1.25) {
    /* sky brightness */
    if (hue < MAX_BRIGHTNESS) {
      hue += 2U;
    }
  }

  if (step >= (HEIGHT * 2.0)) {
    step = 0U;
    // LOG.printf_P(PSTR("• Bright • [%03d]\n"), FastLED.getBrightness());
    FPSdelay = FPS_DELAY;
    if (modes[currentMode].Scale < 5) {
      deltaHue2++;
    }
    if (deltaHue2 >= 4U) deltaHue2 = 0U;  // next Firework type
  }
  // LOG.printf_P(PSTR("• [%03d] | %03d | sky Bright • [%03d]\n"), step, FPSdelay, hue);
  step ++;
}

// =====================================
//             Планета Земля
//              PlanetEarth
//             © SlingMaster
// =====================================
void PlanetEarth() {
  static uint16_t imgW = 0;
  static uint16_t imgH = 0;
  if (HEIGHT < 16U) {
    return;
  }
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(128U, 10U + random8(230U));
    }
#endif
    loadingFlag = false;
    FPSdelay = 96U;
    FastLED.clear();
    String file_name = (modes[currentMode].Scale < 50) ? "globe0" : (HEIGHT >= 24U) ? "globe_big" : "globe1";
    readBinFile("bin/" + file_name + ".img", 4112 );

    imgW = getSizeValue(binImage, 8 );
    imgH = getSizeValue(binImage, 10 );

#ifdef GENERAL_DEBUG
    LOG.printf_P(PSTR("Image • %03d x %02d px\n"), imgW, imgH);
#endif
    scrollImage(imgW, imgH, 0U);
    ff_x = 1U;
  }

  /* scrool index reverse --> */
  // if (ff_x < 1) ff_x = (imgW - imgH);
  ff_x--;
  //  if (ff_x < 1) ff_x = (imgW - 1);
  if (ff_x == 0) {
    scrollImage(imgW, imgH, 0U);
    ff_x = imgW;
  } else {
    scrollImage(imgW, imgH, ff_x);
  }

  /* <-- scrool index ------- */
  //  if (ff_x > (imgW - imgH)) ff_x = 1U;
  //  scrollImage(imgW, imgH, ff_x - 1);
  //  ff_x++;
}

// =====================================
//             Мечта Дизайнера
//                WebTools
//             © SlingMaster
// =====================================
/* --------------------------------- */
int getRandomPos(uint8_t STEP, int prev) {
  uint8_t val = floor(random(0, (STEP * 16 - WIDTH - 1)) / STEP) * STEP;
  /* исключении небольшого поворота */
  if (abs(val - abs(prev)) > (STEP * 3)) {
    return - val;
  } else {
    return - (val + STEP * 3);
  }
}

/* --------------------------------- */
int getHue(uint8_t x, uint8_t y) {
  return ( x * 32 +  y * 24U );
}

/* --------------------------------- */
uint8_t getSaturationStep() {
  return (modes[currentMode].Speed > 170U) ? ((HEIGHT > 24) ? 12 : 24) : 0;
}

/* --------------------------------- */
uint8_t getBrightnessStep() {
  return (modes[currentMode].Speed < 85U) ? ((HEIGHT > 24) ? 16 : 24) : 0;
}

/* --------------------------------- */
void drawPalette(int posX, int posY, uint8_t STEP) {
  int PX, PY;
  const uint8_t SZ = STEP - 1;
  const uint8_t maxY = floor(HEIGHT / SZ);
  uint8_t sat = getSaturationStep();
  uint8_t br  = getBrightnessStep();

  FastLED.clear();
  for (uint8_t y = 0; y < maxY; y++) {
    for (uint8_t x = 0; x < 16; x++) {
      PY = y * STEP;
      PX = posX + x * STEP;
      if ((PX >= - STEP ) && (PY >= - STEP) && (PX < WIDTH) && (PY < HEIGHT)) {
        // LOG.printf_P(PSTR("y: %03d | br • %03d | sat • %03d\n"), y, (240U - br * y), sat);
        drawRecCHSV(PX, PY, PX + SZ, PY + SZ, CHSV( getHue(x, y), (255U - sat * y), (240U - br * y)));
      }
    }
  }
}

/* --------------------------------- */
void selectColor(uint8_t sc) {
  uint8_t offset = (WIDTH >= 16) ? WIDTH * 0.25 : 0;
  hue = getHue(random(offset, WIDTH - offset), random(HEIGHT));
  uint8_t sat = getSaturationStep();
  uint8_t br  = getBrightnessStep();

  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = offset; x < (WIDTH - offset); x++) {
      CHSV curColor = CHSV(hue, (255U - sat * y), (240U - br * y));
      if (curColor == getPixColorXY(x, y)) {
        /* show srlect color */
        drawRecCHSV(x, y, x + sc, y + sc, CHSV( hue, 64U, 255U));
        FastLED.show();
        delay(400);
        drawRecCHSV(x, y, x + sc, y + sc, CHSV( hue, 255U, 255U));
        y = HEIGHT;
        x = WIDTH;
      }
    }
  }
}

/* --------------------------------- */
void WebTools() {
  const uint8_t FPS_D = 10U;
  static uint8_t STEP = 3U;
  static int posX = -STEP;
  static int posY = 0;
  static int nextX = -STEP * 2;
  static bool stop_moving = true;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      // setModeSettings(random(10U, 90U), random(10U, 245U));
    }
#endif
    loadingFlag = false;
    FPSdelay = 1U;
    step = 0;
    STEP = 2U + floor(modes[currentMode].Scale / 35);
    posX = 0;
    posY = 0;
    drawPalette(posX, posY, STEP);
  }
  /* auto scenario */
  switch (step) {
    case 0:     /* restart ----------- */
      nextX = 0;
      FPSdelay = FPS_D;
      break;
    case 64:    /* start move -------- */
      nextX = getRandomPos(STEP, nextX);
      FPSdelay = FPS_D;
      break;
    case 100:    /* find -------------- */
      nextX = getRandomPos(STEP, nextX);
      FPSdelay = FPS_D;
      break;
    case 150:    /* find 2 ----------- */
      nextX = getRandomPos(STEP, nextX);
      FPSdelay = FPS_D;
      break;
    case 200:    /* find 3 ----------- */
      nextX = getRandomPos(STEP, nextX);
      FPSdelay = FPS_D;
      break;
    case 220:   /* select color ------ */
      FPSdelay = 200U;
      selectColor(STEP - 1);
      break;
    case 222:   /* show color -------- */
      FPSdelay = FPS_D;
      nextX = WIDTH;
      break;
  }
  if (posX < nextX) posX++;
  if (posX > nextX) posX--;

  if (stop_moving)   {
    FPSdelay = 80U;
    step++;
  } else {
    drawPalette(posX, posY, STEP);
    if ((nextX == WIDTH) || (nextX == 0)) {
      /* show select color bar gradient */
      // LOG.printf_P(PSTR("step: %03d | Next x: %03d • %03d | fps %03d\n"), step, nextX, posX, FPSdelay);
      if (posX > 1) {
        gradientHorizontal(0, 0, (posX - 1), HEIGHT, hue, hue, 255U, 96U, 255U);
      }
      if (posX > 3) DrawLine(posX - 3, CENTER_Y_MINOR, posX - 3, CENTER_Y_MAJOR, CHSV( hue, 192U, 255U));
    }
  }

  stop_moving = (posX == nextX);
}

// =====================================
//                Contacts
//             © Yaroslaw Turbin
//        Adaptation © SlingMaster
// =====================================

void Contacts() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random(25U, 90U), random(30U, 240U));
    }
#endif
    loadingFlag = false;
    FPSdelay = 80U;
    FastLED.clear();
  }

  int a = millis() / floor((255 - modes[currentMode].Speed) / 10);
  hue = floor(modes[currentMode].Scale / 17);
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      int index = XY(x, y);
      uint8_t color1 = exp_gamma[sin8((x - 8) * cos8((y + 20) * 4) / 4)];
      uint8_t color2 = exp_gamma[(sin8(x * 16 + a / 3) + cos8(y * 8 + a / 2)) / 2];
      uint8_t color3 = exp_gamma[sin8(cos8(x * 8 + a / 3) + sin8(y * 8 + a / 4) + a)];
      if (hue == 0) {
        leds[index].b = color3 / 4;
        leds[index].g = color2;
        leds[index].r = 0;
      } else if (hue == 1) {
        leds[index].b = color1;
        leds[index].g = 0;
        leds[index].r = color3 / 4;
      } else if (hue == 2) {
        leds[index].b = 0;
        leds[index].g = color1 / 4;
        leds[index].r = color3;
      } else if (hue == 3) {
        leds[index].b = color1;
        leds[index].g = color2;
        leds[index].r = color3;
      } else if (hue == 4) {
        leds[index].b = color3;
        leds[index].g = color1;
        leds[index].r = color2;
      } else if (hue == 5) {
        leds[index].b = color2;
        leds[index].g = color3;
        leds[index].r = color1;
      }
    }
  }
}

// ============ Magic Lantern ===========
//             © SlingMaster
//            Чарівний Ліхтар
// --------------------------------------
void MagicLantern() {
  static uint8_t saturation;
  static uint8_t brightness;
  static uint8_t low_br;
  uint8_t delta;
  const uint8_t PADDING = HEIGHT * 0.25;
  const uint8_t WARM_LIGHT = 55U;
  const uint8_t STEP = 4U;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed 210
      setModeSettings(random8(100U), random8(40, 200U));
    }
#endif
    loadingFlag = false;
    deltaValue = 0;
    step = deltaValue;
    if (modes[currentMode].Speed > 52) {
      brightness = map(modes[currentMode].Speed, 1, 255, 50U, 250U);
      low_br = 60U;
    } else {
      brightness = 0U;
      low_br = 0U;
    }
    saturation = (modes[currentMode].Scale > 50U) ? 64U : 0U;
    if (abs (70 - modes[currentMode].Scale) <= 5) saturation = 170U;
    FastLED.clear();

  }
  dimAll(150);
  hue = (modes[currentMode].Scale > 95) ? floor(step / 32) * 32U : modes[currentMode].Scale * 2.55;

  // ------
  for (uint8_t x = 0U; x < WIDTH ; x++) {
    // light ---
    if (low_br > 0) {

      gradientVertical( x - deltaValue, CENTER_Y_MAJOR, x - deltaValue, HEIGHT - PADDING - 1,  WARM_LIGHT, WARM_LIGHT, brightness, low_br, saturation);
      gradientVertical( x - deltaValue, PADDING + 1, x - deltaValue, CENTER_Y_MAJOR, WARM_LIGHT, WARM_LIGHT, low_br, brightness, saturation);
    } else {
      if (x % (STEP + 1) == 0) {
        leds[XY(random8(WIDTH), random8(PADDING + 2, HEIGHT - PADDING - 2))] = CHSV(step - 32U, random8(128U, 255U), 255U);
      }
      if ((modes[currentMode].Speed < 25) & (low_br == 0)) {
        deltaValue = 0;
        // body static --
        if (x % 2 != 0) {
          gradientVertical( x - deltaValue, HEIGHT - PADDING, x - deltaValue, HEIGHT, hue, hue, 64U, 40U, 255U);
          gradientVertical( (WIDTH - x - deltaValue), 0U, (WIDTH - x - deltaValue), PADDING, hue, hue, 40U, 64U, 255U);
        }
      }
    }

    if ((x % STEP == 0) | (x ==  (WIDTH - 1))) {
      // body --
      gradientVertical( x - deltaValue, HEIGHT - PADDING, x - deltaValue, HEIGHT, hue, hue, 2554U, 32U, 255U);
      gradientVertical( (WIDTH - x + deltaValue), 0U,  (WIDTH - x + deltaValue), PADDING, hue, hue, 32U, 255U, 255U);
    }
  }
  // ------

  deltaValue++;
  if (deltaValue >= STEP) {
    deltaValue = 0;
  }

  step++;
}
// ============ Plasma Waves ============
//              © Руслан Ус
//        Adaptation © SlingMaster
//             Плазмові Хвилі
// --------------------------------------

void PlasmaWaves() {
  static int64_t frameCount = 0;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), random8(40, 200U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    hue = modes[currentMode].Scale;
  }
  EVERY_N_MILLISECONDS(1000 / 60) {
    frameCount++;
  }

  uint8_t t1 = cos8((42 * frameCount) / 30);
  uint8_t t2 = cos8((35 * frameCount) / 30);
  uint8_t t3 = cos8((38 * frameCount) / 30);

  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      // Calculate 3 seperate plasma waves, one for each color channel
      uint8_t r = cos8((x << 3) + (t1 >> 1) + cos8(t2 + (y << 3)));
      uint8_t g = cos8((y << 3) + t1 + cos8((t3 >> 2) + (x << 3)));
      uint8_t b = cos8((y << 3) + t2 + cos8(t1 + x + (g >> 2)));

      // uncomment the following to enable gamma correction
      // r = pgm_read_byte_near(exp_gamma + r);
      r = exp_gamma[r];
      g = exp_gamma[g];
      b = exp_gamma[b];

      // g = pgm_read_byte_near(exp_gamma + g);
      // b = pgm_read_byte_near(exp_gamma + b);

      leds[XY(x, y)] = CRGB(r, g, b);
    }
    hue++;
  }
  // blurScreen(beatsin8(3, 64, 80));
}

// ============== Hand Fan ==============
//           на основі коду від
//          © mastercat42@gmail.com
//             © SlingMaster
//                Опахало
// --------------------------------------

void HandFan() {
  const uint8_t V_STEP = 255 / (HEIGHT + 9);
  static uint8_t val_scale;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), random8(210, 255U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    hue = modes[currentMode].Scale * 2.55;
    val_scale = map(modes[currentMode].Speed, 1, 255, 200U, 255U);;
  }
  for (int index = 0; index < NUM_LEDS; index++) {
    leds[index].nscale8(val_scale);
  }

  for (int i = 0; i < HEIGHT; i++) {
    int tmp = sin8(i + (millis() >> 4));
    tmp = map8(tmp, 2, WIDTH - 2);

    leds[XY(WIDTH - tmp, i)] = CHSV(hue, V_STEP * i + 32, 205U);
    leds[XY(WIDTH - tmp - 1, i)] = CHSV(hue, 255U, 255 - V_STEP * i);
    leds[XY(WIDTH - tmp + 1, i)] = CHSV(hue, 255U, 255 - V_STEP * i);

    if ((i % 6 == 0) & (modes[currentMode].Scale > 95U)) {
      hue++;
    }
  }
}

// =============== Bamboo ===============
//             © SlingMaster
//                 Бамбук
// --------------------------------------
uint8_t nextColor(uint8_t posY, uint8_t base, uint8_t next ) {
  const byte posLine = (HEIGHT > 16) ? 4 : 3;
  if ((posY + 1 == posLine) | (posY == posLine)) {
    return next;
  } else {
    return base;
  }
}

// --------------------------------------
void Bamboo() {
  const uint8_t gamma[7] = {0, 32, 144, 160, 196, 208, 230};
  static float index;
  const byte DELTA = 4U;
  const uint8_t VG_STEP = 64U;
  const uint8_t V_STEP = 32U;
  const byte posLine = (HEIGHT > 16) ? 4 : 3;
  const uint8_t SX = 5;
  const uint8_t SY = 10;
  static float deltaX = 0;
  static bool direct = false;
  uint8_t posY;
  static uint8_t colLine;
  const float STP = 0.2;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), random8(128, 255U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    index = STP;
    uint8_t idx = map(modes[currentMode].Scale, 5, 95, 0U, 6U);;
    colLine = gamma[idx];
    step = 0U;
  }

  // *** ---
  for (int y = 0; y < HEIGHT + SY; y++) {
    if (modes[currentMode].Scale < 50U) {
      if (step % 128 == 0U) {
        deltaX += STP * ((direct) ? -1 : 1);
        if ((deltaX > 1) | (deltaX < -1)) direct = !direct;
      }
    } else {
      deltaX = 0;
    }
    posY = y;
    for (int x = 0; x < WIDTH + SX; x++) {
      if (y == posLine) {
        drawPixelXYF(x , y - 1, CHSV(colLine, 255U, 128U));
        drawPixelXYF(x, y, CHSV(colLine, 255U, 96U));
        if (HEIGHT > 16) {
          drawPixelXYF(x, y - 2, CHSV(colLine, 10U, 64U));
        }
      }
      if ((x % SX == 0U) & (y % SY == 0U)) {
        for (int i = 1; i < (SY - 3); i++) {
          if (i < 3) {
            posY = y - i + 1 - DELTA + index;
            drawPixelXYF(x - 3 + deltaX, posY, CHSV(nextColor(posY, 96, colLine), 255U, 255 - V_STEP * i));
            posY = y - i + index;
            drawPixelXYF(x + deltaX, posY, CHSV(nextColor(posY, 96, colLine), 255U, 255 - VG_STEP * i));
          }
          posY = y - i - DELTA + index;
          drawPixelXYF(x - 4 + deltaX, posY , CHSV(nextColor(posY, 96, colLine), 180U, 255 - V_STEP * i));
          posY = y - i + 1 + index;
          drawPixelXYF(x - 1 + deltaX, posY , CHSV(nextColor(posY, ((i == 1) ? 96 : 80), colLine), 255U, 255 - V_STEP * i));
        }
      }
    }
    step++;
  }
  if (index >= SY)  {
    index = 0;
  }
  fadeToBlackBy(leds, NUM_LEDS, 60);
  index += STP;
}

// ============ Light Filter ============
//             © SlingMaster
//              Cвітлофільтр
// --------------------------------------
void LightFilter() {
  static int64_t frameCount =  0;
  const byte END = WIDTH - 1;
  static byte dX;
  static bool direct;
  static byte divider;
  static byte deltaValue = 0;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), random8(40, 160U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;

    divider = floor(modes[currentMode].Scale / 25);
    direct = true;
    dX = 1;
    pcnt = 0;
    frameCount = 0;
    hue2 == 32;
    clearNoiseArr();
    FastLED.clear();
  }

  // EVERY_N_MILLISECONDS(1000 / 30) {
  frameCount++;
  pcnt++;
  // }

  uint8_t t1 = cos8((42 * frameCount) / 30);
  uint8_t t2 = cos8((35 * frameCount) / 30);
  uint8_t t3 = cos8((38 * frameCount) / 30);
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  if (direct) {
    if (dX < END) {
      dX++;
    }
  } else {
    if (dX > 0) {
      dX--;
    }
  }
  if (pcnt > 128) {
    pcnt = 0;
    direct = !direct;
    if (divider > 2) {
      if (dX == 0) {
        deltaValue++;
        if (deltaValue > 2) {
          deltaValue = 0;
        }
      }
    } else {
      deltaValue = divider;
    }

  }

  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      if (x != END - dX) {
        r = cos8((y << 3) + (t1 >> 1) + cos8(t2 + (x << 3)));
        g = cos8((y << 3) + t1 + cos8((t3 >> 2) + (x << 3)));
        b = cos8((y << 3) + t2 + cos8(t1 + x + (g >> 2)));

      } else {
        // line gold -------
        r = 255U;
        g = 255U;
        b = 255U;
      }

      uint8_t val = dX * 8;
      switch (deltaValue) {
        case 0:
          if (r > val) {
            r = r - val;
          } else {
            r = 0;
          }
          if (g > val) {
            g = g - val / 2;
          } else {
            g = 0;
          }
          break;
        case 1:
          if (g > val) {
            g = g - val;
          } else {
            g = 0;
          }
          if (b > val) {
            b = b - val / 2;
          } else {
            b = 0;
          }
          break;
        case 2:
          if (b > val) {
            b = b - val;
          } else {
            b = 0;
          }
          if (r > val) {
            r = r - val / 2;
          } else {
            r = 0;
          }
          break;
      }

      r = exp_gamma[r];
      g = exp_gamma[g];
      b = exp_gamma[b];

      leds[XY(x, y)] = CRGB(r, g, b);
    }
  }
  hue++;
}

// ========== New Year's Сard ===========
//             © SlingMaster
//           Новорічна листівка
// --------------------------------------

void NewYearsCard() {
  static const uint8_t gamma[3][30] = {
    {
      0x20, 0x20, 0x48, 0x45, 0x50, 0x50, 0x59, 0x20, 0x4E, 0x45,
      0x57, 0x20, 0x59, 0x45, 0x41, 0x52, 0x21, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x02
    },
    {
      0x20, 0x20, 0xD0, 0x97, 0x20, 0xD0, 0x9D, 0xD0, 0x9E, 0xD0,
      0x92, 0xD0, 0x98, 0xD0, 0x9C, 0x20, 0xD0, 0xA0, 0xD0, 0x9E,
      0xD0, 0x9A, 0xD0, 0x9E, 0xD0, 0x9C, 0x21, 0x20, 0x00, 0x02
    },
    {
      0x20, 0x20, 0x20, 0xD0, 0x9F, 0xD0, 0xA3, 0xD0, 0xA2, 0xD0,
      0x98, 0xD0, 0x9D, 0x20, 0xD0, 0xA5, 0xD0, 0xA3, 0xD0, 0x99,
      0xD0, 0x9B, 0xD0, 0x9E, 0x21, 0x21, 0x21, 0x20, 0x00, 0x02
    }
  };

  const byte GRID = WIDTH / 4U;
  static int64_t frameCount =  0;
  const byte END = WIDTH - GRID; // + 2;
  const byte tree_h = (HEIGHT > 24) ? 24 : HEIGHT;
  const float STEP = 16.0 / HEIGHT;
  const byte MAX = 3U;
  static byte dX;
  static bool direct;
  static byte divider;
  static byte index;
  static byte shadow = 2;


  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), random8(40, 160U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;

    divider = floor(modes[currentMode].Scale / 25);
    index = 0;
    if (eff_valid < 2) {
      if (divider >= 2)    {
        shadow = 1;
      } else {
        shadow = 0;
      }
    } else {
      shadow = eff_valid;
    }

    direct = true;
    dX = GRID;
    pcnt = 0;
    frameCount = 0;
    hue2 == 32;
    clearNoiseArr();
    FastLED.clear();
  }

  EVERY_N_MILLISECONDS(1000 / 60) {
    frameCount++;
    pcnt++;
  }

  uint8_t t1 = cos8((42 * frameCount) / 30);
  uint8_t t2 = cos8((35 * frameCount) / 30);
  uint8_t t3 = cos8((38 * frameCount) / 30);
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  if (direct) {
    if (dX < (END - GRID)) {
      dX++;
      index = 0;
    }
  } else {
    if (dX > 0) {
      dX--;
    }
  }
  if (pcnt > 80) {
    pcnt = 0;
    direct = !direct;
  }

  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      if (x >= END - dX) {
        r = sin8((x - 8) * cos8((y + 20) * 4) / 4);
        g = cos8((y << 3) + t1 + cos8((t3 >> 2) + (x << 3)));
        b = cos8((y << 3) + t2 + cos8(t1 + x + (g >> 2)));
      } else {
        if (x < (END - 1 - dX)) {
          // gradient -------
          r = (shadow == gamma[shadow][29]) ? 200U : 0;
          g = divider == 2U ? 0 : y * 2;
          b = divider == 2U ? 0 : 96U + y * 2;
        } else {
          // line gold -------
          r = 160U;
          g = 144U;
          b = 64U;
        }
      }
      uint8_t val = dX * 16;
      if (r > val) {
        r = r - val;
      } else {
        r = 0;
      }

      r = exp_gamma[r];
      g = exp_gamma[g];
      b = exp_gamma[b];

      leds[XY(x, y)] = CRGB(r, g, b);
    }
    hue++;
  }
  
  float delta = 0.0;
  uint8_t posX = 0;
  for (uint8_t x = 0U; x < END - dX; x++) {
    if (x % 8 == 0) {
      // nature -----
      delta = 0.0;
      for (uint8_t y = 2U; y < tree_h; y++) {
        if (y % 3 == 0U) {
          uint8_t posX = delta;
          DrawLine(x - MAX + posX - deltaValue, y, x + MAX - posX - deltaValue, y, 0x007F00);
          delta = delta + STEP;
          if ( delta > MAX) delta = MAX;
        }
      }
      if ((x - deltaValue ) >= 0) {
        gradientVertical( x - deltaValue, 0U, x - deltaValue, tree_h, 90U, 90U, 190U, 96U, 255U);
      }
    }
  }

  VirtualSnow(divider);
  // restore background --------
  if ((gamma[shadow][index] != 32U) & (dX > (END - GRID - 1))) {
    drawLetter((shadow == 0U) ? shadow : 208, gamma[shadow][index], CENTER_X_MINOR - 1, CRGB::Red, 1);
  }

  if (pcnt % 4U == 0U) {
    index++;
    if (gamma[shadow][index] > 0xC0) {
      index++;
    }
    if (index > 28) {
      index = 0;
    }
  }

  deltaValue++;
  if (deltaValue >= 8) {
    deltaValue = 0;
  }
  hue2 += 2;
}

