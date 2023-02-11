
// ============= Tixy Land ==============
//        © Martin Kleppe @aemkei
//github.com/owenmcateer/tixy.land-display
//      Create Script Change Effects
//             © SlingMaster
// ======================================
//   набор мат. функций и примитивов для
//            обсчета эффектов
//       © Dmytro Korniienko (kDn)
// ======================================

#define M_PI_2  1.57079632679489661923
static const PROGMEM float LUT[102] = {
  0,           0.0099996664, 0.019997334, 0.029991005, 0.039978687,
  0.049958397, 0.059928156,  0.069885999, 0.079829983, 0.089758173,
  0.099668652, 0.10955953,   0.11942893,  0.12927501,  0.13909595,
  0.14888994,  0.15865526,   0.16839015,  0.17809294,  0.18776195,
  0.19739556,  0.20699219,   0.21655031,  0.22606839,  0.23554498,
  0.24497867,  0.25436807,   0.26371184,  0.27300870,  0.28225741,
  0.29145679,  0.30060568,   0.30970293,  0.31874755,  0.32773849,
  0.33667481,  0.34555557,   0.35437992,  0.36314702,  0.37185606,
  0.38050637,  0.38909724,   0.39762798,  0.40609807,  0.41450688,
  0.42285392,  0.43113875,   0.43936089,  0.44751999,  0.45561564,
  0.46364760,  0.47161558,   0.47951928,  0.48735857,  0.49513325,
  0.50284320,  0.51048833,   0.51806855,  0.52558380,  0.53303409,
  0.54041952,  0.54774004,   0.55499572,  0.56218672,  0.56931317,
  0.57637525,  0.58337301,   0.59030676,  0.59717667,  0.60398299,
  0.61072594,  0.61740589,   0.62402308,  0.63057774,  0.63707036,
  0.64350110,  0.64987046,   0.65617871,  0.66242629,  0.66861355,
  0.67474097,  0.68080884,   0.68681765,  0.69276786,  0.69865984,
  0.70449406,  0.71027100,   0.71599114,  0.72165483,  0.72726268,
  0.73281509,  0.73831260,   0.74375558,  0.74914461,  0.75448018,
  0.75976276,  0.76499283,   0.77017093,  0.77529752,  0.78037310,
  0.78539819,  0.79037325
};

// --------------------------------------
float atan2_fast(float y, float x) {
  //http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
  //Volkan SALMA

  const float ONEQTR_PI = PI / 4.0;
  const float THRQTR_PI = 3.0 * PI / 4.0;
  float r, angle;
  float abs_y = fabs(y) + 1e-10f;      // kludge to prevent 0/0 condition
  if ( x < 0.0f ) {
    r = (x + abs_y) / (abs_y - x);
    angle = THRQTR_PI;
  } else {
    r = (x - abs_y) / (x + abs_y);
    angle = ONEQTR_PI;
  }
  angle += (0.1963f * r * r - 0.9817f) * r;
  if ( y < 0.0f ) {
    return ( -angle );    // negate if in quad III or IV
  } else {
    return ( angle );
  }
}

// --------------------------------------
float atan_fast(float x) {
  /* A fast look-up method with enough accuracy */
  if (x > 0) {
    if (x <= 1) {
      int index = round(x * 100);
      return LUT[index];
    } else {
      float re_x = 1 / x;
      int index = round(re_x * 100);
      return (M_PI_2 - LUT[index]);
    }
  } else {
    if (x >= -1) {
      float abs_x = -x;
      int index = round(abs_x * 100);
      return -(LUT[index]);
    } else {
      float re_x = 1 / (-x);
      int index = round(re_x * 100);
      return (LUT[index] - M_PI_2);
    }
  }
}

// --------------------------------------
float tan2pi_fast(float x) {
  float y = (1 - x * x);
  return x * (((-0.000221184 * y + 0.0024971104) * y - 0.02301937096) * y + 0.3182994604 + 1.2732402998 / y);
}


