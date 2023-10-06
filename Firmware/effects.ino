// ============= ЭФФЕКТЫ ===============
// несколько общих переменных и буферов, которые могут использоваться в любом эффекте
#define SQRT_VARIANT sqrt3                         // выбор основной функции для вычисления квадратного корня sqrtf или sqrt3 для ускорения
uint8_t hue, hue2;                                 // постепенный сдвиг оттенка или какой-нибудь другой цикличный счётчик
uint8_t deltaHue, deltaHue2;                       // ещё пара таких же, когда нужно много
uint8_t step;                                      // какой-нибудь счётчик кадров или последовательностей операций
uint8_t pcnt;                                      // какой-то счётчик какого-то прогресса
uint8_t deltaValue;                                // просто повторно используемая переменная
float speedfactor;                                 // регулятор скорости в эффектах реального времени
float emitterX, emitterY;                          // какие-то динамичные координаты
CRGB ledsbuff[NUM_LEDS];                           // копия массива leds[] целиком
#define NUM_LAYERSMAX 2
uint8_t noise3d[NUM_LAYERSMAX][WIDTH][HEIGHT];     // двухслойная маска или хранилище свойств в размер всей матрицы
uint8_t line[WIDTH];                               // свойство пикселей в размер строки матрицы
uint8_t shiftHue[HEIGHT];                          // свойство пикселей в размер столбца матрицы
uint8_t shiftValue[HEIGHT];                        // свойство пикселей в размер столбца матрицы ещё одно
uint16_t ff_x, ff_y, ff_z;                         // большие счётчики

uint8_t noise2[2][WIDTH + 1][HEIGHT + 1];

//массивы состояния объектов, которые могут использоваться в любом эффекте
#define trackingOBJECT_MAX_COUNT                         (100U)  // максимальное количество отслеживаемых объектов (очень влияет на расход памяти)
float   trackingObjectPosX[trackingOBJECT_MAX_COUNT];
float   trackingObjectPosY[trackingOBJECT_MAX_COUNT];
float   trackingObjectSpeedX[trackingOBJECT_MAX_COUNT];
float   trackingObjectSpeedY[trackingOBJECT_MAX_COUNT];
float   trackingObjectShift[trackingOBJECT_MAX_COUNT];
uint8_t trackingObjectHue[trackingOBJECT_MAX_COUNT];
uint8_t trackingObjectState[trackingOBJECT_MAX_COUNT];
bool    trackingObjectIsShift[trackingOBJECT_MAX_COUNT];
#define enlargedOBJECT_MAX_COUNT                     (WIDTH * 2) // максимальное количество сложных отслеживаемых объектов (меньше, чем trackingOBJECT_MAX_COUNT)
uint8_t enlargedObjectNUM;                                       // используемое в эффекте количество объектов
long    enlargedObjectTime[enlargedOBJECT_MAX_COUNT];
float    liquidLampHot[enlargedOBJECT_MAX_COUNT];
float    liquidLampSpf[enlargedOBJECT_MAX_COUNT];
unsigned liquidLampMX[enlargedOBJECT_MAX_COUNT];
unsigned liquidLampSC[enlargedOBJECT_MAX_COUNT];
unsigned liquidLampTR[enlargedOBJECT_MAX_COUNT];


// стандартные функции библиотеки LEDraw от @Palpalych (для адаптаций его эффектов)
void blurScreen(fract8 blur_amount, CRGB *LEDarray = leds) {
  blur2d(LEDarray, WIDTH, HEIGHT, blur_amount);
}

void dimAll(uint8_t value, CRGB *LEDarray = leds) {
  //for (uint16_t i = 0; i < NUM_LEDS; i++) {
  //  leds[i].nscale8(value); //fadeToBlackBy
  //}
  // теперь короткий вариант
  nscale8(LEDarray, NUM_LEDS, value);
  //fadeToBlackBy(LEDarray, NUM_LEDS, 255U - value); // эквивалент
}

//константы размера матрицы вычисляется только здесь и не меняется в эффектах
const uint8_t CENTER_X_MINOR =  (WIDTH / 2) -  ((WIDTH - 1) & 0x01); // центр матрицы по ИКСУ, сдвинутый в меньшую сторону, если ширина чётная
const uint8_t CENTER_Y_MINOR = (HEIGHT / 2) - ((HEIGHT - 1) & 0x01); // центр матрицы по ИГРЕКУ, сдвинутый в меньшую сторону, если высота чётная
const uint8_t CENTER_X_MAJOR =   WIDTH / 2  + (WIDTH % 2);           // центр матрицы по ИКСУ, сдвинутый в большую сторону, если ширина чётная
const uint8_t CENTER_Y_MAJOR =  HEIGHT / 2  + (HEIGHT % 2);          // центр матрицы по ИГРЕКУ, сдвинутый в большую сторону, если высота чётная

#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
void setModeSettings(uint8_t Scale = 0U, uint8_t Speed = 0U) {
  modes[currentMode].Scale = Scale ? Scale : pgm_read_byte(&defaultSettings[currentMode][2]);
  modes[currentMode].Speed = Speed ? Speed : pgm_read_byte(&defaultSettings[currentMode][1]);
  selectedSettings = 0U;
#ifdef USE_BLYNK
  updateRemoteBlynkParams();
#endif
}
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

// ======================================
//   Дополнительные функции рисования
// ======================================
void DrawLine(int x1, int y1, int x2, int y2, CRGB color) {
  int deltaX = abs(x2 - x1);
  int deltaY = abs(y2 - y1);
  int signX = x1 < x2 ? 1 : -1;
  int signY = y1 < y2 ? 1 : -1;
  int error = deltaX - deltaY;

  drawPixelXY(x2, y2, color);
  while (x1 != x2 || y1 != y2) {
    drawPixelXY(x1, y1, color);
    int error2 = error * 2;
    if (error2 > -deltaY) {
      error -= deltaY;
      x1 += signX;
    }
    if (error2 < deltaX) {
      error += deltaX;
      y1 += signY;
    }
  }
}

// ======================================
//по мотивам /https://gist.github.com/sutaburosu/32a203c2efa2bb584f4b846a91066583
void drawPixelXYF(float x, float y, CRGB color)  {
  //  if (x<0 || y<0) return; //не похоже, чтобы отрицательные значения хоть как-нибудь учитывались тут // зато с этой строчкой пропадает нижний ряд
  // extract the fractional parts and derive their inverses
  uint8_t xx = (x - (int)x) * 255, yy = (y - (int)y) * 255, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
#define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)
                  };
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    int16_t xn = x + (i & 1), yn = y + ((i >> 1) & 1);
    CRGB clr = getPixColorXY(xn, yn);
    clr.r = qadd8(clr.r, (color.r * wu[i]) >> 8);
    clr.g = qadd8(clr.g, (color.g * wu[i]) >> 8);
    clr.b = qadd8(clr.b, (color.b * wu[i]) >> 8);
    drawPixelXY(xn, yn, clr);
  }
}

// ======================================
void DrawLineF(float x1, float y1, float x2, float y2, CRGB color) {
  float deltaX = std::fabs(x2 - x1);
  float deltaY = std::fabs(y2 - y1);
  float error = deltaX - deltaY;

  float signX = x1 < x2 ? 0.5 : -0.5;
  float signY = y1 < y2 ? 0.5 : -0.5;

  while (x1 != x2 || y1 != y2) { // (true) - а я то думаю - "почему функция часто вызывает вылет по вачдогу?" А оно вон оно чё, Михалычь!
    if ((signX > 0 && x1 > x2 + signX) || (signX < 0 && x1 < x2 + signX)) break;
    if ((signY > 0 && y1 > y2 + signY) || (signY < 0 && y1 < y2 + signY)) break;
    drawPixelXYF(x1, y1, color); // интересно, почему тут было обычное drawPixelXY() ???
    float error2 = error;
    if (error2 > -deltaY) {
      error -= deltaY;
      x1 += signX;
    }
    if (error2 < deltaX) {
      error += deltaX;
      y1 += signY;
    }
  }
}

/* kostyamat добавил функция уменьшения яркости */
CRGB makeDarker( const CRGB& color, fract8 howMuchDarker) {
  CRGB newcolor = color;
  //newcolor.nscale8( 255 - howMuchDarker);
  newcolor.fadeToBlackBy(howMuchDarker);//эквивалент
  return newcolor;
}

// ======================================
void drawCircleF(float x0, float y0, float radius, CRGB color) {
  float x = 0, y = radius, error = 0;
  float delta = 1. - 2. * radius;
  while (y >= 0) {
    drawPixelXYF(fmod(x0 + x + WIDTH, WIDTH), y0 + y, color); // сделал, чтобы круги были бесшовными по оси х
    drawPixelXYF(fmod(x0 + x + WIDTH, WIDTH), y0 - y, color);
    drawPixelXYF(fmod(x0 - x + WIDTH, WIDTH), y0 + y, color);
    drawPixelXYF(fmod(x0 - x + WIDTH, WIDTH), y0 - y, color);
    error = 2. * (delta + y) - 1.;
    if (delta < 0 && error <= 0) {
      ++x;
      delta += 2. * x + 1.;
      continue;
    }
    error = 2. * (delta - x) - 1.;
    if (delta > 0 && error > 0) {
      --y;
      delta += 1. - 2. * y;
      continue;
    }
    ++x;
    delta += 2. * (x - y);
    --y;
  }
}


// палитра для типа реалистичного водопада (если ползунок Масштаб выставить на 100)
extern const TProgmemRGBPalette16 WaterfallColors_p FL_PROGMEM = {0x000000, 0x060707, 0x101110, 0x151717, 0x1C1D22, 0x242A28, 0x363B3A, 0x313634, 0x505552, 0x6B6C70, 0x98A4A1, 0xC1C2C1, 0xCACECF, 0xCDDEDD, 0xDEDFE0, 0xB2BAB9};

// добавлено изменение текущей палитры (используется во многих эффектах ниже для бегунка Масштаб)
const TProgmemRGBPalette16 *palette_arr[] = {
  &PartyColors_p,
  &OceanColors_p,
  &LavaColors_p,
  &HeatColors_p,
  &WaterfallColors_p,
  &CloudColors_p,
  &ForestColors_p,
  &RainbowColors_p,
  &RainbowStripeColors_p
};
const TProgmemRGBPalette16 *curPalette = palette_arr[0];
void setCurrentPalette() {
  if (modes[currentMode].Scale > 100U) modes[currentMode].Scale = 100U; // чтобы не было проблем при прошивке без очистки памяти
  curPalette = palette_arr[(uint8_t)(modes[currentMode].Scale / 100.0F * ((sizeof(palette_arr) / sizeof(TProgmemRGBPalette16 *)) - 0.01F))];
}
// при таком количестве палитр (9шт) каждый диапазон Масштаба (от 1 до 100) можно разбить на участки по 11 значений
// значения от 0 до 10 = ((modes[currentMode].Scale - 1U) % 11U)
// значения от 1 до 11 = ((modes[currentMode].Scale - 1U) % 11U + 1U)
// а 100е значение Масштаба можно использовать для белого цвета


// дополнительные палитры для пламени
// для записи в PROGMEM преобразовывал из 4 цветов в 16 на сайте https://colordesigner.io/gradient-generator, но не уверен, что это эквивалент CRGBPalette16()
// значения цветовых констант тут: https://github.com/FastLED/FastLED/wiki/Pixel-reference
extern const TProgmemRGBPalette16 WoodFireColors_p FL_PROGMEM = {CRGB::Black, 0x330e00, 0x661c00, 0x992900, 0xcc3700, CRGB::OrangeRed, 0xff5800, 0xff6b00, 0xff7f00, 0xff9200, CRGB::Orange, 0xffaf00, 0xffb900, 0xffc300, 0xffcd00, CRGB::Gold};             //* Orange
extern const TProgmemRGBPalette16 NormalFire_p FL_PROGMEM = {CRGB::Black, 0x330000, 0x660000, 0x990000, 0xcc0000, CRGB::Red, 0xff0c00, 0xff1800, 0xff2400, 0xff3000, 0xff3c00, 0xff4800, 0xff5400, 0xff6000, 0xff6c00, 0xff7800};                             // пытаюсь сделать что-то более приличное
extern const TProgmemRGBPalette16 NormalFire2_p FL_PROGMEM = {CRGB::Black, 0x560000, 0x6b0000, 0x820000, 0x9a0011, CRGB::FireBrick, 0xc22520, 0xd12a1c, 0xe12f17, 0xf0350f, 0xff3c00, 0xff6400, 0xff8300, 0xffa000, 0xffba00, 0xffd400};                      // пытаюсь сделать что-то более приличное
extern const TProgmemRGBPalette16 LithiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x240707, 0x470e0e, 0x6b1414, 0x8e1b1b, CRGB::FireBrick, 0xc14244, 0xd16166, 0xe08187, 0xf0a0a9, CRGB::Pink, 0xff9ec0, 0xff7bb5, 0xff59a9, 0xff369e, CRGB::DeepPink};        //* Red
extern const TProgmemRGBPalette16 SodiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x332100, 0x664200, 0x996300, 0xcc8400, CRGB::Orange, 0xffaf00, 0xffb900, 0xffc300, 0xffcd00, CRGB::Gold, 0xf8cd06, 0xf0c30d, 0xe9b913, 0xe1af1a, CRGB::Goldenrod};           //* Yellow
extern const TProgmemRGBPalette16 CopperFireColors_p FL_PROGMEM = {CRGB::Black, 0x001a00, 0x003300, 0x004d00, 0x006600, CRGB::Green, 0x239909, 0x45b313, 0x68cc1c, 0x8ae626, CRGB::GreenYellow, 0x94f530, 0x7ceb30, 0x63e131, 0x4bd731, CRGB::LimeGreen};     //* Green
extern const TProgmemRGBPalette16 AlcoholFireColors_p FL_PROGMEM = {CRGB::Black, 0x000033, 0x000066, 0x000099, 0x0000cc, CRGB::Blue, 0x0026ff, 0x004cff, 0x0073ff, 0x0099ff, CRGB::DeepSkyBlue, 0x1bc2fe, 0x36c5fd, 0x51c8fc, 0x6ccbfb, CRGB::LightSkyBlue};  //* Blue
extern const TProgmemRGBPalette16 RubidiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x0f001a, 0x1e0034, 0x2d004e, 0x3c0068, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, 0x3c0084, 0x2d0086, 0x1e0087, 0x0f0089, CRGB::DarkBlue};        //* Indigo
extern const TProgmemRGBPalette16 PotassiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x0f001a, 0x1e0034, 0x2d004e, 0x3c0068, CRGB::Indigo, 0x591694, 0x682da6, 0x7643b7, 0x855ac9, CRGB::MediumPurple, 0xa95ecd, 0xbe4bbe, 0xd439b0, 0xe926a1, CRGB::DeepPink}; //* Violet
const TProgmemRGBPalette16 *firePalettes[] = {
  //    &HeatColors_p, // эта палитра уже есть в основном наборе. если в эффекте подключены оба набора палитр, тогда копия не нужна
  &WoodFireColors_p,
  &NormalFire_p,
  &NormalFire2_p,
  &LithiumFireColors_p,
  &SodiumFireColors_p,
  &CopperFireColors_p,
  &AlcoholFireColors_p,
  &RubidiumFireColors_p,
  &PotassiumFireColors_p
};

