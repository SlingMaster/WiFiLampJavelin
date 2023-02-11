// javelin.ino  ------
uint32_t level_timeout;

// =====================================
void Camouflage(uint8_t theme) {
  if (theme > 2) {
    theme = 1;
  }
  const uint32_t dataColors[3][5] = {
    { 0x022401, 0x0F1401, 0x030402, 0x010200, 0x031600 },  // ліс
    { 0x020134, 0x030114, 0x020203, 0x050F18, 0x000616 },  // море
    { 0x211802, 0x040401, 0x050500, 0x080702, 0x161508 }   // пустеля
  };

  uint8_t STEP = 2;
  uint32_t color;
  delay(250);
  FastLED.clear();
  for (uint8_t x = 0; x < WIDTH / STEP; x++) {
    for (uint8_t y = 0; y < HEIGHT / STEP; y++) {
      color = dataColors[theme][random8(5)];
      for (uint8_t xx = 0; xx < STEP; xx++) {
        for (uint8_t yy = 0; yy < STEP; yy++) {
          leds[XY(x * STEP + xx, y * STEP + yy + 1)] = color;
        }
      }
    }
  }

  blur2d(leds, WIDTH, HEIGHT, 64U);
  if ((HEIGHT >= 28) && (WIDTH >= 10))  {
    drawLogo(dataColors[theme][0]);
  }
}

// ======================================
void JavelinStatic(uint8_t val) {
#ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR("JavelinStatic : %02d |\n\r"), val);
#endif
  switch (val) {
    case 0: // close ----
#ifdef JAVELIN
      JavelinLight(0x000000, 0x000000, 0x000000);
      digitalWrite(MB_LED_PIN, HIGH);
#endif
      lendLease = false;
      runEffect();
      ONflag = true;
      break;
    case 1: // open -----
      lendLease = true;
      ONflag = false;
      step = 0;
      Camouflage(camouflage);
#ifdef JAVELIN
      DrawLevel(0, ROUND_MATRIX, ROUND_MATRIX, CHSV {0, 0, 0});
#endif
      FastLED.setBrightness(250);
      FastLED.show();
      break;
    case 2: // fire -----
      if (step == 0) {
        // restore lamp state ---
        if (lendLease == false) {
          ONflag = false;
          FastLED.setBrightness(250);
          Camouflage(camouflage);
        }
#ifdef JAVELIN
        digitalWrite(MB_LED_PIN, LOW);
#endif
        lendLease = true;
        FPSdelay = 5U;
        ONflag = true;
        step = 4;
      }
      break;

    case 3: // done diagnostic -----
#ifdef JAVELIN
      diagnostic = false;
      digitalWrite(MB_LED_PIN, HIGH);
#endif
      Camouflage(0);
      lendLease = true;
      ONflag = false;
      FPSdelay = 5U;
      step = 0;
      FastLED.setBrightness(250);
      FastLED.show();
      break;
  }
}

// ======================================
void JavelinDiagnostic(uint8_t val) {
  if (val > 100) {
    return;
  }
#ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR("JavelinDiagnostic : %02d |\n\r"), val);
#endif
  CHSV color = CHSV(2.5 * (val + 1), 255, 255 );
  switch (val) {
    case 0:
      FastLED.setBrightness(250);
      JavelinStatic(1);
#ifdef JAVELIN
      digitalWrite(MB_LED_PIN, LOW);
#endif
      break;
    case 10:
#ifdef JAVELIN
      digitalWrite(OTA_PIN, LOW);
#endif
      break;
    case 20:
#ifdef JAVELIN
      digitalWrite(OTA_PIN, HIGH);
#endif
      Camouflage(0);
      break;
    case 30:
      Camouflage(1);
      break;
    case 40:
      Camouflage(2);
      break;
    case 50:
      FastLED.clear();
      gradientHorizontal(CENTER_X_MINOR - 1, HEIGHT - 8, CENTER_X_MAJOR + 2, HEIGHT - 7, 150U, 150U, 100U, 28U, 255U);
      gradientHorizontal(CENTER_X_MINOR - 1, HEIGHT - 9, CENTER_X_MAJOR + 2, HEIGHT - 8, 50U, 50U, 28U, 100U, 255U);
      hue = 0;
      break;
    case 60:
      break;
    case 70:
      break;
    case 80:
      break;
    case 90:
      break;
    case 95:
      JavelinStatic(1);
      break;
    case 100:
#ifdef JAVELIN
      digitalWrite(MB_LED_PIN, HIGH);
      DrawLevel(0, 35, 35, CHSV{180, 255, MATRIX_LEVEL});
#endif
      FastLED.show();
      delay(2000);
      // clear indicator ---
      ONflag = false;
#ifdef JAVELIN
      JavelinLight(0x000000, 0x000000, 0x000000);
      DrawLevel(0, 35, 35, CHSV{0, 255, 0});
#endif
      FastLED.show();
      return;
  }

#ifdef JAVELIN
  JavelinLight(color, color, color);
  DrawLevel(0, floor(val / 2.85), 35, CHSV{180, 255, MATRIX_LEVEL});
#endif
  ColoritShow(val);
  FastLED.show();
}