// --------------------------------------
float code(double t, double i, double x, double y) {
  switch (pcnt) {
    /** © Motus Art @motus_art */
    case 1: /* Plasma */
      hue = 96U; hue2 = 224U;
      return (sin16((x + t) * 8192.0) * 0.5 + sin16((y + t) * 8192.0) * 0.5 + sin16((x + y + t) * 8192.0) * 0.3333333333333333) / 32767.0;
      break;

    case 2: /* Up&Down */
      //return sin(cos(x) * y / 8 + t);
      hue = 255U; hue2 = 160U;
      return sin16((cos16(x * 8192.0) / 32767.0 * y / (HEIGHT / 2.0) + t) * 8192.0) / 32767.0;
      break;

    case 3:
      hue = 255U; hue2 = 96U;
      return sin16((atan_fast(y / x) + t) * 8192.0) / 32767.0;
      break;

    /** © tixy.land website */
    case 4: /* Emitting rings */
      hue = 255U; hue2 = 0U;
      return sin16((t - sqrt3((x - (WIDTH / 2)) * (x - (WIDTH / 2)) + (y - (HEIGHT / 2)) * (y - (HEIGHT / 2)))) * 8192.0) / 32767.0;
      break;

    case 5: /* Rotation  */
      hue = 136U; hue2 = 48U;
      return sin16((PI * 2.5 * atan_fast((y - (HEIGHT / 2)) / (x - (WIDTH / 2))) + 5 * t) * 8192.0) / 32767.0;
      break;

    case 6: /* Vertical fade */
      hue = 160U; hue2 = 0U;
      return sin16((y / 8 + t) * 8192.0) / 32767.0;
      break;

    case 7: /* Waves */
      //return sin(x / 2) - sin(x - t) - y + 6;
      hue = 48U; hue2 = 160U;
      return (sin16(x * 4096.0) - sin16((x - t) * 8192.0)) / 32767.0 - y + (HEIGHT / 2);
      break;

    case 8: /* Drop */
      hue = 136U; hue2 = 160U;
      return fmod(8 * t, 13) - sqrt3((x - (WIDTH / 2)) * (x - (WIDTH / 2)) + (y - (HEIGHT / 2)) * (y - (HEIGHT / 2))); //hypot(x - (WIDTH/2), y - (HEIGHT/2));
      break;

    case 9: /* Ripples @thespite */
      hue = 96U; hue2 = 224U;
      return sin16((t - sqrt3(x * x + y * y)) * 8192.0) / 32767.0;
      break;

    case 10: /* Bloop bloop bloop @v21 */
      hue = 136U; hue2 = 160U;
      return (x - (WIDTH / 2)) * (y - (HEIGHT / 2)) - sin16(t * 4096.0) / 512.0;
      break;

    case 11: /* SN0WFAKER */
      // https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgk7c0/
      hue = 96U; hue2 = 160U;
      return sin16((atan_fast((y - (HEIGHT / 2)) / (x - (WIDTH / 2))) + t) * 8192.0) / 32767.0;
      break;
    case 12: /* detunized */
      // https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgk30l/
      hue = 136U; hue2 = 160U;
      return sin16((y / (HEIGHT / 2) + t * 0.5) * 8192.0) / 32767.0 + x / 16 - 0.5;
      break;

    /** © @akella | https://twitter.com/akella/status/1323549082552619008 */
    case 13:
      hue = 255U; hue2 = 0U;
      return sin16((6 * atan2_fast(y - (HEIGHT / 2), x) + t) * 8192.0) / 32767.0;
      break;
    case 14:
      hue = 32U; hue2 = 160U;
      return sin16((i / 5 + t) * 16384.0) / 32767.0;
      break;

    /** © Paul Malin | https://twitter.com/P_Malin/ */

    // sticky blood
    // by @joeytwiddle
    //(t,i,x,y) => y-t*3+9+3*cos(x*3-t)-5*sin(x*7)

    //      if (x < 8) {
    //       // hue = 160U;
    //      } else {
    //       // hue = 96U;
    //      }
    //      if ((y == HEIGHT -1)&(x == 8)) {
    //        hue = hue + 30;
    //        if (hue >= 255U) {
    //          hue = 0;
    //        }
    //      }
    //      hue = t/128+8;

    //    case 19: // !!!! paint
    //      // Matrix Rain https://twitter.com/P_Malin/status/1323583013880553472
    //      //return 1. - fmod((x * x - y + t * (fmod(1 + x * x, 5)) * 6), 16) / 16;
    //      return 1. - fmod((x * x - (HEIGHT - y) + t * (1 + fmod(x * x, 5)) * 3), WIDTH) / HEIGHT;
    //      break;

    case 15: /* Burst */
      // https://twitter.com/P_Malin/status/1323605999274594304
      hue = 136U; hue2 = 160U;
      return -10. / ((x - (WIDTH / 2)) * (x - (WIDTH / 2)) + (y - (HEIGHT / 2)) * (y - (HEIGHT / 2)) - fmod(t * 0.3, 0.7) * 200);
      break;

    case 16: /* Rays */
      hue = 255U; hue2 = 0U;
      return sin16((atan2_fast(x, y) * 5 + t * 2) * 8192.0) / 32767.0;
      break;

    case 17: /* Starfield */
      // org | https://twitter.com/P_Malin/status/1323702220320313346
      hue = 255U; hue2 = 160U;
      return !((int)(x + t * 50 / (fmod(y * y, 5.9) + 1)) & 15) / (fmod(y * y, 5.9) + 1);
      //      {
      //        uint16_t _y = HEIGHT - y;
      //        float d = (fmod(_y * _y + 4, 4.1) + 0.85) * 0.5; // коэффициенты тут отвечают за яркость (размер), скорость, смещение, подбираются экспериментально :)
      //        return !((int)(x + t * 7.0 / d) & 15) / d; // 7.0 - множитель скорости
      //      }
      break;

    case 18:
      hue = 255U; hue2 = 0U;
      return sin16((3.5 * atan2_fast(y - (HEIGHT / 2) + sin16(t * 8192.0) * 0.00006, x - (WIDTH / 2) + sin16(t * 8192.0) * 0.00006) + t * 1.5 + 5) * 8192.0) / 32767.0;
      break;

    case 19:
      hue = 255U; hue2 = 224U;
      return (y - 8) / 3 - tan2pi_fast((x / 6 + 1.87) / PI * 2) * sin16(t * 16834.0) / 32767.0;
      break;

    case 20:
      hue = 136U; hue2 = 160U;
      return (y - 8) / 3 - (sin16((x / 4 + t * 2) * 8192.0) / 32767.0);
      break;

    case 21:
      hue = 72U; hue2 = 96U;
      return cos(sin16(x * t * 819.2) / 32767.0 * PI) + cos16((sin16((y * t / 10 + (sqrt3(abs(cos16(x * t * 8192.0) / 32767.0)))) * 8192.0) / 32767.0 * PI) * 8192.0) / 32767.0;
      break;

    case 22: /* bambuk */
      hue = 96U; hue2 = 80U;
      return sin16(x / 3 * sin16(t * 2730.666666666667) / 2.0) / 32767.0 + cos16(y / 4 * sin16(t * 4096.0) / 2.0) / 32767.0;
      break;

    case 23:
      hue = 0U; hue2 = 224U;
      {
        float _x = x - fmod(t, WIDTH);
        float _y = y - fmod(t, HEIGHT);
        return -.4 / (sqrt3(_x * _x + _y * _y) - fmod(t, 2) * 9);
      }
      break;

    case 24: /* honey */
      hue = 255U; hue2 = 40U;
      return sin16(y * t * 2048.0) / 32767.0 * cos16(x * t * 2048.0) / 32767.0;
      break;

    case 25:
      hue = 96U; hue2 = 160U;
      return atan_fast((x - (WIDTH / 2)) * (y - (HEIGHT / 2))) - 2.5 * sin16(t * 8192.0) / 32767.0;
      break;

    default:
      if (pcnt > 25) {
        deltaHue2 += 32;
      }
      pcnt = 1;
      hue = 96U; hue2 = 0U;
      return sin16(t * 8192.0) / 32767.0;
      break;
  }
}