// =====================================
//           БЕГУЩАЯ СТРОКА
// =====================================
void text_running() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(1U + random8(100U), 50U + random8(100U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  while (!fillString(TextTicker, CHSV(modes[MODE_AMOUNT - 1].Scale * 2.55, 255U, 255U), true) && currentMode == MODE_AMOUNT - 1) {
    parseUDP();
    delay (5);
    HTTP.handleClient();
#ifdef ESP_USE_BUTTON
    buttonTick();
#endif
  }
}

// =====================================
// ======= • ФУНКЦИИ ЭФФЕКТОВ • ========
// =====================================

// =====================================
//               Kонфетти
// =====================================

#define FADE_OUT_SPEED        (70U)                         // скорость затухания
void sparklesRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(4U + random8(97U), 99U + random8(125U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    for (uint16_t i = 0; i < NUM_LEDS; i++)
      if (random8(3U))
        leds[i].nscale8(random8());
      else
        leds[i] = 0U;
  }

  for (uint8_t i = 0; i < modes[currentMode].Scale; i++) {
    uint8_t x = random8(WIDTH);
    uint8_t y = random8(HEIGHT);
    if (getPixColorXY(x, y) == 0U)
    {
      leds[XY(x, y)] = CHSV(random8(), 255U, 255U);
    }
  }
  //fader(FADE_OUT_SPEED);
  dimAll(256U - FADE_OUT_SPEED);
}

void fadePixel(uint8_t i, uint8_t j, uint8_t step)          // новый фейдер
{
  int32_t pixelNum = XY(i, j);
  if (getPixColor(pixelNum) == 0U) return;

  if (leds[pixelNum].r >= 30U ||
      leds[pixelNum].g >= 30U ||
      leds[pixelNum].b >= 30U)
  {
    leds[pixelNum].fadeToBlackBy(step);
  }
  else
  {
    leds[pixelNum] = 0U;
  }
}

// =====================================
//        новый огонь / Водопад
// =====================================
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100
#define COOLINGNEW 32
// 8  практически сплошной поток красивой подсвеченной воды ровным потоком сверху донизу. будто бы на столе стоит маленький "родничок"
// 20 ровный водопад с верщиной на свету, где потоки летящей воды наверху разбиваются ветром в белую пену
// 32 уже не ровный водопад, у которого струи воды долетают до земли неравномерно
// чем больше параметр, тем больше тени снизу
// 55 такое, как на видео

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKINGNEW 80 // 30 // 120 // 90 // 60
// 80 почти все белые струи сверху будут долетать до низа - хорошо при выбранном ползунке Масштаб = 100 (белая вода без подкрашивания)
// 50 чуть больше половины будет долетать. для цветных вариантов жидкости так более эффектно

void fire2012WithPalette() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(random8(7U) ? 46U + random8(26U) : 100U, 195U + random8(40U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  //    bool fire_water = modes[currentMode].Scale <= 50;
  //    uint8_t COOLINGNEW = fire_water ? modes[currentMode].Scale * 2  + 20 : (100 - modes[currentMode].Scale ) *  2 + 20 ;
  //    uint8_t COOLINGNEW = modes[currentMode].Scale * 2  + 20 ;
  // Array of temperature readings at each simulation cell
  //static byte heat[WIDTH][HEIGHT]; будет noise3d[0][WIDTH][HEIGHT]

  for (uint8_t x = 0; x < WIDTH; x++) {
    // Step 1.  Cool down every cell a little
    for (uint8_t i = 0; i < HEIGHT; i++) {
      noise3d[0][x][i] = qsub8(noise3d[0][x][i], random8(0, ((COOLINGNEW * 10) / HEIGHT) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (uint8_t k = HEIGHT - 1; k >= 2; k--) {
      noise3d[0][x][k] = (noise3d[0][x][k - 1] + noise3d[0][x][k - 2] + noise3d[0][x][k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < SPARKINGNEW) {
      uint8_t y = random8(2);
      noise3d[0][x][y] = qadd8(noise3d[0][x][y], random8(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    for (uint8_t j = 0; j < HEIGHT; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8(noise3d[0][x][j], 240);
      if (modes[currentMode].Scale == 100)
        leds[XY(x, (HEIGHT - 1) - j)] = ColorFromPalette(WaterfallColors_p, colorindex);
      else
        leds[XY(x, (HEIGHT - 1) - j)] = ColorFromPalette(CRGBPalette16( CRGB::Black, CHSV(modes[currentMode].Scale * 2.57, 255U, 255U) , CHSV(modes[currentMode].Scale * 2.57, 128U, 255U) , CRGB::White), colorindex);// 2.57 вместо 2.55, потому что 100 для белого цвета
      //leds[XY(x, (HEIGHT - 1) - j)] = ColorFromPalette(fire_water ? HeatColors_p : OceanColors_p, colorindex);
    }
  }
}

// =====================================
//               Oгонь
// =====================================
#define SPARKLES              (1U)                       // вылетающие угольки вкл выкл
#define UNIVERSE_FIRE                                    // универсальный огонь 2-в-1 Цветной+Белый

//uint8_t pcnt = 0U;                                     // внутренний делитель кадров для поднимающегося пламени - переменная вынесена в общий пул, чтобы использовать повторно
//uint8_t deltaHue = 16U;                                // текущее смещение пламени (hueMask) - переменная вынесена в общий пул, чтобы использовать повторно
//uint8_t shiftHue[HEIGHT];                              // массив дороожки горизонтального смещения пламени (hueMask) - вынесен в общий пул массивов переменных
//uint8_t deltaValue = 16U;                              // текущее смещение пламени (hueValue) - переменная вынесена в общий пул, чтобы использовать повторно
//uint8_t shiftValue[HEIGHT];                            // массив дороожки горизонтального смещения пламени (hueValue) - вынесен в общий пул массивов переменных

//these values are substracetd from the generated values to give a shape to the animation
static const uint8_t valueMask[8][16] PROGMEM =
{
  {0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  },
  {0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  },
  {0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  },
  {0  , 32 , 64 , 128, 128, 64 , 32 , 0  , 0  , 32 , 64 , 128, 128, 64 , 32 , 0  },
  {32 , 64 , 96 , 160, 160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 },
  {64 , 96 , 128, 192, 192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128, 96 , 64 },
  {96 , 128, 160, 255, 255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160, 128, 96 },
  {128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128}
};

//these are the hues for the fire,
//should be between 0 (red) to about 25 (yellow)
static const uint8_t hueMask[8][16] PROGMEM =
{
  {25, 22, 11, 1 , 1 , 11, 19, 25, 25, 22, 11, 1 , 1 , 11, 19, 25 },
  {25, 19, 8 , 1 , 1 , 8 , 13, 19, 25, 19, 8 , 1 , 1 , 8 , 13, 19 },
  {19, 16, 8 , 1 , 1 , 8 , 13, 16, 19, 16, 8 , 1 , 1 , 8 , 13, 16 },
  {13, 13, 5 , 1 , 1 , 5 , 11, 13, 13, 13, 5 , 1 , 1 , 5 , 11, 13 },
  {11, 11, 5 , 1 , 1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11 },
  {8 , 5 , 1 , 0 , 0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 , 0 , 1 , 5 , 8  },
  {5 , 1 , 0 , 0 , 0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 , 0 , 0 , 1 , 5  },
  {1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1  }
};

// --------------------------------
void fire() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(30U) ? 1U + random8(100U) : 100U, 200U + random8(35U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    FastLED.clear();
    generateLine();
    pcnt = 0;
  }
  uint8_t baseHue = (float)(modes[currentMode].Scale - 1U) * 2.6;
  fireRoutine(baseHue);
}

// --------------------------------
void fireRoutine(uint8_t baseHue) {
  static const bool isColored = true;
  if (pcnt >= 30) {                                         // внутренний делитель кадров для поднимающегося пламени
    shiftUp();                                              // смещение кадра вверх
    generateLine();                                         // перерисовать новую нижнюю линию случайным образом
    pcnt = 0;
  }

  drawFrame(pcnt, baseHue, isColored);                      // для прошивки где стоит логический параметр
  pcnt += 25;  // делитель кадров: задает скорость подъема пламени 25/100 = 1/4
}

// --------------------------------
// Randomly generate the next line (matrix row)
void generateLine() {
  for (uint8_t x = 0U; x < WIDTH; x++) {
    line[x] = random(127, 255);                             // заполнение случайным образом нижней линии (127, 255) - менее контрастное, (64, 255) - оригинал
  }
}

// --------------------------------
void shiftUp() {                                            //подъем кадра
  for (uint8_t y = HEIGHT - 1U; y > 0U; y--) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      uint8_t newX = x % 16U;                               // сократил формулу без доп. проверок
      if (y > 7U) continue;
      matrixValue[y][newX] = matrixValue[y - 1U][newX];     //смещение пламени (только для зоны очага)
    }
  }

  for (uint8_t x = 0U; x < WIDTH; x++) {                    // прорисовка новой нижней линии
    uint8_t newX = x % 16U;                                 // сократил формулу без доп. проверок
    matrixValue[0U][newX] = line[newX];
  }
}

// draw a frame, interpolating between 2 "key frames"
// @param pcnt percentage of interpolation
// --------------------------------
void drawFrame(uint8_t pcnt, uint8_t baseHue, bool isColored) {                  // прорисовка нового кадра
  int32_t nextv;
  uint8_t baseSat = (baseHue < 254) ? 255U : 0U;  // color or white flame

  //first row interpolates with the "next" line
  deltaHue = random(0U, 2U) ? constrain (shiftHue[0] + random(0U, 2U) - random(0U, 2U), 15U, 17U) : shiftHue[0]; // random(0U, 2U)= скорость смещения языков чем больше 2U - тем медленнее
  // 15U, 17U - амплитуда качания -1...+1 относительно 16U
  // высчитываем плавную дорожку смещения всполохов для нижней строки
  // так как в последствии координаты точки будут исчисляться из остатка, то за базу можем принять кратную ширину матрицы hueMask
  // ширина матрицы hueMask = 16, поэтому нам нужно получить диапазон чисел от 15 до 17
  // далее к предыдущему значению прибавляем случайную 1 и отнимаем случайную 1 - это позволит плавным образом менять значение смещения
  shiftHue[0] = deltaHue;                                   // заносим это значение в стэк

  deltaValue = random(0U, 3U) ? constrain (shiftValue[0] + random(0U, 2U) - random(0U, 2U), 15U, 17U) : shiftValue[0]; // random(0U, 3U)= скорость смещения очага чем больше 3U - тем медленнее
  // 15U, 17U - амплитуда качания -1...+1 относительно 16U
  shiftValue[0] = deltaValue;


  for (uint8_t x = 0U; x < WIDTH; x++) {                                          // прорисовка нижней строки (сначала делаем ее, так как потом будем пользоваться ее значением смещения)
    uint8_t newX = x % 16;                                                        // сократил формулу без доп. проверок
    nextv =                                                               // расчет значения яркости относительно valueMask и нижерасположенной строки.
      (((100.0 - pcnt) * matrixValue[0][newX] + pcnt * line[newX]) / 100.0)
      - pgm_read_byte(&valueMask[0][(x + deltaValue) % 16U]);
    CRGB color = CHSV(                                                            // вычисление цвета и яркости пикселя
                   baseHue + pgm_read_byte(&hueMask[0][(x + deltaHue) % 16U]),    // H - смещение всполохов
                   baseSat,                                                       // S - когда колесо масштаба =100 - белый огонь (экономим на 1 эффекте)
                   (uint8_t)max(0, nextv)                                         // V
                 );
    leds[XY(x, 0)] = color;                                            // прорисовка цвета очага
  }

  //each row interpolates with the one before it
  for (uint8_t y = HEIGHT - 1U; y > 0U; y--) {                                      // прорисовка остальных строк с учетом значения низлежащих
    deltaHue = shiftHue[y];                                                         // извлекаем положение
    shiftHue[y] = shiftHue[y - 1];                                                  // подготавлеваем значение смешения для следующего кадра основываясь на предыдущем
    deltaValue = shiftValue[y];                                                     // извлекаем положение
    shiftValue[y] = shiftValue[y - 1];                                              // подготавлеваем значение смешения для следующего кадра основываясь на предыдущем


    if (y > 8U) {                                                                   // цикл стирания текущей строоки для искр
      for (uint8_t _x = 0U; _x < WIDTH; _x++) {                                     // стираем строчку с искрами (очень не оптимально)
        drawPixelXY(_x, y, 0U);
      }
    }
    for (uint8_t x = 0U; x < WIDTH; x++) {                                          // пересчет координаты x для текущей строки
      uint8_t newX = x % 16U;                                                       // функция поиска позиции значения яркости для матрицы valueMask
      if (y < 8U) {                                                                 // если строка представляет очаг
        nextv =                                                                     // расчет значения яркости относительно valueMask и нижерасположенной строки.
          (((100.0 - pcnt) * matrixValue[y][newX]
            + pcnt * matrixValue[y - 1][newX]) / 100.0)
          - pgm_read_byte(&valueMask[y][(x + deltaValue) % 16U]);

        CRGB color = CHSV(                                                                  // определение цвета пикселя
                       baseHue + pgm_read_byte(&hueMask[y][(x + deltaHue) % 16U ]),         // H - смещение всполохов
                       baseSat,                                                             // S - когда колесо масштаба =100 - белый огонь (экономим на 1 эффекте)
                       (uint8_t)max(0, nextv)                                               // V
                     );
        leds[XY(x, y)] = color;
      }
      else if (y == 8U && SPARKLES) {                                               // если это самая нижняя строка искр - формитуем искорку из пламени
        if (random(0, 20) == 0 && getPixColorXY(x, y - 1U) != 0U) drawPixelXY(x, y, getPixColorXY(x, y - 2U));  // 20 = обратная величина количества искр
        else drawPixelXY(x, y, 0U);
      }
      else if (SPARKLES) {                                                          // если это не самая нижняя строка искр - перемещаем искорку выше
        // старая версия для яркости
        newX = (random(0, 4)) ? x : (x + WIDTH + random(0U, 2U) - random(0U, 2U)) % WIDTH ;   // с вероятностью 1/3 смещаем искорку влево или вправо
        if (getPixColorXY(x, y - 1U) > 0U) drawPixelXY(newX, y, getPixColorXY(x, y - 1U));    // рисуем искорку на новой строчке
      }
    }
  }
}

// =====================================
//         Радуга три в одной
// =====================================
void rainbowHorVertRoutine(bool isVertical) {
  for (uint8_t i = 0U; i < (isVertical ? WIDTH : HEIGHT); i++) {
    //CRGB thisColor;
    //hsv2rgb_spectrum(CHSV(hue + i * (modes[currentMode].Scale % 67U) * 2U, 255U, 255U), thisColor); // так ещё хуже стало на низкой яркости
    CHSV thisColor = CHSV((uint8_t)(hue + i * (modes[currentMode].Scale % 67U) * 2U), 255U, 255U);

    for (uint8_t j = 0U; j < (isVertical ? HEIGHT : WIDTH); j++)
      drawPixelXY((isVertical ? i : j), (isVertical ? j : i), thisColor);
  }
}
void rainbowRoutine() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    uint8_t tmp = 7U + random8(50U);
    if (tmp > 14) tmp += 19U;
    if (tmp > 67) tmp += 6U;
    setModeSettings(tmp , 150U + random8(86U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  hue += 4U;
  if (modes[currentMode].Scale < 34U)           // если масштаб до 34
    rainbowHorVertRoutine(false);
  else if (modes[currentMode].Scale > 67U)      // если масштаб больше 67
    rainbowHorVertRoutine(true);
  else                                          // для масштабов посередине
    for (uint8_t i = 0U; i < WIDTH; i++)
      for (uint8_t j = 0U; j < HEIGHT; j++)
      {
        float twirlFactor = 9.0F * ((modes[currentMode].Scale - 33) / 100.0F);    // на сколько оборотов будет закручена матрица, [0..3]
        CRGB thisColor = CHSV((uint8_t)(hue + ((float)WIDTH / (float)HEIGHT * i + j * twirlFactor) * ((float)255 / (float)maxDim)), 255U, 255U);
        drawPixelXY(i, j, thisColor);
      }
}


// =====================================
//                Пульс
// =====================================
// Stefan Petrick's PULSE Effect mod by PalPalych for GyverLamp

//void drawCircle(int16_t x0, int16_t y0, uint16_t radius, const CRGB & color) {
void drawCircle(int x0, int y0, int radius, const CRGB &color) {
  int a = radius, b = 0;
  int radiusError = 1 - a;

  if (radius == 0) {
    drawPixelXY(x0, y0, color);
    return;
  }

  while (a >= b)  {
    drawPixelXY(a + x0, b + y0, color);
    drawPixelXY(b + x0, a + y0, color);
    drawPixelXY(-a + x0, b + y0, color);
    drawPixelXY(-b + x0, a + y0, color);
    drawPixelXY(-a + x0, -b + y0, color);
    drawPixelXY(-b + x0, -a + y0, color);
    drawPixelXY(a + x0, -b + y0, color);
    drawPixelXY(b + x0, -a + y0, color);
    b++;
    if (radiusError < 0)
      radiusError += 2 * b + 1;
    else
    {
      a--;
      radiusError += 2 * (b - a + 1);
    }
  }
}

// -------------------------------------
void pulse2() {
  pulseRoutine(2U);
}
// -------------------------------------
void pulse4() {
  pulseRoutine(4U);
}
// -------------------------------------
void pulse8() {
  pulseRoutine(8U);
}
void pulseRoutine(uint8_t PMode) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(1U + random8(100U), 170U + random8(62U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  CRGB _pulse_color;


  dimAll(248U);
  uint8_t _sat;
  if (step <= pcnt) {
    for (uint8_t i = 0; i < step; i++ ) {
      uint8_t _dark = qmul8( 2U, cos8 (128U / (step + 1U) * (i + 1U))) ;
      switch (PMode) {
        case 1U:                    // 1 - случайные диски
          deltaHue = hue;
          _pulse_color = CHSV(deltaHue, 255U, _dark);
          break;
        case 2U:                    // 2...17 - перелив цвета дисков
          deltaHue2 = modes[currentMode].Scale;
          _pulse_color = CHSV(hue2, 255U, _dark);
          break;
        case 3U:                    // 18...33 - выбор цвета дисков
          deltaHue = modes[currentMode].Scale * 2.55;
          _pulse_color = CHSV(deltaHue, 255U, _dark);
          break;
        case 4U:                    // 34...50 - дискоцветы
          deltaHue += modes[currentMode].Scale;
          _pulse_color = CHSV(deltaHue, 255U, _dark);
          break;
        case 5U:                    // 51...67 - пузыри цветы
          _sat =  qsub8( 255U, cos8 (128U / (step + 1U) * (i + 1U))) ;
          deltaHue += modes[currentMode].Scale;
          _pulse_color = CHSV(deltaHue, _sat, _dark);
          break;
        case 6U:                    // 68...83 - выбор цвета пузырей
          _sat =  qsub8( 255U, cos8 (128U / (step + 1U) * (i + 1U))) ;
          deltaHue = modes[currentMode].Scale * 2.55;
          _pulse_color = CHSV(deltaHue, _sat, _dark);
          break;
        case 7U:                    // 84...99 - перелив цвета пузырей
          _sat =  qsub8( 255U, cos8 (128U / (step + 1U) * (i + 1U))) ;
          deltaHue2 = modes[currentMode].Scale;
          _pulse_color = CHSV(hue2, _sat, _dark);
          break;
        case 8U:                    // 100 - случайные пузыри
          _sat =  qsub8( 255U, cos8 (128U / (step + 1U) * (i + 1U))) ;
          //deltaHue = hue; // вместо этого будет решулировка сдвига оттенка
          //_pulse_color = CHSV(deltaHue, _sat, _dark);
          deltaHue2 = modes[currentMode].Scale;
          _pulse_color = CHSV(hue2, _sat, _dark);
          break;
      }
      drawCircle(emitterX, emitterY, i, _pulse_color);
    }
  } else {
    emitterX = random8(WIDTH - 5U) + 3U;
    emitterY = random8(HEIGHT - 5U) + 3U;
    hue2 += deltaHue2;
    hue = random8(0U, 255U);
    pcnt = random8(WIDTH >> 2U, (WIDTH >> 1U) + 1U);
    step = 0;
  }
  step++;
  if (modes[currentMode].Speed & 0x01) blurScreen(10U); // убираем квадратики внутри кругов пульса
}

// =====================================
//         цвет + Вода в бассейне
// =====================================
// (с) SottNick. 03.2020
// эффект иммеет шов на стыке краёв матрицы (сзади лампы, как и у других эффектов), зато адаптирован для нестандартных размеров матриц.
// можно было бы сделать абсолютно бесшовный вариант для конкретной матрицы (16х16), но уже была бы заметна зацикленность анимации.

// далее идёт массив из 25 кадров анимации с маской бликов на воде (размер картинки больше размера матрицы, чтобы повторяемость картинки была незаметной)
// бесшовную анимированную текстуру бликов делал в программе Substance Designer (30 дней бесплатно работает) при помощи плагина Bruno Caustics Generator
// но сразу под такой мелкий размер текстура выходит нечёткой, поэтому пришлось делать крупную и потом в фотошопе доводить её до ума
// конвертировал в массив через сервис https://littlevgl.com/image-to-c-array,
// чтобы из ч/б картинки получить массив для коррекции параметра насыщенности цвета, использовал настройки True color -> C array
// последовательность замен полученных блоков массива в ворде: "^p  0x"->"^p  {0x"  ...  ", ^p"->"},^p" ... "},^p#endif"->"}^p },^p {"
// константа aquariumGIF[25][32][32] вынесена в отдельный файл data_gif в котором хранятся подобные константы для всех эффектов


//uint8_t step = 0U;  // GIFframe = 0U; текущий кадр анимации (не важно, какой в начале)
//uint8_t deltaHue = 0U; // GIFshiftx = 0U; какой-то там сдвиг текстуры по радиусу лампы
//uint8_t deltaHue2 = 0U; // GIFshifty = 0U; какой-то там сдвиг текстуры по высоте

#define CAUSTICS_BR                     (100U)                // яркость бликов в процентах (от чистого белого света)

void poolRoutine()
{
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(47U + random8(28U), 201U + random8(38U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    hue = modes[currentMode].Scale * 2.55;
    fillAll(CHSV(hue, 255U, 255U));
    deltaHue = 0U;
    deltaHue2 = 0U;
  }
  if (modes[currentMode].Speed != 255U) {     // если регулятор скорости на максимуме, то будет работать старый эффект "цвет" (без анимации бликов воды)
    if (step > 24U) {                         // количество кадров в анимации -1 (отсчёт с нуля)
      step = 0U;
    }
    if (step > 0U && step < 3U) {             // пару раз за цикл анимации двигаем текстуру по радиусу лампы. а может и не двигаем. как повезёт
      if (random(2U) == 0U) {
        deltaHue++;
        if (deltaHue > 31U) deltaHue = 0U;
      }
    }
    if (step > 11U && step < 14U) {           // пару раз за цикл анимации двигаем текстуру по вертикали. а может и не двигаем. как повезёт
      if (random(2U) == 0U) {
        deltaHue2++;
        if (deltaHue2 > 31U) deltaHue2 = 0U;
      }
    }

    for (uint8_t x = 0U; x < WIDTH ; x++) {
      for (uint8_t y = 0U; y < HEIGHT; y++) {
        // y%32, x%32 - это для масштабирования эффекта на лампы размером большим, чем размер анимации 32х32, а также для произвольного сдвига текстуры
        leds[XY(x, y)] = CHSV(hue, 255U - pgm_read_byte(&aquariumGIF[step][(y + deltaHue2) % 32U][(x + deltaHue) % 32U]) * CAUSTICS_BR / 100U, 255U);
        // чтобы регулятор Масштаб начал вместо цвета регулировать яркость бликов, нужно закомментировать предыдущую строчку и раскоментировать следующую
        //        leds[XY(x, y)] = CHSV(158U, 255U - pgm_read_byte(&aquariumGIF[step][(y+deltaHue2)%32U][(x+deltaHue)%32U]) * modes[currentMode].Scale / 100U, 255U);
      }
    }
    step++;
  }
}

// =====================================
//              Цвета - 2
// =====================================

//#define SECONDS_DELAY (1U) // растягиваем задержку, регулируемую бегунком "Скорость" в указанное количество времени
#define DELAY_MULTIPLIER (20U) //при задержке между кадрами примерно в 50 мс с этим множителем получится 1 с на единицу бегунка Скорость
void colorsRoutine2()
{
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(255U), 210U + random8(46U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = deltaValue; // чтообы при старте эффекта сразу покрасить лампу (для бугунка Масштаб от 246 до 9)
    deltaHue = 1U;     // чтообы при старте эффекта сразу покрасить лампу (для бегунка Масштаб от 10 до 245)
    hue2 = 0U;
  }

  if (modes[currentMode].Scale < 10U || modes[currentMode].Scale > 245U) // если Масштаб небольшой, меняем цвет на это значение регулярно (каждый цикл кратный значению Скорость)
    if (step >= deltaValue) {
      hue += modes[currentMode].Scale;
      step = 0U;
      //for (uint16_t i = 0U; i < NUM_LEDS; i++)
      //  leds[i] = CHSV(hue, 255U, 255U);
      fillAll(CHSV(hue, 255U, 255U));
    }
    else
      step++;
  else                                                                   // если Масштаб большой, тогда смену цвета делаем как бы пульсацией (поменяли, пауза, поменяли, пауза)
    if (deltaHue != 0) {
      if (deltaHue > 127U) {
        hue--;
        deltaHue++;
      }
      else {
        hue++;
        deltaHue--;
      }
      //for (uint16_t i = 0U; i < NUM_LEDS; i++)
      //  leds[i] = CHSV(hue, 255U, 255U);
      fillAll(CHSV(hue, 255U, 255U));
    }
    else if (step >= deltaValue) {
      deltaHue = modes[currentMode].Scale;
      step = 0U;
    }
    else
      //        EVERY_N_SECONDS(SECONDS_DELAY){   // не компилируется такое, блин
      //        EVERY_N_MILLIS(12) {              // и такое тоже
      if (hue2 >= DELAY_MULTIPLIER) {
        step++;
        hue2 = 0U;
      }
      else
        hue2++;
}

// =====================================
//                 Цвет
// =====================================
void colorRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(100U), 96U + random8(160));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    //FastLED.clear(); нафига тут это было?!

    //for (int16_t i = 0U; i < NUM_LEDS; i++)
    //  leds[i] = CHSV(modes[currentMode].Scale * 2.55, modes[currentMode].Speed, 255U);
    fillAll(CHSV(modes[currentMode].Scale * 2.55, modes[currentMode].Speed, 255U));
  }
}

// =====================================
//          Звездопад | Mетель
// =====================================
//SNOWSTORM / МЕТЕЛЬ # STARFALL / ЗВЕЗДОПАД ***** V1.2
// v1.0 - Updating for GuverLamp v1.7 by PalPalych 12.03.2020
// v1.1 - Fix wrong math & full screen drawing by PalPalych 14.03.2020
// v1.2 - Code optimisation + pseudo 3d by PalPalych 21.04.2020
#define e_sns_DENSE (32U) // плотность снега - меньше = плотнее

void stormRoutine2() {// (bool isColored) { // сворачиваем 2 эффекта в 1
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    uint8_t tmp = 175U + random8(39U);
    if (tmp & 0x01)
      setModeSettings(50U + random8(51U), tmp);
    else
      setModeSettings(50U + random8(24U), tmp);
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  bool isColored = modes[currentMode].Speed & 0x01; // сворачиваем 2 эффекта в 1
  // заполняем головами комет
  uint8_t Saturation = 0U;    // цвет хвостов
  uint8_t e_TAIL_STEP = 127U; // длина хвоста
  if (isColored) {
    Saturation = modes[currentMode].Scale * 2.55;
  } else {
    e_TAIL_STEP = 255U - modes[currentMode].Scale * 2.5;
  }
  for (uint8_t x = 0U; x < WIDTH - 1U; x++) { // fix error i != 0U
    if (!random8(e_sns_DENSE) &&
        !getPixColorXY(wrapX(x), HEIGHT - 1U) &&
        !getPixColorXY(wrapX(x + 1U), HEIGHT - 1U) &&
        !getPixColorXY(wrapX(x - 1U), HEIGHT - 1U))
    {
      drawPixelXY(x, HEIGHT - 1U, CHSV(random8(), Saturation, random8(64U, 255U)));
    }
  }

  // сдвигаем по диагонали
  for (uint8_t y = 0U; y < HEIGHT - 1U; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      drawPixelXY(wrapX(x + 1U), y, getPixColorXY(x, y + 1U));
    }
  }

  // уменьшаем яркость верхней линии, формируем "хвосты"
  for (uint8_t i = 0U; i < WIDTH; i++) {
    fadePixel(i, HEIGHT - 1U, e_TAIL_STEP);
  }
}

// =====================================
//                Матрица
// =====================================
void matrixRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(90U), 165U + random8(66U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    FastLED.clear();
  }
  for (uint8_t x = 0U; x < WIDTH; x++) {
    // обрабатываем нашу матрицу снизу вверх до второй сверху строчки
    for (uint8_t y = 0U; y < HEIGHT - 1U; y++) {
      uint32_t thisColor = getPixColorXY(x, y);                                              // берём цвет нашего пикселя
      uint32_t upperColor = getPixColorXY(x, y + 1U);                                        // берём цвет пикселя над нашим
      if (upperColor >= 0x900000 && random(7 * HEIGHT) != 0U)                  // если выше нас максимальная яркость, игнорим этот факт с некой вероятностью или опускаем цепочку ниже
        drawPixelXY(x, y, upperColor);
      else if (thisColor == 0U && random((100 - modes[currentMode].Scale) * HEIGHT) == 0U)  // если наш пиксель ещё не горит, иногда зажигаем новые цепочки
        //else if (thisColor == 0U && random((100 - modes[currentMode].Scale) * HEIGHT*3) == 0U)  // для длинных хвостов
        drawPixelXY(x, y, 0x9bf800);
      else if (thisColor <= 0x050800)                                                        // если наш пиксель почти погас, стараемся сделать затухание медленней
      {
        if (thisColor >= 0x030000)
          drawPixelXY(x, y, 0x020300);
        else if (thisColor != 0U)
          drawPixelXY(x, y, 0U);
      }
      else if (thisColor >= 0x900000)                                                        // если наш пиксель максимальной яркости, резко снижаем яркость
        drawPixelXY(x, y, 0x558800);
      else
        drawPixelXY(x, y, thisColor - 0x0a1000);                                             // в остальных случаях снижаем яркость на 1 уровень
      //drawPixelXY(x, y, thisColor - 0x050800);                                             // для длинных хвостов
    }
    // аналогично обрабатываем верхний ряд пикселей матрицы
    uint32_t thisColor = getPixColorXY(x, HEIGHT - 1U);
    if (thisColor == 0U)                                                                     // если наш верхний пиксель не горит, заполняем его с вероятностью .Scale
    {
      if (random(100 - modes[currentMode].Scale) == 0U)
        drawPixelXY(x, HEIGHT - 1U, 0x9bf800);
    }
    else if (thisColor <= 0x050800)                                                          // если наш верхний пиксель почти погас, стараемся сделать затухание медленней
    {
      if (thisColor >= 0x030000)
        drawPixelXY(x, HEIGHT - 1U, 0x020300);
      else
        drawPixelXY(x, HEIGHT - 1U, 0U);
    }
    else if (thisColor >= 0x900000)                                                          // если наш верхний пиксель максимальной яркости, резко снижаем яркость
      drawPixelXY(x, HEIGHT - 1U, 0x558800);
    else
      drawPixelXY(x, HEIGHT - 1U, thisColor - 0x0a1000);                                     // в остальных случаях снижаем яркость на 1 уровень
    //drawPixelXY(x, HEIGHT - 1U, thisColor - 0x050800);                                     // для длинных хвостов
  }
}

// =====================================
//    Светлячки 2 - Светлячки в банке
//     Мотыльки - Лампа с мотыльками
//            (c) SottNick
// =====================================

//#define trackingOBJECT_MAX_COUNT  (100U)          // максимальное количество мотыльков
#define BUTTERFLY_FIX_COUNT  (20U)                  // количество мотыльков для режима, когда бегунок Масштаб регулирует цвет
// -------------------------------------
void butterflys() { // Moтыльки
  butterflysRoutine(true);
};
// -------------------------------------
void lampWithButterflys() { // Лaмпa c мoтылькaми
  butterflysRoutine(false);
};
// -------------------------------------
void butterflysRoutine(bool isColored)
{
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    if (isColored) {
      uint8_t tmp = 66U + random8(83U);
      setModeSettings((tmp & 0x01) ? 65U + random8(36U) : 15U + random8(26U), tmp);
    }
    else
      setModeSettings(random8(21U) ? (random8(3U) ? 2U + random8(98U) : 1U) : 100U, 20U + random8(155U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  bool isWings = modes[currentMode].Speed & 0x01;
  if (loadingFlag) {
    loadingFlag = false;
    speedfactor = (float)modes[currentMode].Speed / 2048.0f + 0.001f;
    if (isColored) // для режима смены цвета фона фиксируем количество мотыльков
      deltaValue = (modes[currentMode].Scale > trackingOBJECT_MAX_COUNT) ? trackingOBJECT_MAX_COUNT : modes[currentMode].Scale;
    else
      deltaValue = BUTTERFLY_FIX_COUNT;
    for (uint8_t i = 0U; i < trackingOBJECT_MAX_COUNT; i++)
    {
      trackingObjectPosX[i] = random8(WIDTH);
      trackingObjectPosY[i] = random8(HEIGHT);
      trackingObjectSpeedX[i] = 0;
      trackingObjectSpeedY[i] = 0;
      trackingObjectShift[i] = 0;
      trackingObjectHue[i] = (isColored) ? random8() : 255U;
      trackingObjectState[i] = 255U;
    }
    //для инверсии, чтобы сто раз не пересчитывать
    if (modes[currentMode].Scale != 1U)
      hue = (float)(modes[currentMode].Scale - 1U) * 2.6;
    else
      hue = random8();
    //hue2 = (modes[currentMode].Scale == 100U) ? 0U : 255U;  // белый или цветной фон
    if (modes[currentMode].Scale == 100U) { // вместо белого будет желтоватая лампа
      hue2 = 170U;
      hue = 31U;
    }
    else
      hue2 = 255U;
  }
  if (isWings && isColored)
    dimAll(35U); // для крылышков
  else
    FastLED.clear();

  float maxspeed;
  uint8_t tmp;
  if (++step >= deltaValue)
    step = 0U;
  for (uint8_t i = 0U; i < deltaValue; i++)
  {
    trackingObjectPosX[i] += trackingObjectSpeedX[i] * speedfactor;
    trackingObjectPosY[i] += trackingObjectSpeedY[i] * speedfactor;

    if (trackingObjectPosX[i] < 0)
      trackingObjectPosX[i] = (float)(WIDTH - 1) + trackingObjectPosX[i];
    if (trackingObjectPosX[i] > WIDTH - 1)
      trackingObjectPosX[i] = trackingObjectPosX[i] + 1 - WIDTH;

    if (trackingObjectPosY[i] < 0)
    {
      trackingObjectPosY[i] = -trackingObjectPosY[i];
      trackingObjectSpeedY[i] = -trackingObjectSpeedY[i];
      //trackingObjectSpeedX[i] = -trackingObjectSpeedX[i];
    }
    if (trackingObjectPosY[i] > HEIGHT - 1U)
    {
      trackingObjectPosY[i] = (HEIGHT << 1U) - 2U - trackingObjectPosY[i];
      trackingObjectSpeedY[i] = -trackingObjectSpeedY[i];
      //trackingObjectSpeedX[i] = -trackingObjectSpeedX[i];
    }

    //проворот траектории
    maxspeed = fabs(trackingObjectSpeedX[i]) + fabs(trackingObjectSpeedY[i]); // максимальная суммарная скорость
    if (maxspeed == fabs(trackingObjectSpeedX[i] + trackingObjectSpeedY[i]))
    {
      if (trackingObjectSpeedX[i] > 0) // правый верхний сектор вектора
      {
        trackingObjectSpeedX[i] += trackingObjectShift[i];
        if (trackingObjectSpeedX[i] > maxspeed) // если вектор переехал вниз
        {
          trackingObjectSpeedX[i] = maxspeed + maxspeed - trackingObjectSpeedX[i];
          trackingObjectSpeedY[i] = trackingObjectSpeedX[i] - maxspeed;
        }
        else
          trackingObjectSpeedY[i] = maxspeed - fabs(trackingObjectSpeedX[i]);
      }
      else                           // левый нижний сектор
      {
        trackingObjectSpeedX[i] -= trackingObjectShift[i];
        if (trackingObjectSpeedX[i] + maxspeed < 0) // если вектор переехал вверх
        {
          trackingObjectSpeedX[i] = 0 - trackingObjectSpeedX[i] - maxspeed - maxspeed;
          trackingObjectSpeedY[i] = maxspeed - fabs(trackingObjectSpeedX[i]);
        }
        else
          trackingObjectSpeedY[i] = fabs(trackingObjectSpeedX[i]) - maxspeed;
      }
    }
    else //левый верхний и правый нижний секторы вектора
    {
      if (trackingObjectSpeedX[i] > 0) // правый нижний сектор
      {
        trackingObjectSpeedX[i] -= trackingObjectShift[i];
        if (trackingObjectSpeedX[i] > maxspeed) // если вектор переехал наверх
        {
          trackingObjectSpeedX[i] = maxspeed + maxspeed - trackingObjectSpeedX[i];
          trackingObjectSpeedY[i] = maxspeed - trackingObjectSpeedX[i];
        }
        else
          trackingObjectSpeedY[i] = fabs(trackingObjectSpeedX[i]) - maxspeed;
      }
      else                           // левый верхний сектор
      {
        trackingObjectSpeedX[i] += trackingObjectShift[i];
        if (trackingObjectSpeedX[i] + maxspeed < 0) // если вектор переехал вниз
        {
          trackingObjectSpeedX[i] = 0 - trackingObjectSpeedX[i] - maxspeed - maxspeed;
          trackingObjectSpeedY[i] = 0 - trackingObjectSpeedX[i] - maxspeed;
        }
        else
          trackingObjectSpeedY[i] = maxspeed - fabs(trackingObjectSpeedX[i]);
      }
    }

    if (trackingObjectState[i] == 255U)
    {
      if (step == i && random8(2U) == 0U)//(step == 0U && ((pcnt + i) & 0x01))
      {
        trackingObjectState[i] = random8(220U, 244U);
        trackingObjectSpeedX[i] = (float)random8(101U) / 20.0f + 1.0f;
        if (random8(2U) == 0U) trackingObjectSpeedX[i] = -trackingObjectSpeedX[i];
        trackingObjectSpeedY[i] = (float)random8(101U) / 20.0f + 1.0f;
        if (random8(2U) == 0U) trackingObjectSpeedY[i] = -trackingObjectSpeedY[i];
        // проворот траектории
        //trackingObjectShift[i] = (float)random8((fabs(trackingObjectSpeedX[i])+fabs(trackingObjectSpeedY[i]))*2.0+2.0) / 40.0f;
        trackingObjectShift[i] = (float)random8((fabs(trackingObjectSpeedX[i]) + fabs(trackingObjectSpeedY[i])) * 20.0f + 2.0f) / 200.0f;
        if (random8(2U) == 0U) trackingObjectShift[i] = -trackingObjectShift[i];
      }
    }
    else
    {
      if (step == i)
        trackingObjectState[i]++;
      tmp = 255U - trackingObjectState[i];
      if (tmp == 0U || ((uint16_t)(trackingObjectPosX[i] * tmp) % tmp == 0U && (uint16_t)(trackingObjectPosY[i] * tmp) % tmp == 0U))
      {
        trackingObjectPosX[i] = round(trackingObjectPosX[i]);
        trackingObjectPosY[i] = round(trackingObjectPosY[i]);
        trackingObjectSpeedX[i] = 0;
        trackingObjectSpeedY[i] = 0;
        trackingObjectShift[i] = 0;
        trackingObjectState[i] = 255U;
      }
    }

    if (isWings)
      drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], CHSV(trackingObjectHue[i], 255U, (trackingObjectState[i] == 255U) ? 255U : 128U + random8(2U) * 111U)); // это процедура рисования с нецелочисленными координатами. ищите её в прошивке
    else
      drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], CHSV(trackingObjectHue[i], 255U, trackingObjectState[i])); // это процедура рисования с нецелочисленными координатами. ищите её в прошивке
  }

  // постобработка кадра
  if (isColored) {
    for (uint8_t i = 0U; i < deltaValue; i++) // ещё раз рисуем всех Мотыльков, которые "сидят на стекле"
      if (trackingObjectState[i] == 255U)
        drawPixelXY(trackingObjectPosX[i], trackingObjectPosY[i], CHSV(trackingObjectHue[i], 255U, trackingObjectState[i]));
  }
  else {
    //теперь инверсия всей матрицы
    if (modes[currentMode].Scale == 1U)
      if (++deltaHue == 0U) hue++;
    for (uint16_t i = 0U; i < NUM_LEDS; i++)
      leds[i] = CHSV(hue, hue2, 255U - leds[i].r);
  }
}

// =====================================
//              Светлячки
// =====================================
//#define LIGHTERS_AM           (100U)  // для экономии памяти берём trackingOBJECT_MAX_COUNT
void lightersRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(14U + random8(43U), 100U + random8(81U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    //randomSeed(millis());
    if (modes[currentMode].Scale > trackingOBJECT_MAX_COUNT) modes[currentMode].Scale = trackingOBJECT_MAX_COUNT;
    for (uint8_t i = 0U; i < trackingOBJECT_MAX_COUNT; i++)
    {
      trackingObjectPosX[i] = random(0, WIDTH * 10);
      trackingObjectPosY[i] = random(0, HEIGHT * 10);
      trackingObjectSpeedX[i] = random(-10, 10);
      trackingObjectSpeedY[i] = random(-10, 10);
      //lightersColor[i] = CHSV(random(0U, 255U), 255U, 255U);
      trackingObjectHue[i] = random8();
    }
  }
  FastLED.clear();
  if (++step > 20U) step = 0U;
  for (uint8_t i = 0U; i < modes[currentMode].Scale; i++)
  {
    if (step == 0U)                                  // меняем скорость каждые 255 отрисовок
    {
      trackingObjectSpeedX[i] += random(-3, 4);
      trackingObjectSpeedY[i] += random(-3, 4);
      trackingObjectSpeedX[i] = constrain(trackingObjectSpeedX[i], -20, 20);
      trackingObjectSpeedY[i] = constrain(trackingObjectSpeedY[i], -20, 20);
    }

    trackingObjectPosX[i] += trackingObjectSpeedX[i];
    trackingObjectPosY[i] += trackingObjectSpeedY[i];

    if (trackingObjectPosX[i] < 0) trackingObjectPosX[i] = (WIDTH - 1) * 10;
    if (trackingObjectPosX[i] >= (int32_t)(WIDTH * 10)) trackingObjectPosX[i] = 0;

    if (trackingObjectPosY[i] < 0)
    {
      trackingObjectPosY[i] = 0;
      trackingObjectSpeedY[i] = -trackingObjectSpeedY[i];
    }
    if (trackingObjectPosY[i] >= (int32_t)(HEIGHT - 1) * 10)
    {
      trackingObjectPosY[i] = (HEIGHT - 1U) * 10;
      trackingObjectSpeedY[i] = -trackingObjectSpeedY[i];
    }
    //drawPixelXY(trackingObjectPosX[i] / 10, trackingObjectPosY[i] / 10, lightersColor[i]);
    drawPixelXY(trackingObjectPosX[i] / 10, trackingObjectPosY[i] / 10, CHSV(trackingObjectHue[i], 255U, 255U));
  }
}

// =====================================
//         Светлячки со шлейфом
// =====================================
#define BALLS_AMOUNT          (3U)                          // количество "шариков"
#define CLEAR_PATH            (1U)                          // очищать путь
#define BALL_TRACK            (1U)                          // (0 / 1) - вкл/выкл следы шариков
#define TRACK_STEP            (70U)                         // длина хвоста шарика (чем больше цифра, тем хвост короче)
int16_t coord[BALLS_AMOUNT][2U];
int8_t vector[BALLS_AMOUNT][2U];
CRGB ballColors[BALLS_AMOUNT];
void ballsRoutine()
{
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(100U) , 190U + random8(31U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;

    for (uint8_t j = 0U; j < BALLS_AMOUNT; j++) {
      int8_t sign;
      // забиваем случайными данными
      coord[j][0U] = WIDTH / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][0U] = random(4, 15) * sign;
      coord[j][1U] = HEIGHT / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][1U] = random(4, 15) * sign;
      //ballColors[j] = CHSV(random(0, 9) * 28, 255U, 255U);
      // цвет зависит от масштаба
      ballColors[j] = CHSV((modes[currentMode].Scale * (j + 1)) % 256U, 255U, 255U);
    }
  }

  if (!BALL_TRACK) {                                         // режим без следов шариков
    FastLED.clear();
  } else {                                                   // режим со следами
    //fader(TRACK_STEP);
    dimAll(256U - TRACK_STEP);
  }

  // движение шариков
  for (uint8_t j = 0U; j < BALLS_AMOUNT; j++)
  {
    // движение шариков
    for (uint8_t i = 0U; i < 2U; i++)
    {
      coord[j][i] += vector[j][i];
      if (coord[j][i] < 0)
      {
        coord[j][i] = 0;
        vector[j][i] = -vector[j][i];
      }
    }

    if (coord[j][0U] > (int16_t)((WIDTH - 1) * 10))
    {
      coord[j][0U] = (WIDTH - 1) * 10;
      vector[j][0U] = -vector[j][0U];
    }
    if (coord[j][1U] > (int16_t)((HEIGHT - 1) * 10))
    {
      coord[j][1U] = (HEIGHT - 1) * 10;
      vector[j][1U] = -vector[j][1U];
    }
    //leds[XY(coord[j][0U] / 10, coord[j][1U] / 10)] =  ballColors[j];
    drawPixelXYF(coord[j][0U] / 10., coord[j][1U] / 10., ballColors[j]);
  }
}

// =====================================
//              Пейнтбол
// =====================================
#define BORDERTHICKNESS (1U) // глубина бордюра для размытия яркой частицы: 0U - без границы (резкие края); 1U - 1 пиксель (среднее размытие) ; 2U - 2 пикселя (глубокое размытие)
const uint8_t paintWidth = WIDTH - BORDERTHICKNESS * 2;
const uint8_t paintHeight = HEIGHT - BORDERTHICKNESS * 2;

void lightBallsRoutine() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(1U + random8(100U) , 230U + random8(16U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly. Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
  //  uint8_t blurAmount = dim8_raw(beatsin8(3, 64, 100));
  //  blur2d(leds, WIDTH, HEIGHT, blurAmount);

  blurScreen(dim8_raw(beatsin8(3, 64, 100)));

  // Use two out-of-sync sine waves
  uint16_t i = beatsin16( 79, 0, 255); //91
  uint16_t j = beatsin16( 67, 0, 255); //109
  uint16_t k = beatsin16( 53, 0, 255); //73
  uint16_t m = beatsin16( 97, 0, 255); //123

  // The color of each point shifts over time, each at a different speed.
  uint32_t ms = millis() / (modes[currentMode].Scale / 4 + 1);
  leds[XY( highByte(i * paintWidth) + BORDERTHICKNESS, highByte(j * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 29, 200U, 255U);
  leds[XY( highByte(j * paintWidth) + BORDERTHICKNESS, highByte(k * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 41, 200U, 255U);
  leds[XY( highByte(k * paintWidth) + BORDERTHICKNESS, highByte(m * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 37, 200U, 255U);
  leds[XY( highByte(m * paintWidth) + BORDERTHICKNESS, highByte(i * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 53, 200U, 255U);
}


// =====================================
//             Белый свет
//            (c) SottNick
// =====================================
#define BORDERLAND   2              // две дополнительные единицы бегунка Масштаб на границе вертикального и горизонтального варианта эффекта (с каждой стороны границы) будут для света всеми светодиодами в полную силу
void whiteColorStripeRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(11U + random8(83U), 1U + random8(255U / WIDTH + 1U) * WIDTH);
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    FastLED.clear();

    uint8_t thisSize = HEIGHT;
    uint8_t halfScale = modes[currentMode].Scale;
    if (halfScale > 50U) {
      thisSize = WIDTH;
      halfScale = 101U - halfScale;
    }
    halfScale = constrain(halfScale, 0U, 50U - BORDERLAND);

    uint8_t center =  (uint8_t)round(thisSize / 2.0F) - 1U;
    uint8_t offset = (uint8_t)(!(thisSize & 0x01));

    uint8_t fullFill =  center / (50.0 - BORDERLAND) * halfScale;
    uint8_t iPol = (center / (50.0 - BORDERLAND) * halfScale - fullFill) * 255;

    for (int16_t i = center; i >= 0; i--) {
      CRGB color = CHSV(
                     45U,                                                                              // определяем тон
                     map(modes[currentMode].Speed, 0U, 255U, 0U, 170U),                                // определяем насыщенность
                     i > (center - fullFill - 1)                                                       // определяем яркость
                     ? 255U                                                                            // для центральных горизонтальных полос
                     : iPol * (i > center - fullFill - 2));  // для остальных горизонтальных полос яркость равна либо 255, либо 0 в зависимости от масштаба

      if (modes[currentMode].Scale <= 50U) {
        for (uint8_t x = 0U; x < WIDTH; x++) {
          drawPixelXY(x, i, color);                         // при чётной высоте матрицы максимально яркими отрисуются 2 центральных горизонтальных полосы
          drawPixelXY(x, HEIGHT + offset - i - 2U, color);  // при нечётной - одна, но дважды
        }
      } else {
        for (uint8_t y = 0U; y < HEIGHT; y++) {
          drawPixelXY((i + modes[currentMode].Speed - 1U) % WIDTH, y, color);                    // при чётной ширине матрицы максимально яркими отрисуются 2 центральных вертикальных полосы
          drawPixelXY((WIDTH + offset - i + modes[currentMode].Speed - 3U) % WIDTH, y, color);   // при нечётной - одна, но дважды
        }
      }
    }
  }
}

// =====================================
//                Кометы
// =====================================
// далее идут общие процедуры для эффектов от Stefan Petrick, а непосредственно Комета - в самом низу
int8_t zD;
int8_t zF;
// The coordinates for 3 16-bit noise spaces.
#define NUM_LAYERS 1 // в кометах используется 1 слой, но для огня 2018 нужно 2

uint32_t noise32_x[NUM_LAYERSMAX];
uint32_t noise32_y[NUM_LAYERSMAX];
uint32_t noise32_z[NUM_LAYERSMAX];
uint32_t scale32_x[NUM_LAYERSMAX];
uint32_t scale32_y[NUM_LAYERSMAX];

uint8_t noisesmooth;
bool eNs_isSetupped;

void eNs_setup() {
  noisesmooth = 200;
  for (uint8_t i = 0; i < NUM_LAYERS; i++) {
    noise32_x[i] = random16();
    noise32_y[i] = random16();
    noise32_z[i] = random16();
    scale32_x[i] = 6000;
    scale32_y[i] = 6000;
  }
  eNs_isSetupped = true;
}

void FillNoise(int8_t layer) {
  for (uint8_t i = 0; i < WIDTH; i++) {
    int32_t ioffset = scale32_x[layer] * (i - CENTER_X_MINOR);
    for (uint8_t j = 0; j < HEIGHT; j++) {
      int32_t joffset = scale32_y[layer] * (j - CENTER_Y_MINOR);
      int8_t data = inoise16(noise32_x[layer] + ioffset, noise32_y[layer] + joffset, noise32_z[layer]) >> 8;
      int8_t olddata = noise3d[layer][i][j];
      int8_t newdata = scale8( olddata, noisesmooth ) + scale8( data, 255 - noisesmooth );
      data = newdata;
      noise3d[layer][i][j] = data;
    }
  }
}

/* эти функции в данных эффектах не используются, но на всякий случай уже адаптированы
  void MoveX(int8_t delta) {
  //CLS2();
  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH - delta; x++) {
      ledsbuff[XY(x, y)] = leds[XY(x + delta, y)];
    }
    for (uint8_t x = WIDTH - delta; x < WIDTH; x++) {
      ledsbuff[XY(x, y)] = leds[XY(x + delta - WIDTH, y)];
    }
  }
  //CLS();
  // write back to leds
  memcpy(leds, ledsbuff, sizeof(CRGB)* NUM_LEDS);
  //какого хера тут было поштучное копирование - я хз
  //for (uint8_t y = 0; y < HEIGHT; y++) {
  //  for (uint8_t x = 0; x < WIDTH; x++) {
  //    leds[XY(x, y)] = ledsbuff[XY(x, y)];
  //  }
  //}
  }

  void MoveY(int8_t delta) {
  //CLS2();
  for (uint8_t x = 0; x < WIDTH; x++) {
    for (uint8_t y = 0; y < HEIGHT - delta; y++) {
      ledsbuff[XY(x, y)] = leds[XY(x, y + delta)];
    }
    for (uint8_t y = HEIGHT - delta; y < HEIGHT; y++) {
      ledsbuff[XY(x, y)] = leds[XY(x, y + delta - HEIGHT)];
    }
  }
  //CLS();
  // write back to leds
  memcpy(leds, ledsbuff, sizeof(CRGB)* NUM_LEDS);
  //какого хера тут было поштучное копирование - я хз
  //for (uint8_t y = 0; y < HEIGHT; y++) {
  //  for (uint8_t x = 0; x < WIDTH; x++) {
  //    leds[XY(x, y)] = ledsbuff[XY(x, y)];
  //  }
  //}
  }
*/

void MoveFractionalNoiseX(int8_t amplitude = 1, float shift = 0) {
  for (uint8_t y = 0; y < HEIGHT; y++) {
    int16_t amount = ((int16_t)noise3d[0][0][y] - 128) * 2 * amplitude + shift * 256  ;
    int8_t delta = abs(amount) >> 8 ;
    int8_t fraction = abs(amount) & 255;
    for (uint8_t x = 0 ; x < WIDTH; x++) {
      if (amount < 0) {
        zD = x - delta; zF = zD - 1;
      } else {
        zD = x + delta; zF = zD + 1;
      }
      CRGB PixelA = CRGB::Black  ;
      if ((zD >= 0) && (zD < WIDTH)) PixelA = leds[XY(zD, y)];
      CRGB PixelB = CRGB::Black ;
      if ((zF >= 0) && (zF < WIDTH)) PixelB = leds[XY(zF, y)];
      ledsbuff[XY(x, y)] = (PixelA.nscale8(ease8InOutApprox(255 - fraction))) + (PixelB.nscale8(ease8InOutApprox(fraction)));   // lerp8by8(PixelA, PixelB, fraction );
    }
  }
  memcpy(leds, ledsbuff, sizeof(CRGB)* NUM_LEDS);
}

void MoveFractionalNoiseY(int8_t amplitude = 1, float shift = 0) {
  for (uint8_t x = 0; x < WIDTH; x++) {
    int16_t amount = ((int16_t)noise3d[0][x][0] - 128) * 2 * amplitude + shift * 256 ;
    int8_t delta = abs(amount) >> 8 ;
    int8_t fraction = abs(amount) & 255;
    for (uint8_t y = 0 ; y < HEIGHT; y++) {
      if (amount < 0) {
        zD = y - delta; zF = zD - 1;
      } else {
        zD = y + delta; zF = zD + 1;
      }
      CRGB PixelA = CRGB::Black ;
      if ((zD >= 0) && (zD < HEIGHT)) PixelA = leds[XY(x, zD)];
      CRGB PixelB = CRGB::Black ;
      if ((zF >= 0) && (zF < HEIGHT)) PixelB = leds[XY(x, zF)];
      ledsbuff[XY(x, y)] = (PixelA.nscale8(ease8InOutApprox(255 - fraction))) + (PixelB.nscale8(ease8InOutApprox(fraction)));
    }
  }
  memcpy(leds, ledsbuff, sizeof(CRGB)* NUM_LEDS);
}

// NoiseSmearing(by StefanPetrick) Effect mod for GyverLamp by PalPalych
void MultipleStream() { // 2 comets
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      hue = random8();
      hue2 = hue + 85U;
      setModeSettings(1U + random8(25U), 185U + random8(36U));
    }
    else {
      hue = 0U; // 0xFF0000
      hue2 = 43U; // 0xFFFF00
    }
#else
    hue = 0U; // 0xFF0000
    hue2 = 43U; // 0xFFFF00
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    trackingObjectState[0] = WIDTH / 8;
    trackingObjectState[1] = HEIGHT / 8;
    trackingObjectShift[0] = 255. / (WIDTH - 1. - trackingObjectState[0] - trackingObjectState[0]);
    trackingObjectShift[1] = 255. / (HEIGHT - 1. - trackingObjectState[1] - trackingObjectState[1]);
    trackingObjectState[2] = WIDTH / 4;
    trackingObjectState[3] = HEIGHT / 4;
    trackingObjectShift[2] = 255. / (WIDTH - 1. - trackingObjectState[2] - trackingObjectState[2]); // ((WIDTH>10)?9.:5.));
    trackingObjectShift[3] = 255. / (HEIGHT - 1. - trackingObjectState[3] - trackingObjectState[3]); //- ((HEIGHT>10)?9.:5.));
  }

  //dimAll(192); // < -- затухание эффекта для последующего кадрв
  dimAll(255U - modes[currentMode].Scale * 2);


  // gelb im Kreis
  byte xx = trackingObjectState[0] + sin8( millis() / 10) / trackingObjectShift[0];// / 22;
  byte yy = trackingObjectState[1] + cos8( millis() / 10) / trackingObjectShift[1];// / 22;
  if (xx < WIDTH && yy < HEIGHT)
    leds[XY( xx, yy)] = CHSV(hue2 , 255, 255);//0xFFFF00;

  // rot in einer Acht
  xx = trackingObjectState[2] + sin8( millis() / 46) / trackingObjectShift[2];// / 32;
  yy = trackingObjectState[3] + cos8( millis() / 15) / trackingObjectShift[3];// / 32;
  if (xx < WIDTH && yy < HEIGHT)
    leds[XY( xx, yy)] = CHSV(hue , 255, 255);//0xFF0000;

  // Noise
  noise32_x[0] += 3000;
  noise32_y[0] += 3000;
  noise32_z[0] += 3000;
  scale32_x[0] = 8000;
  scale32_y[0] = 8000;
  FillNoise(0);
  MoveFractionalNoiseX(3, 0.33);
  MoveFractionalNoiseY(3);
}
// =====================================
//             Три кометы
// =====================================
void MultipleStream2() { // 3 comets
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      hue = random8();
      hue2 = hue + 85U;
      deltaHue = hue2 + 85U;
      setModeSettings(1U + random8(25U), 185U + random8(36U));
    } else {
      hue = 0U; // 0xFF0000
      hue2 = 43U; // 0xFFFF00
      deltaHue = 171U; //0x0000FF;
    }
#else
    hue = 0U; // 0xFF0000
    hue2 = 43U; // 0xFFFF00
    deltaHue = 171U; //0x0000FF;
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    trackingObjectState[0] = WIDTH / 8;
    trackingObjectState[1] = HEIGHT / 8;
    trackingObjectShift[0] = 255. / (WIDTH - 1. - trackingObjectState[0] - trackingObjectState[0]);
    trackingObjectShift[1] = 255. / (HEIGHT - 1. - trackingObjectState[1] - trackingObjectState[1]);
    trackingObjectState[2] = WIDTH / 4;
    trackingObjectState[3] = HEIGHT / 4;
    trackingObjectShift[2] = 255. / (WIDTH - 1. - trackingObjectState[2] - trackingObjectState[2]); // ((WIDTH>10)?9.:5.));
    trackingObjectShift[3] = 255. / (HEIGHT - 1. - trackingObjectState[3] - trackingObjectState[3]); //- ((HEIGHT>10)?9.:5.));
  }
  // затухание эффекта для последующего кадрв
  dimAll(255U - modes[currentMode].Scale * 2);

  //byte xx = 2 + sin8( millis() / 10) / 22;
  //byte yy = 2 + cos8( millis() / 9) / 22;
  byte xx = trackingObjectState[0] + sin8( millis() / 10) / trackingObjectShift[0];// / 22;
  byte yy = trackingObjectState[1] + cos8( millis() / 9) / trackingObjectShift[1];// / 22;

  if (xx < WIDTH && yy < HEIGHT) {
    leds[XY( xx, yy)] += CHSV(deltaHue , 255, 255);//0x0000FF;
  }

  xx = trackingObjectState[2] + sin8( millis() / 10) / trackingObjectShift[2];// / 32;
  yy = trackingObjectState[3] + cos8( millis() / 7) / trackingObjectShift[3];// / 32;
  if (xx < WIDTH && yy < HEIGHT)
    leds[XY( xx, yy)] += CHSV(hue , 255, 255);//0xFF0000;
  leds[XY( CENTER_X_MINOR, CENTER_Y_MINOR)] += CHSV(hue2 , 255, 255);//0xFFFF00;

  noise32_x[0] += 3000;
  noise32_y[0] += 3000;
  noise32_z[0] += 3000;
  scale32_x[0] = 8000;
  scale32_y[0] = 8000;
  FillNoise(0);
  MoveFractionalNoiseX(2);
  MoveFractionalNoiseY(2, 0.33);
}
// =====================================
//            Пapящий oгoнь
// =====================================
void MultipleStream3() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(1U + random8(26U), 180U + random8(45U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  blurScreen(20); // без размытия как-то пиксельно, по-моему...
  //dimAll(160); // < -- затухание эффекта для последующего кадров
  dimAll(255U - modes[currentMode].Scale * 2);
  for (uint8_t i = 1; i < WIDTH; i += 3) {
    leds[XY( i, CENTER_Y_MINOR)] += CHSV(i * 2 , 255, 255);
  }
  // Noise
  noise32_x[0] += 3000;
  noise32_y[0] += 3000;
  noise32_z[0] += 3000;
  scale32_x[0] = 8000;
  scale32_y[0] = 8000;
  FillNoise(0);
  MoveFractionalNoiseY(3);
  MoveFractionalNoiseX(3);
}

// =====================================
//            Bepxoвoй oгoнь
// =====================================
void MultipleStream5() { // Fractorial Fire
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(1U + random8(26U), 180U + random8(45U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  blurScreen(20); // без размытия как-то пиксельно, по-моему...
  //dimAll(140); // < -- затухание эффекта для последующего кадрв
  dimAll(255U - modes[currentMode].Scale * 2);
  for (uint8_t i = 1; i < WIDTH; i += 2) {
    leds[XY( i, HEIGHT - 1)] += CHSV(i * 2, 255, 255);
  }
  // Noise
  noise32_x[0] += 3000;
  noise32_y[0] += 3000;
  noise32_z[0] += 3000;
  scale32_x[0] = 8000;
  scale32_y[0] = 8000;
  FillNoise(0);
  //MoveX(1);
  //MoveY(1);
  MoveFractionalNoiseY(2, 1);
  MoveFractionalNoiseX(2);
}

// =====================================
//            Paдyжный змeй
// =====================================
void MultipleStream8() {            // Windows
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(random8(2U) ? 1U : 2U + random8(99U), 155U + random8(76U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (loadingFlag) {
    loadingFlag = false;
    if (modes[currentMode].Scale > 1U) {
      hue = (modes[currentMode].Scale - 2U) * 2.6;
    } else {
      hue = random8();
    }
  }
  if (modes[currentMode].Scale <= 1U) {
    hue++;
  }
  dimAll(96); // < -- затухание эффекта для последующего кадра на 96/255*100=37%
  for (uint8_t y = 2; y < HEIGHT - 1; y += 5) {
    for (uint8_t x = 2; x < WIDTH - 1; x += 5) {
      leds[XY(x, y)]  += CHSV(x * y + hue, 255, 255);
      leds[XY(x + 1, y)] += CHSV((x + 4) * y + hue, 255, 255);
      leds[XY(x, y + 1)] += CHSV(x * (y + 4) + hue, 255, 255);
      leds[XY(x + 1, y + 1)] += CHSV((x + 4) * (y + 4) + hue, 255, 255);
    }
  }
  // Noise
  noise32_x[0] += 3000;
  noise32_y[0] += 3000;
  noise32_z[0] += 3000;
  scale32_x[0] = 8000;
  scale32_y[0] = 8000;
  FillNoise(0);

  MoveFractionalNoiseX(3);
  MoveFractionalNoiseY(3);
}

// =====================================
//                Koмeтa
// =====================================
// Follow the Rainbow Comet by Palpalych
//    Effect for GyverLamp 02/03/2020
void RainbowCometRoutine() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(10U + random8(91U), 185U + random8(51U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  dimAll(254U); // < -- затухание эффекта для последующего кадра
  CRGB _eNs_color = CHSV(millis() / modes[currentMode].Scale * 2, 255, 255);
  leds[XY(CENTER_X_MINOR, CENTER_Y_MINOR)] += _eNs_color;
  leds[XY(CENTER_X_MINOR + 1, CENTER_Y_MINOR)] += _eNs_color;
  leds[XY(CENTER_X_MINOR, CENTER_Y_MINOR + 1)] += _eNs_color;
  leds[XY(CENTER_X_MINOR + 1, CENTER_Y_MINOR + 1)] += _eNs_color;

  // Noise
  noise32_x[0] += 1500;
  noise32_y[0] += 1500;
  noise32_z[0] += 1500;
  scale32_x[0] = 8000;
  scale32_y[0] = 8000;
  FillNoise(0);
  MoveFractionalNoiseX(WIDTH / 2U - 1U);
  MoveFractionalNoiseY(HEIGHT / 2U - 1U);
}

// Кометы белые и одноцветные
void ColorCometRoutine() {      // <- ******* для оригинальной прошивки Gunner47 ******* (раскомментить/закоментить)
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(random8(20U) ? 1U + random8(99U) : 100U, 185U + random8(51U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  dimAll(254U); // < -- затухание эффекта для последующего кадра
  CRGB _eNs_color = CRGB::White;

  if (modes[currentMode].Scale < 100) _eNs_color = CHSV((modes[currentMode].Scale) * 2.57, 255, 255); // 2.57 вместо 2.55, потому что при 100 будет белый цвет
  leds[XY(CENTER_X_MINOR, CENTER_Y_MINOR)] += _eNs_color;
  leds[XY(CENTER_X_MINOR + 1, CENTER_Y_MINOR)] += _eNs_color;
  leds[XY(CENTER_X_MINOR, CENTER_Y_MINOR + 1)] += _eNs_color;
  leds[XY(CENTER_X_MINOR + 1, CENTER_Y_MINOR + 1)] += _eNs_color;

  // Noise
  noise32_x[0] += 1500;
  noise32_y[0] += 1500;
  noise32_z[0] += 1500;
  scale32_x[0] = 8000;
  scale32_y[0] = 8000;
  FillNoise(0);
  MoveFractionalNoiseX(WIDTH / 2U - 1U);
  MoveFractionalNoiseY(HEIGHT / 2U - 1U);
}
// =====================================
//                Мячики
// =====================================
//  BouncingBalls2014 is a program that lets you animate an LED strip
//  to look like a group of bouncing balls
//  Daniel Wilson, 2014
//  https://github.com/githubcdr/Arduino/blob/master/bouncingballs/bouncingballs.ino
//  With BIG thanks to the FastLED community!
//  адаптация от SottNick

#define bballsGRAVITY           (-9.81)              // Downward (negative) acceleration of gravity in m/s^2
#define bballsH0                (1)                  // Starting height, in meters, of the ball (strip length)
float bballsVImpact0 = SQRT_VARIANT( -2 * bballsGRAVITY * bballsH0 );  // Impact velocity of the ball when it hits the ground if "dropped" from the top of the strip
void BBallsRoutine() {
  if (loadingFlag)  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(26U + random8(32U), random8(3U) ? ((random8(4U) ? 127U : 0U) + 9U + random8(12U)) : (random8(4U) ? 255U : 127U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    //FastLED.clear();
    enlargedObjectNUM = (modes[currentMode].Scale - 1U) / 99.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
    if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) {
      enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
    }
    for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {             // Initialize variables
      trackingObjectHue[i] = random8();
      trackingObjectState[i] = random8(0U, WIDTH);
      enlargedObjectTime[i] = millis();
      trackingObjectPosY[i] = 0U;                                   // Balls start on the ground
      trackingObjectSpeedY[i] = bballsVImpact0;                     // And "pop" up at vImpact0
      trackingObjectShift[i] = 0.90 - float(i) / pow(enlargedObjectNUM, 2); // это, видимо, прыгучесть. для каждого мячика уникальная изначально
      trackingObjectIsShift[i] = false;
      hue2 = (modes[currentMode].Speed > 127U) ? 255U : 0U;                                           // цветные или белые мячики
      hue = (modes[currentMode].Speed == 128U) ? 255U : 254U - modes[currentMode].Speed % 128U * 2U;  // скорость угасания хвостов 0 = моментально
    }
  }

  float bballsHi;
  float bballsTCycle;
  if (deltaValue++ & 0x01) deltaHue++; // постепенное изменение оттенка мячиков (закомментировать строчку, если не нужно)
  dimAll(hue);
  for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {
    //leds[XY(trackingObjectState[i], trackingObjectPosY[i])] = CRGB::Black; // off for the next loop around  // теперь пиксели гасятся в dimAll()

    bballsTCycle =  (millis() - enlargedObjectTime[i]) / 1000. ; // Calculate the time since the last time the ball was on the ground

    // A little kinematics equation calculates positon as a function of time, acceleration (gravity) and intial velocity
    //bballsHi = 0.5 * bballsGRAVITY * pow(bballsTCycle, 2) + trackingObjectSpeedY[i] * bballsTCycle;
    bballsHi = 0.5 * bballsGRAVITY * bballsTCycle * bballsTCycle + trackingObjectSpeedY[i] * bballsTCycle;

    if ( bballsHi < 0 ) {
      enlargedObjectTime[i] = millis();
      bballsHi = 0; // If the ball crossed the threshold of the "ground," put it back on the ground
      trackingObjectSpeedY[i] = trackingObjectShift[i] * trackingObjectSpeedY[i] ; // and recalculate its new upward velocity as it's old velocity * COR
      if ( trackingObjectSpeedY[i] < 0.01 ) {                                         // If the ball is barely moving, "pop" it back up at vImpact0
        trackingObjectShift[i] = 0.90 - float(random8(9U)) / pow(random8(4U, 9U), 2); // сделал, чтобы мячики меняли свою прыгучесть каждый цикл
        trackingObjectIsShift[i] = trackingObjectShift[i] >= 0.89;                             // если мячик максимальной прыгучести, то разрешаем ему сдвинуться
        trackingObjectSpeedY[i] = bballsVImpact0;
      }
    }

    trackingObjectPosY[i] = constrain(round( bballsHi * (HEIGHT - 1) / bballsH0), 0, HEIGHT - 1);   // Map "h" to a "pos" integer index position on the LED strip
    if (trackingObjectIsShift[i] && (trackingObjectPosY[i] == HEIGHT - 1)) {                        // если мячик получил право, то пускай сдвинется на максимальной высоте 1 раз
      trackingObjectIsShift[i] = false;
      if (trackingObjectHue[i] & 0x01) {                                                            // нечётные налево, чётные направо
        if (trackingObjectState[i] == 0U) trackingObjectState[i] = WIDTH - 1U;
        else --trackingObjectState[i];
      } else {
        if (trackingObjectState[i] == WIDTH - 1U) trackingObjectState[i] = 0U;
        else ++trackingObjectState[i];
      }
    }
    leds[XY(trackingObjectState[i], trackingObjectPosY[i])] = CHSV(trackingObjectHue[i] + deltaHue, hue2, 255U);
    //drawPixelXY(trackingObjectState[i], trackingObjectPosY[i], CHSV(trackingObjectHue[i] + deltaHue, hue2, 255U));  //на случай, если останутся жалобы, что эффект вылетает
  }
}

// =====================================
//              Спирали
// =====================================
/*
   Aurora: https://github.com/pixelmatix/aurora
   https://github.com/pixelmatix/aurora/blob/sm3.0-64x64/PatternSpiro.h
   Copyright (c) 2014 Jason Coon
   Неполная адаптация SottNick
*/
byte spirotheta1 = 0;
byte spirotheta2 = 0;
const uint8_t spiroradiusx = WIDTH / 4;// - 1;
const uint8_t spiroradiusy = HEIGHT / 4;// - 1;
const uint8_t spirocenterX = WIDTH / 2;
const uint8_t spirocenterY = HEIGHT / 2;

const uint8_t spirominx = spirocenterX - spiroradiusx;
const uint8_t spiromaxx = spirocenterX + spiroradiusx - (WIDTH % 2 == 0 ? 1 : 0); //+ 1;
const uint8_t spirominy = spirocenterY - spiroradiusy;
const uint8_t spiromaxy = spirocenterY + spiroradiusy - (HEIGHT % 2 == 0 ? 1 : 0); //+ 1;

uint8_t spirocount = 1;
uint8_t spirooffset = 256 / spirocount;
boolean spiroincrement = false;

boolean spirohandledChange = false;

uint8_t mapsin8(uint8_t theta, uint8_t lowest = 0, uint8_t highest = 255) {
  uint8_t beatsin = sin8(theta);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatsin, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}

uint8_t mapcos8(uint8_t theta, uint8_t lowest = 0, uint8_t highest = 255) {
  uint8_t beatcos = cos8(theta);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatcos, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}

void spiroRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t rnd = random8(6U);
      if (rnd > 1U) rnd++;
      if (rnd > 3U) rnd++;
      setModeSettings(rnd * 11U + 3U, random8(10U) ? 2U + random8(26U) : 255U);
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();
  }

  blurScreen(20); // @Palpalych советует делать размытие
  dimAll(255U - modes[currentMode].Speed / 10);

  boolean change = false;

  for (uint8_t i = 0; i < spirocount; i++) {
    uint8_t x = mapsin8(spirotheta1 + i * spirooffset, spirominx, spiromaxx);
    uint8_t y = mapcos8(spirotheta1 + i * spirooffset, spirominy, spiromaxy);

    uint8_t x2 = mapsin8(spirotheta2 + i * spirooffset, x - spiroradiusx, x + spiroradiusx);
    uint8_t y2 = mapcos8(spirotheta2 + i * spirooffset, y - spiroradiusy, y + spiroradiusy);


    //CRGB color = ColorFromPalette( PartyColors_p, (hue + i * spirooffset), 128U); // вообще-то палитра должна постоянно меняться, но до адаптации этого руки уже не дошли
    //CRGB color = ColorFromPalette(*curPalette, hue + i * spirooffset, 128U); // вот так уже прикручена к бегунку Масштаба. за
    //leds[XY(x2, y2)] += color;
    if (x2 < WIDTH && y2 < HEIGHT) // добавил проверки. не знаю, почему эффект подвисает без них
      leds[XY(x2, y2)] += (CRGB)ColorFromPalette(*curPalette, hue + i * spirooffset);

    if ((x2 == spirocenterX && y2 == spirocenterY) ||
        (x2 == spirocenterX && y2 == spirocenterY)) change = true;
  }

  spirotheta2 += 2;

  //      EVERY_N_MILLIS(12) { маловата задержочка
  spirotheta1 += 1;
  //      }

  EVERY_N_MILLIS(75) {
    if (change && !spirohandledChange) {
      spirohandledChange = true;

      if (spirocount >= WIDTH || spirocount == 1) spiroincrement = !spiroincrement;

      if (spiroincrement) {
        if (spirocount >= 4)
          spirocount *= 2;
        else
          spirocount += 1;
      }
      else {
        if (spirocount > 4)
          spirocount /= 2;
        else
          spirocount -= 1;
      }

      spirooffset = 256 / spirocount;
    }

    if (!change) spirohandledChange = false;
  }

  //      EVERY_N_MILLIS(33) { маловата задержочка
  hue += 1;
  //      }
}

// =====================================
//              МетаБолз
// =====================================
// https://gist.github.com/StefanPetrick/170fbf141390fafb9c0c76b8a0d34e54
// Stefan Petrick's MetaBalls Effect mod by PalPalych for GyverLamp
/*
  Metaballs proof of concept by Stefan Petrick (mod by Palpalych for GyverLamp 27/02/2020)
  ...very rough 8bit math here...
  read more about the concept of isosurfaces and metaballs:
  https://www.gamedev.net/articles/programming/graphics/exploring-metaballs-and-isosurfaces-in-2d-r2556
*/
void MetaBallsRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(8U) * 11U + 1U + random8(11U), 50U + random8(121U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();
    speedfactor = modes[currentMode].Speed / 127.0;
  }

  // get some 2 random moving points
  uint16_t param1 = millis() * speedfactor;
  uint8_t x2 = inoise8(param1, 25355, 685 ) / WIDTH;
  uint8_t y2 = inoise8(param1, 355, 11685 ) / HEIGHT;

  uint8_t x3 = inoise8(param1, 55355, 6685 ) / WIDTH;
  uint8_t y3 = inoise8(param1, 25355, 22685 ) / HEIGHT;

  // and one Lissajou function
  uint8_t x1 = beatsin8(23 * speedfactor, 0, WIDTH - 1U);
  uint8_t y1 = beatsin8(28 * speedfactor, 0, HEIGHT - 1U);

  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {

      // calculate distances of the 3 points from actual pixel
      // and add them together with weightening
      uint8_t  dx =  abs(x - x1);
      uint8_t  dy =  abs(y - y1);
      uint8_t dist = 2 * SQRT_VARIANT((dx * dx) + (dy * dy));

      dx =  abs(x - x2);
      dy =  abs(y - y2);
      dist += SQRT_VARIANT((dx * dx) + (dy * dy));

      dx =  abs(x - x3);
      dy =  abs(y - y3);
      dist += SQRT_VARIANT((dx * dx) + (dy * dy));

      // inverse result
      //byte color = modes[currentMode].Speed * 10 / dist;
      //byte color = 1000U / dist; кажется, проблема была именно тут в делении на ноль
      byte color = (dist == 0) ? 255U : 1000U / dist;

      // map color between thresholds
      if (color > 0 && color < 60) {
        if (modes[currentMode].Scale == 100U)
          drawPixelXY(x, y, CHSV(color * 9, 255, 255));// это оригинальный цвет эффекта
        else
          drawPixelXY(x, y, ColorFromPalette(*curPalette, color * 9));
      } else {
        if (modes[currentMode].Scale == 100U)
          drawPixelXY(x, y, CHSV(0, 255, 255)); // в оригинале центральный глаз почему-то красный
        else
          drawPixelXY(x, y, ColorFromPalette(*curPalette, 0U));
      }
      // show the 3 points, too
      drawPixelXY(x1, y1, CRGB(255, 255, 255));
      drawPixelXY(x2, y2, CRGB(255, 255, 255));
      drawPixelXY(x3, y3, CRGB(255, 255, 255));
    }
  }
}

// =====================================
//         SINUSOID3 / СИНУСОИД3
// =====================================
/*
  Sinusoid3 by Stefan Petrick (mod by Palpalych for GyverLamp 27/02/2020)
  read more about the concept: https://www.youtube.com/watch?v=mubH-w_gwdA
  https://gist.github.com/StefanPetrick/dc666c1b4851d5fb8139b73719b70149
*/
// v1.7.0 - Updating for GuverLamp v1.7 by PalPalych 12.03.2020
// 2nd upd by Stepko https://wokwi.com/arduino/projects/287675911209222664
// 3rd proper by SottNick

void Sinusoid3Routine() {
  CRGB color;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(100U);
      setModeSettings(tmp + 1U, 4U + random8(183U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    deltaValue = (modes[currentMode].Speed - 1U) % 9U;                    // количество режимов
    emitterX = WIDTH * 0.5;
    emitterY = HEIGHT * 0.5;
    speedfactor = 0.00145 * modes[currentMode].Speed + 0.015;
  }
  float e_s3_size = 3. * modes[currentMode].Scale / 100.0 + 2;                // amplitude of the curves
  uint32_t time_shift = millis() & 0xFFFFFF; // overflow protection
  uint16_t _scale = (((modes[currentMode].Scale - 1U) % 9U) * 10U + 80U) << 7U;  // = fmap(scale, 1, 255, 0.1, 3);
  float _scale2 = (float)((modes[currentMode].Scale - 1U) % 9U) * 0.2 + 0.4;  // для спиралей на sinf
  uint16_t _scale3 = ((modes[currentMode].Scale - 1U) % 9U) * 1638U + 3276U;     // для спиралей на sin16

  float center1x = float(e_s3_size * sin16(speedfactor * 72.0874 * time_shift)) / 0x7FFF - emitterX;
  float center1y = float(e_s3_size * cos16(speedfactor * 98.301  * time_shift)) / 0x7FFF - emitterY;
  float center2x = float(e_s3_size * sin16(speedfactor * 68.8107 * time_shift)) / 0x7FFF - emitterX;
  float center2y = float(e_s3_size * cos16(speedfactor * 65.534  * time_shift)) / 0x7FFF - emitterY;
  float center3x = float(e_s3_size * sin16(speedfactor * 134.3447 * time_shift)) / 0x7FFF - emitterX;
  float center3y = float(e_s3_size * cos16(speedfactor * 170.3884 * time_shift)) / 0x7FFF - emitterY;

  switch (deltaValue) {
    case 0://Sinusoid I
      for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
          float cx = x + center1x;
          float cy = y + center1y;
          int8_t v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
          color.r = v;
          cx = x + center3x;
          cy = y + center3y;
          v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
          color.b = v;
          drawPixelXY(x, y, color);
        }
      }
      break;
    case 1: //Sinusoid II ???
      for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
          float cx = x + center1x;
          float cy = y + center1y;
          //int8_t v = 127 * (0.001 * time_shift * speedfactor + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 32767.0);
          uint8_t v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
          color.r = v;

          cx = x + center2x;
          cy = y + center2y;
          //v = 127 * (float(0.001 * time_shift * speedfactor) + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 32767.0);
          v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
          //color.g = (uint8_t)v >> 1;
          color.g = (v - (min(v, color.r) >> 1)) >> 1;
          //color.b = (uint8_t)v >> 2;
          color.b = color.g >> 2;
          color.r = max(v, color.r);
          drawPixelXY(x, y, color);
        }
      }
      break;
    case 2://Sinusoid III
      for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
          float cx = x + center1x;
          float cy = y + center1y;
          int8_t v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
          color.r = v;

          cx = x + center2x;
          cy = y + center2y;
          v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
          color.b = v;

          cx = x + center3x;
          cy = y + center3y;
          v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
          color.g = v;
          drawPixelXY(x, y, color);
        }
      }
      break;
    case 3: //Sinusoid IV
      for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
          float cx = x + center1x;
          float cy = y + center1y;
          int8_t v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy) + time_shift * speedfactor * 100)) / 0x7FFF);
          color.r = ~v;

          cx = x + center2x;
          cy = y + center2y;
          v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy) + time_shift * speedfactor * 100)) / 0x7FFF);
          color.g = ~v;

          cx = x + center3x;
          cy = y + center3y;
          v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy) + time_shift * speedfactor * 100)) / 0x7FFF);
          color.b = ~v;
          drawPixelXY(x, y, color);
        }
      }

      break;
    case 4: //changed by stepko //colored sinusoid
      for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
          float cx = x + center1x;
          float cy = y + center1y;
          int8_t v = 127 * (1 + float(sin16(_scale * (beatsin16(2, 1000, 1750) / 2550.) * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF); // + time_shift * speedfactor * 5 // mass colors plus by SottNick
          color.r = v;

          //v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy) + time_shift * speedfactor * 7)) / 0x7FFF);
          //v = 127 * (1 + sinf (_scale2 * SQRT_VARIANT(((cx * cx) + (cy * cy)))  + 0.001 * time_shift * speedfactor));
          v = 127 * (1 + float(sin16(_scale * (beatsin16(1, 570, 1050) / 2250.) * SQRT_VARIANT(((cx * cx) + (cy * cy)))  + 13 * time_shift * speedfactor)) / 0x7FFF); // вместо beatsin сперва ставил просто * 0.41
          color.b = v;

          //v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy) + time_shift * speedfactor * 19)) / 0x7FFF);
          //v = 127 * (1 + sinf (_scale2 * SQRT_VARIANT(((cx * cx) + (cy * cy)))  + 0.0025 * time_shift * speedfactor));
          v = 127 * (1 + float(cos16(_scale * (beatsin16(3, 1900, 2550) / 2550.) * SQRT_VARIANT(((cx * cx) + (cy * cy)))  + 41 * time_shift * speedfactor)) / 0x7FFF); // вместо beatsin сперва ставил просто * 0.53
          color.g = v;
          drawPixelXY(x, y, color);
        }
      }
      break;
    case 5: //changed by stepko //sinusoid in net
      for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
          float cx = x + center1x;
          float cy = y + center1y;
          int8_t v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy) + time_shift * speedfactor * 5)) / 0x7FFF);
          color.g = ~v;

          //v = 127 * (1 + float(sin16(_scale * x) + 0.01 * time_shift * speedfactor) / 0x7FFF);
          v = 127 * (1 + float(sin16(_scale * (x + 0.005 * time_shift * speedfactor))) / 0x7FFF); // proper by SottNick

          color.b = ~v;

          //v = 127 * (1 + float(sin16(_scale * y * 127 + float(0.011 * time_shift * speedfactor))) / 0x7FFF);
          v = 127 * (1 + float(sin16(_scale * (y + 0.0055 * time_shift * speedfactor))) / 0x7FFF); // proper by SottNick
          color.r = ~v;
          drawPixelXY(x, y, color);
        }
      }
      break;
    case 6: //changed by stepko //spiral
      for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
          float cx = x + center1x;
          float cy = y + center1y;
          //uint8_t v = 127 * (1 + float(sin16(_scale * (2 * atan2(cy, cx) + hypot(cy, cx)) + time_shift * speedfactor * 5)) / 0x7FFF);
          uint8_t v = 127 * (1 + sinf (3 * atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
          //uint8_t v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
          //вырезаем центр спирали - proper by SottNick
          float d = SQRT_VARIANT(cx * cx + cy * cy) / 10.; // 10 - это радиус вырезаемого центра в каких-то условных величинах. 10 = 1 пиксель, 20 = 2 пикселя. как-то так
          if (d < 0.06) d = 0.06;
          if (d < 1) // просто для ускорения расчётов
            v = constrain(v - int16_t(1 / d / d), 0, 255);
          //вырезали
          color.r = v;

          cx = x + center2x;
          cy = y + center2y;
          //v = 127 * (1 + float(sin16(_scale * (2 * atan2(cy, cx) + hypot(cy, cx)) + time_shift * speedfactor * 5)) / 0x7FFF);
          v = 127 * (1 + sinf (3 * atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
          //v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
          //вырезаем центр спирали
          d = SQRT_VARIANT(cx * cx + cy * cy) / 10.; // 10 - это радиус вырезаемого центра в каких-то условных величинах. 10 = 1 пиксель, 20 = 2 пикселя. как-то так
          if (d < 0.06) d = 0.06;
          if (d < 1) // просто для ускорения расчётов
            v = constrain(v - int16_t(1 / d / d), 0, 255);
          //вырезали
          color.b = v;

          cx = x + center3x;
          cy = y + center3y;
          //v = 127 * (1 + float(sin16(_scale * (2 * atan2(cy, cx) + hypot(cy, cx)) + time_shift * speedfactor * 5)) / 0x7FFF);
          //v = 127 * (1 + sinf (3* atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
          v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
          //вырезаем центр спирали
          d = SQRT_VARIANT(cx * cx + cy * cy) / 10.; // 10 - это радиус вырезаемого центра в каких-то условных величинах. 10 = 1 пиксель, 20 = 2 пикселя. как-то так
          if (d < 0.06) d = 0.06;
          if (d < 1) // просто для ускорения расчётов
            v = constrain(v - int16_t(1 / d / d), 0, 255);
          //вырезали
          color.g = v;
          drawPixelXY(x, y, color);
        }
      }
      break;
    case 7: //variant by SottNick
      for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
          float cx = x + center1x;
          float cy = y + center1y;
          //uint8_t v = 127 * (1 + float(sin16(_scale * (2 * atan2(cy, cx) + hypot(cy, cx)) + time_shift * speedfactor * 5)) / 0x7FFF);
          //uint8_t v = 127 * (1 + float(sin16(3* atan2(cy, cx) + _scale *  hypot(cy, cx) + time_shift * speedfactor * 5)) / 0x7FFF);
          //uint8_t v = 127 * (1 + sinf (3* atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
          uint8_t v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
          //вырезаем центр спирали
          float d = SQRT_VARIANT(cx * cx + cy * cy) / 10.; // 10 - это радиус вырезаемого центра в каких-то условных величинах. 10 = 1 пиксель, 20 = 2 пикселя. как-то так
          if (d < 0.06) d = 0.06;
          if (d < 1) // просто для ускорения расчётов
            v = constrain(v - int16_t(1 / d / d), 0, 255);
          //вырезали
          color.g = v;

          cx = x + center3x;
          cy = y + center3y;
          //v = 127 * (1 + sinf (3* atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
          v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
          //вырезаем центр спирали
          d = SQRT_VARIANT(cx * cx + cy * cy) / 10.; // 10 - это радиус вырезаемого центра в каких-то условных величинах. 10 = 1 пиксель, 20 = 2 пикселя. как-то так
          if (d < 0.06) d = 0.06;
          if (d < 1) // просто для ускорения расчётов
            v = constrain(v - int16_t(1 / d / d), 0, 255);
          //вырезали
          color.r = v;

          drawPixelXY(x, y, color);
          //nblend(leds[XY(x, y)], color, 150);
        }
      }
      break;
    case 8: //variant by SottNick
      for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
          float cx = x + center1x;
          float cy = y + center1y;
          //uint8_t v = 127 * (1 + float(sin16(_scale * (2 * atan2(cy, cx) + hypot(cy, cx)) + time_shift * speedfactor * 5)) / 0x7FFF);
          //uint8_t v = 127 * (1 + sinf (3* atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
          //uint8_t v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
          uint8_t v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
          color.g = v;

          cx = x + center2x;
          cy = y + center2y;
          //v = 127 * (1 + float(sin16(_scale * (2 * atan2(cy, cx) + hypot(cy, cx)) + time_shift * speedfactor * 5)) / 0x7FFF);
          //v = 127 * (1 + sinf (3* atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
          v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
          //вырезаем центр спирали
          float d = SQRT_VARIANT(cx * cx + cy * cy) / 16.; // 16 - это радиус вырезаемого центра в каких-то условных величинах. 10 = 1 пиксель, 20 = 2 пикселя. как-то так
          if (d < 0.06) d = 0.06;
          if (d < 1) // просто для ускорения расчётов
            v = constrain(v - int16_t(1 / d / d), 0, 255);
          //вырезали
          color.g = max(v, color.g);
          color.b = v;// >> 1;
          //color.r = v >> 1;

          drawPixelXY(x, y, color);
          //nblend(leds[XY(x, y)], color, 150);
        }
      }
      break;
  }
}