// ======================================
void ColoritShow(uint8_t val) {
  CHSV color1 = CHSV(250, 255, 100);
  CHSV color2 = CHSV(250, 255, 32);
  if (val < 50) {
    return;
  }
  uint8_t y = val - 49;
  if (y >= HEIGHT) {
    if (val < 95) {
      //  VerticalScroll(1);
      dimAll(128);
    }
    return;
  }
  if ((y < HEIGHT - 11) | (y > HEIGHT - 6)) {
    if (val % 2) {
      leds[XY(CENTER_X_MINOR, y)] = color2;
      leds[XY(CENTER_X_MAJOR, y)] = color2;
      leds[XY(CENTER_X_MINOR - 1, y)] = color2;
      leds[XY(CENTER_X_MAJOR + 1, y)] = color2;
    } else {
      leds[XY(CENTER_X_MINOR, y)] = color1;
      leds[XY(CENTER_X_MAJOR, y)] = color1;
      leds[XY(CENTER_X_MINOR - 2, y)] = color2;
      leds[XY(CENTER_X_MAJOR + 2, y)] = color2;
    }
  } else {
    hue += 16U;
    gradientHorizontal(CENTER_X_MINOR - 2, HEIGHT - 8, CENTER_X_MAJOR + 3, HEIGHT - 6, 150U, 150U, 100 + hue, 28 + hue, 255U);
    gradientHorizontal(CENTER_X_MINOR - 2, HEIGHT - 10, CENTER_X_MAJOR + 3, HEIGHT - 8, 50U, 50U, 28 + hue, 100 + hue, 255U);

  }
}

// =====================================
void drawLogo( uint32_t ink) {
  static const bool data[28][7] = {
    // N
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 1, 1, 1, 0 },
    { 0, 1, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 0 },
    // I
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 0 },
    // L
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 0 },
    // E
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0, 1, 0 },
    { 0, 1, 0, 1, 0, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 0 },
    // V
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 1, 0 },
    { 0, 1, 1, 1, 1, 0, 0 },
    // A
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 0, 1, 0, 0, 0 },
    { 0, 0, 1, 1, 1, 1, 0 },
    // J
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 }
  };
  uint8_t posX = CENTER_Y_MINOR - 10;
  uint8_t posY = floor(HEIGHT * 0.5) + 13;
  uint32_t color;
  for (uint8_t y = 0; y < 28; y++) {
    for (uint8_t x = 0; x < 7; x++) {
      color = (data[y][x]) ? ink : 0x000000;
      leds[XY(posX + x, posY - y)] = color;
    }
  }
}


// ======================================
void Javelin() {
  if (step > 3) {
    if (step < 128) {
      for (uint8_t x = 0; x < WIDTH; (x += 2)) {
        leds[XY(x, 0)] = CHSV(random8(10), 255, 255 - step * 2);
      }
#ifdef JAVELIN
      if (step == 4) {
        JavelinLight(0xFF0000, 0xFF0000, 0xFF0000);
      }
      if (step == 20) {
        JavelinLight(0x000000, 0x7F5F00, 0x000000);
      }
#endif
    }
    if (step > 48) {
#ifdef JAVELIN
      JavelinLight(0x000000, 0x000000, 0x000000);
#else
      for (uint8_t x = 0; x < WIDTH; (x++)) {
        leds[XY(x, HEIGHT - 1)] = CHSV(255, 255, ((step >= 127) ? 255 - step * 2 : step * 2) );
      }
#endif
#ifdef JAVELIN
      for (uint8_t x = 0; x < ROUND_MATRIX; (x++)) {
        leds[NUM_LEDS + x] = CHSV(255, 255, ((step >= 127) ? 255 - step * 2 : step * 2) );
      }
#endif
    }
    step += 4;
    if (step >= 250) {
      step = 0;
#ifdef JAVELIN
      digitalWrite(MB_LED_PIN, HIGH);
#endif
      FPSdelay = 254U;
    }
  }
}


// ======================================
#ifdef  JAVELIN
uint8_t br_led;
uint8_t sp_led;
uint8_t sc_led;



// ======================================
void DrawLevel(uint8_t startPos, uint8_t val, uint8_t maxVal, CHSV color) {
  uint16_t index;
  index = NUM_LEDS + startPos;
  for (int x = 0; x < maxVal; x++) {
    leds[index + x] = val <= x ? CHSV{0, 255, 0} : color;
  }
}