// --------------------------------------
void processFrame(double t, double x, double y) {
  double i = (y * WIDTH) + x;
  double frame = constrain(code(t, i, x, y), -1, 1) * 255;
  if (frame > 0) {
    if ( hue == 255U) {
      drawPixelXY(x, y, CRGB(frame, frame, frame));
    } else {
      drawPixelXY(x, y, CHSV(hue, frame, frame));
    }
  } else {
    if (frame < 0) {
      if (modes[currentMode].Scale < 5) deltaHue2 = 0;
      drawPixelXY(x, y, CHSV(hue2 + deltaHue2, frame * -1, frame * -1));
    } else {
      drawPixelXY(x, y, CRGB::Black);
    }
  }
}

// --------------------------------------
void TixyLand() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), random8(255U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    deltaHue = 0;
    pcnt = map(modes[currentMode].Speed, 5, 250, 1U, 25U);
    FPSdelay = 1;
    deltaHue2 = modes[currentMode].Scale * 2.55;
    hue = 255U; hue2 = 0U;
  }
  // *****
  unsigned long milli = millis();
  double t = milli / 1000.0;

  EVERY_N_SECONDS(eff_interval) {
    if ((modes[currentMode].Speed < 5) || (modes[currentMode].Speed > 250)) {
      pcnt++;
    }
  }
  for ( double x = 0; x < WIDTH; x++) {
    for ( double y = 0; y < HEIGHT; y++) {
      processFrame(t, x, y);
    }
  }
}