// =====================================
//          Boдoпaд 4 в 1
// =====================================
// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
extern const TProgmemRGBPalette16 WaterfallColors4in1_p FL_PROGMEM = {
  CRGB::Black,
  CRGB::DarkSlateGray,
  CRGB::DimGray,
  CRGB::LightSlateGray,

  CRGB::DimGray,
  CRGB::DarkSlateGray,
  CRGB::Silver,
  CRGB::DarkCyan,

  CRGB::Lavender,
  CRGB::Silver,
  CRGB::Azure,
  CRGB::LightGrey,

  CRGB::GhostWhite,
  CRGB::Silver,
  CRGB::White,
  CRGB::RoyalBlue
};

void fire2012WithPalette4in1() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    uint8_t tmp = random(3U);
    if (tmp == 0U)
      tmp = 16U + random8(16U);
    else if (tmp == 1U)
      tmp = 48U;
    else
      tmp = 80U + random8(4U);
    setModeSettings(tmp, 185U + random8(40U)); // 16-31, 48, 80-83 - остальное отстой
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  uint8_t rCOOLINGNEW = constrain((uint16_t)(modes[currentMode].Scale % 16) * 32 / HEIGHT + 16, 1, 255) ;
  // Array of temperature readings at each simulation cell
  //static byte heat[WIDTH][HEIGHT]; будет noise3d[0][WIDTH][HEIGHT]

  for (uint8_t x = 0; x < WIDTH; x++) {
    // Step 1.  Cool down every cell a little
    for (uint8_t i = 0; i < HEIGHT; i++) {
      //noise3d[0][x][i] = qsub8(noise3d[0][x][i], random8(0, ((rCOOLINGNEW * 10) / HEIGHT) + 2));
      noise3d[0][x][i] = qsub8(noise3d[0][x][i], random8(0, rCOOLINGNEW));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (uint8_t k = HEIGHT - 1; k >= 2; k--) {
      noise3d[0][x][k] = (noise3d[0][x][k - 1] + noise3d[0][x][k - 2] + noise3d[0][x][k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < SPARKINGNEW) {
      uint8_t y = random8(2);
      noise3d[0][x][y] = qadd8(noise3d[0][x][y], random8(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    for (uint8_t j = 0; j < HEIGHT; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8(noise3d[0][x][j], 240);
      if  (modes[currentMode].Scale < 16) {            // Lavafall
        leds[XY(x, (HEIGHT - 1) - j)] = ColorFromPalette(LavaColors_p, colorindex);
      } else if (modes[currentMode].Scale < 32) {      // Firefall
        leds[XY(x, (HEIGHT - 1) - j)] = ColorFromPalette(HeatColors_p, colorindex);
      } else if (modes[currentMode].Scale < 48) {      // Waterfall
        leds[XY(x, (HEIGHT - 1) - j)] = ColorFromPalette(WaterfallColors4in1_p, colorindex);
      } else if (modes[currentMode].Scale < 64) {      // Skyfall
        leds[XY(x, (HEIGHT - 1) - j)] = ColorFromPalette(CloudColors_p, colorindex);
      } else if (modes[currentMode].Scale < 80) {      // Forestfall
        leds[XY(x, (HEIGHT - 1) - j)] = ColorFromPalette(ForestColors_p, colorindex);
      } else if (modes[currentMode].Scale < 96) {      // Rainbowfall
        leds[XY(x, (HEIGHT - 1) - j)] = ColorFromPalette(RainbowColors_p, colorindex);
      } else {                      // Aurora
        leds[XY(x, (HEIGHT - 1) - j)] = ColorFromPalette(RainbowStripeColors_p, colorindex);
      }
    }
  }
}

// =====================================
//          Paзнoцвeтный дoждь
//             © Shaitan
// =====================================
void RainRoutine() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(random8(10U) ? 2U + random8(99U) : 1U , 185U + random8(52U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  for (uint8_t x = 0U; x < WIDTH; x++) {
    // заполняем случайно верхнюю строку
    CRGB thisColor = getPixColorXY(x, HEIGHT - 1U);
    if ((uint32_t)thisColor == 0U) {
      if (random8(0, 50) == 0U) {
        if (modes[currentMode].Scale == 1) drawPixelXY(x, HEIGHT - 1U, CHSV(random(0, 9) * 28, 255U, 255U)); // Радужный дождь
        else if (modes[currentMode].Scale == 100) drawPixelXY(x, HEIGHT - 1U, 0xE0FFFF - 0x101010 * random(0, 4)); // Снег
        else
          drawPixelXY(x, HEIGHT - 1U, CHSV(modes[currentMode].Scale * 2.4 + random(0, 16), 255, 255)); // Цветной дождь
      }
    } else {
      leds[XY(x, HEIGHT - 1U)] -= CHSV(0, 0, random(96, 128));
    }
  }
  // сдвигаем всё вниз
  for (uint8_t x = 0U; x < WIDTH; x++) {
    for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
    {
      drawPixelXY(x, y, getPixColorXY(x, y + 1U));
    }
  }
}

// =====================================
//              Призмата
// =====================================
// Prismata Loading Animation
// https://github.com/pixelmatix/aurora/blob/master/PatternPendulumWave.h
// Адаптация от © SottNick

void PrismataRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(100U), 35U + random8(100U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    setCurrentPalette();
  }
  //  EVERY_N_MILLIS(33) { маловата задержочка
  hue++; // используем переменную сдвига оттенка из функций радуги, чтобы не занимать память
  blurScreen(20); // @Palpalych посоветовал делать размытие
  dimAll(255U - (modes[currentMode].Scale - 1U) % 11U * 3U);

  for (uint8_t x = 0; x < WIDTH; x++) {
    // вместо 28 в оригинале было 280, умножения на .Speed не было, а вместо >>17 было (<<8)>>24. короче, оригинальная скорость достигается при бегунке .Speed=20
    uint8_t beat = (GET_MILLIS() * (accum88(x + 1)) * 28 * modes[currentMode].Speed) >> 17;
    uint8_t y = scale8(sin8(beat), HEIGHT - 1);
    drawPixelXY(x, y, ColorFromPalette(*curPalette, x * 7 + hue));
  }
}


// =====================================
//                Стая
// =====================================
// https://github.com/pixelmatix/aurora/blob/master/PatternFlock.h
// Адаптация от © SottNick и @kDn

template <class T>
class Vector2 {
  public:
    T x, y;

    Vector2() : x(0), y(0) {}
    Vector2(T x, T y) : x(x), y(y) {}
    Vector2(const Vector2& v) : x(v.x), y(v.y) {}

    Vector2& operator=(const Vector2& v) {
      x = v.x;
      y = v.y;
      return *this;
    }

    bool isEmpty() {
      return x == 0 && y == 0;
    }

    bool operator==(Vector2& v) {
      return x == v.x && y == v.y;
    }

    bool operator!=(Vector2& v) {
      return !(x == y);
    }

    Vector2 operator+(Vector2& v) {
      return Vector2(x + v.x, y + v.y);
    }
    Vector2 operator-(Vector2& v) {
      return Vector2(x - v.x, y - v.y);
    }

    Vector2& operator+=(Vector2& v) {
      x += v.x;
      y += v.y;
      return *this;
    }
    Vector2& operator-=(Vector2& v) {
      x -= v.x;
      y -= v.y;
      return *this;
    }

    Vector2 operator+(double s) {
      return Vector2(x + s, y + s);
    }
    Vector2 operator-(double s) {
      return Vector2(x - s, y - s);
    }
    Vector2 operator*(double s) {
      return Vector2(x * s, y * s);
    }
    Vector2 operator/(double s) {
      return Vector2(x / s, y / s);
    }

    Vector2& operator+=(double s) {
      x += s;
      y += s;
      return *this;
    }
    Vector2& operator-=(double s) {
      x -= s;
      y -= s;
      return *this;
    }
    Vector2& operator*=(double s) {
      x *= s;
      y *= s;
      return *this;
    }
    Vector2& operator/=(double s) {
      x /= s;
      y /= s;
      return *this;
    }

    void set(T x, T y) {
      this->x = x;
      this->y = y;
    }

    void rotate(double deg) {
      double theta = deg / 180.0 * M_PI;
      double c = cos(theta);
      double s = sin(theta);
      double tx = x * c - y * s;
      double ty = x * s + y * c;
      x = tx;
      y = ty;
    }

    Vector2& normalize() {
      if (length() == 0) return *this;
      *this *= (1.0 / length());
      return *this;
    }

    float dist(Vector2 v) const {
      Vector2 d(v.x - x, v.y - y);
      return d.length();
    }
    float length() const {
      return sqrt(x * x + y * y);
    }

    float mag() const {
      return length();
    }

    float magSq() {
      return (x * x + y * y);
    }

    void truncate(double length) {
      double angle = atan2f(y, x);
      x = length * cos(angle);
      y = length * sin(angle);
    }

    Vector2 ortho() const {
      return Vector2(y, -x);
    }

    static float dot(Vector2 v1, Vector2 v2) {
      return v1.x * v2.x + v1.y * v2.y;
    }
    static float cross(Vector2 v1, Vector2 v2) {
      return (v1.x * v2.y) - (v1.y * v2.x);
    }

    void limit(float max) {
      if (magSq() > max * max) {
        normalize();
        *this *= max;
      }
    }
};

typedef Vector2<float> PVector;

// Flocking
// Daniel Shiffman <http://www.shiffman.net>
// The Nature of Code, Spring 2009

// Boid class
// Methods for Separation, Cohesion, Alignment added

class Boid {
  public:

    PVector location;
    PVector velocity;
    PVector acceleration;
    float maxforce;    // Maximum steering force
    float maxspeed;    // Maximum speed

    float desiredseparation = 4;
    float neighbordist = 8;
    byte colorIndex = 0;
    float mass;

    boolean enabled = true;

    Boid() {}

    Boid(float x, float y) {
      acceleration = PVector(0, 0);
      velocity = PVector(randomf(), randomf());
      location = PVector(x, y);
      maxspeed = 1.5;
      maxforce = 0.05;
    }

    static float randomf() {
      return mapfloat(random(0, 255), 0, 255, -.5, .5);
    }

    static float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    void run(Boid boids [], uint8_t boidCount) {
      flock(boids, boidCount);
      update();
      // wrapAroundBorders();
      // render();
    }

    // Method to update location
    void update() {
      // Update velocity
      velocity += acceleration;
      // Limit speed
      velocity.limit(maxspeed);
      location += velocity;
      // Reset acceleration to 0 each cycle
      acceleration *= 0;
    }

    void applyForce(PVector force) {
      // We could add mass here if we want A = F / M
      acceleration += force;
    }

    void repelForce(PVector obstacle, float radius) {
      //Force that drives boid away from obstacle.

      PVector futPos = location + velocity; //Calculate future position for more effective behavior.
      PVector dist = obstacle - futPos;
      float d = dist.mag();

      if (d <= radius) {
        PVector repelVec = location - obstacle;
        repelVec.normalize();
        if (d != 0) { //Don't divide by zero.
          // float scale = 1.0 / d; //The closer to the obstacle, the stronger the force.
          repelVec.normalize();
          repelVec *= (maxforce * 7);
          if (repelVec.mag() < 0) { //Don't let the boids turn around to avoid the obstacle.
            repelVec.y = 0;
          }
        }
        applyForce(repelVec);
      }
    }

    // We accumulate a new acceleration each time based on three rules
    void flock(Boid boids [], uint8_t boidCount) {
      PVector sep = separate(boids, boidCount);   // Separation
      PVector ali = align(boids, boidCount);      // Alignment
      PVector coh = cohesion(boids, boidCount);   // Cohesion
      // Arbitrarily weight these forces
      sep *= 1.5;
      ali *= 1.0;
      coh *= 1.0;
      // Add the force vectors to acceleration
      applyForce(sep);
      applyForce(ali);
      applyForce(coh);
    }

    // Separation
    // Method checks for nearby boids and steers away
    PVector separate(Boid boids [], uint8_t boidCount) {
      PVector steer = PVector(0, 0);
      int count = 0;
      // For every boid in the system, check if it's too close
      for (int i = 0; i < boidCount; i++) {
        Boid other = boids[i];
        if (!other.enabled)
          continue;
        float d = location.dist(other.location);
        // If the distance is greater than 0 and less than an arbitrary amount (0 when you are yourself)
        if ((d > 0) && (d < desiredseparation)) {
          // Calculate vector pointing away from neighbor
          PVector diff = location - other.location;
          diff.normalize();
          diff /= d;        // Weight by distance
          steer += diff;
          count++;            // Keep track of how many
        }
      }
      // Average -- divide by how many
      if (count > 0) {
        steer /= (float) count;
      }

      // As long as the vector is greater than 0
      if (steer.mag() > 0) {
        // Implement Reynolds: Steering = Desired - Velocity
        steer.normalize();
        steer *= maxspeed;
        steer -= velocity;
        steer.limit(maxforce);
      }
      return steer;
    }

    // Alignment
    // For every nearby boid in the system, calculate the average velocity
    PVector align(Boid boids [], uint8_t boidCount) {
      PVector sum = PVector(0, 0);
      int count = 0;
      for (int i = 0; i < boidCount; i++) {
        Boid other = boids[i];
        if (!other.enabled)
          continue;
        float d = location.dist(other.location);
        if ((d > 0) && (d < neighbordist)) {
          sum += other.velocity;
          count++;
        }
      }
      if (count > 0) {
        sum /= (float) count;
        sum.normalize();
        sum *= maxspeed;
        PVector steer = sum - velocity;
        steer.limit(maxforce);
        return steer;
      }
      else {
        return PVector(0, 0);
      }
    }

    // Cohesion
    // For the average location (i.e. center) of all nearby boids, calculate steering vector towards that location
    PVector cohesion(Boid boids [], uint8_t boidCount) {
      PVector sum = PVector(0, 0);   // Start with empty vector to accumulate all locations
      int count = 0;
      for (int i = 0; i < boidCount; i++) {
        Boid other = boids[i];
        if (!other.enabled)
          continue;
        float d = location.dist(other.location);
        if ((d > 0) && (d < neighbordist)) {
          sum += other.location; // Add location
          count++;
        }
      }
      if (count > 0) {
        sum /= count;
        return seek(sum);  // Steer towards the location
      }
      else {
        return PVector(0, 0);
      }
    }

    // A method that calculates and applies a steering force towards a target
    // STEER = DESIRED MINUS VELOCITY
    PVector seek(PVector target) {
      PVector desired = target - location;  // A vector pointing from the location to the target
      // Normalize desired and scale to maximum speed
      desired.normalize();
      desired *= maxspeed;
      // Steering = Desired minus Velocity
      PVector steer = desired - velocity;
      steer.limit(maxforce);  // Limit to maximum steering force
      return steer;
    }

    // A method that calculates a steering force towards a target
    // STEER = DESIRED MINUS VELOCITY
    void arrive(PVector target) {
      PVector desired = target - location;  // A vector pointing from the location to the target
      float d = desired.mag();
      // Normalize desired and scale with arbitrary damping within 100 pixels
      desired.normalize();
      if (d < 4) {
        float m = map(d, 0, 100, 0, maxspeed);
        desired *= m;
      }
      else {
        desired *= maxspeed;
      }

      // Steering = Desired minus Velocity
      PVector steer = desired - velocity;
      steer.limit(maxforce);  // Limit to maximum steering force
      applyForce(steer);
      //Serial.println(d);
    }

    void wrapAroundBorders() {
      if (location.x < 0) location.x = WIDTH - 1;
      if (location.y < 0) location.y = HEIGHT - 1;
      if (location.x >= WIDTH) location.x = 0;
      if (location.y >= HEIGHT) location.y = 0;
    }

    void avoidBorders() {
      PVector desired = velocity;

      if (location.x < 8) desired = PVector(maxspeed, velocity.y);
      if (location.x >= WIDTH - 8) desired = PVector(-maxspeed, velocity.y);
      if (location.y < 8) desired = PVector(velocity.x, maxspeed);
      if (location.y >= HEIGHT - 8) desired = PVector(velocity.x, -maxspeed);

      if (desired != velocity) {
        PVector steer = desired - velocity;
        steer.limit(maxforce);
        applyForce(steer);
      }

      if (location.x < 0) location.x = 0;
      if (location.y < 0) location.y = 0;
      if (location.x >= WIDTH) location.x = WIDTH - 1;
      if (location.y >= HEIGHT) location.y = HEIGHT - 1;
    }

    bool bounceOffBorders(float bounce) {
      bool bounced = false;

      if (location.x >= WIDTH) {
        location.x = WIDTH - 1;
        velocity.x *= -bounce;
        bounced = true;
      }
      else if (location.x < 0) {
        location.x = 0;
        velocity.x *= -bounce;
        bounced = true;
      }

      if (location.y >= HEIGHT) {
        location.y = HEIGHT - 1;
        velocity.y *= -bounce;
        bounced = true;
      }
      else if (location.y < 0) {
        location.y = 0;
        velocity.y *= -bounce;
        bounced = true;
      }

      return bounced;
    }

    void render() {
      // // Draw a triangle rotated in the direction of velocity
      // float theta = velocity.heading2D() + radians(90);
      // fill(175);
      // stroke(0);
      // pushMatrix();
      // translate(location.x,location.y);
      // rotate(theta);
      // beginShape(TRIANGLES);
      // vertex(0, -r*2);
      // vertex(-r, r*2);
      // vertex(r, r*2);
      // endShape();
      // popMatrix();
      // backgroundLayer.drawPixel(location.x, location.y, CRGB::Blue);
    }
};

static const uint8_t AVAILABLE_BOID_COUNT = 20U;
Boid boids[AVAILABLE_BOID_COUNT];

static const uint8_t boidCount = 10;
Boid predator;
PVector wind;
bool predatorPresent = true;

// -------------------------------------
void  flock() { // Cтaя
  flockRoutine(false);
};



// =====================================
//           Cтaя и xищник
// =====================================
// -------------------------------------
void flockAndPredator(void) { // (24U) Cтaя и xищник
  flockRoutine(true);
};


// -------------------------------------
void flockRoutine(bool predatorIs) {
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //setModeSettings(random8(8U)*11U+1U+random8(11U), 1U+random8(255U));
      uint8_t tmp = random8(5U);// 0, 1, 5, 6, 7 - остальные 4 палитры с чёрным цветом - стая будет исчезать периодически (2, 3, 4, 8)
      if (tmp > 1U) tmp += 3U;
      setModeSettings(tmp * 11U + 2U + random8(10U), 1U + random8(255U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();

    for (int i = 0; i < boidCount; i++) {
      boids[i] = Boid(0, 0);//WIDTH - 1U, HEIGHT - 1U);
      boids[i].maxspeed = 0.380 * modes[currentMode].Speed / 127.0 + 0.380 / 2;
      boids[i].maxforce = 0.015 * modes[currentMode].Speed / 127.0 + 0.015 / 2;
    }
    predatorPresent = predatorIs && random8(2U);
    //if (predatorPresent) { нужно присвоить ему значения при первом запуске, иначе он с нулями будет жить
    predator = Boid(0, 0);//WIDTH + WIDTH - 1, HEIGHT + HEIGHT - 1);
    predator.maxspeed = 0.385 * modes[currentMode].Speed / 127.0 + 0.385 / 2;
    predator.maxforce = 0.020 * modes[currentMode].Speed / 127.0 + 0.020 / 2;
    predator.neighbordist = 8.0; // было 16.0 и хищник гонял по одной линии всегда
    predator.desiredseparation = 0.0;
    //}
  }

  blurScreen(15); // @Palpalych советует делать размытие
  //myLamp.dimAll(254U - (31-(myLamp.effects.getScale()%32))*8);
  dimAll(255U - (modes[currentMode].Scale - 1U) % 11U * 3);

  bool applyWind = random(0, 255) > 240;
  if (applyWind) {
    wind.x = Boid::randomf() * .015 * modes[currentMode].Speed / 127.0 + .015 / 2;
    wind.y = Boid::randomf() * .015 * modes[currentMode].Speed / 127.0 + .015 / 2;
  }

  CRGB color = ColorFromPalette(*curPalette, hue);


  for (int i = 0; i < boidCount; i++) {
    Boid * boid = &boids[i];

    if (predatorPresent) {
      // flee from predator
      boid->repelForce(predator.location, 10);
    }

    boid->run(boids, boidCount);
    boid->wrapAroundBorders();
    PVector location = boid->location;
    // PVector velocity = boid->velocity;
    // backgroundLayer.drawLine(location.x, location.y, location.x - velocity.x, location.y - velocity.y, color);
    // effects.leds[XY(location.x, location.y)] += color;
    //drawPixelXY(location.x, location.y, color);
    drawPixelXYF(location.x, location.y, color);

    if (applyWind) {
      boid->applyForce(wind);
      applyWind = false;
    }
  }

  if (predatorPresent) {
    predator.run(boids, boidCount);
    predator.wrapAroundBorders();
    color = ColorFromPalette(*curPalette, hue + 128);
    PVector location = predator.location;
    // PVector velocity = predator.velocity;
    // backgroundLayer.drawLine(location.x, location.y, location.x - velocity.x, location.y - velocity.y, color);
    // effects.leds[XY(location.x, location.y)] += color;

    //drawPixelXY(location.x, location.y, color);
    drawPixelXYF(location.x, location.y, color);
  }

  EVERY_N_MILLIS(333) {
    hue++;
  }

  EVERY_N_SECONDS(30) {
    predatorPresent = predatorIs && !predatorPresent;
  }
}

// =====================================
//                Вихри
// =====================================
// https://github.com/pixelmatix/aurora/blob/master/PatternFlowField.h
// Адаптация © SottNick
// используются переменные эффекта Стая. Без него работать не будет.

static const uint8_t ff_speed = 1; // чем выше этот параметр, тем короче переходы (градиенты) между цветами. 1 - это самое красивое
static const uint8_t ff_scale = 26; // чем больше этот параметр, тем больше "языков пламени" или как-то так. 26 - это норм

//--------------------------------------
void whirl() {
  whirlRoutine(true);
}
//--------------------------------------
void whirlColor() {
  whirlRoutine(false);
}

void whirlRoutine(bool oneColor) {
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      if (oneColor)
        setModeSettings(random8(30U) ? 1U + random8(99U) : 100U, 221U + random8(32U));
      else {
        uint8_t tmp = random8(5U);
        if (tmp > 1U) tmp += 3U;
        setModeSettings(tmp * 11U + 3U, 221U + random8(32U));
      }
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();

    ff_x = random16();
    ff_y = random16();
    ff_z = random16();

    for (uint8_t i = 0; i < AVAILABLE_BOID_COUNT; i++) {
      boids[i] = Boid(random8(WIDTH), 0);
    }
  }
  dimAll(240);

  for (uint8_t i = 0; i < AVAILABLE_BOID_COUNT; i++) {
    Boid * boid = &boids[i];

    int ioffset = ff_scale * boid->location.x;
    int joffset = ff_scale * boid->location.y;

    byte angle = inoise8(ff_x + ioffset, ff_y + joffset, ff_z);

    boid->velocity.x = (float) sin8(angle) * 0.0078125 - 1.0;
    boid->velocity.y = -((float)cos8(angle) * 0.0078125 - 1.0);
    boid->update();

    if (oneColor)
      //drawPixelXY(boid->location.x, boid->location.y, CHSV(modes[currentMode].Scale * 2.55, (modes[currentMode].Scale == 100) ? 0U : 255U, 255U)); // цвет белый для .Scale=100
      drawPixelXYF(boid->location.x, boid->location.y, CHSV(modes[currentMode].Scale * 2.55, (modes[currentMode].Scale == 100) ? 0U : 255U, 255U)); // цвет белый для .Scale=100
    else
      //drawPixelXY(boid->location.x, boid->location.y, ColorFromPalette(*curPalette, angle + hue)); // + hue постепенно сдвигает палитру по кругу
      drawPixelXYF(boid->location.x, boid->location.y, ColorFromPalette(*curPalette, angle + hue)); // + hue постепенно сдвигает палитру по кругу

    if (boid->location.x < 0 || boid->location.x >= WIDTH || boid->location.y < 0 || boid->location.y >= HEIGHT) {
      boid->location.x = random(WIDTH);
      boid->location.y = 0;
    }
  }

  EVERY_N_MILLIS(200) {
    hue++;
  }

  ff_x += ff_speed;
  ff_y += ff_speed;
  ff_z += ff_speed;
}

// =====================================
//                Волны
// =====================================
// https://github.com/pixelmatix/aurora/blob/master/PatternWave.h
// Адаптация от © SottNick

byte waveThetaUpdate = 0;
byte waveThetaUpdateFrequency = 0;
byte waveTheta = 0;
byte hueUpdate = 0;
byte hueUpdateFrequency = 0;
byte waveRotation = 0;
uint8_t waveScale = 256 / WIDTH;
uint8_t waveCount = 1;

void WaveRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(5U);// 0, 1, 5, 6, 7 - остальные 4 палитры с чёрным цветом - будет мерцать (2, 3, 4, 8)
      if (tmp > 1U) tmp += 3U;
      setModeSettings(tmp * 11U + 1U + random8(4U), 220U + random8(17U) * 2U);
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();//а вот тут явно накосячено. палитры наложены на угол поворота несинхронно, но исправлять особого смысла нет

    //waveRotation = random(0, 4);// теперь вместо этого регулятор Масштаб
    waveRotation = (modes[currentMode].Scale % 11U) % 4U;//(modes[currentMode].Scale - 1) / 25U;
    //waveCount = random(1, 3);// теперь вместо этого чётное/нечётное у регулятора Скорость
    waveCount = modes[currentMode].Speed & 0x01;//% 2;
    //waveThetaUpdateFrequency = random(1, 2);
    //hueUpdateFrequency = random(1, 6);
  }

  dimAll(254);

  int n = 0;

  switch (waveRotation) {
    case 0:
      for (uint8_t x = 0; x < WIDTH; x++) {
        n = quadwave8(x * 2 + waveTheta) / waveScale;
        drawPixelXY(x, n, ColorFromPalette(*curPalette, hue + x));
        if (waveCount != 1)
          drawPixelXY(x, HEIGHT - 1 - n, ColorFromPalette(*curPalette, hue + x));
      }
      break;

    case 1:
      for (uint8_t y = 0; y < HEIGHT; y++) {
        n = quadwave8(y * 2 + waveTheta) / waveScale;
        drawPixelXY(n, y, ColorFromPalette(*curPalette, hue + y));
        if (waveCount != 1)
          drawPixelXY(WIDTH - 1 - n, y, ColorFromPalette(*curPalette, hue + y));
      }
      break;

    case 2:
      for (uint8_t x = 0; x < WIDTH; x++) {
        n = quadwave8(x * 2 - waveTheta) / waveScale;
        drawPixelXY(x, n, ColorFromPalette(*curPalette, hue + x));
        if (waveCount != 1)
          drawPixelXY(x, HEIGHT - 1 - n, ColorFromPalette(*curPalette, hue + x));
      }
      break;

    case 3:
      for (uint8_t y = 0; y < HEIGHT; y++) {
        n = quadwave8(y * 2 - waveTheta) / waveScale;
        drawPixelXY(n, y, ColorFromPalette(*curPalette, hue + y));
        if (waveCount != 1)
          drawPixelXY(WIDTH - 1 - n, y, ColorFromPalette(*curPalette, hue + y));
      }
      break;
  }


  if (waveThetaUpdate >= waveThetaUpdateFrequency) {
    waveThetaUpdate = 0;
    waveTheta++;
  }
  else {
    waveThetaUpdate++;
  }

  if (hueUpdate >= hueUpdateFrequency) {
    hueUpdate = 0;
    hue++;
  }
  else {
    hueUpdate++;
  }

  blurScreen(20); // @Palpalych советует делать размытие. вот в этом эффекте его явно не хватает...
}

// =====================================
//              Oгoнь 2018
// =====================================
// https://gist.github.com/StefanPetrick/819e873492f344ebebac5bcd2fdd8aa8
// https://gist.github.com/StefanPetrick/1ba4584e534ba99ca259c1103754e4c5
// Адаптация от © SottNick

void Fire2018_2() {
  //  const uint8_t CENTER_Y_MAJOR =  HEIGHT / 2 + (HEIGHT % 2);
  //  const uint8_t CENTER_X_MAJOR =  WIDTH / 2  + (WIDTH % 2) ;

#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(1U + random8(50U), 195U + random8(44U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)


  // some changing values
  uint16_t ctrl1 = inoise16(11 * millis(), 0, 0);
  uint16_t ctrl2 = inoise16(13 * millis(), 100000, 100000);
  uint16_t  ctrl = ((ctrl1 + ctrl2) / 2);

  // parameters for the heatmap
  uint16_t speed = 25;
  noise32_x[0] = 3 * ctrl * speed;
  noise32_y[0] = 20 * millis() * speed;
  noise32_z[0] = 5 * millis() * speed ;
  scale32_x[0] = ctrl1 / 2;
  scale32_y[0] = ctrl2 / 2;

  //calculate the noise data
  uint8_t layer = 0;

  for (uint8_t i = 0; i < WIDTH; i++) {
    uint32_t ioffset = scale32_x[layer] * (i - CENTER_X_MAJOR);
    for (uint8_t j = 0; j < HEIGHT; j++) {
      uint32_t joffset = scale32_y[layer] * (j - CENTER_Y_MAJOR);
      uint16_t data = ((inoise16(noise32_x[layer] + ioffset, noise32_y[layer] + joffset, noise32_z[layer])) + 1);
      noise3d[layer][i][j] = data >> 8;
    }
  }

  // parameters for te brightness mask
  speed = 20;
  noise32_x[1] = 3 * ctrl * speed;
  noise32_y[1] = 20 * millis() * speed;
  noise32_z[1] = 5 * millis() * speed ;
  scale32_x[1] = ctrl1 / 2;
  scale32_y[1] = ctrl2 / 2;

  //calculate the noise data
  layer = 1;
  for (uint8_t i = 0; i < WIDTH; i++) {
    uint32_t ioffset = scale32_x[layer] * (i - CENTER_X_MAJOR);
    for (uint8_t j = 0; j < HEIGHT; j++) {
      uint32_t joffset = scale32_y[layer] * (j - CENTER_Y_MAJOR);
      uint16_t data = ((inoise16(noise32_x[layer] + ioffset, noise32_y[layer] + joffset, noise32_z[layer])) + 1);
      noise3d[layer][i][j] = data >> 8;
    }
  }

  // draw lowest line - seed the fire
  for (uint8_t x = 0; x < WIDTH; x++) {
    ledsbuff[XY(x, HEIGHT - 1)].r =  noise3d[0][WIDTH - 1 - x][CENTER_Y_MAJOR - 1]; // хз, почему взято с середины. вожможно, нужно просто с 7 строки вне зависимости от высоты матрицы
  }


  //copy everything one line up
  for (uint8_t y = 0; y < HEIGHT - 1; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      ledsbuff[XY(x, y)].r = ledsbuff[XY(x, y + 1)].r;
    }
  }

  //dim
  for (uint8_t y = 0; y < HEIGHT - 1; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      uint8_t dim = noise3d[0][x][y];
      // high value = high flames
      dim = dim / 1.7;
      dim = 255 - dim;
      ledsbuff[XY(x, y)].r = scale8(ledsbuff[XY(x, y)].r , dim);
    }
  }

  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      // map the colors based on heatmap
      //leds[XY(x, HEIGHT - 1 - y)] = CRGB( ledsbuff[XY(x, y)].r, 1 , 0);
      //leds[XY(x, HEIGHT - 1 - y)] = CRGB( ledsbuff[XY(x, y)].r, ledsbuff[XY(x, y)].r * 0.153, 0);// * 0.153 - лучший оттенок
      leds[XY(x, HEIGHT - 1 - y)] = CRGB( ledsbuff[XY(x, y)].r, (float)ledsbuff[XY(x, y)].r * modes[currentMode].Scale * 0.01, 0);


      //пытался понять, как регулировать оттенок пламени...
      //  if (modes[currentMode].Scale > 50)
      //    leds[XY(x, HEIGHT - 1 - y)] = CRGB( ledsbuff[XY(x, y)].r, ledsbuff[XY(x, y)].r * (modes[currentMode].Scale % 50)  * 0.051, 0);
      //  else
      //    leds[XY(x, HEIGHT - 1 - y)] = CRGB( ledsbuff[XY(x, y)].r, 1 , ledsbuff[XY(x, y)].r * modes[currentMode].Scale * 0.051);
      //примерно понял

      // dim the result based on 2nd noise layer
      leds[XY(x, HEIGHT - 1 - y)].nscale8(noise3d[1][x][y]);
    }
  }

}