// ======================================
void ClearLevelIndicator() {
  uint16_t index;
  index = NUM_LEDS;
  for (int x = 0; x < ROUND_MATRIX; x++) {
    leds[NUM_LEDS + x] = CHSV{0, 255, 0};
  }
}

// ======================================
//void VerticalScroll(byte direct) {
//  if (direct == 0U) {
//    return;
//  }
//  for (uint8_t x = 0U; x < WIDTH; x++) {
//    for (uint8_t y = HEIGHT; y > 0U; y--) {
//      if (direct == 1) {
//        // scroll up --------------
//        leds[XY( x, y)] = getPixColorXY(x, y - 1U);
//      }
//      if (direct == 2) {
//        // scroll down  -----------
//        leds[XY( x, HEIGHT - y)] = getPixColorXY(x, HEIGHT - y + 1U);
//      }
//    }
//  }
//}

// ======================================
void javelinConnect(uint8_t color) {
  progress++;
#ifdef GENERAL_DEBUG
  // LOG.printf_P(PSTR("• javelinConnec | espMode • %d | %d \n\r"), espMode, progress);
#endif

  uint8_t br = 64U + 32 * progress;
  u8_t color_ntp;
#ifdef USE_NTP
  // error ntp ------------------
  color_ntp = 255;        // если при включенном NTP время не получено, будем красным цветом мигать
#else
  color_ntp = 176U;       // иначе скромно синим - нормальная ситуация при отсутствии NTP
#endif // USE_NTP
  if (progress >= 100) {
    DrawLevel(0, 0, ROUND_MATRIX, CHSV{0, 255, 190});
    return;
  }
  if (pcnt > 50) {
    if (progress >= ROUND_MATRIX) {
      leds[NUM_LEDS + ROUND_MATRIX - 1] = CHSV{color_ntp, 255, 0};
      progress = 0;
    }
    leds[NUM_LEDS + progress - 1] = CHSV{color_ntp, 255, 0};
    leds[NUM_LEDS + progress] = CHSV{color_ntp, 255, 255};
    loadingFlag = false;
    return;
  } else {
    pcnt++;
  }
  leds[NUM_LEDS + 7 + progress] = CHSV{color, 255, br};
  leds[NUM_LEDS + 8 - progress] = CHSV{color, 255, br};
  leds[NUM_LEDS + 18 + progress] = CHSV{color, 255, br};
  leds[NUM_LEDS + 19 - progress] = CHSV{color, 255, br};
  leds[NUM_LEDS + 29 + progress] = CHSV{color, 255, br};
  leds[NUM_LEDS + 30 - progress] = CHSV{color, 255, br};
  if (progress > 5) {
    progress = 0;
    DrawLevel(0, 0, ROUND_MATRIX, CHSV{0, 255, 190});
  }
}

// =====================================
void buttonJavelinTick() {
  touchJavelin.tick();
  uint8_t clickCount = touchJavelin.hasClicks() ? touchJavelin.getClicks() : 0U;

  // кнопка только начала удерживаться
  if (touchJavelin.isHolded()) {
    progress = 0;
    // LOG.printf_P("touchJavelin | isHolded");
  }

  // кнопка нажата и удерживается
  if (touchJavelin.isStep() | diagnostic) {
    int8_t but = touchJavelin.getHoldClicks();
#ifdef GENERAL_DEBUG
    // LOG.printf_P(PSTR("• Progress : %03d | %02d\n\r"), progress, but);
#endif
    // 1 click | start diagnostic =======
    JavelinDiagnostic(progress);
    if (progress == 110) {
      diagnostic = false;
      JavelinStatic(0);
    } else {
      delay((diagnostic ? 150 : 10));
      progress++;
    }
  }

  if (clickCount >= 1U) {
#ifdef GENERAL_DEBUG
    LOG.printf_P(PSTR("Button Javelin | click Count: %d\n\r"), clickCount);
#endif
  }

  // 1 click | fire ===================
  if (clickCount == 1U) {
    JavelinStatic(1);
    delay(200);
    JavelinStatic(2);
  }
  // 2 click | stop diagnostic ========
  if (clickCount == 2U) {
    JavelinDiagnostic(100);
    progress = 0;
    JavelinStatic(0);
  }

  // 3 click | work group =============
  if (clickCount == 3U) {
    if (ONflag) {
      WORKGROUP = (WORKGROUP == 1) ? 0 : 1;
      CRGB val = (WORKGROUP == 1) ? 0xFF00FF : 0x00FFFF;
      JavelinLight(val, val, val);
      DrawLevel(0, ROUND_MATRIX, ROUND_MATRIX, CHSV((WORKGROUP ? 192U : 128U), 255U, 255U));
      FastLED.show();
      delay(250U);
    }
  }
  // 4 click | set default camouflage =
  if (clickCount == 4U) {
    camouflage++;
    if (camouflage > 2) {
      camouflage = 0;
    }
    JavelinStatic(1);
    jsonWrite(configSetup, "camouflage", camouflage);
    delay(1000U);
    JavelinStatic(0);
  }
  // 5 click | reset default effects ==
  if (clickCount == 5U) {
    ResetDefaultEffects();
  }
}


