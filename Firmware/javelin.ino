// javelin.ino
#ifdef  JAVELIN

// =====================================

// ======================================
void DrawLevel(uint8_t startPos, uint8_t val, uint8_t maxVal, CHSV color) {
  uint16_t index;
  index = NUM_LEDS + startPos;
  for (int x = 0; x < maxVal; x++) {
    leds[index + x] = val <= x ? CHSV{0, 255, 0} : color;
  }
}

// --------------------------------------
void javelinConnect(uint8_t color) {
  progress++;
  LOG.printf_P(PSTR("• noTimeWarning | espMode • %d | %d \n\r"), espMode, progress);
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
    if (progress == 110) {
      progress = 0;
    }
    LOG.printf_P("touchJavelin | isHolded");
  }

  // кнопка нажата и удерживается
  if (touchJavelin.isStep()) {
    int8_t but = touchJavelin.getHoldClicks();
#ifdef GENERAL_DEBUG
    // LOG.printf_P(PSTR("• Progress : %03d | %02d\n\r"), progress, but);
#endif

    JavelinDiagnostic(progress);
    if (progress == 110) {
      JavelinStatic(0);
      printMSG("Test Passed", false);
    } else {
      delay(10);
      progress++;
    }
  }

  if (clickCount >= 1U) {
#ifdef GENERAL_DEBUG
    LOG.printf_P(PSTR("Button Javelin | click Count: %d\n\r"), clickCount);
#endif
  }

  // 1 click | start diagnostic =======
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
}
#endif

// =====================================
void drawLogo() {
  //  static const uint32_t colors[5] = { 0x326408, 0x334406, 0x1d230f, 0x010800, 0x235400 };
  static const uint32_t colors[5] = { 0x022401, 0x081401, 0x030402, 0x010200, 0x031600 };
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
      color = (data[y][x]) ? colors[1] : 0x000000;
      leds[XY(posX + x, posY - y)] = color;
    }
  }
}

// =====================================
void Camouflage(uint8_t theme) {
  //  static const uint32_t colors[5] = { 0xb3b488, 0x53643f, 0x4d432f, 0x010101, 0x53643f };
  //  static const uint32_t colors[5] = { 0x326408, 0x334406, 0x1d230f, 0x010800, 0x235400 };
  //  static const uint32_t colors[5] = { 0x124402, 0x132401, 0x080307, 0x010400, 0x081600 };
  static const uint32_t dataColors[3][5] = {
    { 0x022401, 0x081401, 0x030402, 0x010200, 0x031600 },  // ліс
    { 0x074777, 0x0316b0, 0x020409, 0x79757c, 0x000102 },  // море
    { 0x022401, 0x081401, 0x030402, 0x010200, 0x031600 }   // пустеля
  };
  static const uint32_t colors[5] =  { 0x022401, 0x081401, 0x030402, 0x010200, 0x031600 };

  static uint8_t STEP = 3;
  static uint32_t color;
  for (uint8_t x = 0; x < WIDTH / STEP; x++) {
    for (uint8_t y = 0; y < HEIGHT / STEP; y++) {
      color = dataColors[theme][random8(5)];
      uint32_t color2 = colors[random8(5)];
      for (uint8_t xx = 0; xx < STEP; xx++) {
        for (uint8_t yy = 0; yy < STEP; yy++) {
          leds[XY(x * STEP + xx, y * STEP + yy + 1)] = color;
        }
      }
    }
  }
  blur2d(leds, WIDTH, HEIGHT, 128);
  if ((HEIGHT >= 28) && (WIDTH >= 10))  {
    drawLogo();
  }
}


// ======================================
void Javelin() {
  //  if (step < 3) {
  //    return;
  //  }
  if (step > 3) {
    if (step < 128) {
      for (uint8_t x = 0; x < WIDTH; (x += 2)) {
        leds[XY(x, 0)] = CHSV(random8(10), 255, 255 - step * 2);
      }
      if (step == 4) {
        JavelinLight(0x00FF0000, 0x00FF0000, 0x00FF0000);
      }
      if (step == 20) {
        JavelinLight(0x00000000, 0x00000000, 0x00000000);
      }
    }
    if (step > 32) {
      for (uint8_t x = 0; x < WIDTH; (x++)) {
        leds[XY(x, HEIGHT - 1)] = CHSV(255, 255, ((step >= 127) ? 255 - step * 2 : step * 2) );
      }
      for (uint8_t x = 0; x < ROUND_MATRIX; (x++)) {
        leds[NUM_LEDS + x] = CHSV(255, 255, ((step >= 127) ? 255 - step * 2 : step * 2) );
      }
    }
    step += 4;
    if (step >= 250) {
      step = 0;
      digitalWrite(MB_LED_PIN, HIGH);
      FPSdelay = 254U;
    }
  }
}