// =====================================
//              Oгoнь 2012
// =====================================
// там выше есть его копии для эффектов Водопад и Водопад 4 в 1
// по идее, надо бы объединить и оптимизировать, но мелких отличий довольно много
// based on FastLED example Fire2012WithPalette: https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino

void fire2012again() {
  if (loadingFlag) {
    loadingFlag = false;
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = 17U + random8(55U);
      if (tmp > 22) tmp += 28;
      setModeSettings(tmp, 185U + random8(50U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    if (modes[currentMode].Scale > 100) {
      modes[currentMode].Scale = 100; // чтобы не было проблем при прошивке без очистки памяти
    }
    if (modes[currentMode].Scale > 50)
      curPalette = firePalettes[(uint8_t)((modes[currentMode].Scale - 50) / 50.0F * ((sizeof(firePalettes) / sizeof(TProgmemRGBPalette16 *)) - 0.01F))];
    else
      curPalette = palette_arr[(uint8_t)(modes[currentMode].Scale / 50.0F * ((sizeof(palette_arr) / sizeof(TProgmemRGBPalette16 *)) - 0.01F))];
  }

#if HEIGHT/6 > 6
#define FIRE_BASE 6
#else
#define FIRE_BASE HEIGHT/6+1
#endif
  // COOLING: How much does the air cool as it rises?
  // Less cooling = taller flames.  More cooling = shorter flames.
#define cooling 70U
  // SPARKING: What chance (out of 255) is there that a new spark will be lit?
  // Higher chance = more roaring fire.  Lower chance = more flickery fire.
#define sparking 130U
  // SMOOTHING; How much blending should be done between frames
  // Lower = more blending and smoother flames. Higher = less blending and flickery flames
#define fireSmoothing 80U
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(256));

  // Loop for each column individually
  for (uint8_t x = 0; x < WIDTH; x++) {
    // Step 1.  Cool down every cell a little
    for (uint8_t i = 0; i < HEIGHT; i++) {
      noise3d[0][x][i] = qsub8(noise3d[0][x][i], random(0, ((cooling * 10) / HEIGHT) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (uint8_t k = HEIGHT - 1; k > 0; k--) { // fixed by SottNick
      noise3d[0][x][k] = (noise3d[0][x][k - 1] + noise3d[0][x][k - 1] + noise3d[0][x][wrapY(k - 2)]) / 3; // fixed by SottNick
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < sparking) {
      uint8_t j = random8(FIRE_BASE);
      noise3d[0][x][j] = qadd8(noise3d[0][x][j], random(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    // Blend new data with previous frame. Average data between neighbouring pixels
    for (uint8_t y = 0; y < HEIGHT; y++)
      nblend(leds[XY(x, y)], ColorFromPalette(*curPalette, ((noise3d[0][x][y] * 0.7) + (noise3d[0][wrapX(x + 1)][y] * 0.3))), fireSmoothing);
  }
}


// ============= ЭФФЕКТЫ ОСАДКИ / ТУЧКА В БАНКЕ / ГРОЗА В БАНКЕ ===============
// https://github.com/marcmerlin/FastLED_NeoMatrix_SmartMatrix_LEDMatrix_GFX_Demos/blob/master/FastLED/Sublime_Demos/Sublime_Demos.ino
// там по ссылке ещё остались эффекты с 3 по 9 (в SimplePatternList перечислены)

//прикольная процедура добавляет блеск почти к любому эффекту после его отрисовки https://www.youtube.com/watch?v=aobtR1gIyIo
//void addGlitter( uint8_t chanceOfGlitter){
//  if ( random8() < chanceOfGlitter) leds[ random16(NUM_LEDS) ] += CRGB::White;
//}

//static uint8_t intensity = 42;  // будет бегунок масштаба

// Array of temp cells (used by fire, theMatrix, coloredRain, stormyRain)
// uint8_t **tempMatrix; = noise3d[0][WIDTH][HEIGHT]
// uint8_t *splashArray; = line[WIDTH] из эффекта Огонь

CRGB solidRainColor = CRGB(60, 80, 90);

uint8_t wrapX(int8_t x) {
  return (x + WIDTH) % WIDTH;
}
uint8_t wrapY(int8_t y) {
  return (y + HEIGHT) % HEIGHT;
}

void rain(byte backgroundDepth, byte maxBrightness, byte spawnFreq, byte tailLength, CRGB rainColor, bool splashes, bool clouds, bool storm) {
  ff_x = random16();
  ff_y = random16();
  ff_z = random16();

  CRGB lightningColor = CRGB(72, 72, 80);
  CRGBPalette16 rain_p( CRGB::Black, rainColor );
#ifdef SMARTMATRIX
  CRGBPalette16 rainClouds_p( CRGB::Black, CRGB(75, 84, 84), CRGB(49, 75, 75), CRGB::Black );
#else
  CRGBPalette16 rainClouds_p( CRGB::Black, CRGB(15, 24, 24), CRGB(9, 15, 15), CRGB::Black );
#endif

  //fadeToBlackBy( leds, NUM_LEDS, 255-tailLength);
  dimAll(tailLength);

  // Loop for each column individually
  for (uint8_t x = 0; x < WIDTH; x++) {
    // Step 1.  Move each dot down one cell
    for (uint8_t i = 0; i < HEIGHT; i++) {
      if (noise3d[0][x][i] >= backgroundDepth) {  // Don't move empty cells
        if (i > 0) noise3d[0][x][wrapY(i - 1)] = noise3d[0][x][i];
        noise3d[0][x][i] = 0;
      }
    }

    // Step 2.  Randomly spawn new dots at top
    if (random8() < spawnFreq) {
      noise3d[0][x][HEIGHT - 1] = random(backgroundDepth, maxBrightness);
    }

    // Step 3. Map from tempMatrix cells to LED colors
    for (uint8_t y = 0; y < HEIGHT; y++) {
      if (noise3d[0][x][y] >= backgroundDepth) {  // Don't write out empty cells
        leds[XY(x, y)] = ColorFromPalette(rain_p, noise3d[0][x][y]);
      }
    }

    // Step 4. Add splash if called for
    if (splashes) {
      // FIXME, this is broken
      byte j = line[x];
      byte v = noise3d[0][x][0];

      if (j >= backgroundDepth) {
        leds[XY(wrapX(x - 2), 0)] = ColorFromPalette(rain_p, j / 3);
        leds[XY(wrapX(x + 2), 0)] = ColorFromPalette(rain_p, j / 3);
        line[x] = 0;   // Reset splash
      }

      if (v >= backgroundDepth) {
        leds[XY(wrapX(x - 1), 1)] = ColorFromPalette(rain_p, v / 2);
        leds[XY(wrapX(x + 1), 1)] = ColorFromPalette(rain_p, v / 2);
        line[x] = v; // Prep splash for next frame
      }
    }

    // Step 5. Add lightning if called for
    if (storm) {
      //uint8_t lightning[WIDTH][HEIGHT];
      // ESP32 does not like static arrays  https://github.com/espressif/arduino-esp32/issues/2567
      uint8_t *lightning = (uint8_t *) malloc(WIDTH * HEIGHT);
      while (lightning == NULL) {
        Serial.println("lightning malloc failed");
      }


      if (random16() < 72) {    // Odds of a lightning bolt
        lightning[scale8(random8(), WIDTH - 1) + (HEIGHT - 1) * WIDTH] = 255; // Random starting location
        for (uint8_t ly = HEIGHT - 1; ly > 1; ly--) {
          for (uint8_t lx = 1; lx < WIDTH - 1; lx++) {
            if (lightning[lx + ly * WIDTH] == 255) {
              lightning[lx + ly * WIDTH] = 0;
              uint8_t dir = random8(4);
              switch (dir) {
                case 0:
                  leds[XY(lx + 1, ly - 1)] = lightningColor;
                  lightning[(lx + 1) + (ly - 1) * WIDTH] = 255; // move down and right
                  break;
                case 1:
                  leds[XY(lx, ly - 1)] = CRGB(128, 128, 128); // я без понятия, почему у верхней молнии один оттенок, а у остальных - другой
                  lightning[lx + (ly - 1) * WIDTH] = 255;  // move down
                  break;
                case 2:
                  leds[XY(lx - 1, ly - 1)] = CRGB(128, 128, 128);
                  lightning[(lx - 1) + (ly - 1) * WIDTH] = 255; // move down and left
                  break;
                case 3:
                  leds[XY(lx - 1, ly - 1)] = CRGB(128, 128, 128);
                  lightning[(lx - 1) + (ly - 1) * WIDTH] = 255; // fork down and left
                  leds[XY(lx - 1, ly - 1)] = CRGB(128, 128, 128);
                  lightning[(lx + 1) + (ly - 1) * WIDTH] = 255; // fork down and right
                  break;
              }
            }
          }
        }
      }
      free(lightning);
    }

    // Step 6. Add clouds if called for
    if (clouds) {
      uint16_t noiseScale = 250;  // A value of 1 will be so zoomed in, you'll mostly see solid colors. A value of 4011 will be very zoomed out and shimmery
      //const uint16_t cloudHeight = (HEIGHT*0.2)+1;
      const uint8_t cloudHeight = HEIGHT * 0.4 + 1; // это уже 40% c лишеним, но на высоких матрицах будет чуть меньше

      // This is the array that we keep our computed noise values in
      //static uint8_t noise[WIDTH][cloudHeight];
      static uint8_t *noise = (uint8_t *) malloc(WIDTH * cloudHeight);

      while (noise == NULL) {
        Serial.println("noise malloc failed");
      }
      int xoffset = noiseScale * x + hue;

      for (uint8_t z = 0; z < cloudHeight; z++) {
        int yoffset = noiseScale * z - hue;
        uint8_t dataSmoothing = 192;
        uint8_t noiseData = qsub8(inoise8(ff_x + xoffset, ff_y + yoffset, ff_z), 16);
        noiseData = qadd8(noiseData, scale8(noiseData, 39));
        noise[x * cloudHeight + z] = scale8( noise[x * cloudHeight + z], dataSmoothing) + scale8( noiseData, 256 - dataSmoothing);
        nblend(leds[XY(x, HEIGHT - z - 1)], ColorFromPalette(rainClouds_p, noise[x * cloudHeight + z]), (cloudHeight - z) * (250 / cloudHeight));
      }
      ff_z ++;
    }
  }
}

uint8_t myScale8(uint8_t x) { // даёт масштабировать каждые 8 градаций (от 0 до 7) бегунка Масштаб в значения от 0 до 255 по типа синусоиде
  uint8_t x8 = x % 8U;
  uint8_t x4 = x8 % 4U;
  if (x4 == 0U)
    if (x8 == 0U)       return 0U;
    else                return 255U;
  else if (x8 < 4U)     return (1U   + x4 * 72U); // всего 7шт по 36U + 3U лишних = 255U (чтобы восхождение по синусоиде не было зеркально спуску)
  //else
  return (253U - x4 * 72U); // 253U = 255U - 2U
}

// =====================================
//                Ocaдки
// =====================================
void coloredRain() { // внимание! этот эффект заточен на работу бегунка Масштаб в диапазоне от 0 до 255. пока что единственный.
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    uint8_t tmp = 1U + random8(255U);
    if ((tmp % 4U == 0U) && (tmp % 8U != 0U)) tmp--;
    setModeSettings(tmp, 165U + random8(76U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  // я хз, как прикрутить а 1 регулятор и длину хвостов и цвет капель
  // ( Depth of dots, maximum brightness, frequency of new dots, length of tails, color, splashes, clouds, ligthening )
  //rain(60, 200, map8(intensity,5,100), 195, CRGB::Green, false, false, false); // было CRGB::Green
  if (modes[currentMode].Scale > 247U)
    rain(60, 200, map8(42, 5, 100), myScale8(modes[currentMode].Scale), solidRainColor, false, false, false);
  else
    rain(60, 200, map8(42, 5, 100), myScale8(modes[currentMode].Scale), CHSV(modes[currentMode].Scale, 255U, 255U), false, false, false);
}

// =====================================
//             Tyчкa в бaнкe
// =====================================
void simpleRain() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(random8(2U) ? 2U + random8(7U) : 9U + random8(70U), 220U + random8(22U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  // ( Depth of dots, maximum brightness, frequency of new dots, length of tails, color, splashes, clouds, ligthening )
  //rain(60, 200, map8(intensity,2,60), 10, solidRainColor, true, true, false);
  rain(60, 180, (modes[currentMode].Scale - 1) * 2.58, 30, solidRainColor, true, true, false);
}

// =====================================
//             Гроза в банке
// =====================================
void stormyRain() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(random8(2U) ? 2U + random8(15U) : 17U + random8(64U), 220U + random8(22U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  // ( Depth of dots, maximum brightness, frequency of new dots, length of tails, color, splashes, clouds, ligthening )
  //rain(0, 90, map8(intensity,0,150)+60, 10, solidRainColor, true, true, true);
  rain(60, 160, (modes[currentMode].Scale - 1) * 2.58, 30, solidRainColor, true, true, true);
}


// =====================================
//                Mepцaниe
// =====================================
//             © SottNick

#define TWINKLES_SPEEDS 4     // всего 4 варианта скоростей мерцания
#define TWINKLES_MULTIPLIER 6 // слишком медленно, если на самой медленной просто по единичке к яркости добавлять

void twinklesRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(8U) * 11U + 2U + random8(9U) , 180U + random8(69U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();
    hue = 0U;
    deltaValue = (modes[currentMode].Scale - 1U) % 11U + 1U;  // вероятность пикселя загореться от 1/1 до 1/11
    for (uint32_t idx = 0; idx < NUM_LEDS; idx++) {
      if (random8(deltaValue) == 0) {
        ledsbuff[idx].r = random8();                          // оттенок пикселя
        ledsbuff[idx].g = random8(1, TWINKLES_SPEEDS * 2 + 1); // скорость и направление (нарастает 1-4 или угасает 5-8)
        ledsbuff[idx].b = random8();                          // яркость
      }
      else
        ledsbuff[idx] = 0;                                    // всё выкл
    }
  }
  for (uint32_t idx = 0; idx < NUM_LEDS; idx++) {
    if (ledsbuff[idx].b == 0) {
      if (random8(deltaValue) == 0 && hue > 0) { // если пиксель ещё не горит, зажигаем каждый ХЗй
        ledsbuff[idx].r = random8();                          // оттенок пикселя
        ledsbuff[idx].g = random8(1, TWINKLES_SPEEDS + 1);    // скорость и направление (нарастает 1-4, но не угасает 5-8)
        ledsbuff[idx].b = ledsbuff[idx].g;                    // яркость
        hue--; // уменьшаем количество погасших пикселей
      }
    }
    else if (ledsbuff[idx].g <= TWINKLES_SPEEDS) {            // если нарастание яркости
      if (ledsbuff[idx].b > 255U - ledsbuff[idx].g - TWINKLES_MULTIPLIER) {           // если досигнут максимум
        ledsbuff[idx].b = 255U;
        ledsbuff[idx].g = ledsbuff[idx].g + TWINKLES_SPEEDS;
      }
      else
        ledsbuff[idx].b = ledsbuff[idx].b + ledsbuff[idx].g + TWINKLES_MULTIPLIER;
    }
    else {                                                    // если угасание яркости
      if (ledsbuff[idx].b <= ledsbuff[idx].g - TWINKLES_SPEEDS + TWINKLES_MULTIPLIER) { // если досигнут минимум
        ledsbuff[idx].b = 0;                                  // всё выкл
        hue++; // считаем количество погасших пикселей
      }
      else
        ledsbuff[idx].b = ledsbuff[idx].b - ledsbuff[idx].g + TWINKLES_SPEEDS - TWINKLES_MULTIPLIER;
    }
    if (ledsbuff[idx].b == 0)
      leds[idx] = 0U;
    else
      leds[idx] = ColorFromPalette(*curPalette, ledsbuff[idx].r, ledsbuff[idx].b);
  }
}

// =====================================
//           Mячики бeз гpaниц
// =====================================
// Aurora : https://github.com/pixelmatix/aurora/blob/master/PatternBounce.h
// Copyright © 2014 Jason Coon
// v1.0 - Updating for GuverLamp v1.7 by Palpalych 14.04.2020
//#define e_bnc_COUNT (WIDTH) // теперь enlargedObjectNUM. хз, почему использовалась ширина матрицы тут, если по параметру идёт обращение к массиву boids, у которого может быть меньший размер
#define e_bnc_SIDEJUMP (true)
PVector gravity = PVector(0, -0.0125);
void bounceRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(9U) * 11U + 3U + random8(9U), random8(4U) ? 3U + random8(26U) : 255U);
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();
    //hue2 = 255U - scale8(64U, myScale8(modes[currentMode].Scale * 2.55));
    //hue2 = 254U - ((modes[currentMode].Scale - 1U) % 11U) * 3;
    enlargedObjectNUM = (modes[currentMode].Scale - 1U) % 11U / 10.0 * (AVAILABLE_BOID_COUNT - 1U) + 1U;
    uint8_t colorWidth = 256U / enlargedObjectNUM;
    for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
      Boid boid = Boid(i % WIDTH, 0);//random8(HEIGHT));//HEIGHT / 8
      boid.velocity.x = 0;
      //boid.location.y = 0;//(HEIGHT -1) / 4;
      boid.velocity.y = i * -0.01;
      boid.colorIndex = colorWidth * i;
      boid.maxforce = 10;
      boid.maxspeed = 10;
      boids[i] = boid;
    }
  }
  blurScreen(beatsin8(5U, 1U, 5U));
  dimAll(255U - modes[currentMode].Speed); // dimAll(hue2);
  for (uint8_t i = 0; i < enlargedObjectNUM; i++)
  {
    Boid boid = boids[i];
    boid.applyForce(gravity);
    boid.update();
    if (boid.location.x >= WIDTH) boid.location.x = boid.location.x - WIDTH; // это только
    else if (boid.location.x < 0) boid.location.x = boid.location.x + WIDTH; // для субпиксельной версии
    CRGB color = ColorFromPalette(*curPalette, boid.colorIndex); // boid.colorIndex + hue
    //drawPixelXY((uint32_t)(boid.location.x) % WIDTH, boid.location.y, color);
    //drawPixelXYFseamless(boid.location.x, boid.location.y, color); вот это я тупанул
    drawPixelXYF(boid.location.x, boid.location.y, color);

    if (boid.location.y <= 0)
    {
      boid.location.y = 0;
      boid.velocity.y = -boid.velocity.y;
      boid.velocity.x *= 0.9;
      if (!random8() || boid.velocity.y < 0.01)
      {
#if e_bnc_SIDEJUMP
        boid.applyForce(PVector((float)random(127) / 255 - 0.25, (float)random(255) / 255));
#else
        boid.applyForce(PVector(0, (float)random(255) / 255));
#endif
      }
    }
    boids[i] = boid;
  }
}



// ============== Smoke ================
//             © SottNick
//             ЭФФЕКТ ДЫМ
//--------------------------------------
void Smoke() {
  MultipleStreamSmoke(false);
}
//--------------------------------------
void SmokeColor() {
  MultipleStreamSmoke(true);
}
//--------------------------------------
void MultipleStreamSmoke(bool isColored) {
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(9U);
      setModeSettings(isColored ? 1U + tmp*tmp : (random8(10U) ? 1U + random8(99U) : 100U), 145U + random8(56U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    hue2 = 0U;
  }
  dimAll(254U);
  deltaHue++;
  CRGB color;
  if (isColored) {
    if (hue2 == modes[currentMode].Scale) {
      hue2 = 0U;
      hue = random8();
    }
    if (deltaHue & 0x01) {  //((deltaHue >> 2U) == 0U) // какой-то умножитель охота подключить к задержке смены цвета, но хз какой...
      hue2++;
    }
    hsv2rgb_spectrum(CHSV(hue, 255U, 127U), color);
  } else {
    hsv2rgb_spectrum(CHSV((modes[currentMode].Scale - 1U) * 2.6, (modes[currentMode].Scale > 98U) ? 0U : 255U, 127U), color);
  }

  if (random8(WIDTH) != 0U) { // встречная спираль движется не всегда синхронно основной
    deltaHue2--;
  }
  for (uint8_t y = 0; y < HEIGHT; y++) {
    leds[XY((deltaHue  + y + 1U) % WIDTH, HEIGHT - 1U - y)] += color;
    leds[XY((deltaHue  + y     ) % WIDTH, HEIGHT - 1U - y)] += color; //color2
    leds[XY((deltaHue2 + y     ) % WIDTH,               y)] += color;
    leds[XY((deltaHue2 + y + 1U) % WIDTH,               y)] += color; //color2
  }

  // Noise
  // скорость движения по массиву noise
  //uint32_t mult = 500U * ((modes[currentMode].Scale - 1U) % 10U);
  noise32_x[0] += 1500;//1000;
  noise32_y[0] += 1500;//1000;
  noise32_z[0] += 1500;//1000;
  scale32_x[0] = 4000;
  scale32_y[0] = 4000;
  FillNoise(0);

  // допустимый отлёт зажжённого пикселя от изначально присвоенного местоположения (от 0 до указанного значения. дробное)
  //mult = (modes[currentMode].Brightness - 1U) % 10U;
  MoveFractionalNoiseX(3);//4
  MoveFractionalNoiseY(3);//4

  blurScreen(20); // без размытия как-то пиксельно, наверное...
}

// =====================================
//               Пикacco
// =====================================
// взято откуда-то by @obliterator или им написано
// https://github.com/DmytroKorniienko/FireLamp_JeeUI/blob/templ/src/effects.cpp
//вместо класса Particle будем повторно использовать переменные из эффекта мячики и мотыльки

void PicassoGenerate(bool reset) {
  if (loadingFlag)  {
    loadingFlag = false;
    //setCurrentPalette();
    //FastLED.clear();
    // not for 3in1
    //    enlargedObjectNUM = (modes[currentMode].Scale - 1U) / 99.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
    //enlargedObjectNUM = (modes[currentMode].Scale - 1U) % 11U / 10.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
    if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
    if (enlargedObjectNUM < 2U) enlargedObjectNUM = 2U;

    double minSpeed = 0.2, maxSpeed = 0.8;

    for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {
      trackingObjectPosX[i] = random8(WIDTH);
      trackingObjectPosY[i] = random8(HEIGHT);

      //curr->color = CHSV(random(1U, 255U), 255U, 255U);
      trackingObjectHue[i] = random8();

      trackingObjectSpeedY[i] = +((-maxSpeed / 3) + (maxSpeed * (float)random8(1, 100) / 100));
      trackingObjectSpeedY[i] += trackingObjectSpeedY[i] > 0 ? minSpeed : -minSpeed;

      trackingObjectShift[i] = +((-maxSpeed / 2) + (maxSpeed * (float)random8(1, 100) / 100));
      trackingObjectShift[i] += trackingObjectShift[i] > 0 ? minSpeed : -minSpeed;

      trackingObjectState[i] = trackingObjectHue[i];

    }
  }
  for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {
    if (reset) {
      trackingObjectState[i] = random8();
      trackingObjectSpeedX[i] = (trackingObjectState[i] - trackingObjectHue[i]) / 25;
    }
    if (trackingObjectState[i] != trackingObjectHue[i] && trackingObjectSpeedX[i]) {
      trackingObjectHue[i] += trackingObjectSpeedX[i];
    }
  }

}

void PicassoPosition() {
  for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {
    if (trackingObjectPosX[i] + trackingObjectSpeedY[i] > WIDTH || trackingObjectPosX[i] + trackingObjectSpeedY[i] < 0) {
      trackingObjectSpeedY[i] = -trackingObjectSpeedY[i];
    }

    if (trackingObjectPosY[i] + trackingObjectShift[i] > HEIGHT || trackingObjectPosY[i] + trackingObjectShift[i] < 0) {
      trackingObjectShift[i] = -trackingObjectShift[i];
    }

    trackingObjectPosX[i] += trackingObjectSpeedY[i];
    trackingObjectPosY[i] += trackingObjectShift[i];
  };
}

void PicassoRoutine() {
#if false  //defined(singleUSE_RANDOM_SETS_IN_APP) || defined(singleRANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(17U + random8(64U) , 190U + random8(41U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  PicassoGenerate(false);
  PicassoPosition();
  for (uint8_t i = 0 ; i < enlargedObjectNUM - 2U ; i += 2) {
    DrawLine(trackingObjectPosX[i], trackingObjectPosY[i], trackingObjectPosX[i + 1U], trackingObjectPosY[i + 1U], CHSV(trackingObjectHue[i], 255U, 255U));
  }

  EVERY_N_MILLIS(20000) {
    PicassoGenerate(true);
  }
  blurScreen(80);
}

void PicassoRoutine2() {
#if false //defined(singleUSE_RANDOM_SETS_IN_APP) || defined(singleRANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(17U + random8(27U) , 185U + random8(46U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  PicassoGenerate(false);
  PicassoPosition();
  dimAll(180);
  for (uint8_t i = 0 ; i < enlargedObjectNUM - 1U ; i++) {
    DrawLineF(trackingObjectPosX[i], trackingObjectPosY[i], trackingObjectPosX[i + 1U], trackingObjectPosY[i + 1U], CHSV(trackingObjectHue[i], 255U, 255U));
  }

  EVERY_N_MILLIS(20000) {
    PicassoGenerate(true);
  }

  blurScreen(80);
}

void PicassoRoutine3() {
#if false //defined(singleUSE_RANDOM_SETS_IN_APP) || defined(singleRANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(19U + random8(31U) , 150U + random8(63U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  PicassoGenerate(false);
  PicassoPosition();
  dimAll(180);
  for (uint8_t i = 0 ; i < enlargedObjectNUM - 2U ; i += 2) {
    drawCircleF(fabs(trackingObjectPosX[i] - trackingObjectPosX[i + 1U]), fabs(trackingObjectPosY[i] - trackingObjectPosX[i + 1U]), fabs(trackingObjectPosX[i] - trackingObjectPosY[i]), CHSV(trackingObjectHue[i], 255U, 255U));
  }
  EVERY_N_MILLIS(20000) {
    PicassoGenerate(true);
  }
  blurScreen(80);
}

void picassoSelector() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    uint8_t tmp = random8(3U);
    if (tmp == 2U) setModeSettings(4U + random8(42U) , 190U + random8(41U));
    else if (tmp) setModeSettings(39U + random8(8U) , 185U + random8(46U));
    else setModeSettings(73U + random8(10U) , 150U + random8(63U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  if (loadingFlag) {
    if (modes[currentMode].Scale < 34U)           // если масштаб до 34
      enlargedObjectNUM = (modes[currentMode].Scale - 1U) / 32.0 * (enlargedOBJECT_MAX_COUNT - 3U) + 3U;
    else if (modes[currentMode].Scale >= 68U)      // если масштаб больше 67
      enlargedObjectNUM = (modes[currentMode].Scale - 68U) / 32.0 * (enlargedOBJECT_MAX_COUNT - 3U) + 3U;
    else                                          // для масштабов посередине
      enlargedObjectNUM = (modes[currentMode].Scale - 34U) / 33.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
  }

  if (modes[currentMode].Scale < 34U)           // если масштаб до 34
    PicassoRoutine();
  else if (modes[currentMode].Scale > 67U)      // если масштаб больше 67
    PicassoRoutine3();
  else                                          // для масштабов посередине
    PicassoRoutine2();
}

// =====================================
//                Пpыгyны
// =====================================
// взято откуда-то by @obliterator
// https://github.com/DmytroKorniienko/FireLamp_JeeUI/blob/templ/src/effects.cpp
//вместо класса Leaper будем повторно использовать переменные из эффекта мячики и мотыльки

void LeapersRestart_leaper(uint8_t l) {
  // leap up and to the side with some random component
  trackingObjectSpeedX[l] = (1 * (float)random8(1, 100) / 100);
  trackingObjectSpeedY[l] = (2 * (float)random8(1, 100) / 100);

  // for variety, sometimes go 50% faster
  if (random8() < 12) {
    trackingObjectSpeedX[l] += trackingObjectSpeedX[l] * 0.5;
    trackingObjectSpeedY[l] += trackingObjectSpeedY[l] * 0.5;
  }

  // leap towards the centre of the screen
  if (trackingObjectPosX[l] > (WIDTH / 2)) {
    trackingObjectSpeedX[l] = -trackingObjectSpeedX[l];
  }
}

void LeapersMove_leaper(uint8_t l) {
#define GRAVITY            0.06
#define SETTLED_THRESHOLD  0.1
#define WALL_FRICTION      0.95
#define WIND               0.95    // wind resistance

  trackingObjectPosX[l] += trackingObjectSpeedX[l];
  trackingObjectPosY[l] += trackingObjectSpeedY[l];

  // bounce off the floor and ceiling?
  if (trackingObjectPosY[l] < 0 || trackingObjectPosY[l] > HEIGHT - 1) {
    trackingObjectSpeedY[l] = (-trackingObjectSpeedY[l] * WALL_FRICTION);
    trackingObjectSpeedX[l] = ( trackingObjectSpeedX[l] * WALL_FRICTION);
    trackingObjectPosY[l] += trackingObjectSpeedY[l];
    if (trackingObjectPosY[l] < 0)
      trackingObjectPosY[l] = 0; // settled on the floor?
    if (trackingObjectPosY[l] <= SETTLED_THRESHOLD && fabs(trackingObjectSpeedY[l]) <= SETTLED_THRESHOLD) {
      LeapersRestart_leaper(l);
    }
  }

  // bounce off the sides of the screen?
  if (trackingObjectPosX[l] <= 0 || trackingObjectPosX[l] >= WIDTH - 1) {
    trackingObjectSpeedX[l] = (-trackingObjectSpeedX[l] * WALL_FRICTION);
    if (trackingObjectPosX[l] <= 0) {
      //trackingObjectPosX[l] = trackingObjectSpeedX[l]; // the bug?
      trackingObjectPosX[l] = -trackingObjectPosX[l];
    } else {
      //trackingObjectPosX[l] = WIDTH - 1 - trackingObjectSpeedX[l]; // the bug?
      trackingObjectPosX[l] = WIDTH + WIDTH - 2 - trackingObjectPosX[l];
    }
  }

  trackingObjectSpeedY[l] -= GRAVITY;
  trackingObjectSpeedX[l] *= WIND;
  trackingObjectSpeedY[l] *= WIND;
}


void LeapersRoutine() {
  //unsigned num = map(scale, 0U, 255U, 6U, sizeof(boids) / sizeof(*boids));
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(8U) * 11U + 5U + random8(7U) , 185U + random8(56U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();
    //FastLED.clear();
    //enlargedObjectNUM = (modes[currentMode].Scale - 1U) / 99.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
    enlargedObjectNUM = (modes[currentMode].Scale - 1U) % 11U / 10.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
    if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
    //if (enlargedObjectNUM < 2U) enlargedObjectNUM = 2U;

    for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {
      trackingObjectPosX[i] = random8(WIDTH);
      trackingObjectPosY[i] = random8(HEIGHT);

      //curr->color = CHSV(random(1U, 255U), 255U, 255U);
      trackingObjectHue[i] = random8();
    }
  }

  //myLamp.dimAll(0); накой хрен делать затухание на 100%?
  FastLED.clear();

  for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
    LeapersMove_leaper(i);
    //drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], CHSV(trackingObjectHue[i], 255U, 255U));
    drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], ColorFromPalette(*curPalette, trackingObjectHue[i]));
  };
  blurScreen(20);
}

// =====================================
//             Лaвoвaя лaмпa
// =====================================
//             © SottNick

void LavaLampGetspeed(uint8_t l) {
  //trackingObjectSpeedY[l] = (float)random8(1, 11) / 10.0; // скорость пузырей 10 градаций?
  trackingObjectSpeedY[l] = (float)random8(5, 11) / (257U - modes[currentMode].Speed) / 4.0; // если скорость кадров фиксированная
}

void drawBlob(uint8_t l, CRGB color) { //раз круги нарисовать не получается, будем попиксельно вырисовывать 2 варианта пузырей
  if (trackingObjectShift[l] == 2) {
    for (int8_t x = -2; x < 3; x++) {
      for (int8_t y = -2; y < 3; y++) {
        if (abs(x) + abs(y) < 4) {
          drawPixelXYF(fmod(trackingObjectPosX[l] + x + WIDTH, WIDTH), trackingObjectPosY[l] + y, color);
        }
      }
    }
  } else {
    for (int8_t x = -1; x < 3; x++) {
      for (int8_t y = -1; y < 3; y++) {
        if (!(x == -1 && (y == -1 || y == 2) || x == 2 && (y == -1 || y == 2))) {
          drawPixelXYF(fmod(trackingObjectPosX[l] + x + WIDTH, WIDTH), trackingObjectPosY[l] + y, color);
        }
      }
    }
  }
}

void LavaLampRoutine() {
  //unsigned num = map(scale, 0U, 255U, 6U, sizeof(boids) / sizeof(*boids));
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(30U) ? (random8(3U) ? 2U + random8(98U) : 1U) : 100U, 50U + random8(196U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    enlargedObjectNUM = (WIDTH / 2) -  ((WIDTH - 1) & 0x01);

    uint8_t shift = random8(2);
    for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
      trackingObjectPosY[i] = 0;//random8(HEIGHT);
      trackingObjectPosX[i] = i * 2U + shift;
      LavaLampGetspeed(i);
      trackingObjectShift[i] = random8(1, 3); // присваивается случайный целочисленный радиус пузырям от 1 до 2
    }
    if (modes[currentMode].Scale != 1U) {
      hue = modes[currentMode].Scale * 2.57;
    }
  }
  if (modes[currentMode].Scale == 1U) {
    hue2++;
    if (hue2 % 0x10 == 0U) hue++;
  }
  CRGB color = CHSV(hue, (modes[currentMode].Scale < 100U) ? 255U : 0U, 255U);
  FastLED.clear();

  for (uint8_t i = 0; i < enlargedObjectNUM; i++) { //двигаем по аналогии с https://jiwonk.im/lavalamp/
    //LavaLampMove_leaper(i);
    if (trackingObjectPosY[i] + trackingObjectShift[i] >= HEIGHT - 1)
      trackingObjectPosY[i] += (trackingObjectSpeedY[i] * ((HEIGHT - 1 - trackingObjectPosY[i]) / trackingObjectShift[i] + 0.005));
    else if (trackingObjectPosY[i] - trackingObjectShift[i] <= 0)
      trackingObjectPosY[i] += (trackingObjectSpeedY[i] * (trackingObjectPosY[i] / trackingObjectShift[i] + 0.005));
    else
      trackingObjectPosY[i] += trackingObjectSpeedY[i];

    // bounce off the floor and ceiling?
    if (trackingObjectPosY[i] < 0.01) {                  // почему-то при нуле появляется мерцание (один кадр, еле заметно)
      LavaLampGetspeed(i);
      //trackingObjectShift[i] = 1+2*trackingObjectSpeedY[i]; менять радиус после отскока - плохая идея
      trackingObjectPosY[i] = 0.01;
    }
    else if (trackingObjectPosY[i] > HEIGHT - 1.01) {    // тоже на всякий пожарный
      LavaLampGetspeed(i);
      //trackingObjectShift[i] = 1+2*trackingObjectSpeedY[i]; менять радиус после отскока - плохая идея
      trackingObjectSpeedY[i] = -trackingObjectSpeedY[i];
      trackingObjectPosY[i] = HEIGHT - 1.01;
    }
    drawBlob(i, color); // раз круги выглядят убого, рисуем попиксельно 2 размера пузырей
  };
  blurScreen(20);
}

// =====================================
//                 Teни
// =====================================
//             © vvip-68
// https://github.com/vvip-68/GyverPanelWiFi/blob/master/firmware/GyverPanelWiFi_v1.04/effects.ino

void shadowsRoutine() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(1U, 1U + random8(255U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(map(modes[currentMode].Speed, 1, 255, 100, 255), 32, map(modes[currentMode].Speed, 1, 255, 60, 255));//beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;

  byte effectBrightness = modes[currentMode].Scale * 2.55; //getBrightnessCalculated(globalBrightness, effectContrast[thisMode]);


  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for ( uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV( hue8, sat8, map8(bri8, map(effectBrightness, 32, 255, 32, 125), map(effectBrightness, 32, 255, 125, 250)));

    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS - 1) - pixelnumber;

    nblend( leds[pixelnumber], newcolor, 64);
  }
}
// =====================================
//                 ДНК
// =====================================
//      © Yaroslaw Turbin 04.09.2020
/*
  База https://pastebin.com/jwvC1sNF адаптация и доработки kostyamat
  https://pastebin.com/jwvC1sNF
  2 DNA spiral with subpixel
  16x16 rgb led matrix demo
  https://vk.com/ldirko
  https://www.reddit.com/user/ldirko/
  https://www.reddit.com/r/FastLED/comments/gogs4n/i_made_7x11_matrix_for_my_ntp_clock_project_then/

  this is update for DNA procedure https://pastebin.com/Qa8A5NvW
  add subpixel render foк nice smooth look
*/

void wu_pixel(uint32_t x, uint32_t y, CRGB * col) {      //awesome wu_pixel procedure by reddit u/sutaburosu
  // extract the fractional parts and derive their inverses
  uint8_t xx = x & 0xff, yy = y & 0xff, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
#define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)
                  };
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    uint16_t xy = XY((x >> 8) + (i & 1), (y >> 8) + ((i >> 1) & 1));
    if (xy < NUM_LEDS) {
      leds[xy].r = qadd8(leds[xy].r, col->r * wu[i] >> 8);
      leds[xy].g = qadd8(leds[xy].g, col->g * wu[i] >> 8);
      leds[xy].b = qadd8(leds[xy].b, col->b * wu[i] >> 8);
    }
  }
}

void DNARoutine() {
  if (loadingFlag)  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(100U), 1U + random8(200U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    step = map8(modes[currentMode].Speed, 10U, 60U);
    hue = modes[currentMode].Scale;
    deltaHue = hue > 50U;
    if (deltaHue)
      hue = 101U - hue;
    hue = 255U - map( 51U - hue, 1U, 50U, 0, 255U);
  }
  double freq = 3000;
  float mn = 255.0 / 13.8;

  fadeToBlackBy(leds, NUM_LEDS, step);
  uint16_t ms = millis();

  if (deltaHue) {
    for (uint8_t i = 0; i < WIDTH; i++) {
      uint32_t x = beatsin16(step, 0, (HEIGHT - 1) * 256, 0, i * freq);
      uint32_t y = i * 256;
      uint32_t x1 = beatsin16(step, 0, (HEIGHT - 1) * 256, 0, i * freq + 32768);

      CRGB col = CHSV(ms / 29 + i * 255 / (WIDTH - 1), 255, qadd8(hue, beatsin8(step, 60, 255U, 0, i * mn)));
      CRGB col1 = CHSV(ms / 29 + i * 255 / (WIDTH - 1) + 128, 255, qadd8(hue, beatsin8(step, 60, 255U, 0, i * mn + 128)));
      wu_pixel (y , x, &col);
      wu_pixel (y , x1, &col1);
    }
  } else {
    for (uint8_t i = 0; i < HEIGHT; i++) {
      uint32_t x = beatsin16(step, 0, (WIDTH - 1) * 256, 0, i * freq);
      uint32_t y = i * 256;
      uint32_t x1 = beatsin16(step, 0, (WIDTH - 1) * 256, 0, i * freq + 32768);

      CRGB col = CHSV(ms / 29 + i * 255 / (HEIGHT - 1), 255, qadd8(hue, beatsin8(step, 60, 255U, 0, i * mn)));
      CRGB col1 = CHSV(ms / 29 + i * 255 / (HEIGHT - 1) + 128, 255, qadd8(hue, beatsin8(step, 60, 255U, 0, i * mn + 128)));
      wu_pixel (x , y, &col);
      wu_pixel (x1 , y, &col1);
    }
  }
  blurScreen(16);
}

// =====================================
//                Змейки
// =====================================
//             © SottNick
/*
  #define enlargedOBJECT_MAX_COUNT (WIDTH * 2)              // максимальное количество червяков
  uint8_t enlargedObjectNUM;                                // выбранное количество червяков
  long  enlargedObjectTime[enlargedOBJECT_MAX_COUNT] ;      // тут будет траектория тела червяка
  float trackingObjectPosX[trackingOBJECT_MAX_COUNT];       // тут будет позиция головы
  float trackingObjectPosY[trackingOBJECT_MAX_COUNT];       // тут будет позиция головы
  float trackingObjectSpeedX[trackingOBJECT_MAX_COUNT];     // тут будет скорость червяка
  float trackingObjectSpeedY[trackingOBJECT_MAX_COUNT];     // тут будет дробная часть позиции головы
  float trackingObjectShift[trackingOBJECT_MAX_COUNT];      // не пригодилось пока что
  uint8_t trackingObjectHue[trackingOBJECT_MAX_COUNT];      // тут будет начальный цвет червяка
  uint8_t trackingObjectState[trackingOBJECT_MAX_COUNT];    // тут будет направление червяка
*/

#define SNAKES_LENGTH (8U) // длина червяка от 2 до 15 (+ 1 пиксель голова/хвостик), ограничена размером переменной для хранения трактории тела червяка
void snakesRoutine() {
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(8U);
      setModeSettings(8U + tmp * tmp, 20U + random8(120U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    speedfactor = (float)modes[currentMode].Speed / 555.0f + 0.001f;
    enlargedObjectNUM = (modes[currentMode].Scale - 1U) / 99.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
    if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
    for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
      enlargedObjectTime[i] = 0;
      trackingObjectPosX[i] = random8(WIDTH);
      trackingObjectPosY[i] = random8(HEIGHT);
      trackingObjectSpeedX[i] = (255. + random8()) / 255.;
      trackingObjectSpeedY[i] = 0;
      trackingObjectHue[i] = random8();
      trackingObjectState[i] = random8(4);//     B00           направление головы змейки
    }
  }
  FastLED.clear();

  int8_t dx, dy;
  for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
    trackingObjectSpeedY[i] += trackingObjectSpeedX[i] * speedfactor;
    if (trackingObjectSpeedY[i] >= 1) {
      trackingObjectSpeedY[i] = trackingObjectSpeedY[i] - (int)trackingObjectSpeedY[i];
      if (random8(9U) == 0U) // вероятность поворота
        if (random8(2U)) { // <- поворот налево
          enlargedObjectTime[i] = (enlargedObjectTime[i] << 2) | B01; // младший бит = поворот
          switch (trackingObjectState[i]) {
            case B10:
              trackingObjectState[i] = B01;
              if (trackingObjectPosY[i] == 0U) trackingObjectPosY[i] = HEIGHT - 1U;
              else trackingObjectPosY[i]--;
              break;
            case B11:
              trackingObjectState[i] = B00;
              if (trackingObjectPosY[i] >= HEIGHT - 1U) trackingObjectPosY[i] = 0U;
              else trackingObjectPosY[i]++;
              break;
            case B00:
              trackingObjectState[i] = B10;
              if (trackingObjectPosX[i] == 0U) trackingObjectPosX[i] = WIDTH - 1U;
              else trackingObjectPosX[i]--;
              break;
            case B01:
              trackingObjectState[i] = B11;
              if (trackingObjectPosX[i] >= WIDTH - 1U) trackingObjectPosX[i] = 0U;
              else trackingObjectPosX[i]++;
              break;
          }
        } else { // -> поворот направо
          enlargedObjectTime[i] = (enlargedObjectTime[i] << 2) | B11; // младший бит = поворот, старший = направо
          switch (trackingObjectState[i]) {
            case B11:
              trackingObjectState[i] = B01;
              if (trackingObjectPosY[i] == 0U) trackingObjectPosY[i] = HEIGHT - 1U;
              else trackingObjectPosY[i]--;
              break;
            case B10:
              trackingObjectState[i] = B00;
              if (trackingObjectPosY[i] >= HEIGHT - 1U) trackingObjectPosY[i] = 0U;
              else trackingObjectPosY[i]++;
              break;
            case B01:
              trackingObjectState[i] = B10;
              if (trackingObjectPosX[i] == 0U) trackingObjectPosX[i] = WIDTH - 1U;
              else trackingObjectPosX[i]--;
              break;
            case B00:
              trackingObjectState[i] = B11;
              if (trackingObjectPosX[i] >= WIDTH - 1U)  trackingObjectPosX[i] = 0U;
              else trackingObjectPosX[i]++;
              break;
          }
        } else { // двигаем без поворота
        enlargedObjectTime[i] = (enlargedObjectTime[i] << 2);
        switch (trackingObjectState[i]) {
          case B01:
            if (trackingObjectPosY[i] == 0U) trackingObjectPosY[i] = HEIGHT - 1U;
            else trackingObjectPosY[i]--;
            break;
          case B00:
            if (trackingObjectPosY[i] >= HEIGHT - 1U) trackingObjectPosY[i] = 0U;
            else  trackingObjectPosY[i]++;
            break;
          case B10:
            if (trackingObjectPosX[i] == 0U) trackingObjectPosX[i] = WIDTH - 1U;
            else  trackingObjectPosX[i]--;
            break;
          case B11:
            if (trackingObjectPosX[i] >= WIDTH - 1U)  trackingObjectPosX[i] = 0U;
            else trackingObjectPosX[i]++;
            break;
        }
      }
    }

    switch (trackingObjectState[i]) {
      case B01:
        dy = 1;
        dx = 0;
        break;
      case B00:
        dy = -1;
        dx = 0;
        break;
      case B10:
        dy = 0;
        dx = 1;
        break;
      case B11:
        dy = 0;
        dx = -1;
        break;
    }
    long temp = enlargedObjectTime[i];
    uint8_t x = trackingObjectPosX[i];
    uint8_t y = trackingObjectPosY[i];
    //CHSV color = CHSV(trackingObjectHue[i], 255U, 255U);
    //drawPixelXY(x, y, color);
    //drawPixelXYF(x, y, CHSV(trackingObjectHue[i], 255U, trackingObjectSpeedY[i] * 255)); // тут рисуется голова // слишком сложно для простого сложения цветов
    leds[XY(x, y)] += CHSV(trackingObjectHue[i], 255U, trackingObjectSpeedY[i] * 255); // тут рисуется голова

    for (uint8_t m = 0; m < SNAKES_LENGTH; m++) { // 16 бит распаковываем, 14 ещё остаётся без дела в запасе, 2 на хвостик
      x = (WIDTH + x + dx) % WIDTH;
      y = (HEIGHT + y + dy) % HEIGHT;
      //drawPixelXYF(x, y, CHSV(trackingObjectHue[i] + m*4U, 255U, 255U)); // тут рисуется тело // слишком сложно для простого сложения цветов
      //leds[XY(x,y)] += CHSV(trackingObjectHue[i] + m*4U, 255U, 255U); // тут рисуется тело
      leds[XY(x, y)] += CHSV(trackingObjectHue[i] + (m + trackingObjectSpeedY[i]) * 4U, 255U, 255U); // тут рисуется тело

      if (temp & B01) {       // младший бит = поворот, старший = направо
        temp = temp >> 1;
        if (temp & B01) {     // старший бит = направо
          if (dx == 0) {
            dx = 0 - dy;
            dy = 0;
          } else {
            dy = dx;
            dx = 0;
          }
        } else {              // иначе налево
          if (dx == 0) {
            dx = dy;
            dy = 0;
          } else {
            dy = 0 - dx;
            dx = 0;
          }
        }
        temp = temp >> 1;
      } else {                // если без поворота
        temp = temp >> 2;
      }
    }
    x = (WIDTH + x + dx) % WIDTH;
    y = (HEIGHT + y + dy) % HEIGHT;
    leds[XY(x, y)] += CHSV(trackingObjectHue[i] + (SNAKES_LENGTH + trackingObjectSpeedY[i]) * 4U, 255U, (1 - trackingObjectSpeedY[i]) * 255); // хвостик
  }
}

// =====================================
//  Жидкая лампа || Жидкaя лaмпa (auto)
// =====================================
//            © obliterator
//  с генератором палитр by SottNick
// https://github.com/DmytroKorniienko/FireLamp_JeeUI/commit/9bad25adc2c917fbf3dfa97f4c498769aaf76ebe

//аналог ардуино функции map(), но только для float
float fmap(const float x, const float in_min, const float in_max, const float out_min, const float out_max) {
  return (out_max - out_min) * (x - in_min) / (in_max - in_min) + out_min;
}

float mapcurve(const float x, const float in_min, const float in_max, const float out_min, const float out_max, float (*curve)(float, float, float, float)) {
  if (x <= in_min) return out_min;
  if (x >= in_max) return out_max;
  return curve((x - in_min), out_min, (out_max - out_min), (in_max - in_min));
}

float InQuad(float t, float b, float c, float d) {
  t /= d;
  return c * t * t + b;
}

float OutQuart(float t, float b, float c, float d) {
  t = t / d - 1;
  return -c * (t * t * t * t - 1) + b;
}
float InOutQuad(float t, float b, float c, float d) {
  t /= d / 2;
  if (t < 1) return c / 2 * t * t + b;
  --t;
  return -c / 2 * (t * (t - 2) - 1) + b;
}

unsigned MASS_MIN = 10;
unsigned MASS_MAX = 50;

// массивы для метаболов (используем повторно всё подряд)
/*
  uint8_t trackingObjectHue[enlargedOBJECT_MAX_COUNT];
  float position_x = 0;
  float trackingObjectPosX[enlargedOBJECT_MAX_COUNT];
  float position_y = 0;
  float trackingObjectPosY[enlargedOBJECT_MAX_COUNT];
  float speed_x = 0;
  float trackingObjectSpeedX[enlargedOBJECT_MAX_COUNT];
  float speed_y = 0;
  float trackingObjectSpeedY[enlargedOBJECT_MAX_COUNT];
  float rad = 0;
  float trackingObjectShift[enlargedOBJECT_MAX_COUNT];
  float hot = 0;
  float liquidLampHot[enlargedOBJECT_MAX_COUNT];
  float spf = 0;
  float liquidLampSpf[enlargedOBJECT_MAX_COUNT];
  int mass = 0;
  uint8_t trackingObjectState[enlargedOBJECT_MAX_COUNT];
  unsigned mx = 0;
  unsigned liquidLampMX[enlargedOBJECT_MAX_COUNT];
  unsigned sc = 0;
  unsigned liquidLampSC[enlargedOBJECT_MAX_COUNT];
  unsigned tr = 0;
  unsigned liquidLampTR[enlargedOBJECT_MAX_COUNT];
*/

void LiquidLampPosition() {
  //bool physic_on = modes[currentMode].Speed & 0x01;
  for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
    liquidLampHot[i] += mapcurve(trackingObjectPosY[i], 0, HEIGHT - 1, 5, -5, InOutQuad) * speedfactor;

    float heat = (liquidLampHot[i] / trackingObjectState[i]) - 1;
    if (heat > 0 && trackingObjectPosY[i] < HEIGHT - 1) {
      trackingObjectSpeedY[i] += heat * liquidLampSpf[i];
    }
    if (trackingObjectPosY[i] > 0) {
      trackingObjectSpeedY[i] -= 0.07;
    }

    if (trackingObjectSpeedY[i]) trackingObjectSpeedY[i] *= 0.85;
    trackingObjectPosY[i] += trackingObjectSpeedY[i] * speedfactor;

    if (trackingObjectSpeedX[i]) trackingObjectSpeedX[i] *= 0.7;
    trackingObjectPosX[i] += trackingObjectSpeedX[i] * speedfactor;

    if (trackingObjectPosX[i] > WIDTH - 1) trackingObjectPosX[i] -= WIDTH - 1;
    if (trackingObjectPosX[i] < 0) trackingObjectPosX[i] += WIDTH - 1;
    if (trackingObjectPosY[i] > HEIGHT - 1) trackingObjectPosY[i] = HEIGHT - 1;
    if (trackingObjectPosY[i] < 0) trackingObjectPosY[i] = 0;
  };
}