//#ifdef JAVELIN
// ======================================
void JavelinLight(CRGB val1, CRGB val2, CRGB val3) {
  /* left */
  leds[ NUM_LEDS + ROUND_MATRIX] = val1;
  /* center */
  leds[ NUM_LEDS + ROUND_MATRIX + 1] = val2;
  /* right */
  leds[ NUM_LEDS + ROUND_MATRIX + 2] = val3;
}

// ======================================
uint8_t NormalizeBrightness() {
  int bri = constrain(200 - FastLED.getBrightness(), 48, 200);
  //  LOG.printf_P(PSTR("Brightness : %03d | REAL • %03d | BR • %d\n\r"), modes[currentMode].Brightness, FastLED.getBrightness(), bri);
  return bri;
}

// ======================================
void ShowStateIndicator(uint8_t br_level) {
  uint8_t bar_size = 10;
  static bool flag;

  if ((millis() - level_timeout >= 500U)) {
    level_timeout = millis();
    flag = !flag;
  }

  // ROUND_MATRIX -----
  if (eff_auto) {
    leds[NUM_LEDS] = flag ? 0x000000 : 0x7F007F;;
    leds[NUM_LEDS + 2] = !flag ? 0x000000 : 0x7F007F;;
  } else {
    leds[NUM_LEDS] = CHSV{0, 0, 0};
    leds[NUM_LEDS + 2] = CHSV{0, 0, 0};
  }

  // MARCKERS ---------
  if ((flag) & (WORKGROUP == 1U)) {
    leds[NUM_LEDS + 1] = CHSV{0, 205U, br_level * 0.7};
    leds[NUM_LEDS + bar_size + 3] = CHSV{172, 205U, br_level * 0.7};
    leds[NUM_LEDS + bar_size * 2 + 4] = CHSV{96, 205U, br_level * 0.7};
  } else {
    leds[NUM_LEDS + 1] = CHSV{0, 0, br_level * 0.7};
    leds[NUM_LEDS + bar_size + 3] = CHSV{0, 0, br_level * 0.7};
    leds[NUM_LEDS + bar_size * 2 + 4] = CHSV{0, 0, br_level * 0.7};
  }
  JavelinLight(0xB0A000, 0x000000, 0x001FFF);
}

// ======================================
void StateLampIndicator() {
  if (lendLease) {
    return;
  };
  uint8_t DELAY;
  uint8_t val;
  uint8_t bar_size = 10;  // max levels for speed | brightness | scale
  uint8_t br_level = NormalizeBrightness();
  //---------------------------------------

  DELAY = map(modes[currentMode].Speed, 30, 254, 1U, 10U);
  // speed ------
  val = floor((256 - FPSdelay) * bar_size / 255);
  if (((level_timeout % DELAY) == 0U) & (sp_led != val)) {
    if (sp_led < val) {
      sp_led++;
    } else {
      sp_led--;
    }
    //  LOG.printf_P(PSTR("     Speed : %03d | %02d\n\r"), modes[currentMode].Speed, val);
  }
  DrawLevel(3, sp_led, bar_size, CHSV{90, 255, br_level});
  // brightness --
  if (gb) {
    val = global_br * bar_size / 127;
  } else {
    val = floor(FastLED.getBrightness() * bar_size / 255);
  }

  if (((level_timeout % DELAY) == 0U) & (br_led != val)) {
    if (br_led < val) {
      br_led++;
    } else {
      br_led--;
    }
  }
  DrawLevel(bar_size + 4, br_led + 1, bar_size, CHSV{60, 200, br_level});
  // scale -------
  val = floor(modes[currentMode].Scale * bar_size / 100);
  if (((level_timeout % DELAY) == 0U) & (sc_led != val)) {
    if (sc_led < val) {
      sc_led++;
    } else {
      sc_led--;
    }
    //  LOG.printf_P(PSTR("     Scale : %03d | %02d\n\r"), modes[currentMode].Scale, val);
  }

  DrawLevel(bar_size * 2 + 5, sc_led, bar_size, CHSV{160, 255, br_level});
  ShowStateIndicator(br_level);
  level_timeout ++;
}

#endif
