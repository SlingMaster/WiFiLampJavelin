/* notusedeff.ino
   закладка з робочими ефектами видалиними з прошивки (надоїли або є подібнї) щоб зменшити розмір прошивки
  бо вже розмір прошивки вже досягає критичних розмірїв*/
// #define USE_EFFECTS
#ifdef USE_EFFECTS
// =====================================
//               ЭФФЕКТИ
// =====================================
// =====================================
//                Пламя
//             © SottNick
// =====================================
/*По мотивам https://goldenandy.blogspot.com/2021/05/ws2812.html | by Андрей Локтев
  характеристики языков пламени
  x, dx; => trackingObjectPosX, trackingObjectSpeedX;
  y, dy; => trackingObjectPosY, trackingObjectSpeedY;
  ttl; => trackingObjectState;
  uint8_t hue; => float   trackingObjectShift
  uint8_t saturation; => 255U
  uint8_t value; => trackingObjectHue;

  характеристики изображения CHSV picture[WIDTH][HEIGHT]
  uint8_t .hue; => noise3d[0][WIDTH][HEIGHT]
  uint8_t .sat; => shiftValue[HEIGHT] (не хватило двухмерного массива на насыщенность)
  uint8_t .val; => noise3d[1][WIDTH][HEIGHT]
*/

#define FLAME_MAX_DY        256 // максимальная вертикальная скорость перемещения языков пламени за кадр.  имеется в виду 256/256 =   1 пиксель за кадр
#define FLAME_MIN_DY        128 // минимальная вертикальная скорость перемещения языков пламени за кадр.   имеется в виду 128/256 = 0.5 пикселя за кадр
#define FLAME_MAX_DX         32 // максимальная горизонтальная скорость перемещения языков пламени за кадр. имеется в виду 32/256 = 0.125 пикселя за кадр
#define FLAME_MIN_DX       (-FLAME_MAX_DX)
#define FLAME_MAX_VALUE     255 // максимальная начальная яркость языка пламени
#define FLAME_MIN_VALUE     176 // минимальная начальная яркость языка пламени

//пришлось изобрести очередную функцию субпиксельной графики. на этот раз бесшовная по ИКСу, работающая в цветовом пространстве HSV и без смешивания цветов
void wu_pixel_maxV(int16_t item) {
  //uint8_t xx = trackingObjectPosX[item] & 0xff, yy = trackingObjectPosY[item] & 0xff, ix = 255 - xx, iy = 255 - yy;
  uint8_t xx = (trackingObjectPosX[item] - (int)trackingObjectPosX[item]) * 255, yy = (trackingObjectPosY[item] - (int)trackingObjectPosY[item]) * 255, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
#define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)
                  };
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t x1 = (int8_t)(trackingObjectPosX[item] + (i & 1)) % WIDTH; //делаем бесшовный по ИКСу
    uint8_t y1 = (int8_t)(trackingObjectPosY[item] + ((i >> 1) & 1));
    if (y1 < HEIGHT && trackingObjectHue[item] * wu[i] >> 8 >= noise3d[1][x1][y1]) {
      noise3d[0][x1][y1] = trackingObjectShift[item];
      shiftValue[y1] = 255U;//saturation;
      noise3d[1][x1][y1] = trackingObjectHue[item] * wu[i] >> 8;
    }
  }
}