void LiquidLampPhysic() {
  for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
    //Particle *p1 = (Particle *)&particles[i];
    // отключаем физику на границах, чтобы не слипались шары
    if (trackingObjectPosY[i] < 3 || trackingObjectPosY[i] > HEIGHT - 1) continue;
    for (uint8_t j = 0; j < enlargedObjectNUM; j++) {
      //Particle *p2 = (Particle *)&particles[j];
      if (trackingObjectPosY[j] < 3 || trackingObjectPosY[j] > HEIGHT - 1) continue;
      float radius = 3;//(trackingObjectShift[i] + trackingObjectShift[j]);
      if (trackingObjectPosX[i] + radius > trackingObjectPosX[j]
          && trackingObjectPosX[i] < radius + trackingObjectPosX[j]
          && trackingObjectPosY[i] + radius > trackingObjectPosY[j]
          && trackingObjectPosY[i] < radius + trackingObjectPosY[j]
         ) {
        //float dist = EffectMath::distance(p1->position_x, p1->position_y, p2->position_x, p2->position_y);
        float dx =  min((float)fabs(trackingObjectPosX[i] - trackingObjectPosX[j]), (float)WIDTH + trackingObjectPosX[i] - trackingObjectPosX[j]); //по идее бесшовный икс
        float dy =  fabs(trackingObjectPosY[i] - trackingObjectPosY[j]);
        float dist = SQRT_VARIANT((dx * dx) + (dy * dy));

        if (dist <= radius) {
          float nx = (trackingObjectPosX[j] - trackingObjectPosX[i]) / dist;
          float ny = (trackingObjectPosY[j] - trackingObjectPosY[i]) / dist;
          float p = 2 * (trackingObjectSpeedX[i] * nx + trackingObjectSpeedY[i] * ny - trackingObjectSpeedX[j] * nx - trackingObjectSpeedY[j] * ny) / (trackingObjectState[i] + trackingObjectState[j]);
          float pnx = p * nx, pny = p * ny;
          trackingObjectSpeedX[i] = trackingObjectSpeedX[i] - pnx * trackingObjectState[i];
          trackingObjectSpeedY[i] = trackingObjectSpeedY[i] - pny * trackingObjectState[i];
          trackingObjectSpeedX[j] = trackingObjectSpeedX[j] + pnx * trackingObjectState[j];
          trackingObjectSpeedY[j] = trackingObjectSpeedY[j] + pny * trackingObjectState[j];
        }
      }
    }
  }
}