// ======================================
void JavelinStatic(uint8_t val) {
  LOG.printf_P(PSTR("JavelinStatic : %02d |\n\r"), val);
  switch (val) {
    case 0: // close ----
      JavelinLight(0x00000000, 0x00000000, 0x00000000);
      runEffect();
      ONflag = true;
      break;
    case 1: // open -----
      ONflag = false;
      step = 0;
      FastLED.setBrightness(250);

      FastLED.clear();
      Camouflage(0);
      FastLED.show();
      break;
    case 2: // fire -----
      if (step == 0) {
        digitalWrite(MB_LED_PIN, LOW);
        lendLease = true;
        FPSdelay = 1U;
        ONflag = true;
        step = 4;
      }
      break;
  }
}

#ifdef JAVELIN
// ======================================
void JavelinLight(uint32_t val1, uint32_t val2, uint32_t val3) {
  /* left */
  leds[ NUM_LEDS + ROUND_MATRIX] = val1;
  /* center */
  leds[ NUM_LEDS + ROUND_MATRIX + 1] = val2;
  /* right */
  leds[ NUM_LEDS + ROUND_MATRIX + 2] = val3;
}

// ======================================
void JavelinDiagnostic(uint8_t val) {
  if (val > 100) {
    return;
  }
#ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR("JavelinDiagnostic : %02d |\n\r"), val);
#endif

  switch (val) {
    case 0:
      FastLED.clear();
      digitalWrite(MB_LED_PIN, LOW);
      //
      //      ONflag = false; // off
      //      changePower();
      return;
    case 10:
      JavelinStatic(1);
      //      ONflag = true; // on
      //      changePower();
      return;
    case 20:
      digitalWrite(OTA_PIN, LOW);
      break;
    case 30:
      digitalWrite(OTA_PIN, HIGH);
      break;
    case 40:
      JavelinLight(0x00707000, 0x00000000, 0x00003F7F);
      Camouflage(1);
      break;
    case 50:

      break;
    case 60:
      Camouflage(2);
      break;
    case 70:

      break;
    case 80:

      break;
    case 90:

      break;
    case 100:
      digitalWrite(MB_LED_PIN, HIGH);
      FastLED.clear();
      Camouflage(0);
      JavelinLight(0x00000000, 0x00000000, 0x00000000);
      FastLED.show();
      delay(1000);
      return;
  }

  DrawLevel(0, floor(val / 2.85), 35, CHSV{180, 255, MATRIX_LEVEL});
  FastLED.show();
}

// ======================================
uint8_t NormalizeBrightness() {
  int br = constrain(200 - FastLED.getBrightness(), 48, 200);
  //  LOG.printf_P(PSTR("Brightness : %03d | REAL • %03d | BR • %d\n\r"), modes[currentMode].Brightness, FastLED.getBrightness(), br);
  return br;
}


// ======================================
void StateLampIndicator() {
  uint8_t val;
  uint8_t bar_size = 10;  // max levels for speed | brightness | scale
  uint8_t br_level = NormalizeBrightness();
  //---------------------------------------
  if (lendLease) {
    return;
  };

  // speed -------
  val = floor(modes[currentMode].Speed * bar_size / 255);
  //  LOG.printf_P(PSTR("     Speed : %03d | %02d\n\r"), modes[currentMode].Speed, val);
  DrawLevel(3, val, bar_size, CHSV{90, 255, br_level});

  // brightness --
  // val = floor(modes[currentMode].Brightness * bar_size / 255);
  val = floor(FastLED.getBrightness() * bar_size / 255);
  // LOG.printf_P(PSTR("Brightness : %03d | %02d • %03d\n\r"), modes[currentMode].Brightness, val, br_level);
  DrawLevel(bar_size + 4, val + 1, bar_size, CHSV{32, 255, br_level});

  // scale -------
  val = floor(modes[currentMode].Scale * bar_size / 100);
  //  LOG.printf_P(PSTR("     Scale : %03d | %02d\n\r"), modes[currentMode].Scale, val);
  DrawLevel(bar_size * 2 + 5, val, bar_size, CHSV{160, 255, br_level});
}

#endif