void execStringsFlame() { // внимание! эффект заточен на бегунок Масштаб с диапазоном от 0 до 255
  int16_t i, j;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(255U), 20U + random8(236U)); // на свякий случай пусть будет от 1 до 255, а не от нуля
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    enlargedObjectNUM = (modes[currentMode].Speed - 1U) / 254.0 * (trackingOBJECT_MAX_COUNT - 1U) + 1U;
    if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
    if (currentMode >= EFF_MATRIX) {
      ff_x = WIDTH * 2.4;
      enlargedObjectNUM = (ff_x > enlargedOBJECT_MAX_COUNT) ? enlargedOBJECT_MAX_COUNT : ff_x;
    }

    hue = map8(myScale8(modes[currentMode].Scale + 3U), 3, 10); // минимальная живучесть/высота языка пламени ...ttl
    hue2 = map8(myScale8(modes[currentMode].Scale + 3U), 6, 31); // максимальная живучесть/высота языка пламени ...ttl
    for (i = 0; i < trackingOBJECT_MAX_COUNT; i++) // чистим массив объектов от того, что не похоже на языки пламени
      if (trackingObjectState[i] > 30U || trackingObjectPosY[i] >= HEIGHT || trackingObjectPosX[i] >= WIDTH || trackingObjectPosY[i] <= 0) {
        trackingObjectHue[i] = 0U;
        trackingObjectState[i] = random8(20);
      }
    for (i = 0; i < WIDTH; i++) { // заполняем массив изображения из массива leds обратным преобразованием, которое нихрена не работает
      for (j = 0; j < HEIGHT; j++ ) {
        CHSV tHSV = rgb2hsv_approximate(leds[XY(i, j)]);
        noise3d[0][i][j] = tHSV.hue;
        if (tHSV.val > 100U) { // такая защита от пересвета более-менее достаточна
          shiftValue[j] = tHSV.sat;
          if (tHSV.sat < 100U) { // для перехода с очень тусклых эффектов, использующих заливку белым или почти белым светом
            noise3d[1][i][j] = tHSV.val / 3U;
          } else {
            noise3d[1][i][j] = tHSV.val - 32U;
          }
        } else {
          noise3d[1][i][j] = 0U;
        }
      }
    }
  }

  // угасание предыдущего кадра
  for (i = 0; i < WIDTH; i++) {
    for (j = 0; j < HEIGHT; j++ ) {
      noise3d[1][i][j] = (uint16_t)noise3d[1][i][j] * 237U >> 8;
    }
  }

  // цикл перебора языков пламени
  for (i = 0; i < enlargedObjectNUM; i++) {
    if (trackingObjectState[i]) { // если ещё не закончилась его жизнь
      wu_pixel_maxV(i);

      j = trackingObjectState[i];
      trackingObjectState[i]--;

      trackingObjectPosX[i] += trackingObjectSpeedX[i];
      trackingObjectPosY[i] += trackingObjectSpeedY[i];

      trackingObjectHue[i] = (trackingObjectState[i] * trackingObjectHue[i] + j / 2) / j;

      // если вышел за верхнюю границу или потух, то и жизнь закончилась
      if (trackingObjectPosY[i] >= HEIGHT || trackingObjectHue[i] < 2U)
        trackingObjectState[i] = 0;

      // если вылез за край матрицы по горизонтали, перекинем на другую сторону
      if (trackingObjectPosX[i] < 0)
        trackingObjectPosX[i] += WIDTH;
      else if (trackingObjectPosX[i] >= WIDTH)
        trackingObjectPosX[i] -= WIDTH;

    } else { // если жизнь закончилась, перезапускаем
      trackingObjectState[i] = random8(hue, hue2);
      trackingObjectShift[i] = (uint8_t)(254U + modes[currentMode].Scale + random8(20U)); // 254 - это шаг в обратную сторону от выбранного пользователем оттенка (стартовый оттенок диапазона)
      // 20 - это диапазон из градиента цвета от выбранного пользователем оттенка (диапазон от 254 до 254+20)
      trackingObjectPosX[i] = (float)random(WIDTH * 255U) / 255.;
      trackingObjectPosY[i] = -.9;
      trackingObjectSpeedX[i] = (float)(FLAME_MIN_DX + random8(FLAME_MAX_DX - FLAME_MIN_DX)) / 256.;
      trackingObjectSpeedY[i] = (float)(FLAME_MIN_DY + random8(FLAME_MAX_DY - FLAME_MIN_DY)) / 256.;
      trackingObjectHue[i] = FLAME_MIN_VALUE + random8(FLAME_MAX_VALUE - FLAME_MIN_VALUE + 1U);
      //saturation = 255U;
    }
  }

  //выводим кадр на матрицу
  for (i = 0; i < WIDTH; i++) {
    for (j = 0; j < HEIGHT; j++) {
      hsv2rgb_spectrum(CHSV(noise3d[0][i][j], shiftValue[j], noise3d[1][i][j]), leds[XY(i, j)]);
    }
  }
}