// =====================================
//                Stars
//            © Marc Merlin
//               доробки
//    © SottNick © kostyamat, © kDn
//      Adaptation © SlingMaster
//                Зірки
// =====================================
void drawStar(float xlocl, float ylocl, float biggy, float little, int16_t points, float dangle, uint8_t koler) { // random multipoint star
  float radius2 = 255.0 / points;
  for (int i = 0; i < points; i++) {
    DrawLine(xlocl + ((little * (sin8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), ylocl + ((little * (cos8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), ColorFromPalette(*curPalette, koler));
    DrawLine(xlocl + ((little * (sin8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), ylocl + ((little * (cos8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), ColorFromPalette(*curPalette, koler));

  }
}

// --------------------------------------
void EffectStars() {
#define STARS_NUM (8U)
#define STAR_BLENDER (128U)
#define CENTER_DRIFT_SPEED (6U)
  static uint8_t spd;
  static uint8_t points[STARS_NUM];
  static float color[STARS_NUM] ;
  static int delay_arr[STARS_NUM];
  static float counter;
  static float driftx;
  static float  drifty;
  static float cangle;
  static float  sangle;
  static uint8_t stars_count;
  static uint8_t blur;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), random8(80U, 255U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    counter = 0.0;
    // стартуем с центра
    driftx = (float)WIDTH / 2.0;
    drifty = (float)HEIGHT / 2.0;

    cangle = (float)(sin8(random8(25, 220)) - 128.0f) / 128.0f; //angle of movement for the center of animation gives a float value between -1 and 1
    sangle = (float)(sin8(random8(25, 220)) - 128.0f) / 128.0f; //angle of movement for the center of animation in the y direction gives a float value between -1 and 1
    spd = modes[currentMode].Speed;
    blur = modes[currentMode].Scale / 2;
    stars_count = WIDTH / 2U;

    if (stars_count > STARS_NUM) stars_count = STARS_NUM;
    for (uint8_t num = 0; num < stars_count; num++) {
      points[num] = map(modes[currentMode].Scale, 1, 255, 3U, 7U); //5; // random8(3, 6);                              // количество углов в звезде
      delay_arr[num] = spd / 5 + (num << 2) + 2U;               // задержка следующего пуска звезды
      color[num] = random8();
    }
  }
  // fadeToBlackBy(leds, NUM_LEDS, 245);
  fadeToBlackBy(leds, NUM_LEDS, 165);
  float speedFactor = ((float)spd / 380.0 + 0.05);
  counter += speedFactor;                                                   // определяет то, с какой скоростью будет приближаться звезда

  if (driftx > (WIDTH - spirocenterX / 2U)) cangle = 0 - fabs(cangle);      //change directin of drift if you get near the right 1/4 of the screen
  if (driftx < spirocenterX / 2U) cangle = fabs(cangle);                    //change directin of drift if you get near the right 1/4 of the screen
  if ((uint16_t)counter % CENTER_DRIFT_SPEED == 0) driftx = driftx + (cangle * speedFactor); //move the x center every so often
  if (drifty > ( HEIGHT - spirocenterY / 2U)) sangle = 0 - fabs(sangle);    // if y gets too big, reverse
  if (drifty < spirocenterY / 2U) sangle = fabs(sangle);                    // if y gets too small reverse

  if ((uint16_t)counter % CENTER_DRIFT_SPEED == 0) drifty = drifty + (sangle * speedFactor); //move the y center every so often

  for (uint8_t num = 0; num < stars_count; num++) {
    if (counter >= delay_arr[num]) {              //(counter >= ringdelay)
      if (counter - delay_arr[num] <= WIDTH + 5) {
        drawStar(driftx, drifty, 2 * (counter - delay_arr[num]), (counter - delay_arr[num]), points[num], STAR_BLENDER + color[num], color[num]);
        color[num] += speedFactor;                // в зависимости от знака - направление вращения
      } else {
        delay_arr[num] = counter + (stars_count << 1) + 1U; // задержка следующего пуска звезды
      }
    }
  }
  blur2d(leds, WIDTH, HEIGHT, blur);
}