// генератор палитр для Жидкой лампы (c) SottNick
static const uint8_t MBVioletColors_arr[5][4] PROGMEM = {     // та же палитра, но в формате CHSV
  {0  , 0  , 255, 255}, //  0, 255,   0,   0, // red
  {1  , 155, 209, 255}, //  1,  46, 124, 255, // сделал поярче цвет воды
  {80 , 170, 255, 140}, // 80,   0,   0, 139, // DarkBlue
  {150, 213, 255, 128}, //150, 128,   0, 128, // purple
  {255, 0  , 255, 255}  //255, 255,   0,   0  // red again
};
CRGBPalette16 myPal;
void fillMyPal16(uint8_t hue, bool isInvert = false) {
  int8_t lastSlotUsed = -1;
  uint8_t istart8, iend8;
  CRGB rgbstart, rgbend;

  // начинаем с нуля
  if (isInvert) {
    //с неявным преобразованием оттенков цвета получаются, как в фотошопе, но для данного эффекта не красиво выглядят
    //rgbstart = CHSV(256 + hue - pgm_read_byte(&MBVioletColors_arr[0][1]), pgm_read_byte(&MBVioletColors_arr[0][2]), pgm_read_byte(&MBVioletColors_arr[0][3])); // начальная строчка палитры с инверсией
    hsv2rgb_spectrum(CHSV(256 + hue - pgm_read_byte(&MBVioletColors_arr[0][1]), pgm_read_byte(&MBVioletColors_arr[0][2]), pgm_read_byte(&MBVioletColors_arr[0][3])), rgbstart);
  } else {
    //rgbstart = CHSV(hue + pgm_read_byte(&MBVioletColors_arr[0][1]), pgm_read_byte(&MBVioletColors_arr[0][2]), pgm_read_byte(&MBVioletColors_arr[0][3])); // начальная строчка палитры
    hsv2rgb_spectrum(CHSV(hue + pgm_read_byte(&MBVioletColors_arr[0][1]), pgm_read_byte(&MBVioletColors_arr[0][2]), pgm_read_byte(&MBVioletColors_arr[0][3])), rgbstart);
  }
  int indexstart = 0; // начальный индекс палитры
  for (uint8_t i = 1U; i < 5U; i++) { // в палитре @obliterator всего 5 строчек
    int indexend = pgm_read_byte(&MBVioletColors_arr[i][0]);
    if (isInvert) {
      //rgbend = CHSV(256 + hue - pgm_read_byte(&MBVioletColors_arr[i][1]), pgm_read_byte(&MBVioletColors_arr[i][2]), pgm_read_byte(&MBVioletColors_arr[i][3])); // следующая строчка палитры с инверсией
      hsv2rgb_spectrum(CHSV(256 + hue - pgm_read_byte(&MBVioletColors_arr[i][1]), pgm_read_byte(&MBVioletColors_arr[i][2]), pgm_read_byte(&MBVioletColors_arr[i][3])), rgbend);
    } else {
      //rgbend = CHSV(hue + pgm_read_byte(&MBVioletColors_arr[i][1]), pgm_read_byte(&MBVioletColors_arr[i][2]), pgm_read_byte(&MBVioletColors_arr[i][3])); // следующая строчка палитры
      hsv2rgb_spectrum(CHSV(hue + pgm_read_byte(&MBVioletColors_arr[i][1]), pgm_read_byte(&MBVioletColors_arr[i][2]), pgm_read_byte(&MBVioletColors_arr[i][3])), rgbend);
    }
    istart8 = indexstart / 16;
    iend8   = indexend   / 16;
    if ((istart8 <= lastSlotUsed) && (lastSlotUsed < 15)) {
      istart8 = lastSlotUsed + 1;
      if (iend8 < istart8)
        iend8 = istart8;
    }
    lastSlotUsed = iend8;
    fill_gradient_RGB( myPal, istart8, rgbstart, iend8, rgbend);
    indexstart = indexend;
    rgbstart = rgbend;
  }
}

// --------------------------------------
void LiquidLamp() {
  LiquidLampRoutine(true);
}

// --------------------------------------
void LiquidLampAuto() {
  LiquidLampRoutine(false);
}

// --------------------------------------
void LiquidLampRoutine(bool isColored) {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //1-9,31-38,46-48,93-99
      //1-17,28-38,44-48,89-99
      uint8_t tmp = random8(28U);
      if (tmp > 9U) tmp += 21U;
      if (tmp > 38U) tmp += 7U;
      if (tmp > 48U) tmp += 44U;
      setModeSettings(isColored ? tmp : 27U + random8(54U), 30U + random8(170U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    speedfactor = modes[currentMode].Speed / 64.0 + 0.1; // 127 БЫЛО

    if (isColored) {
      fillMyPal16((modes[currentMode].Scale - 1U) * 2.55, !(modes[currentMode].Scale & 0x01));
      enlargedObjectNUM = enlargedOBJECT_MAX_COUNT / 2U - 2U; //14U;
    }
    else {
      enlargedObjectNUM = (modes[currentMode].Scale - 1U) / 99.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
      hue = random8();
      deltaHue = random8(2U);
      fillMyPal16(hue, deltaHue);
    }
    if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
    else if (enlargedObjectNUM < 2U) enlargedObjectNUM = 2U;

    double minSpeed = 0.2, maxSpeed = 0.8;

    for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {
      trackingObjectPosX[i] = random8(WIDTH);
      trackingObjectPosY[i] = 0; //random8(HEIGHT);
      trackingObjectState[i] = random(MASS_MIN, MASS_MAX);
      liquidLampSpf[i] = fmap(trackingObjectState[i], MASS_MIN, MASS_MAX, 0.0015, 0.0005);
      trackingObjectShift[i] = fmap(trackingObjectState[i], MASS_MIN, MASS_MAX, 2, 3);
      liquidLampMX[i] = map(trackingObjectState[i], MASS_MIN, MASS_MAX, 60, 80); // сила возмущения
      liquidLampSC[i] = map(trackingObjectState[i], MASS_MIN, MASS_MAX, 6, 10); // радиус возмущения
      liquidLampTR[i] = liquidLampSC[i]  * 2 / 3; // отсечка расчетов (оптимизация скорости)
    }

  }

  LiquidLampPosition();
  //bool physic_on = modes[currentMode].Speed & 0x01;
  //if (physic_on)
  LiquidLampPhysic;

  if (!isColored) {
    hue2++;
    if (hue2 % 0x10 == 0U) {
      hue++;
      fillMyPal16(hue, deltaHue);
    }
  }

  for (uint8_t x = 0; x < WIDTH; x++) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      float sum = 0;
      //for (unsigned i = 0; i < numParticles; i++) {
      for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
        //Particle *p1 = (Particle *)&particles[i];
        if (abs(x - trackingObjectPosX[i]) > liquidLampTR[i] || abs(y - trackingObjectPosY[i]) > liquidLampTR[i]) continue;
        //float d = EffectMath::distance(x, y, p1->position_x, p1->position_y);
        float dx =  min((float)fabs(trackingObjectPosX[i] - (float)x), (float)WIDTH + trackingObjectPosX[i] - (float)x); //по идее бесшовный икс
        float dy =  fabs(trackingObjectPosY[i] - (float)y);
        float d = SQRT_VARIANT((dx * dx) + (dy * dy));

        if (d < trackingObjectShift[i]) {
          sum += mapcurve(d, 0, trackingObjectShift[i], 255, liquidLampMX[i], InQuad);
        }
        else if (d < liquidLampSC[i]) {
          sum += mapcurve(d, trackingObjectShift[i], liquidLampSC[i], liquidLampMX[i], 0, OutQuart);
        }
        if (sum >= 255) {
          sum = 255;
          break;
        }
      }
      if (sum < 16) sum = 16;// отрезаем смазанный кусок палитры из-за отсутствия параметра NOBLEND
      CRGB color = ColorFromPalette(myPal, sum); // ,255, NOBLEND
      drawPixelXY(x, y, color);
    }
  }
}

// =====================================
//                Попкорн
// =====================================
//     © Aaron Gotwalt (Soulmate)
// https://editor.soulmatelights.com/gallery/117
//     переосмысление (c) SottNick

/*
  uint8_t NUM_ROCKETS = 10;
  enlargedObjectNUM = (modes[currentMode].Scale - 1U) % 11U / 10.0 * (AVAILABLE_BOID_COUNT - 1U) + 1U;
  typedef struct { int32_t x, y, xd, yd; } Rocket;
  float trackingObjectPosX[trackingOBJECT_MAX_COUNT];
  float trackingObjectPosY[trackingOBJECT_MAX_COUNT];
  float trackingObjectSpeedX[trackingOBJECT_MAX_COUNT];
  float trackingObjectSpeedY[trackingOBJECT_MAX_COUNT];
*/

void popcornRestart_rocket(uint8_t r) {
  //deltaHue = !deltaHue; // "Мальчик" <> "Девочка"
  trackingObjectSpeedX[r] = (float)(random(-(WIDTH * HEIGHT + (WIDTH * 2)), WIDTH * HEIGHT + (WIDTH * 2))) / 256.0; // * (deltaHue ? 1 : -1); // Наклон. "Мальчики" налево, "девочки" направо. :)
  if ((trackingObjectPosX[r] < 0 && trackingObjectSpeedX[r] < 0) || (trackingObjectPosX[r] > (WIDTH - 1) && trackingObjectSpeedX[r] > 0)) { // меняем направление только после выхода за пределы экрана
    // leap towards the centre of the screen
    trackingObjectSpeedX[r] = -trackingObjectSpeedX[r];
  }
  // controls the leap height
  trackingObjectSpeedY[r] = (float)(random8() * 8 + HEIGHT * 10) / 256.0;
  trackingObjectHue[r] = random8();
  trackingObjectPosX[r] = random8(WIDTH);
}

void popcornRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(9U) * 11U + 3U + random8(9U), 5U + random8(67U) * 2U + (random8(4U) ? 0U : 1U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    speedfactor = fmap((float)modes[currentMode].Speed, 1., 255., 0.25, 1.0);
    //speedfactor = (float)modes[currentMode].Speed / 127.0f + 0.001f;

    setCurrentPalette();
    enlargedObjectNUM = (modes[currentMode].Scale - 1U) % 11U / 10.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
    if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;

    for (uint8_t r = 0; r < enlargedObjectNUM; r++) {
      trackingObjectPosX[r] = random8(WIDTH);
      trackingObjectPosY[r] = random8(HEIGHT);
      trackingObjectSpeedX[r] = 0;
      trackingObjectSpeedY[r] = -1;
      trackingObjectHue[r] = random8();
    }
  }
  float popcornGravity = 0.1 * speedfactor;
  fadeToBlackBy(leds, NUM_LEDS, 60);

  for (uint8_t r = 0; r < enlargedObjectNUM; r++) {
    // add the X & Y velocities to the positions
    trackingObjectPosX[r] += trackingObjectSpeedX[r] ;
    if (trackingObjectPosX[r] > WIDTH - 1)
      trackingObjectPosX[r] = trackingObjectPosX[r] - (WIDTH - 1);
    if (trackingObjectPosX[r] < 0)
      trackingObjectPosX[r] = WIDTH - 1 + trackingObjectPosX[r];
    trackingObjectPosY[r] += trackingObjectSpeedY[r] * speedfactor;

    if (trackingObjectPosY[r] > HEIGHT - 1) {
      trackingObjectPosY[r] = HEIGHT + HEIGHT - 2 - trackingObjectPosY[r];
      trackingObjectSpeedY[r] = -trackingObjectSpeedY[r];
    }


    // bounce off the floor?
    if (trackingObjectPosY[r] < 0 && trackingObjectSpeedY[r] < -0.7) { // 0.7 вычислено в экселе. скорость свободного падения ниже этой не падает. если ниже, значит ещё есть ускорение
      trackingObjectSpeedY[r] = (-trackingObjectSpeedY[r]) * 0.9375;//* 240) >> 8;
      trackingObjectPosY[r] = -trackingObjectPosY[r];
    }

    // settled on the floor?
    if (trackingObjectPosY[r] <= -1) popcornRestart_rocket(r);

    // bounce off the sides of the screen?
    // popcornGravity
    trackingObjectSpeedY[r] -= popcornGravity;

    // viscosity
    trackingObjectSpeedX[r] *= 0.875;
    trackingObjectSpeedY[r] *= 0.875;

    //void popcornPaint() {
    // make the acme gray, because why not
    if (-0.004 > trackingObjectSpeedY[r] and trackingObjectSpeedY[r] < 0.004) {
      drawPixelXYF(trackingObjectPosX[r], trackingObjectPosY[r],
                   (modes[currentMode].Speed & 0x01) ? ColorFromPalette(*curPalette, trackingObjectHue[r]) : CRGB::Pink);
    } else {
      drawPixelXYF(trackingObjectPosX[r], trackingObjectPosY[r],
                   (modes[currentMode].Speed & 0x01) ? CRGB::Gray : ColorFromPalette(*curPalette, trackingObjectHue[r]));
    }
  }
}

// =====================================
//              Осциллятор
// =====================================
//             © SottNick
/*
  Реакция Белоусова-Жаботинского
  по наводке https://www.wikiwand.com/ru/%D0%9A%D0%BB%D0%B5%D1%82%D0%BE%D1%87%D0%BD%D1%8B%D0%B9_%D0%B0%D0%B2%D1%82%D0%BE%D0%BC%D0%B0%D1%82
*/

void drawPixelXYFseamless(float x, float y, CRGB color) {
  uint8_t xx = (x - (int)x) * 255, yy = (y - (int)y) * 255, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
#define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)
                  };
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t xn = (int8_t)(x + (i & 1)) % WIDTH;
    uint8_t yn = (int8_t)(y + ((i >> 1) & 1)) % HEIGHT;
    CRGB clr = getPixColorXY(xn, yn);
    clr.r = qadd8(clr.r, (color.r * wu[i]) >> 8);
    clr.g = qadd8(clr.g, (color.g * wu[i]) >> 8);
    clr.b = qadd8(clr.b, (color.b * wu[i]) >> 8);
    drawPixelXY(xn, yn, clr);
  }
}