// =====================================
//            Koдoвый зaмoк
// =====================================
//             © SottNick
/*
  из-за повторного использоваия переменных от других эффектов теперь в этом коде невозможно что-то понять.
  поэтому для понимания придётся сперва заменить названия переменных на человеческие. но всё равно это песец, конечно.
  uint8_t deltaHue2; // максимальне количество пикселей в кольце (толщина кольца) от 1 до HEIGHT / 2 + 1
  uint8_t deltaHue; // количество колец от 2 до HEIGHT
  uint8_t noise3d[1][1][HEIGHT]; // начальный оттенок каждого кольца (оттенка из палитры) 0-255
  uint8_t shiftValue[HEIGHT]; // местоположение начального оттенка кольца 0-WIDTH-1
  uint8_t shiftHue[HEIGHT]; // 4 бита на ringHueShift, 4 на ringHueShift2
  ringHueShift[ringsCount]; // шаг градиета оттенка внутри кольца -8 - +8 случайное число
  ringHueShift2[ringsCount]; // обычная скорость переливания оттенка всего кольца -8 - +8 случайное число
  uint8_t deltaValue; // кольцо, которое в настоящий момент нужно провернуть
  uint8_t step; // оставшееся количество шагов, на которое нужно провернуть активное кольцо - случайное от WIDTH/5 до WIDTH-3
  uint8_t hue, hue2; // количество пикселей в нижнем (hue) и верхнем (hue2) кольцах
*/
void ringsRoutine() {
  uint8_t h, x, y;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(90U + random8(6U), 175U + random8(61U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();

    //deltaHue2 = (modes[currentMode].Scale - 1U) / 99.0 * (HEIGHT / 2 - 1U) + 1U; // толщина кольца в пикселях. если на весь бегунок масштаба (от 1 до HEIGHT / 2 + 1)
    deltaHue2 = (modes[currentMode].Scale - 1U) % 11U + 1U; // толщина кольца от 1 до 11 для каждой из палитр
    deltaHue = HEIGHT / deltaHue2 + ((HEIGHT % deltaHue2 == 0U) ? 0U : 1U); // количество колец
    hue2 = deltaHue2 - (deltaHue2 * deltaHue - HEIGHT) / 2U; // толщина верхнего кольца. может быть меньше нижнего
    hue = HEIGHT - hue2 - (deltaHue - 2U) * deltaHue2; // толщина нижнего кольца = всё оставшееся
    for (uint8_t i = 0; i < deltaHue; i++) {
      noise3d[0][0][i] = random8(257U - WIDTH / 2U); // начальный оттенок кольца из палитры 0-255 за минусом длины кольца, делённой пополам
      shiftHue[i] = random8();
      shiftValue[i] = 0U; //random8(WIDTH); само прокрутится постепенно
      step = 0U;
      deltaValue = random8(deltaHue);
    }
  }
  for (uint8_t i = 0; i < deltaHue; i++) {
    if (i != deltaValue) {                      // если это не активное кольцо
      h = shiftHue[i] & 0x0F;                   // сдвигаем оттенок внутри кольца

      if (h > 8U) {
        //noise3d[0][0][i] += (uint8_t)(7U - h);  // с такой скоростью сдвиг оттенка от вращения кольца не отличается
        noise3d[0][0][i]--;
      } else {
        //noise3d[0][0][i] += h;
        noise3d[0][0][i]++;
      }

    } else {
      if (step == 0) {// если сдвиг активного кольца завершён, выбираем следующее
        deltaValue = random8(deltaHue);
        do {
          step = WIDTH - 3U - random8((WIDTH - 3U) * 2U); // проворот кольца от хз до хз
        } while (step < WIDTH / 5U || step > 255U - WIDTH / 5U);
      } else {
        if (step > 127U) {
          step++;
          shiftValue[i] = (shiftValue[i] + 1U) % WIDTH;
        } else {
          step--;
          shiftValue[i] = (shiftValue[i] - 1U + WIDTH) % WIDTH;
        }
      }
    }
    // отрисовываем кольца
    h = (shiftHue[i] >> 4) & 0x0F; // берём шаг для градиента вутри кольца
    if (h > 8U)
      h = 7U - h;
    for (uint8_t j = 0U; j < ((i == 0U) ? hue : ((i == deltaHue - 1U) ? hue2 : deltaHue2)); j++) // от 0 до (толщина кольца - 1)
    {
      y = i * deltaHue2 + j - ((i == 0U) ? 0U : deltaHue2 - hue);
      // mod для чётных скоростей by @kostyamat - получается какая-то другая фигня. не стоит того
      //for (uint8_t k = 0; k < WIDTH / ((modes[currentMode].Speed & 0x01) ? 2U : 4U); k++) // полукольцо для нечётных скоростей и четверть кольца для чётных
      for (uint8_t k = 0; k < WIDTH / 2U; k++)  {                                           // полукольцо
        x = (shiftValue[i] + k) % WIDTH;                                                    // первая половина кольца
        leds[XY(x, y)] = ColorFromPalette(*curPalette, noise3d[0][0][i] + k * h);
        x = (WIDTH - 1 + shiftValue[i] - k) % WIDTH;                                        // вторая половина кольца (зеркальная первой)
        leds[XY(x, y)] = ColorFromPalette(*curPalette, noise3d[0][0][i] + k * h);
      }
      if (WIDTH & 0x01) {                                                                   // если число пикселей по ширине матрицы нечётное, тогда не забываем и про среднее значение
        x = (shiftValue[i] + WIDTH / 2U) % WIDTH;
        leds[XY(x, y)] = ColorFromPalette(*curPalette, noise3d[0][0][i] + WIDTH / 2U * h);
      }
    }
  }
}




/*
  // =====================================
  //          Блуждающий кубик
  // =====================================
  //
  #define RANDOM_COLOR          (1U)                          // случайный цвет при отскоке
  int16_t coordB[2U];
  int8_t vectorB[2U];
  CHSV _pulse_color;
  CRGB ballColor;

  void ballRoutine() {
  if (loadingFlag) {
  #if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(13U + random8(88U) , 155U + random8(46U));
    }
  #endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    //FastLED.clear();

    for (uint8_t i = 0U; i < 2U; i++) {
      coordB[i] = WIDTH / 2 * 10;
      vectorB[i] = random(8, 20);
    }
    // ballSize;
    deltaValue = map(modes[currentMode].Scale * 2.55, 0U, 255U, 2U, max((uint8_t)min(WIDTH, HEIGHT) / 3, 4));
    ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
    _pulse_color = CHSV(random(0, 9) * 28, 255U, 255U);
  }

  //  if (!(modes[currentMode].Scale & 0x01))
  //  {
  //    hue += (modes[currentMode].Scale - 1U) % 11U * 8U + 1U;

  //    ballColor = CHSV(hue, 255U, 255U);
  //  }

  if ((modes[currentMode].Scale & 0x01)) {
    for (uint8_t i = 0U; i < deltaValue; i++) {
      for (uint8_t j = 0U; j < deltaValue; j++) {
        leds[XY(coordB[0U] / 10 + i, coordB[1U] / 10 + j)] = _pulse_color;
      }
    }
  }
  for (uint8_t i = 0U; i < 2U; i++) {
    coordB[i] += vectorB[i];
    if (coordB[i] < 0) {
      coordB[i] = 0;
      vectorB[i] = -vectorB[i];
      if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255U, 255U); // if (RANDOM_COLOR && (modes[currentMode].Scale & 0x01))
      //vectorB[i] += random(0, 6) - 3;
    }
  }
  if (coordB[0U] > (int16_t)((WIDTH - deltaValue) * 10)) {
    coordB[0U] = (WIDTH - deltaValue) * 10;
    vectorB[0U] = -vectorB[0U];
    if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
    //vectorB[0] += random(0, 6) - 3;
  }
  if (coordB[1U] > (int16_t)((HEIGHT - deltaValue) * 10)) {
    coordB[1U] = (HEIGHT - deltaValue) * 10;
    vectorB[1U] = -vectorB[1U];
    if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
    //vectorB[1] += random(0, 6) - 3;
  }

  //  if (modes[currentMode].Scale & 0x01)
  //    dimAll(135U);
  // dimAll(255U - (modes[currentMode].Scale - 1U) % 11U * 24U);
  //  else
  FastLED.clear();

  for (uint8_t i = 0U; i < deltaValue; i++) {
    for (uint8_t j = 0U; j < deltaValue; j++) {
      leds[XY(coordB[0U] / 10 + i, coordB[1U] / 10 + j)] = ballColor;
    }
  }
  }
*/



// =====================================
//            Kyбик Pyбикa
// =====================================
//             © SottNick
#define PAUSE_MAX 7 // пропустить 7 кадров после завершения анимации сдвига ячеек
uint8_t razmerX, razmerY; // размеры ячеек по горизонтали / вертикали
uint8_t shtukX, shtukY; // количество ячеек по горизонтали / вертикали
uint8_t poleX, poleY; // размер всего поля по горизонтали / вертикали (в том числе 1 дополнительная пустая дорожка-разделитель с какой-то из сторон)
int8_t globalShiftX, globalShiftY; // нужно ли сдвинуть всё поле по окончаии цикла и в каком из направлений (-1, 0, +1)
bool seamlessX; // получилось ли сделать поле по Х бесшовным
bool krutimVertikalno; // направление вращения в данный момент

void cube2dRoutine() {
  uint8_t x, y;
  uint8_t anim0; // будем считать тут начальный пиксель для анимации сдвига строки/колонки
  int8_t shift, kudaVse; // какое-то расчётное направление сдвига (-1, 0, +1)
  CRGB color, color2;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(9U) * 11U + random8(8U); // масштаб 1-7, палитры все 9
      if (tmp == 45U) tmp = 100U; //+ белый цвет
      setModeSettings(tmp, 175U + random8(66U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    setCurrentPalette();
    FastLED.clear();

    razmerX = (modes[currentMode].Scale - 1U) % 11U + 1U; // размер ячейки от 1 до 11 пикселей для каждой из 9 палитр
    razmerY = razmerX;
    if (modes[currentMode].Speed & 0x01) // по идее, ячейки не обязательно должны быть квадратными, поэтому можно тут поизвращаться
      razmerY = (razmerY << 1U) + 1U;

    shtukY = HEIGHT / (razmerY + 1U);
    if (shtukY < 2U)
      shtukY = 2U;
    y = HEIGHT / shtukY - 1U;
    if (razmerY > y)
      razmerY = y;
    poleY = (razmerY + 1U) * shtukY;
    shtukX = WIDTH / (razmerX + 1U);
    if (shtukX < 2U)
      shtukX = 2U;
    x = WIDTH / shtukX - 1U;
    if (razmerX > x)
      razmerX = x;
    poleX = (razmerX + 1U) * shtukX;
    seamlessX = (poleX == WIDTH);
    deltaHue = 0U;
    deltaHue2 = 0U;
    globalShiftX = 0;
    globalShiftY = 0;

    for (uint8_t j = 0U; j < shtukY; j++) {
      y = j * (razmerY + 1U); // + deltaHue2 т.к. оно =0U
      for (uint8_t i = 0U; i < shtukX; i++) {
        x = i * (razmerX + 1U); // + deltaHue т.к. оно =0U
        if (modes[currentMode].Scale == 100U) {
          color = CHSV(45U, 0U, 128U + random8(128U));
        } else {
          color = ColorFromPalette(*curPalette, random8());
        }
        for (uint8_t k = 0U; k < razmerY; k++) {
          for (uint8_t m = 0U; m < razmerX; m++) {
            leds[XY(x + m, y + k)] = color;
          }
        }
      }
    }
    step = 4U; // текущий шаг сдвига первоначально с перебором (от 0 до deltaValue-1)
    deltaValue = 4U; // всего шагов сдвига (от razmer? до (razmer?+1) * shtuk?)
    hue2 = 0U; // осталось шагов паузы
  }

  //двигаем, что получилось...
  if (hue2 == 0 && step < deltaValue) { // если пауза закончилась, а цикл вращения ещё не завершён
    step++;
    if (krutimVertikalno) {
      for (uint8_t i = 0U; i < shtukX; i++) {
        x = (deltaHue + i * (razmerX + 1U)) % WIDTH;
        if (noise3d[0][i][0] > 0) { // в нулевой ячейке храним оставшееся количество ходов прокрутки
          noise3d[0][i][0]--;
          shift = noise3d[0][i][1] - 1; // в первой ячейке храним направление прокрутки

          if (globalShiftY == 0)
            anim0 = (deltaHue2 == 0U) ? 0U : deltaHue2 - 1U;
          else if (globalShiftY > 0)
            anim0 = deltaHue2;
          else
            anim0 = deltaHue2 - 1U;

          if (shift < 0) { // если крутим столбец вниз
            color = leds[XY(x, anim0)];                                   // берём цвет от нижней строчки
            for (uint8_t k = anim0; k < anim0 + poleY - 1; k++) {
              color2 = leds[XY(x, k + 1)];                                // берём цвет от строчки над нашей
              for (uint8_t m = x; m < x + razmerX; m++) {
                leds[XY(m % WIDTH, k)] = color2;                          // копируем его на всю нашу строку
              }
            }
            for (uint8_t m = x; m < x + razmerX; m++) {
              leds[XY(m % WIDTH, anim0 + poleY - 1)] = color;             // цвет нижней строчки копируем на всю верхнюю
            }
          }
          else if (shift > 0) // если крутим столбец вверх
          {
            color = leds[XY(x, anim0 + poleY - 1)];                       // берём цвет от верхней строчки
            for (uint8_t k = anim0 + poleY - 1; k > anim0 ; k--) {
              color2 = leds[XY(x, k - 1)];                                // берём цвет от строчки под нашей
              for (uint8_t m = x; m < x + razmerX; m++) {
                leds[XY(m % WIDTH, k)] = color2;                          // копируем его на всю нашу строку
              }
            }
            for (uint8_t m = x; m < x + razmerX; m++) {
              leds[XY(m % WIDTH, anim0)] = color;                         // цвет верхней строчки копируем на всю нижнюю
            }
          }
        }
      }
    } else {
      for (uint8_t j = 0U; j < shtukY; j++) {
        y = deltaHue2 + j * (razmerY + 1U);
        if (noise3d[0][0][j] > 0) { // в нулевой ячейке храним оставшееся количество ходов прокрутки
          noise3d[0][0][j]--;
          shift = noise3d[0][1][j] - 1; // в первой ячейке храним направление прокрутки
          if (seamlessX) anim0 = 0U;
          else if (globalShiftX == 0) anim0 = (deltaHue == 0U) ? 0U : deltaHue - 1U;
          else if (globalShiftX > 0) anim0 = deltaHue;
          else anim0 = deltaHue - 1U;

          if (shift < 0) { // если крутим строку влево
            color = leds[XY(anim0, y)];                            // берём цвет от левой колонки (левого пикселя)
            for (uint8_t k = anim0; k < anim0 + poleX - 1; k++) {
              color2 = leds[XY(k + 1, y)];                         // берём цвет от колонки (пикселя) правее
              for (uint8_t m = y; m < y + razmerY; m++) {
                leds[XY(k, m)] = color2;                           // копируем его на всю нашу колонку
              }
            }
            for (uint8_t m = y; m < y + razmerY; m++) {
              leds[XY(anim0 + poleX - 1, m)] = color;              // цвет левой колонки копируем на всю правую
            }
          }
          else if (shift > 0) // если крутим столбец вверх
          {
            color = leds[XY(anim0 + poleX - 1, y)];                // берём цвет от правой колонки
            for (uint8_t k = anim0 + poleX - 1; k > anim0 ; k--) {
              color2 = leds[XY(k - 1, y)];                         // берём цвет от колонки левее
              for (uint8_t m = y; m < y + razmerY; m++) {
                leds[XY(k, m)] = color2;                           // копируем его на всю нашу колонку
              }
            }
            for (uint8_t m = y; m < y + razmerY; m++) {
              leds[XY(anim0, m)] = color;                          // цвет правой колонки копируем на всю левую
            }
          }
        }
      }
    }
  }
  else if (hue2 != 0U) hue2--;                                    // пропускаем кадры после прокрутки кубика (делаем паузу)
  if (step >= deltaValue) { // если цикл вращения завершён, меняем местами соответствующие ячейки (цвет в них) и точку первой ячейки
    step = 0U;
    hue2 = PAUSE_MAX;
    //если часть ячеек двигалась на 1 пиксель, пододвигаем глобальные координаты начала
    deltaHue2 = deltaHue2 + globalShiftY; //+= globalShiftY;
    globalShiftY = 0;
    //deltaHue += globalShiftX; для бесшовной не годится
    deltaHue = (WIDTH + deltaHue + globalShiftX) % WIDTH;
    globalShiftX = 0;

    //пришла пора выбрать следующие параметры вращения
    kudaVse = 0;
    krutimVertikalno = random8(2U);
    if (krutimVertikalno) {             // идём по горизонтали, крутим по вертикали (столбцы двигаются)
      for (uint8_t i = 0U; i < shtukX; i++) {
        noise3d[0][i][1] = random8(3);
        shift = noise3d[0][i][1] - 1; // в первой ячейке храним направление прокрутки
        if (kudaVse == 0) kudaVse = shift;
        else if (shift != 0 && kudaVse != shift) kudaVse = 50;
      }
      deltaValue = razmerY + ((deltaHue2 - kudaVse >= 0 && deltaHue2 - kudaVse + poleY < (int)HEIGHT) ? random8(2U) : 1U);

      if (deltaValue == razmerY) { // значит полюбому kudaVse было = (-1, 0, +1) - и для нуля в том числе мы двигаем весь куб на 1 пиксель
        globalShiftY = 1 - kudaVse; //временно на единичку больше, чем надо
        for (uint8_t i = 0U; i < shtukX; i++)
          if (noise3d[0][i][1] == 1U) { // если ячейка никуда не планировала двигаться
            noise3d[0][i][1] = globalShiftY;
            noise3d[0][i][0] = 1U; // в нулевой ячейке храним количество ходов сдвига
          } else {
            noise3d[0][i][0] = deltaValue; // в нулевой ячейке храним количество ходов сдвига
          }
        globalShiftY--;
      } else {
        x = 0;
        for (uint8_t i = 0U; i < shtukX; i++) {
          if (noise3d[0][i][1] != 1U) {
            y = random8(shtukY);
            if (y > x)
              x = y;
            noise3d[0][i][0] = deltaValue * (x + 1U); // в нулевой ячейке храним количество ходов сдвига
          }
        }
        deltaValue = deltaValue * (x + 1U);
      }
    } else  {// идём по вертикали, крутим по горизонтали (строки двигаются)

      for (uint8_t j = 0U; j < shtukY; j++) {
        noise3d[0][1][j] = random8(3);
        shift = noise3d[0][1][j] - 1;                 // в первой ячейке храним направление прокрутки
        if (kudaVse == 0) kudaVse = shift;
        else if (shift != 0 && kudaVse != shift) kudaVse = 50;
      }
      if (seamlessX) {
        deltaValue = razmerX + ((kudaVse < 50) ? random8(2U) : 1U);
      } else {
        deltaValue = razmerX + ((deltaHue - kudaVse >= 0 && deltaHue - kudaVse + poleX < (int)WIDTH) ? random8(2U) : 1U);
      }
      if (deltaValue == razmerX) {                // значит полюбому kudaVse было = (-1, 0, +1) - и для нуля в том числе мы двигаем весь куб на 1 пиксель
        globalShiftX = 1 - kudaVse; //временно на единичку больше, чем надо
        for (uint8_t j = 0U; j < shtukY; j++) {
          if (noise3d[0][1][j] == 1U) { // если ячейка никуда не планировала двигаться
            noise3d[0][1][j] = globalShiftX;
            noise3d[0][0][j] = 1U; // в нулевой ячейке храним количество ходов сдвига
          } else {
            noise3d[0][0][j] = deltaValue; // в нулевой ячейке храним количество ходов сдвига
          }
          globalShiftX--;
        }
      } else {
        y = 0;
        for (uint8_t j = 0U; j < shtukY; j++) {
          if (noise3d[0][1][j] != 1U) {
            x = random8(shtukX);
            if (x > y)
              y = x;
            noise3d[0][0][j] = deltaValue * (x + 1U); // в нулевой ячейке храним количество ходов сдвига
          }
        }
        deltaValue = deltaValue * (y + 1U);
      }
    }
  }
}

// =====================================
//               Snowfall
//            © SlingMaster
//               Снігопад
// =====================================
void Snowfall() {
  static byte divider;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed 100 - 190
      setModeSettings(random8(100U), random8(100U, 190U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    clearNoiseArr();
    divider = floor(modes[currentMode].Scale / 25);
  }

  if (divider == 1) {
    dimAll(40);
  } else {
    if (divider == 2) {
      gradientVertical(0, 0, WIDTH, HEIGHT, 160, 160, 255, 128, 255U);
    } else {
      FastLED.clear();
    }
  }
  VirtualSnow(divider);
}

// =====================================
//                Чacы
// =====================================
//             © SottNick

#ifdef USE_TIME_EFFECT
#define CLOCK_SAVE_MODE     // удалите или закомментируйте эту строчку, чтобы цифры всегда оставались на одном месте, не двигались по вертикали (не хорошо для светодиодов. выгорают зря)
#if (HEIGHT > 12) || (HEIGHT < 11)
#define CLOCK_BLINKING      // удалите или закомментируйте эту строчку, чтобы точки не мигали
#endif
//uint8_t hue, hue2; // храним тут часы и минуты
//uint8_t deltaHue, deltaHue2; // храним здесь задержки мигания точек
//uint8_t deltaValue; // счётчик цикла / яркости точек на часах
//uint8_t poleX, poleY; // храним здесь сдвиг циферблата по горизонтали и вертикали (переменные объявлены в эффекте Кубик Рубика)
static const uint8_t clockFont3x5[10][3] PROGMEM = { // цифры зеркально и на левом боку (так проще рисовать в циклах и экономнее для памяти)
  { B11111,
    B10001,
    B11111
  },
  { B01001,
    B11111,
    B00001
  },
  { B10011,
    B10101,
    B01001
  },
  { B10001,
    B10101,
    B01010
  },
  { B11100,
    B00100,
    B11111
  },
  { B11101,
    B10101,
    B10010
  },
  { B01111,
    B10101,
    B10111
  },
  { B10011,
    B10100,
    B11000
  },
  { B11111,
    B10101,
    B11111
  },
  { B11101,
    B10101,
    B11110
  }
};

void drawDig3x5(uint8_t x, uint8_t y, uint8_t num, CRGB color) { // uint8_t hue, uint8_t sat, uint8_t bri = 255U
  for (uint8_t i = 0U; i < 3U; i++) {
    uint8_t m = pgm_read_byte(&clockFont3x5[num][i]);
    for (uint8_t j = 0U; j < 5U; j++) {
      if ((m >> j) & 0x01) drawPixelXY((x + i) % WIDTH, (y + j) % HEIGHT, color);
    }
  }
}

#if HEIGHT > 10 // часы в столбик будут только если высота 11 пикселей и больше
void clockRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(20) ? 7U + random8(86U) : 100U, modes[currentMode].Speed);
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    poleX = (modes[currentMode].Speed - 1U) % WIDTH; //смещение цифр по горизонтали
#ifdef CLOCK_BLINKING
#if HEIGHT > 13
    poleY = (modes[currentMode].Speed - 1U) / WIDTH % (HEIGHT - 13U);  //смещение цифр по вертикали (для режима CLOCK_SAVE_MODE будет меняться само)
#else
    poleY = 0U;
#endif
#else
#if HEIGHT > 12
    poleY = (modes[currentMode].Speed - 1U) / WIDTH % (HEIGHT - 12U);  //смещение цифр по вертикали (для режима CLOCK_SAVE_MODE будет меняться само)
#else // и для 12 и для 11 смещаться некуда. всё впритык
    poleY = 0U;
#endif
#endif
    hue2 = 255U; // количество минут в данный момент (первоначально запредельое значение)
    deltaHue2 = 0; // яркость точки в данный момент
    deltaValue = modes[currentMode].Scale * 2.55; // выбранный оттенок цифр
  }

  time_t currentLocalTime = getCurrentLocalTime();

  if (minute(currentLocalTime) != hue2) {
#ifdef CLOCK_SAVE_MODE
#ifdef CLOCK_BLINKING
#if HEIGHT > 13
    poleY = (poleY + 1U) % (HEIGHT - 13U);
#endif
#else
#if HEIGHT > 12
    poleY = (poleY + 1U) % (HEIGHT - 12U);
#endif
#endif
#endif
    step = 1U; // = CLOCK_REFRESH_DELAY; раньше делал постепенное затухание. получалось хуже
    hue = hour(currentLocalTime);
    hue2 = minute(currentLocalTime);
  }
  if (step > 0) { // тут меняются цифры на часах
    step--;
    uint8_t sat = (modes[currentMode].Scale == 100) ? 0U : 255U;
    FastLED.clear();
    // рисуем цифры
#ifdef CLOCK_BLINKING
    drawDig3x5(   poleX,               (poleY + 8U), hue  / 10U % 10U, CHSV(deltaValue, sat, 255U));
    drawDig3x5(  (poleX + 4U) % WIDTH, (poleY + 8U), hue        % 10U, CHSV(deltaValue, sat, 255U));
#else
#if HEIGHT > 11
    drawDig3x5( poleX,               (poleY + 7U), hue  / 10U % 10U, CHSV(deltaValue, sat, 255U));
    drawDig3x5((poleX + 4U) % WIDTH, (poleY + 7U), hue        % 10U, CHSV(deltaValue, sat, 255U));
#else // если матрица всего 11 пикселей в высоту, можно сэкономить 1 и впихнуть часы в неё. но если меньше, нужно брать код эффекта с высотой цифр 4 пикселя, а не 5
    drawDig3x5( poleX,               (poleY + 6U), hue  / 10U % 10U, CHSV(deltaValue, sat, 255U));
    drawDig3x5((poleX + 4U) % WIDTH, (poleY + 6U), hue        % 10U, CHSV(deltaValue, sat, 255U));
#endif
#endif
    drawDig3x5(     poleX, poleY,                      hue2 / 10U % 10U, CHSV(deltaValue, sat, 255U));
    drawDig3x5(    (poleX + 4U) % WIDTH, poleY,        hue2       % 10U, CHSV(deltaValue, sat, 255U));
  }

#ifdef CLOCK_BLINKING
  // тут мигают точки
  if (deltaHue2 & 0x01) {
    deltaHue2 = deltaHue2 - ((deltaHue2 >  15U) ? 16U : 15U);//- ((deltaHue2 >  63U) ? 64U : 63U);
  } else {
    deltaHue2 = deltaHue2 + ((deltaHue2 < 240U) ? 16U : 15U);//+ ((deltaHue2 < 192U) ? 64U : 63U);
  }
  drawPixelXY((poleX + 2U) % WIDTH, poleY + 6U, CHSV(deltaValue, (modes[currentMode].Scale == 100) ? 0U : 255U, deltaHue2)); // цвет белый для .Scale=100
  drawPixelXY((poleX + 4U) % WIDTH, poleY + 6U, CHSV(deltaValue, (modes[currentMode].Scale == 100) ? 0U : 255U, deltaHue2)); // цвет белый для .Scale=100

#endif //#ifdef CLOCK_BLINKING
}
#else // для матриц и гирлянд от 6 до 10 пикселей в высоту #if HEIGHT > 10
void clockRoutine() { // чтобы цифры были не в столбик, а в строчку
  if (loadingFlag)  {
    loadingFlag = false;
    poleX = (modes[currentMode].Speed - 1U) % WIDTH; //смещение цифр по горизонтали
    poleY = (modes[currentMode].Speed - 1U) / WIDTH % (HEIGHT - 5U);  //смещение цифр по вертикали (для режима CLOCK_SAVE_MODE будет меняться само)
    hue2 = 255U; // количество минут в данный момент (первоначально запредельое значение)
    deltaHue2 = 0; // яркость точки в данный момент
    deltaValue = modes[currentMode].Scale * 2.55; // выбранный оттенок цифр
  }
  time_t currentLocalTime = getCurrentLocalTime();
  if (minute(currentLocalTime) != hue2) {
#ifdef CLOCK_SAVE_MODE
    poleY = (poleY + 1U) % (HEIGHT - 5U);
#endif
    step = 1U;                                      // = CLOCK_REFRESH_DELAY; раньше делал постепенное затухание. получалось хуже
    hue = hour(currentLocalTime);
    hue2 = minute(currentLocalTime);
  }
  if (step > 0) {                                   // тут меняются цифры на часах
    step--;
    uint8_t sat = (modes[currentMode].Scale == 100) ? 0U : 255U;
    FastLED.clear();
    // рисуем цифры
    drawDig3x5( poleX               , poleY, hue  / 10U % 10U, CHSV(deltaValue, sat, 255U));
    drawDig3x5((poleX +  4U) % WIDTH, poleY, hue        % 10U, CHSV(deltaValue, sat, 255U));
    drawDig3x5((poleX +  9U) % WIDTH, poleY, hue2 / 10U % 10U, CHSV(deltaValue, sat, 255U));
    drawDig3x5((poleX + 13U) % WIDTH, poleY, hue2       % 10U, CHSV(deltaValue, sat, 255U));
  }

#ifdef CLOCK_BLINKING
  // тут мигают точки
  if (deltaHue2 & 0x01) {
    deltaHue2 = deltaHue2 - ((deltaHue2 >  15U) ? 16U : 15U);//- ((deltaHue2 >  63U) ? 64U : 63U);
  } else {
    deltaHue2 = deltaHue2 + ((deltaHue2 < 240U) ? 16U : 15U);//+ ((deltaHue2 < 192U) ? 64U : 63U);
  }
  drawPixelXY((poleX + 8U) % WIDTH, poleY + 1U, CHSV(deltaValue, (modes[currentMode].Scale == 100) ? 0U : 255U, deltaHue2)); // цвет белый для .Scale=100
  drawPixelXY((poleX + 8U) % WIDTH, poleY + 3U, CHSV(deltaValue, (modes[currentMode].Scale == 100) ? 0U : 255U, deltaHue2)); // цвет белый для .Scale=100
#endif //#ifdef CLOCK_BLINKING
}
#endif //#if HEIGHT > 10
#endif


// =====================================
//           Rainbow Diagonal
//           © Andrew Tuline
// =====================================
void Rainbow45() {
  // очередная радуга надо попробовать на Lamp Javelin
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      // XY tells us the index of a given X/Y coordinate
      int index = XY(x, y);
      hue = x * 10 + y * 10;
      hue += sin8(millis() / 20 + y * 5 + x * 7);
      leds[index] = CHSV(hue, 255, 255);
    }
  }
}

#endif