uint8_t calcNeighbours(uint8_t x, uint8_t y, uint8_t n) {
  return (noise3d[0][(x + 1) % WIDTH][y] == n) +
         (noise3d[0][x][(y + 1) % HEIGHT] == n) +
         (noise3d[0][(x + WIDTH - 1) % WIDTH][y] == n) +
         (noise3d[0][x][(y + HEIGHT - 1) % HEIGHT] == n) +
         (noise3d[0][(x + 1) % WIDTH][(y + 1) % HEIGHT] == n) +
         (noise3d[0][(x + WIDTH - 1) % WIDTH][(y + 1) % HEIGHT] == n) +
         (noise3d[0][(x + WIDTH - 1) % WIDTH][(y + HEIGHT - 1) % HEIGHT] == n) +
         (noise3d[0][(x + 1) % WIDTH][(y + HEIGHT - 1) % HEIGHT] == n);
}

void oscillatingRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(6U); // 4 палитры по 6? (0, 1, 6, 7) + цвет + смена цвета
      if (tmp < 4U) {
        if (tmp > 1U) tmp += 4U;
        tmp = tmp * 6U + 1U;
      }
      else if (tmp == 4U) tmp = 51U + random8(49U);
      else tmp = 100U;
      setModeSettings(tmp, 185U + random8(40U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    step = 0U;
    //setCurrentPalette();
    if (modes[currentMode].Scale > 100U) modes[currentMode].Scale = 100U; // чтобы не было проблем при прошивке без очистки памяти
    if (modes[currentMode].Scale <= 50U) {
      curPalette = palette_arr[(uint8_t)(modes[currentMode].Scale / 50.0F * ((sizeof(palette_arr) / sizeof(TProgmemRGBPalette16 *)) - 0.01F))];
    }
    //случайное заполнение
    for (uint8_t i = 0; i < WIDTH; i++) {
      for (uint8_t j = 0; j < HEIGHT; j++) {
        noise3d[1][i][j] = random8(3);
        noise3d[0][i][j] = noise3d[1][i][j];
      }
    }
  }

  hue++;
  CRGB currColors[3];
  if (modes[currentMode].Scale == 100U) {
    currColors[0U] = CHSV(hue, 255U, 255U);
    currColors[1U] = CHSV(hue, 128U, 255U);
    currColors[2U] = CHSV(hue, 255U, 128U);
  }
  else if (modes[currentMode].Scale > 50U) {
    //uint8_t temp = (modes[currentMode].Scale - 50U) * 1.275;
    currColors[0U] = CHSV((modes[currentMode].Scale - 50U) * 5.1, 255U, 255U);
    currColors[1U] = CHSV((modes[currentMode].Scale - 50U) * 5.1, 128U, 255U);
    currColors[2U] = CHSV((modes[currentMode].Scale - 50U) * 5.1, 255U, 128U);
  } else {
    for (uint8_t c = 0; c < 3; c++) {
      currColors[c] = ColorFromPalette(*curPalette, c * 85U + hue);
    }
  }
  FastLED.clear();

  // расчёт химической реакции и отрисовка мира
  uint16_t colorCount[3] = {0U, 0U, 0U};
  for (uint8_t x = 0; x < WIDTH; x++) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      if (noise3d[0][x][y] == 0U) {
        colorCount[0U]++;
        if (calcNeighbours(x, y, 1U) > 2U) noise3d[1][x][y] = 1U;
      } else if (noise3d[0][x][y] == 1U) {
        colorCount[1U]++;
        if (calcNeighbours(x, y, 2U) > 2U) noise3d[1][x][y] = 2U;
      } else {                //if (noise3d[0][x][y] == 2U){
        colorCount[2U]++;
        if (calcNeighbours(x, y, 0U) > 2U) noise3d[1][x][y] = 0U;
      }
      drawPixelXYFseamless((float)x + 0.5, (float)y + 0.5, currColors[noise3d[1][x][y]]);
    }
  }

  // проверка зацикливания
  if (colorCount[0] == deltaHue && colorCount[1] == deltaHue2 && colorCount[2] == deltaValue) {
    step++;
    if (step > 10U) {
      if (colorCount[0] < colorCount[1]) step = 0;
      else step = 1;

      if (colorCount[2] < colorCount[step]) step = 2;
      colorCount[step] = 0U;
      step = 0U;
    }
  } else {
    step = 0U;
  }

  // вброс хаоса
  if (hue == hue2) { // чтобы не каждый ход
    hue2 += random8(220U) + 36U;
    uint8_t tx = random8(WIDTH);
    deltaHue = noise3d[1][tx][0U] + 1U;
    if (deltaHue > 2U) deltaHue = 0U;
    noise3d[1][tx][0U] = deltaHue;
    noise3d[1][(tx + 1U) % WIDTH][0U] = deltaHue;
    noise3d[1][(tx + 2U) % WIDTH][0U] = deltaHue;
  }

  deltaHue = colorCount[0];
  deltaHue2 = colorCount[1];
  deltaValue = colorCount[2];

  // вброс исчезнувшего цвета
  for (uint8_t c = 0; c < 3; c++) {
    if (colorCount[c] < 6U) {
      uint8_t tx = random8(WIDTH);
      uint8_t ty = random8(HEIGHT);
      if (random8(2U)) {
        noise3d[1][tx][ty] = c;
        noise3d[1][(tx + 1U) % WIDTH][ty] = c;
        noise3d[1][(tx + 2U) % WIDTH][ty] = c;
      } else {
        noise3d[1][tx][ty] = c;
        noise3d[1][tx][(ty + 1U) % HEIGHT] = c;
        noise3d[1][tx][(ty + 2U) % HEIGHT] = c;
      }
    }
  }

  // перенос на следующий цикл
  for (uint8_t x = 0; x < WIDTH; x++) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      noise3d[0][x][y] = noise3d[1][x][y];
    }
  }
}

// =====================================
//              Огонь 2020
// =====================================
//             © SottNick
/*
  сильно по мотивам https://pastebin.com/RG0QGzfK
  Perlin noise fire procedure by Yaroslaw Turbin
  https://www.reddit.com/r/FastLED/comments/hgu16i/my_fire_effect_implementation_based_on_perlin/
  float   trackingObjectPosX[SPARKLES_NUM]; // это для искорок. по идее должны быть uint8_t, но были только такие
  float   trackingObjectPosY[SPARKLES_NUM];
  uint8_t shiftHue[HEIGHT];
  uint16_t ff_y, ff_z; используем для сдвига нойза переменные из общих
  uint8_t deltaValue;
*/

#define SPARKLES_NUM  (WIDTH / 8U) // не более чем  enlargedOBJECT_MAX_COUNT (WIDTH * 2)
void fire2020Routine2() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(100U), 195U + random8(40U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    if (modes[currentMode].Scale > 100U) modes[currentMode].Scale = 100U; // чтобы не было проблем при прошивке без очистки памяти
    deltaValue = modes[currentMode].Scale * 0.0899;// /100.0F * ((sizeof(palette_arr) /sizeof(TProgmemRGBPalette16 *))-0.01F));
    if (deltaValue == 3U || deltaValue == 4U) {
      curPalette =  palette_arr[deltaValue]; // (uint8_t)(modes[currentMode].Scale/100.0F * ((sizeof(palette_arr) /sizeof(TProgmemRGBPalette16 *))-0.01F))];
    } else {
      curPalette = firePalettes[deltaValue]; // (uint8_t)(modes[currentMode].Scale/100.0F * ((sizeof(firePalettes)/sizeof(TProgmemRGBPalette16 *))-0.01F))];
    }
    deltaValue = (((modes[currentMode].Scale - 1U) % 11U + 1U) << 4U) - 8U; // ширина языков пламени (масштаб шума Перлина)
    deltaHue = map(deltaValue, 8U, 168U, 8U, 84U); // высота языков пламени должна уменьшаться не так быстро, как ширина
    step = map(255U - deltaValue, 87U, 247U, 4U, 32U); // вероятность смещения искорки по оси ИКС
    for (uint8_t j = 0; j < HEIGHT; j++) {
      shiftHue[j] = (HEIGHT - 1 - j) * 255 / (HEIGHT - 1); // init colorfade table
    }

    for (uint8_t i = 0; i < SPARKLES_NUM; i++) {
      trackingObjectPosY[i] = random8(HEIGHT);
      trackingObjectPosX[i] = random8(WIDTH);
    }
  }
  for (uint8_t i = 0; i < WIDTH; i++) {
    for (uint8_t j = 0; j < HEIGHT; j++) {
      //if (modes[currentMode].Brightness & 0x01)
      //      leds[XY(i,HEIGHT-1U-j)] = ColorFromPalette(*curPalette, qsub8(inoise8(i * deltaValue, (j+ff_y+random8(2)) * deltaHue, ff_z), shiftHue[j]), 255U);
      //else // немного сгладим картинку
      nblend(leds[XY(i, HEIGHT - 1U - j)], ColorFromPalette(*curPalette, qsub8(inoise8(i * deltaValue, (j + ff_y + random8(2)) * deltaHue, ff_z), shiftHue[j]), 255U), 160U);
    }
  }

  //вставляем искорки из отдельного массива
  for (uint8_t i = 0; i < SPARKLES_NUM; i++) {
    if (trackingObjectPosY[i] > 3U) {
      leds[XY(trackingObjectPosX[i], trackingObjectPosY[i])] = leds[XY(trackingObjectPosX[i], 3U)];
      leds[XY(trackingObjectPosX[i], trackingObjectPosY[i])].fadeToBlackBy( trackingObjectPosY[i] * 2U );
    }
    trackingObjectPosY[i]++;
    if (trackingObjectPosY[i] >= HEIGHT) {
      trackingObjectPosY[i] = random8(4U);
      trackingObjectPosX[i] = random8(WIDTH);
    }
    if (!random8(step))
      trackingObjectPosX[i] = (WIDTH + (uint8_t)trackingObjectPosX[i] + 1U - random8(3U)) % WIDTH;
  }
  ff_y++;
  if (ff_y & 0x01)
    ff_z++;
}

// =====================================
//               Кипение
// =====================================
//             © SottNick
/*
  по мотивам LDIRKO Ленд - эффект номер 10 ...ldir... Yaroslaw Turbin, 18.11.2020
  https://vk.com/ldirko | https://www.reddit.com/user/ldirko/
*/
void LLandRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(6U);
      if (tmp > 1U) tmp += 3U;
      tmp = tmp * 11U + 4U + random8(8U);
      if (tmp > 97U) tmp = 94U;
      setModeSettings(tmp, 200U + random8(46U)); // масштаб 4-11, палитры 0, 1, 5, 6, 7, 8 (кроме 2, 3, 4)
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();
    //speedfactor = fmap(modes[currentMode].Speed, 1., 255., 20., 1.) / 16.;
    deltaValue = 10U * ((modes[currentMode].Scale - 1U) % 11U + 1U);// значения от 1 до 11
    // значения от 0 до 10 = ((modes[currentMode].Scale - 1U) % 11U)

  }
  hue2 += 32U;
  if (hue2 < 32U) hue++;
  ff_y += 16U;

  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      drawPixelXY(x, y, ColorFromPalette (*curPalette, map(inoise8(x * deltaValue, y * deltaValue - ff_y, ff_z) - y * 255 / (HEIGHT - 1), 0, 255, 205, 255) + hue, 255));
    }
  }
  ff_z++;
}

// =====================================
//              Пpитяжeниe
// =====================================
//          Адаптация © SottNick

// https://github.com/pixelmatix/aurora/blob/master/PatternAttract.h
// используются переменные эффекта Стая. Без него работать не будет.

void attractRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(8U);
      if (tmp > 3U) tmp++;
      setModeSettings(tmp * 11U + 3U + random8(9U), 180U + random8(56U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();

    enlargedObjectNUM = (modes[currentMode].Scale - 1U) % 11U + 1U;//(modes[currentMode].Scale - 1U) / 99.0 * (AVAILABLE_BOID_COUNT - 1U) + 1U;

    for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
      boids[i] = Boid(random8(WIDTH), random8(HEIGHT));//WIDTH - 1, HEIGHT - i);
      boids[i].mass = ((float)random8(33U, 134U)) / 100.; // random(0.1, 2); // сюда можно поставить регулятор разлёта. чем меньше число, тем дальше от центра будет вылет
      boids[i].velocity.x = ((float) random8(46U, 100U)) / 500.0;
      if (random8(2U)) {
        boids[i].velocity.x = -boids[i].velocity.x;
      }
      boids[i].velocity.y = 0;
      boids[i].colorIndex = random8();//i * 32;
    }
  }
  dimAll(220);

  PVector attractLocation = PVector(WIDTH * 0.5, HEIGHT * 0.5);
  // перемножаем и получаем 5.

  for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
    Boid boid = boids[i];
    PVector force = attractLocation - boid.location;    // Calculate direction of force // и вкорячиваем сюда регулировку скорости
    float d = force.mag();                              // Distance between objects
    d = constrain(d, 5.0f, HEIGHT * 2.);                // Limiting the distance to eliminate "extreme" results for very close or very far objects
    force.normalize();                                  // Normalize vector (distance doesn't matter here, we just want this vector for direction)
    float strength = (5. * boid.mass) / (d * d);        // Calculate gravitional force magnitude 5.=attractG*attractMass
    force *= strength;                                  // Get force vector --> magnitude * direction

    boid.applyForce(force);
    boid.update();
    drawPixelXYF(boid.location.x, boid.location.y, ColorFromPalette(*curPalette, boid.colorIndex + hue));
    boids[i] = boid;
  }
  EVERY_N_MILLIS(200) {
    hue++;
  }
}

// =====================================
//           Капли на стекле
// =====================================
// https://github.com/DmytroKorniienko/FireLamp_JeeUI/blob/master/src/effects.cpp

void newMatrixRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(30U) ? (random8(40U) ? 2U + random8(99U) : 1U) : 100U, 12U + random8(68U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();

    enlargedObjectNUM = map(modes[currentMode].Speed, 1, 255, 1, trackingOBJECT_MAX_COUNT);
    speedfactor = 0.136f; // фиксируем хорошую скорость

    for (uint8_t i = 0U; i < enlargedObjectNUM; i++) {
      trackingObjectPosX[i] = random8(WIDTH);
      trackingObjectPosY[i] = random8(HEIGHT);
      trackingObjectSpeedY[i] = random8(150, 250) / 100.;
      trackingObjectState[i] = random8(127U, 255U);
    }
    hue = modes[currentMode].Scale * 2.55;
  }
  dimAll(246); // для фиксированной скорости

  CHSV color;

  for (uint8_t i = 0U; i < enlargedObjectNUM; i++) {
    trackingObjectPosY[i] -= trackingObjectSpeedY[i] * speedfactor;

    if (modes[currentMode].Scale == 100U) {
      color = rgb2hsv_approximate(CRGB::Gray);
      color.val = trackingObjectState[i];
    } else if (modes[currentMode].Scale == 1U) {
      color = CHSV(++hue, 255, trackingObjectState[i]);
    } else {
      color = CHSV(hue, 255, trackingObjectState[i]);
    }
    drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], color);

#define GLUK 20 // вероятность горизонтального сдвига капли
    if (random8() < GLUK) {
      //trackingObjectPosX[i] = trackingObjectPosX[i] + random(-1, 2);
      trackingObjectPosX[i] = (uint8_t)(trackingObjectPosX[i] + WIDTH - 1U + random8(3U)) % WIDTH ;
      trackingObjectState[i] = random8(196, 255);
    }

    if (trackingObjectPosY[i] < -1) {
      trackingObjectPosX[i] = random8(WIDTH);
      trackingObjectPosY[i] = random8(HEIGHT - HEIGHT / 2, HEIGHT);
      trackingObjectSpeedY[i] = random8(150, 250) / 100.;
      trackingObjectState[i] = random8(127U, 255U);
    }
  }
}

// =====================================
//             Дымовые шашки
// =====================================
//             © Stepkok

/* https://editor.soulmatelights.com/gallery/505
   https://github.com/DmytroKorniienko/FireLamp_JeeUI/blob/master/src/effects.cpp */

void smokeballsRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(9U) * 11U + 3U + random8(9U), 1U + random8(255U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();

    enlargedObjectNUM = enlargedObjectNUM = (modes[currentMode].Scale - 1U) % 11U + 1U;
    speedfactor = fmap(modes[currentMode].Speed, 1., 255., .02, .1); // попробовал разные способы управления скоростью. Этот максимально приемлемый, хотя и сильно тупой.

    for (byte j = 0; j < enlargedObjectNUM; j++) {
      trackingObjectShift[j] =  random((WIDTH * 10) - ((WIDTH / 3) * 20)); // сумма trackingObjectState + trackingObjectShift не должна выскакивать за макс.Х
      //trackingObjectSpeedX[j] = EffectMath::randomf(5., (float)(16 * WIDTH)); //random(50, 16 * WIDTH) / random(1, 10);
      trackingObjectSpeedX[j] = (float)random(25, 80 * WIDTH) / 5.;
      trackingObjectState[j] = random((WIDTH / 2) * 10, (WIDTH / 3) * 20);
      trackingObjectHue[j] = random8();//(9) * 28;
      trackingObjectPosX[j] = trackingObjectShift[j];
    }
  }

  for (byte x = 0; x < WIDTH; x++) {
    for (float y = (float)HEIGHT; y > 0.; y -= speedfactor) {
      drawPixelXY(x, y, getPixColorXY(x, y - 1));
    }
  }

  //dimAll(240); фиксированное число - очень плохо, когда матрицы разной высоты // fadeToBlackBy(leds, NUM_LEDS, 10);
  fadeToBlackBy(leds, NUM_LEDS, 128U / HEIGHT);
  if (modes[currentMode].Speed & 0x01) {
    blurScreen(20);
  }

  for (byte j = 0; j < enlargedObjectNUM; j++) {
    trackingObjectPosX[j] = beatsin16((uint8_t)(trackingObjectSpeedX[j] * (speedfactor * 5.)), trackingObjectShift[j], trackingObjectState[j] + trackingObjectShift[j], trackingObjectHue[j] * 256, trackingObjectHue[j] * 8);
    drawPixelXYF(trackingObjectPosX[j] / 10., 0.05, ColorFromPalette(*curPalette, trackingObjectHue[j]));
  }

  EVERY_N_SECONDS(20) {
    for (byte j = 0; j < enlargedObjectNUM; j++) {
      trackingObjectShift[j] += random(-20, 20);
      trackingObjectHue[j] += 28;
    }
  }
  loadingFlag = random8() > 253U;
}

// =====================================
//                Nexus
// =====================================
//             © kostyamat
/* https://github.com/DmytroKorniienko/FireLamp_JeeUI/blob/master/src/effects.cpp
  #define enlargedOBJECT_MAX_COUNT            (WIDTH * 2)   // максимальное количество червяков
  uint8_t enlargedObjectNUM;                                // выбранное количество червяков
  float trackingObjectPosX[trackingOBJECT_MAX_COUNT];       // тут будет позиция головы
  float trackingObjectPosY[trackingOBJECT_MAX_COUNT];       // тут будет позиция головы
  float trackingObjectSpeedX[trackingOBJECT_MAX_COUNT];     // тут будет скорость червяка
  uint8_t trackingObjectHue[trackingOBJECT_MAX_COUNT];      // тут будет цвет червяка
  uint8_t trackingObjectState[trackingOBJECT_MAX_COUNT];    // тут будет направление червяка
*/

void nexusReset(uint8_t i) {
  trackingObjectHue[i] = random8();
  trackingObjectState[i] = random8(4);
  trackingObjectSpeedX[i] = (float)random8(5, 11) / 70 + speedfactor; // делаем частицам немного разное ускорение и сразу пересчитываем под общую скорость
  switch (trackingObjectState[i]) {
    case B01:
      trackingObjectPosY[i] = HEIGHT;
      trackingObjectPosX[i] = random8(WIDTH);
      break;
    case B00:
      trackingObjectPosY[i] = -1;
      trackingObjectPosX[i] = random8(WIDTH);
      break;
    case B10:
      trackingObjectPosX[i] = WIDTH;
      trackingObjectPosY[i] = random8(HEIGHT);
      break;
    case B11:
      trackingObjectPosX[i] = -1;
      trackingObjectPosY[i] = random8(HEIGHT);
      break;
  }
}

void nexusRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(2U) ? 11U + random8(15U) : 26U + random8(55U), 1U + random8(161U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    speedfactor = fmap(modes[currentMode].Speed, 1, 255, 0.1, .33);//(float)modes[currentMode].Speed / 555.0f + 0.001f;

    enlargedObjectNUM = (modes[currentMode].Scale - 1U) / 99.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
    if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
    for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
      trackingObjectPosX[i] = random8(WIDTH);
      trackingObjectPosY[i] = random8(HEIGHT);
      trackingObjectSpeedX[i] = (float)random8(5, 11) / 70 + speedfactor; // делаем частицам немного разное ускорение и сразу пересчитываем под общую скорость
      trackingObjectHue[i] = random8();
      trackingObjectState[i] = random8(4);//     B00           // задаем направление
    }
    deltaValue = 255U - map(modes[currentMode].Speed, 1, 255, 11, 33);

  }
  dimAll(deltaValue);

  for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
    switch (trackingObjectState[i]) {
      case B01:
        trackingObjectPosY[i] -= trackingObjectSpeedX[i];
        if (trackingObjectPosY[i] <= -1)
          nexusReset(i);
        break;
      case B00:
        trackingObjectPosY[i] += trackingObjectSpeedX[i];
        if (trackingObjectPosY[i] >= HEIGHT)
          nexusReset(i);
        break;
      case B10:
        trackingObjectPosX[i] -= trackingObjectSpeedX[i];
        if (trackingObjectPosX[i] <= -1)
          nexusReset(i);
        break;
      case B11:
        trackingObjectPosX[i] += trackingObjectSpeedX[i];
        if (trackingObjectPosX[i] >= WIDTH)
          nexusReset(i);
        break;
    }
    drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i],  CHSV(trackingObjectHue[i], 255U, 255));
  }
}

// =====================================
//                Oкeaн
// =====================================
/* "Pacifica" перенос кода kostyamat
  Gentle, blue-green ocean waves.
  December 2019, Mark Kriegsman and Mary Corey March.
  For Dan.
  https://raw.githubusercontent.com/FastLED/FastLED/master/examples/Pacifica/Pacifica.ino
  https://github.com/DmytroKorniienko/FireLamp_JeeUI/blob/master/src/effects.cpp */

static const TProgmemRGBPalette16 pacifica_palette_1 FL_PROGMEM =
{ 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
  0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50
};
static const TProgmemRGBPalette16 pacifica_palette_2 FL_PROGMEM =
{ 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
  0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F
};
static const TProgmemRGBPalette16 pacifica_palette_3 FL_PROGMEM =
{ 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33,
  0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF
};

// Add one layer of waves into the led array
void pacifica_one_layer(CRGB *leds, const TProgmemRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff) {
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for ( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps(CRGB *leds) {
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );

  for ( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if ( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors(CRGB *leds) {
  for ( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145);
    leds[i].green = scale8( leds[i].green, 200);
    leds[i] |= CRGB( 2, 5, 7);
  }
}

void pacificRoutine() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(100U, 1U + random8(255U));
  }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / map(modes[currentMode].Speed, 1, 255, 620, 60);
  uint32_t deltams2 = (deltams * speedfactor2) / map(modes[currentMode].Speed, 1, 255, 620, 60);
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011, 10, 13));
  sCIStart2 -= (deltams21 * beatsin88(777, 8, 11));
  sCIStart3 -= (deltams1 * beatsin88(501, 5, 7));
  sCIStart4 -= (deltams2 * beatsin88(257, 4, 6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer(&*leds, pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0 - beat16( 301) );
  pacifica_one_layer(&*leds, pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer(&*leds, pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10, 38), 0 - beat16(503));
  pacifica_one_layer(&*leds, pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10, 28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps(&*leds);

  // Deepen the blues and greens a bit
  pacifica_deepen_colors(&*leds);
  blurScreen(20);
}

// =====================================
//              Иcтoчник
// =====================================
//             © SottNick
/* по мотивам Эффектов Particle System
  https://github.com/fuse314/arduino-particle-sys
  https://github.com/giladaya/arduino-particle-sys
  https://www.youtube.com/watch?v=S6novCRlHV8&t=51s
  #include <ParticleSys.h>
  при попытке вытащить из этой библиотеки только минимально необходимое выяснилось, что там очередной (третий) вариант реализации субпиксельной графики.
  ну его нафиг. лучше будет повторить визуал имеющимися в прошивке средствами.
*/

void particlesUpdate2(uint8_t i) {
  //age
  trackingObjectState[i]--; //ttl // ещё и сюда надо speedfactor вкорячить. удачи там!

  //apply velocity
  trackingObjectPosX[i] += trackingObjectSpeedX[i];
  trackingObjectPosY[i] += trackingObjectSpeedY[i];
  if (trackingObjectState[i] == 0 || trackingObjectPosX[i] <= -1 || trackingObjectPosX[i] >= WIDTH || trackingObjectPosY[i] <= -1 || trackingObjectPosY[i] >= HEIGHT)
    trackingObjectIsShift[i] = false;
}

/*выглядит как
   https://github.com/fuse314/arduino-particle-sys/blob/master/examples/StarfieldFastLED/StarfieldFastLED.ino */

void starfield2Emit(uint8_t i) {
  if (hue++ & 0x01)
    hue2++;//counter++;
  //source->update(g); хз зачем это было в оригинале - там только смерть source.isAlive высчитывается, вроде

  trackingObjectPosX[i] = WIDTH * 0.5;//CENTER_X_MINOR;// * RENDERER_RESOLUTION; //  particle->x = source->x;
  trackingObjectPosY[i] = HEIGHT * 0.5;//CENTER_Y_MINOR;// * RENDERER_RESOLUTION; //  // particle->y = source->y;

  trackingObjectSpeedX[i] = ((float)random8() - 127.) / 512.; // random(_hVar)-_constVel; // particle->vx
  trackingObjectSpeedY[i] = SQRT_VARIANT(0.0626 - trackingObjectSpeedX[i] * trackingObjectSpeedX[i]); // SQRT_VARIANT(pow(_constVel,2)-pow(trackingObjectSpeedX[i],2)); // particle->vy зависит от particle->vx - не ошибка
  if (random8(2U)) {
    trackingObjectSpeedY[i] = -trackingObjectSpeedY[i];
  }
  trackingObjectState[i] = random8(50, 250); // random8(minLife, maxLife);// particle->ttl
  if (modes[currentMode].Speed & 0x01) trackingObjectHue[i] = hue2;// (counter/2)%255; // particle->hue
  else trackingObjectHue[i] = random8();
  trackingObjectIsShift[i] = true; // particle->isAlive
}

void starfield2Routine() {
  if (loadingFlag)  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(25U + random8(76U), 185U + random8(30U) * 2U + (random8(6U) ? 0U : 1U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    enlargedObjectNUM = (modes[currentMode].Scale - 1U) / 99.0 * (trackingOBJECT_MAX_COUNT - 1U) + 1U;
    if (enlargedObjectNUM > trackingOBJECT_MAX_COUNT) enlargedObjectNUM = trackingOBJECT_MAX_COUNT;
    // deltaValue = 1; // количество зарождающихся частиц за 1 цикл //perCycle = 1;
    deltaValue = enlargedObjectNUM / (SQRT_VARIANT(CENTER_X_MAJOR * CENTER_X_MAJOR + CENTER_Y_MAJOR * CENTER_Y_MAJOR) * 4U) + 1U; // 4 - это потому что за 1 цикл частица пролетает ровно четверть расстояния между 2мя соседними пикселями
    for (int i = 0; i < enlargedObjectNUM; i++)
      trackingObjectIsShift[i] = false; // particle->isAlive
  }
  step = deltaValue; //счётчик количества частиц в очереди на зарождение в этом цикле
  //renderer.fade(leds); = fadeToBlackBy(128); = dimAll(255-128)
  //dimAll(255-128/.25*speedfactor); ахах-ха. очередной эффект, к которому нужно будет "подобрать коэффициенты"
  dimAll(127);

  //go over particles and update matrix cells on the way
  for (int i = 0; i < enlargedObjectNUM; i++) {
    if (!trackingObjectIsShift[i] && step) {
      //emitter->emit(&particles[i], this->g);
      starfield2Emit(i);
      step--;
    }
    if (trackingObjectIsShift[i]) { // particle->isAlive
      //particles[i].update(this->g);
      particlesUpdate2(i);

      //generate RGB values for particle
      CRGB baseRGB = CHSV(trackingObjectHue[i], 255, 255); // particles[i].hue

      //baseRGB.fadeToBlackBy(255-trackingObjectState[i]);
      baseRGB.nscale8(trackingObjectState[i]);//эквивалент
      drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], baseRGB);
    }
  }
}

// =====================================
//                 Фея
// =====================================
//             © SottNick
#define FAIRY_BEHAVIOR        //типа сложное поведение
void fairyEmit(uint8_t i) {   //particlesEmit(Particle_Abstract *particle, ParticleSysConfig *g)

  if (deltaHue++ & 0x01)
    if (hue++ & 0x01) hue2++;
  trackingObjectPosX[i] = boids[0].location.x;
  trackingObjectPosY[i] = boids[0].location.y;

  // хотите навставлять speedfactor? - тут не забудьте
  // trackingObjectSpeedX[i] = ((float)random8()-127.)/512./0.25*speedfactor; // random(_hVar)-_constVel; // particle->vx
  trackingObjectSpeedX[i] = ((float)random8() - 127.) / 512.; // random(_hVar)-_constVel; // particle->vx
  //trackingObjectSpeedY[i] = SQRT_VARIANT((speedfactor*speedfactor+0.0001)-trackingObjectSpeedX[i]*trackingObjectSpeedX[i]); // SQRT_VARIANT(pow(_constVel,2)-pow(trackingObjectSpeedX[i],2)); // particle->vy зависит от particle->vx - не ошибка
  trackingObjectSpeedY[i] = SQRT_VARIANT(0.0626 - trackingObjectSpeedX[i] * trackingObjectSpeedX[i]); // SQRT_VARIANT(pow(_constVel,2)-pow(trackingObjectSpeedX[i],2)); // particle->vy зависит от particle->vx - не ошибка
  if (random8(2U)) {
    trackingObjectSpeedY[i] = -trackingObjectSpeedY[i];
  }

  trackingObjectState[i] = random8(20, 80); // random8(minLife, maxLife);// particle->ttl
  trackingObjectHue[i] = hue2;// (counter/2)%255; // particle->hue
  trackingObjectIsShift[i] = true; // particle->isAlive
}

void fairyRoutine() {
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(14U + random8(87U), 190U + random8(40U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    //speedfactor = (float)modes[currentMode].Speed / 510.0f + 0.001f;

    deltaValue = 10; // количество зарождающихся частиц за 1 цикл //perCycle = 1;
    enlargedObjectNUM = (modes[currentMode].Scale - 1U) / 99.0 * (trackingOBJECT_MAX_COUNT - 1U) + 1U;
    if (enlargedObjectNUM > trackingOBJECT_MAX_COUNT) enlargedObjectNUM = trackingOBJECT_MAX_COUNT;
    for (int i = 0; i < enlargedObjectNUM; i++)
      trackingObjectIsShift[i] = false; // particle->isAlive

    // лень было придумывать алгоритм для траектории феи, поэтому это будет нулевой "бойд" из эффекта Притяжение
    boids[0] = Boid(random8(WIDTH), random8(HEIGHT));//WIDTH - 1, HEIGHT - 1);
    //boids[0].location.x = random8(WIDTH);
    //boids[0].location.y = random8(HEIGHT);
    boids[0].mass = 0.5;//((float)random8(33U, 134U)) / 100.; // random(0.1, 2); // сюда можно поставить регулятор разлёта. чем меньше число, тем дальше от центра будет вылет
    boids[0].velocity.x = ((float) random8(46U, 100U)) / 500.0;
    if (random8(2U)) boids[0].velocity.x = -boids[0].velocity.x;
    boids[0].velocity.y = 0;
    hue = random8();//boids[0].colorIndex =
#ifdef FAIRY_BEHAVIOR
    deltaHue2 = 1U;
#endif;
  }
  step = deltaValue; //счётчик количества частиц в очереди на зарождение в этом цикле

#ifdef FAIRY_BEHAVIOR
  if (!deltaHue && deltaHue2 && fabs(boids[0].velocity.x) + fabs(boids[0].velocity.y) < 0.15) {
    deltaHue2 = 0U;

    boids[1].velocity.x = ((float)random8() + 255.) / 4080.;
    boids[1].velocity.y = ((float)random8() + 255.) / 2040.;
    if (boids[0].location.x > WIDTH * 0.5) boids[1].velocity.x = -boids[1].velocity.x;
    if (boids[0].location.y > HEIGHT * 0.5) boids[1].velocity.y = -boids[1].velocity.y;
  }
  if (!deltaHue2) {
    step = 1U;

    boids[0].location.x += boids[1].velocity.x;
    boids[0].location.y += boids[1].velocity.y;
    deltaHue2 = (boids[0].location.x <= 0 || boids[0].location.x >= WIDTH - 1 || boids[0].location.y <= 0 || boids[0].location.y >= HEIGHT - 1);
  }
  else
#endif // FAIRY_BEHAVIOR
  {
    PVector attractLocation = PVector(WIDTH * 0.5, HEIGHT * 0.5);
    //float attractMass = 10;
    //float attractG = .5;
    // перемножаем и получаем 5.
    Boid boid = boids[0];
    PVector force = attractLocation - boid.location;      // Calculate direction of force
    float d = force.mag();                                // Distance between objects
    d = constrain(d, 5.0f, HEIGHT);//видео снято на 5.0f  // Limiting the distance to eliminate "extreme" results for very close or very far objects
    //d = constrain(d, modes[currentMode].Scale / 10.0, HEIGHT);

    force.normalize();                                    // Normalize vector (distance doesn't matter here, we just want this vector for direction)
    float strength = (5. * boid.mass) / (d * d);          // Calculate gravitional force magnitude 5.=attractG*attractMass
    //float attractMass = (modes[currentMode].Scale) / 10.0 * .5;
    //strength = (attractMass * boid.mass) / (d * d);
    force *= strength;                                    // Get force vector --> magnitude * direction
    boid.applyForce(force);
    boid.update();

    if (boid.location.x <= -1) boid.location.x = -boid.location.x;
    else if (boid.location.x >= WIDTH) boid.location.x = -boid.location.x + WIDTH + WIDTH;
    if (boid.location.y <= -1) boid.location.y = -boid.location.y;
    else if (boid.location.y >= HEIGHT) boid.location.y = -boid.location.y + HEIGHT + HEIGHT;
    boids[0] = boid;

    //EVERY_N_SECONDS(20)
    if (!deltaHue) {
      if (random8(3U)) {
        d = ((random8(2U)) ? boids[0].velocity.x : boids[0].velocity.y) * ((random8(2U)) ? .2 : -.2);
        boids[0].velocity.x += d;
        boids[0].velocity.y -= d;
      } else {
        if (fabs(boids[0].velocity.x) < 0.02)  boids[0].velocity.x = -boids[0].velocity.x;
        else if (fabs(boids[0].velocity.y) < 0.02)  boids[0].velocity.y = -boids[0].velocity.y;
      }
    }
  }


  //renderer.fade(leds); = fadeToBlackBy(128); = dimAll(255-128)
  //dimAll(255-128/.25*speedfactor); очередной эффект, к которому нужно будет "подобрать коэффициенты"
  //if (modes[currentMode].Speed & 0x01)
  dimAll(127);

  //go over particles and update matrix cells on the way
  for (int i = 0; i < enlargedObjectNUM; i++) {
    if (!trackingObjectIsShift[i] && step) {
      //emitter->emit(&particles[i], this->g);
      fairyEmit(i);
      step--;
    }
    if (trackingObjectIsShift[i]) { // particle->isAlive
      //particles[i].update(this->g);
      if (modes[currentMode].Scale & 0x01 && trackingObjectSpeedY[i] > -1) trackingObjectSpeedY[i] -= 0.05; //apply acceleration
      particlesUpdate2(i);

      //generate RGB values for particle
      CRGB baseRGB = CHSV(trackingObjectHue[i], 255, 255); // particles[i].hue

      //baseRGB.fadeToBlackBy(255-trackingObjectState[i]);
      baseRGB.nscale8(trackingObjectState[i]);//эквивалент
      drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], baseRGB);
    }
  }
  drawPixelXYF(boids[0].location.x, boids[0].location.y, CHSV(hue, 160U, 255U));//boid.colorIndex + hue
}

// =====================================
//            Цветные драже
// =====================================
//             © SottNick
/* по мотивам визуала эффекта by Yaroslaw Turbin 14.12.2020
  https://vk.com/ldirko программный код которого он запретил брать */

void sandRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(100U) , 140U + random8(100U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    pcnt = 0U;// = HEIGHT;
  }

  // если насыпалось уже достаточно, бахаем рандомные песчинки
  uint8_t temp = map8(random8(), modes[currentMode].Scale * 2.55, 255U);
  if (pcnt >= map8(temp, 2U, HEIGHT - 3U)) {
    //temp = 255U - temp + 2;
    //if (temp < 2) temp = 255;
    temp = HEIGHT + 1U - pcnt;
    if (!random8(4U)) {             // иногда песка осыпается до половины разом
      if (random8(2U)) temp = 2U;
      else temp = 3U;
    }
    for (uint8_t y = 0; y < pcnt; y++)
      for (uint8_t x = 0; x < WIDTH; x++)
        if (!random8(temp))
          leds[XY(x, y)] = 0;
  }

  pcnt = 0U;
  // осыпаем всё, что есть на экране
  for (uint8_t y = 1; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      if (leds[XY(x, y)])                                                          // проверяем для каждой песчинки
        if (!leds[XY(x, y - 1)]) {                                                 // если под нами пусто, просто падаем
          leds[XY(x, y - 1)] = leds[XY(x, y)];
          leds[XY(x, y)] = 0;
        }
        else if (x > 0U && !leds[XY(x - 1, y - 1)] && x < WIDTH - 1 && !leds[XY(x + 1, y - 1)]) { // если под нами пик
          if (random8(2U)) leds[XY(x - 1, y - 1)] = leds[XY(x, y)];
          else leds[XY(x - 1, y - 1)] = leds[XY(x, y)];

          leds[XY(x, y)] = 0;
          pcnt = y - 1;
        }
        else if (x > 0U && !leds[XY(x - 1, y - 1)]) {                              // если под нами склон налево
          leds[XY(x - 1, y - 1)] = leds[XY(x, y)];
          leds[XY(x, y)] = 0;
          pcnt = y - 1;
        }
        else if (x < WIDTH - 1 && !leds[XY(x + 1, y - 1)]) {                       // если под нами склон направо
          leds[XY(x + 1, y - 1)] = leds[XY(x, y)];
          leds[XY(x, y)] = 0;
          pcnt = y - 1;
        } else {                                                                       // если под нами плато
          pcnt = y;
        }
    }
  }
  // эмиттер новых песчинок
  if (!leds[XY(CENTER_X_MINOR, HEIGHT - 2)] && !leds[XY(CENTER_X_MAJOR, HEIGHT - 2)] && !random8(3)) {
    temp = random8(2) ? CENTER_X_MINOR : CENTER_X_MAJOR;
    leds[XY(temp, HEIGHT - 1)] = CHSV(random8(), 255U, 255U);
  }
}

// =====================================
//           Плазменная лампа
//             © stepko
//  выбор палитры и багфикс © SottNick
// =====================================
void spiderRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(5U);
      if (tmp > 1U) tmp += 3U;
      setModeSettings(tmp * 11U + 3U + random8(7U), 1U + random8(180U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();
    pcnt = (modes[currentMode].Scale - 1U) % 11U + 1U; // количество линий от 1 до 11 для каждой из 9 палитр
    speedfactor = fmap(modes[currentMode].Speed, 1, 255, 20., 2.);
  }
  if (hue2++ & 0x01 && deltaHue++ & 0x01 && deltaHue2++ & 0x01) hue++; // хз. как с 60ю кадрами в секунду скорость замедлять...
  dimAll(205);
  float time_shift = millis() & 0x7FFFFF;             // overflow protection proper by SottNick
  time_shift /= speedfactor;
  for (uint8_t c = 0; c < pcnt; c++) {
    float xx = 2. + sin8(time_shift + 6000 * c) / 12.;
    float yy = 2. + cos8(time_shift + 9000 * c) / 12.;
    DrawLineF(xx, yy, (float)WIDTH - xx - 1, (float)HEIGHT - yy - 1, ColorFromPalette(*curPalette, hue + c * (255 / pcnt)));
  }
}

// =====================================
//            Северное Сияние
//        © kostyamat 05.02.2021
// =====================================
/* идеи подсмотрены тут https://www.reddit.com/r/FastLED/comments/jyly1e/challenge_fastled_sketch_that_fits_entirely_in_a/
  особая благодарность https://www.reddit.com/user/ldirko/ Yaroslaw Turbin aka ldirko
  вместо набора палитр в оригинальном эффекте сделан генератор палитр
  CRGBPalette16 myPal; уже есть эта переменная в эффекте Жидкая лампа
*/

// генератор палитр для Северного сияния (c) SottNick
static const uint8_t MBAuroraColors_arr[5][4] PROGMEM = // палитра в формате CHSV
{ //№, цвет, насыщенность, яркость
  {0  , 0 , 255,   0},    // black
  {80 , 0 , 255, 255},
  {130, 25, 220, 255},
  {180, 25, 185, 255},
  {255, 25, 155, 255}     //245
};
#define AURORA_COLOR_RANGE 10     // (+/-10 единиц оттенка) диапазон, в котором плавает цвет сияния относительно выбранного оттенка 
#define AURORA_COLOR_PERIOD 2     // (2 раза в минуту) частота, с которой происходит колебание выбранного оттенка в разрешённом диапазоне

void fillMyPal16_2(uint8_t hue, bool isInvert = false) {
  // я бы, конечно, вместо копии функции генерации палитры "_2"
  // лучше бы сделал её параметром указатель на массив с базовой палитрой,
  // но я пониятия не имею, как это делается с грёбаным PROGMEM

  int8_t lastSlotUsed = -1;
  uint8_t istart8, iend8;
  CRGB rgbstart, rgbend;

  // начинаем с нуля
  if (isInvert) {
    //с неявным преобразованием оттенков цвета получаются, как в фотошопе, но для данного эффекта не красиво выглядят
    //rgbstart = CHSV(256 + hue - pgm_read_byte(&MBAuroraColors_arr[0][1]), pgm_read_byte(&MBAuroraColors_arr[0][2]), pgm_read_byte(&MBAuroraColors_arr[0][3])); // начальная строчка палитры с инверсией
    hsv2rgb_spectrum(CHSV(256 + hue - pgm_read_byte(&MBAuroraColors_arr[0][1]), pgm_read_byte(&MBAuroraColors_arr[0][2]), pgm_read_byte(&MBAuroraColors_arr[0][3])), rgbstart);
  } else {
    //rgbstart = CHSV(hue + pgm_read_byte(&MBAuroraColors_arr[0][1]), pgm_read_byte(&MBAuroraColors_arr[0][2]), pgm_read_byte(&MBAuroraColors_arr[0][3])); // начальная строчка палитры
    hsv2rgb_spectrum(CHSV(hue + pgm_read_byte(&MBAuroraColors_arr[0][1]), pgm_read_byte(&MBAuroraColors_arr[0][2]), pgm_read_byte(&MBAuroraColors_arr[0][3])), rgbstart);
  }
  int indexstart = 0;                           // начальный индекс палитры
  for (uint8_t i = 1U; i < 5U; i++) {           // в палитре @obliterator всего 5 строчек
    int indexend = pgm_read_byte(&MBAuroraColors_arr[i][0]);
    if (isInvert) {
      hsv2rgb_spectrum(CHSV(hue + pgm_read_byte(&MBAuroraColors_arr[i][1]), pgm_read_byte(&MBAuroraColors_arr[i][2]), pgm_read_byte(&MBAuroraColors_arr[i][3])), rgbend);
    } else {
      hsv2rgb_spectrum(CHSV(256 + hue - pgm_read_byte(&MBAuroraColors_arr[i][1]), pgm_read_byte(&MBAuroraColors_arr[i][2]), pgm_read_byte(&MBAuroraColors_arr[i][3])), rgbend);
    }
    istart8 = indexstart / 16;
    iend8   = indexend   / 16;
    if ((istart8 <= lastSlotUsed) && (lastSlotUsed < 15)) {
      istart8 = lastSlotUsed + 1;
      if (iend8 < istart8) iend8 = istart8;
    }
    lastSlotUsed = iend8;
    fill_gradient_RGB( myPal, istart8, rgbstart, iend8, rgbend);
    indexstart = indexend;
    rgbstart = rgbend;
  }
}

unsigned long polarTimer;
/*float adjastHeight;   // используем emitterX
  uint16_t adjScale;      // используем ff_y*/

void polarRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(3U) ? 1U + random8(99U) : 100U, 1U + random8(170U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    emitterX = 400. / HEIGHT; // а это - максимум без яркой засветки крайних рядов матрицы (сверху и снизу)
    ff_y = map(WIDTH, 8, 64, 310, 63);
    ff_z = ff_y;
    speedfactor = map(modes[currentMode].Speed, 1, 255, 128, 16); // _speed = map(speed, 1, 255, 128, 16);
  }

  if (modes[currentMode].Scale == 100) {
    if (hue2++ & 0x01 && deltaHue++ & 0x01 && deltaHue2++ & 0x01) hue++; // это ж бред, но я хз. как с 60ю кадрами в секунду можно эффективно скорость замедлять...
    fillMyPal16_2((uint8_t)((modes[currentMode].Scale - 1U) * 2.55) + hue, modes[currentMode].Scale & 0x01);
  } else {
    fillMyPal16_2((uint8_t)((modes[currentMode].Scale - 1U) * 2.55) + AURORA_COLOR_RANGE - beatsin8(AURORA_COLOR_PERIOD, 0U, AURORA_COLOR_RANGE + AURORA_COLOR_RANGE), modes[currentMode].Scale & 0x01);
  }

  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) {
      polarTimer++;
      //uint16_t i = x*y;
      leds[XY(x, y)] =
        ColorFromPalette(myPal,
                         qsub8(
                           inoise8(polarTimer % 2 + x * ff_z,
                                   y * 16 + polarTimer % 16,
                                   polarTimer / speedfactor
                                  ),
                           fabs((float)HEIGHT / 2 - (float)y) * emitterX
                         )
                        );
    }
  }
}

// =====================================
//                 Шары
//   © stepko and kostyamat 07.02.2021
// =====================================

float randomf(float min, float max) {
  return fmap((float)random16(4095), 0.0, 4095.0, min, max);
}

void ballsfill_circle(float cx, float cy, float radius, CRGB col) {
  radius -= 0.5;
  for (int y = -radius; y <= radius; y++) {
    for (int x = -radius; x <= radius; x++) {
      if (x * x + y * y <= radius * radius)
        drawPixelXYF(cx + x, cy + y, col);
    }
  }
}

void spheresRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(8U) * 11U + 6U + random8(6U), 1U + random8(255U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();
    speedfactor = fmap(modes[currentMode].Speed, 1, 255, 0.15, 0.5);

    enlargedObjectNUM = (modes[currentMode].Scale - 1U) % 11U + 1U;
    //if (enlargedObjectNUM > AVAILABLE_BOID_COUNT) enlargedObjectNUM = AVAILABLE_BOID_COUNT;
    emitterY = .5 + HEIGHT / 4. / (2. - 1. / enlargedObjectNUM); // radiusMax

    for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
      trackingObjectShift[i] = randomf(0.5, emitterY); // radius[i] = randomf(0.5, radiusMax);
      trackingObjectSpeedX[i] = randomf(0.5, 1.1) * speedfactor; // ball[i][2] =
      trackingObjectSpeedY[i] = randomf(0.5, 1.1) * speedfactor; // ball[i][3] =
      trackingObjectPosX[i] = random8(WIDTH);  // ball[i][0] = random(0, WIDTH);
      trackingObjectPosY[i] = random8(HEIGHT); // ball[i][1] = random(0, HEIGHT);
      trackingObjectHue[i] = random8();        // color[i] = random(0, 255);
    }
  }

  dimAll(255 - map(modes[currentMode].Speed, 1, 255, 5, 20)); //fadeToBlackBy(leds, NUM_LEDS, map(speed, 1, 255, 5, 20));

  for (byte i = 0; i < enlargedObjectNUM; i++) {
    if (trackingObjectIsShift[i]) {  // тут у нас шарики надуваются\сдуваются по ходу движения
      trackingObjectShift[i] += (fabs(trackingObjectSpeedX[i]) > fabs(trackingObjectSpeedY[i]) ? fabs(trackingObjectSpeedX[i]) : fabs(trackingObjectSpeedY[i])) * 0.1 * speedfactor;
      if (trackingObjectShift[i] >= emitterY) {
        trackingObjectIsShift[i] = false;
      }
    } else {
      trackingObjectShift[i] -= (fabs(trackingObjectSpeedX[i]) > fabs(trackingObjectSpeedY[i]) ? fabs(trackingObjectSpeedX[i]) : fabs(trackingObjectSpeedY[i])) * 0.1 * speedfactor;
      if (trackingObjectShift[i] < 1.) {
        trackingObjectIsShift[i] = true;
        trackingObjectHue[i] = random(0, 255);
      }
    }

    if (trackingObjectShift[i] > 1) {
      ballsfill_circle(trackingObjectPosY[i], trackingObjectPosX[i], trackingObjectShift[i], ColorFromPalette(*curPalette, trackingObjectHue[i]));
    } else {
      drawPixelXYF(trackingObjectPosY[i], trackingObjectPosX[i], ColorFromPalette(*curPalette, trackingObjectHue[i]));
    }

    if (trackingObjectPosX[i] + trackingObjectShift[i] >= HEIGHT - 1)
      trackingObjectPosX[i] += (trackingObjectSpeedX[i] * ((HEIGHT - 1 - trackingObjectPosX[i]) / trackingObjectShift[i] + 0.005));
    else if (trackingObjectPosX[i] - trackingObjectShift[i] <= 0)
      trackingObjectPosX[i] += (trackingObjectSpeedX[i] * (trackingObjectPosX[i] / trackingObjectShift[i] + 0.005));
    else
      trackingObjectPosX[i] += trackingObjectSpeedX[i];
    //-----------------------
    if (trackingObjectPosY[i] + trackingObjectShift[i] >= WIDTH - 1)
      trackingObjectPosY[i] += (trackingObjectSpeedY[i] * ((WIDTH - 1 - trackingObjectPosY[i]) / trackingObjectShift[i] + 0.005));
    else if (trackingObjectPosY[i] - trackingObjectShift[i] <= 0)
      trackingObjectPosY[i] += (trackingObjectSpeedY[i] * (trackingObjectPosY[i] / trackingObjectShift[i] + 0.005));
    else
      trackingObjectPosY[i] += trackingObjectSpeedY[i];
    //------------------------
    if (trackingObjectPosX[i] < 0.01) {
      trackingObjectSpeedX[i] = randomf(0.5, 1.1) * speedfactor;
      trackingObjectPosX[i] = 0.01;
    }
    else if (trackingObjectPosX[i] > HEIGHT - 1.01) {
      trackingObjectSpeedX[i] = randomf(0.5, 1.1) * speedfactor;
      trackingObjectSpeedX[i] = -trackingObjectSpeedX[i];
      trackingObjectPosX[i] = HEIGHT - 1.01;
    }
    //----------------------
    if (trackingObjectPosY[i] < 0.01) {
      trackingObjectSpeedY[i] = randomf(0.5, 1.1) * speedfactor;
      trackingObjectPosY[i] = 0.01;
    }
    else if (trackingObjectPosY[i] > WIDTH - 1.01) {
      trackingObjectSpeedY[i] = randomf(0.5, 1.1) * speedfactor;
      trackingObjectSpeedY[i] = -trackingObjectSpeedY[i];
      trackingObjectPosY[i] = WIDTH - 1.01;
    }
  }
  blurScreen(48);
}

// =====================================
//                Магма
//             © SottNick
// =====================================
void magmaRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //палитры 0,1,5,6,7
      uint8_t tmp = random8(6U);
      if (tmp > 1U) tmp += 3U;
      setModeSettings(tmp * 11U + 2U + random8(7U) , 185U + random8(48U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    //setCurrentPalette();
    deltaValue = modes[currentMode].Scale * 0.0899;// /100.0F * ((sizeof(palette_arr) /sizeof(TProgmemRGBPalette16 *))-0.01F));
    if (deltaValue == 3U || deltaValue == 4U)
      curPalette =  palette_arr[deltaValue]; // (uint8_t)(modes[currentMode].Scale/100.0F * ((sizeof(palette_arr) /sizeof(TProgmemRGBPalette16 *))-0.01F))];
    else
      curPalette = firePalettes[deltaValue]; // (uint8_t)(modes[currentMode].Scale/100.0F * ((sizeof(firePalettes)/sizeof(TProgmemRGBPalette16 *))-0.01F))];
    //deltaValue = (((modes[currentMode].Scale - 1U) % 11U + 1U) << 4U) - 8U; // ширина языков пламени (масштаб шума Перлина)
    deltaValue = 12U;
    deltaHue = 10U;// map(deltaValue, 8U, 168U, 8U, 84U); // высота языков пламени должна уменьшаться не так быстро, как ширина
    //step = map(255U-deltaValue, 87U, 247U, 4U, 32U); // вероятность смещения искорки по оси ИКС
    for (uint8_t j = 0; j < HEIGHT; j++) {
      shiftHue[j] = (HEIGHT - 1 - j) * 255 / (HEIGHT - 1); // init colorfade table
    }

    enlargedObjectNUM = (modes[currentMode].Scale - 1U) % 11U / 10.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
    if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
    //if (enlargedObjectNUM < 2U) enlargedObjectNUM = 2U;

    for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {
      trackingObjectPosX[i] = random8(WIDTH);
      trackingObjectPosY[i] = random8(HEIGHT);
      trackingObjectHue[i] = 50U; random8();
    }
  }

  dimAll(181);

  for (uint8_t i = 0; i < WIDTH; i++) {
    for (uint8_t j = 0; j < HEIGHT; j++) {
      //leds[XY(i,HEIGHT-1U-j)] = ColorFromPalette(*curPalette, qsub8(inoise8(i * deltaValue, (j+ff_y+random8(2)) * deltaHue, ff_z), shiftHue[j]), 255U);
      drawPixelXYF(i, HEIGHT - 1U - j, ColorFromPalette(*curPalette, qsub8(inoise8(i * deltaValue, (j + ff_y + random8(2)) * deltaHue, ff_z), shiftHue[j]), 255U));
    }
  }

  for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
    LeapersMove_leaper(i);
    if (trackingObjectPosY[i] >= HEIGHT / 4U)
      drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], ColorFromPalette(*curPalette, trackingObjectHue[i]));
  };

  //blurScreen(20);
  ff_y++;
  if (ff_y & 0x01)
    ff_z++;
}

// =====================================
//             Огонь 2021
//             © SottNick
// =====================================
/*На основе алгоритма https://editor.soulmatelights.com/gallery/546-fire
  by Stepko */

#define FIXED_SCALE_FOR_Y 4U      // менять нельзя. корректировка скорости ff_x =... подогнана под него

void Fire2021Routine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = 1U + random8(89U); // пропускаем белую палитру
      if (tmp > 44U) tmp += 11U;
      setModeSettings(tmp, 42U + random8(155U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    if (modes[currentMode].Scale > 100U) modes[currentMode].Scale = 100U; // чтобы не было проблем при прошивке без очистки памяти
    deltaValue = modes[currentMode].Scale * 0.0899;// /100.0F * ((sizeof(palette_arr) /sizeof(TProgmemRGBPalette16 *))-0.01F));
    if (deltaValue == 3U || deltaValue == 4U) {
      curPalette =  palette_arr[deltaValue]; // (uint8_t)(modes[currentMode].Scale/100.0F * ((sizeof(palette_arr) /sizeof(TProgmemRGBPalette16 *))-0.01F))];
    } else {
      curPalette = firePalettes[deltaValue]; // (uint8_t)(modes[currentMode].Scale/100.0F * ((sizeof(firePalettes)/sizeof(TProgmemRGBPalette16 *))-0.01F))];
    }
    deltaValue = (modes[currentMode].Scale - 1U) % 11U + 1U;
    if (modes[currentMode].Speed & 0x01) {
      ff_x = modes[currentMode].Speed;
      deltaHue2 = FIXED_SCALE_FOR_Y;
    } else {
      if (deltaValue > FIXED_SCALE_FOR_Y) {
        speedfactor = .4 * (deltaValue - FIXED_SCALE_FOR_Y) + FIXED_SCALE_FOR_Y;
      } else {
        speedfactor = deltaValue;
      }
      ff_x = round(modes[currentMode].Speed * 64. / (0.1686 * speedfactor * speedfactor * speedfactor - 1.162 * speedfactor * speedfactor + 3.6694 * speedfactor + 56.394)); // Ааааа! это тупо подбор коррекции. очень приблизитеьный
      deltaHue2 = deltaValue;
    }
    if (ff_x > 255U) ff_x = 255U;
    if (ff_x == 0U) ff_x = 1U;
    step = map(ff_x * ff_x, 1U, 65025U, (deltaHue2 - 1U) / 2U + 1U, deltaHue2 * 18U + 44);
    pcnt = map(step, 1U, 255U, 20U, 128U); // nblend 3th param
    deltaValue = 0.7 * deltaValue * deltaValue + 31.3; // ширина языков пламени (масштаб шума Перлина)
    deltaHue2 = 0.7 * deltaHue2 * deltaHue2 + 31.3; // высота языков пламени (масштаб шума Перлина)
  }

  ff_y += step; //static uint32_t t += speed;
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) {
      int16_t Bri = inoise8(x * deltaValue, (y * deltaHue2) - ff_y, ff_z) - (y * (255 / HEIGHT));
      byte Col = Bri;//inoise8(x * deltaValue, (y * deltaValue) - ff_y, ff_z) - (y * (255 / HEIGHT));
      if (Bri < 0)
        Bri = 0;
      if (Bri != 0)
        Bri = 256 - (Bri * 0.2);
      nblend(leds[XY(x, y)], ColorFromPalette(*curPalette, Col, Bri), pcnt);
    }
  }
  if (!random8()) ff_z++;
}

// =====================================
//               Люмeньep
//              © SottNick
// =====================================
#define DIMSPEED (254U - floor(550U / WIDTH / HEIGHT + 1.5))

void lumenjerRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(17U); //= random8(19U);
      if (tmp > 2U) tmp += 2U;
      tmp = (uint8_t)(tmp * 5.556 + 3.);
      if (tmp > 100U) tmp = 100U;
      setModeSettings(tmp, 190U + random8(56U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    if (modes[currentMode].Scale > 100) modes[currentMode].Scale = 100; // чтобы не было проблем при прошивке без очистки памяти
    if (modes[currentMode].Scale > 50) {
      curPalette = firePalettes[(uint8_t)((modes[currentMode].Scale - 50) / 50.0F * ((sizeof(firePalettes) / sizeof(TProgmemRGBPalette16 *)) - 0.01F))];
    } else {
      curPalette = palette_arr[(uint8_t)(modes[currentMode].Scale / 50.0F * ((sizeof(palette_arr) / sizeof(TProgmemRGBPalette16 *)) - 0.01F))];
    }
    deltaHue = -1;
    deltaHue2 = -1;
    FastLED.clear();
  }
  dimAll(DIMSPEED);

  deltaHue = random8(3) ? deltaHue : -deltaHue;
  deltaHue2 = random8(3) ? deltaHue2 : -deltaHue2;
#if (WIDTH % 2 == 0 && HEIGHT % 2 == 0)
  hue = (WIDTH + hue + (int8_t)deltaHue * (bool)random8(64)) % WIDTH;
#else
  hue = (WIDTH + hue + (int8_t)deltaHue) % WIDTH;
#endif
  hue2 = (HEIGHT + hue2 + (int8_t)deltaHue2) % HEIGHT;

  if (modes[currentMode].Scale == 100U) {
    // leds[XY(hue, hue2)] += CHSV(random8(), 255U, 255U);
    step += 2;
    drawPixelXY(hue, hue2, CHSV(step, 255U, 255 - step / 3));
    drawPixelXY(WIDTH - hue, HEIGHT - hue2, CHSV(step + 128, 255U, 170 + step / 3));
  } else {
    drawPixelXY(hue, hue2, ColorFromPalette(*curPalette, step++));
    drawPixelXY(WIDTH - hue, HEIGHT - hue2, ColorFromPalette(*curPalette, step + 64));
  }
}

// =====================================
//            Цветные кудри
//           Color Frizzles
//             © Stepko
//       адаптация © SlingMaster
// =====================================
void ColorFrizzles() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random(10U, 90U), 128);
    }
    loadingFlag = false;
    FPSdelay = 10U;
    deltaValue = 0;
#endif
  }

  if (modes[currentMode].Scale > 50) {
    if (FPSdelay > 48) deltaValue = 0;
    if (FPSdelay < 5) deltaValue = 1;

    if (deltaValue == 1) {
      FPSdelay++;
    } else {
      FPSdelay--;
    }
    blur2d(leds, WIDTH, HEIGHT, 16);
  } else {
    FPSdelay = 20;
    dimAll(240U);
  }
  // LOG.printf_P(PSTR("| deltaValue • %03d | fps %03d\n"), deltaValue, FPSdelay);
  for (byte i = 8; i--;) {
    leds[XY(beatsin8(12 + i, 0, WIDTH - 1), beatsin8(15 - i, 0, HEIGHT - 1))] = CHSV(beatsin8(12, 0, 255), 255, (255 - FPSdelay * 2));
  }
}

// =====================================
//              RadialWave
//            Радіальна хвиля
//               © Stepko
// =====================================
/* --------------------------------- */
void RadialWave() {
  static uint32_t t;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(50U, random(25U, 255U));
    }
#endif

    loadingFlag = false;
    FastLED.clear();
    for (int8_t x = -CENTER_X_MAJOR; x < CENTER_X_MAJOR; x++) {
      for (int8_t y = -CENTER_Y_MAJOR; y < CENTER_Y_MAJOR; y++) {
        noise3d[0][x + CENTER_X_MAJOR][y + CENTER_Y_MAJOR] = (atan2(x, y) / PI) * 128 + 127;   // thanks ldirko
        noise3d[1][x + CENTER_X_MAJOR][y + CENTER_Y_MAJOR] = hypot(x, y);                      // thanks Sutaburosu
      }
    }
  }

  t++;
  for (uint8_t x = 0; x < WIDTH; x++) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      byte angle = noise3d[0][x][y];
      byte radius = noise3d[1][x][y];
      leds[XY(x, y)] = CHSV(t + radius * (255 / WIDTH), 255, sin8(t * 4 + sin8(t * 4 - radius * (255 / WIDTH)) + angle * 3));
    }
  }
}

// ============  FireSparks =============
//               © Stepko
//    updated with Sparks © kostyamat
//             EFF_FIRE_SPARK
//            Fire with Sparks
//---------------------------------------
uint16_t RGBweight(uint16_t idx) {
  return (leds[idx].r + leds[idx].g + leds[idx].b);
}
class Spark {
  private:
    CRGB color;
    uint8_t Bri;
    uint8_t Hue;
    float x, y, speedy = (float)random(5, 30) / 10;

  public:
    void addXY(float nx, float ny) {
      //drawPixelXYF(x, y, 0);
      x += nx;
      y += ny * speedy;
    }

    float getY() {
      return y;
    }

    void reset() {
      uint32_t peak = 0;
      speedy = (float)random(5, 30) / 10;
      y = random(HEIGHT / 4, HEIGHT / 2);
      for (uint8_t i = 0; i < WIDTH; i++) {
        uint32_t temp = RGBweight(XY(i, y));
        if (temp > peak) {
          x = i;
          peak = temp;
        }
      }

      color = leds[XY(x, y)];
    }

    void draw() {
      color.fadeLightBy(256 / (HEIGHT * 0.75));
      drawPixelXYF(x, y, color);
    }
};

const byte sparksCount = WIDTH / 4;
Spark sparks[sparksCount];

//---------------------------------------
void  FireSparks() {
  bool withSparks = false; // true/false
  static uint32_t t;
  const uint8_t spacer = HEIGHT / 4;
  byte scale = 50;

  if (loadingFlag) {

#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random(0U, 99U), random(20U, 100U));
    }
#endif
    loadingFlag = false;
    FPSdelay = DYNAMIC;
    for (byte i = 0; i < sparksCount; i++) sparks[i].reset();
  }
  withSparks = modes[currentMode].Scale >= 50;
  t += modes[currentMode].Speed;

  if (withSparks)
    for (byte i = 0; i < sparksCount; i++) {
      sparks[i].addXY((float)random(-1, 2) / 2, 0.75);
      if (sparks[i].getY() > HEIGHT and !random(0, 50)) sparks[i].reset();
      else sparks[i].draw();
    }

  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) {
      int16_t Bri = inoise8(x * scale, (y * scale) - t) - ((withSparks ? y + spacer : y) * (255 / HEIGHT));
      byte Col = Bri;
      if (Bri < 0) Bri = 0; if (Bri != 0) Bri = 256 - (Bri * 0.2);
      nblend(leds[XY(x, y)], ColorFromPalette(HeatColors_p, Col, Bri), modes[currentMode].Speed);
    }
  }
}

// =====================================
//               DropInWater
//                © Stepko
//        Adaptation © SlingMaster
// =====================================
CRGBPalette16 currentPalette(PartyColors_p);
void DropInWater() {
#define Sat (255)
#define MaxRad WIDTH + HEIGHT
  static int rad[(HEIGHT + WIDTH) / 8];
  static byte posx[(HEIGHT + WIDTH) / 8], posy[(HEIGHT + WIDTH) / 8];

  if (loadingFlag) {

#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random(0U, 100U), random(160U, 215U));
    }
#endif
    loadingFlag = false;
    hue = modes[currentMode].Scale * 2.55;
    for (int i = 0; i < ((HEIGHT + WIDTH) / 8) - 1; i++)  {
      posx[i] = random(WIDTH - 1);
      posy[i] = random(HEIGHT - 1);
      rad[i] = random(-1, MaxRad);
    }
  }
  fill_solid( currentPalette, 16, CHSV(hue, Sat, 230));
  currentPalette[10] = CHSV(hue, Sat - 60, 255);
  currentPalette[9] = CHSV(hue, 255 - Sat, 210);
  currentPalette[8] = CHSV(hue, 255 - Sat, 210);
  currentPalette[7] = CHSV(hue, Sat - 60, 255);
  fillAll(ColorFromPalette(currentPalette, 1));

  for (uint8_t i = ((HEIGHT + WIDTH) / 8 - 1); i > 0 ; i--) {
    drawCircle(posx[i], posy[i], rad[i], ColorFromPalette(currentPalette, (256 / 16) * 8.5 - rad[i]));
    drawCircle(posx[i], posy[i], rad[i] - 1, ColorFromPalette(currentPalette, (256 / 16) * 7.5 - rad[i]));
    if (rad[i] >= MaxRad) {
      rad[i] = 0; // random(-1, MaxRad);
      posx[i] = random(WIDTH);
      posy[i] = random(HEIGHT);
    } else {
      rad[i]++;
    }
  }
  if (modes[currentMode].Scale == 100) {
    hue++;
  }
  blur2d(leds, WIDTH, HEIGHT, 64);
}

// =====================================
//            Flower Ruta
//    © Stepko and © Sutaburosu
//     Adaptation © SlingMaster
//             22/05/22
// =====================================
/* --------------------------------- */
void FlowerRuta() {
  static uint8_t PETALS;
  static uint32_t t;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random8(1U, 255U), random8(150U, 255U));
    }
#endif
    loadingFlag = false;
    PETALS = map(modes[currentMode].Scale, 1, 100, 2U, 5U);
    LOG.printf_P(PSTR("Scale: %03d | PETALS : %02d | Speed %03d\n"), modes[currentMode].Scale, PETALS, modes[currentMode].Speed);
    FastLED.clear();
    for (int8_t x = -CENTER_X_MAJOR; x < CENTER_X_MAJOR; x++) {
      for (int8_t y = -CENTER_Y_MAJOR; y < CENTER_Y_MAJOR; y++) {
        noise3d[0][x + CENTER_X_MAJOR][y + CENTER_Y_MAJOR] = (atan2(x, y) / PI) * 128 + 127; // thanks ldirko
        noise3d[1][x + CENTER_X_MAJOR][y + CENTER_Y_MAJOR] = hypot(x, y);                    // thanks Sutaburosu
      }
    }
  }

  t++;
  for (uint8_t x = 0; x < WIDTH; x++) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      byte angle = noise3d[0][x][y];
      byte radius = noise3d[1][x][y];
      leds[XY(x, y)] = CHSV(t + radius * (255 / WIDTH), 255, sin8(sin8(t + angle * PETALS + ( radius * (255 / WIDTH))) + t * 4 + sin8(t * 4 - radius * (255 / WIDTH)) + angle * PETALS));
    }
  }
}
