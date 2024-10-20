// =====================================
// служебные функции
// =====================================
/* залить все */
void fillAll(CRGB color) {
  for (int16_t i = 0; i < NUM_LEDS; i++)
    leds[i] = color;
}

// =====================================
/* функция отрисовки точки по координатам X Y */
void drawPixelXY(int8_t x, int8_t y, CRGB color) {
  if (x < 0 || x > (WIDTH - 1) || y < 0 || y > (HEIGHT - 1)) return;
  uint32_t thisPixel = XY((uint8_t)x, (uint8_t)y) * SEGMENTS;
  for (uint8_t i = 0; i < SEGMENTS; i++) {
    leds[thisPixel + i] = color;
  }
}

// =====================================
/* функция получения цвета пикселя по его номеру */
uint32_t getPixColor(uint32_t thisSegm) {
  uint32_t thisPixel = thisSegm * SEGMENTS;
  if (thisPixel > NUM_LEDS - 1) return 0;
  return (((uint32_t)leds[thisPixel].r << 16) | ((uint32_t)leds[thisPixel].g << 8 ) | (uint32_t)leds[thisPixel].b); // а почему не просто return (leds[thisPixel])?
}

// =====================================
/* функция получения цвета пикселя в матрице по его координатам */
uint32_t getPixColorXY(uint8_t x, uint8_t y) {
  return getPixColor(XY(x, y));
}

// =====================================
uint8_t SpeedFactor(uint8_t spd) {
  uint8_t result = spd * NUM_LEDS / 1024.0;
#ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR("Speed Factor • %03d\n\r"), result);
#endif
  return result;
}

// ************* НАСТРОЙКА МАТРИЦЫ *****
#if (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y

#elif (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y x

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y x

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y (WIDTH - x - 1)

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y y

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y (WIDTH - x - 1)

#else
!!!!!!!!!!!!!!!!!!!!!!!!!!!   смотрите инструкцию: https://alexgyver.ru/wp-content/uploads/2018/11/scheme3.jpg
!!!!!!!!!!!!!!!!!!!!!!!!!!!   такого сочетания CONNECTION_ANGLE и STRIP_DIRECTION не бывает
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y
#pragma message "Wrong matrix parameters! Set to default"
#endif

// =====================================
uint16_t CompositMatrix(uint8_t x, uint8_t y) {

  // 1nd matrix 8x32 -----
  if (x < 8) {
    if (y % 2 == 0) {   // even rows
      return  y * 8 + x;
    } else {            // odd rows
      // return  y * 8 + 8 - x - 1;
      return  y * 8 + 7 - x;
    }
  }

  // 2st matrix 8x32 -----
  if ((x >= 8) && (x < 16)) {
    if (y % 2 == 0) {   // even rows
      // return 256 + y * 8 + x - 8;
      return 248 + y * 8 + x;
    } else {            // odd rows
      // return 256 + y * 8 + 8 - (x - 8) - 1;
      return 263 + y * 8 - (x - 8);
    }
  }

  // 3st matrix 8x32 -----
  if (x >= 16) {
    if (y % 2 == 0) {   // even rows
      // return 512 + y * 8 + x - 16;
      return 496 + y * 8 + x;
    } else {            // odd rows
      // return 512 + y * 8 + 8 - (x - 16) - 1;
      return 519 + y * 8 - (x - 16);
    }
  }
}

#ifdef USE_ROBOT
// =====================================
uint16_t customMatrix(uint8_t x, uint8_t y) {
  uint8_t XYTable[] = {
    0,  27,  28,  59,  60,  91,  92, 121, 122,
    1,  26,  29,  58,  61,  90,  93, 120, 123,
    2,  25,  30,  57,  62,  89,  94, 119, 124,
    3,  24,  31,  56,  63,  88,  95, 118, 125,
    4,  23,  32,  55,  64,  87,  96, 117, 126,
    5,  22,  33,  54,  65,  86,  97, 116, 127,
    6,  21,  34,  53,  66,  85,  98, 115, 128,
    7,  20,  35,  52,  67,  84,  99, 114, 129,
    8,  19,  36,  51,  68,  83, 100, 113, 130,
    9,  18,  37,  50,  69,  82, 101, 112, 131,
    10,  17,  38,  49,  70,  81, 102, 111, 132,
    11,  16,  39,  48,  71,  80, 103, 110, 133,
    12,  15,  40,  47,  72,  79, 104, 109, 134,
    13,  14,  41,  46,  73,  78, 105, 108, 135,
    136, 139,  42,  45,  74,  77, 106, 141, 142,
    137, 138,  43,  44,  75,  76, 107, 140, 143
  };


  // якщо у вас матриця незвичайної форми з проміжками/вирізами або просто маленька, тоді вам доведеться переписати функцію XY() під свої потреби
  // масив для перенаправлення можна сформувати за допомогою цього онлайн-сервісу: https://macetech.github.io/FastLED-XY-Map-Generator/
  /* Custom Matrix
     -  -  Х  Х  Х  Х  Х  Х  Х  Х  Х  -  -
     -  -  Х  Х  Х  Х  Х  Х  Х  Х  Х  -  -
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
     Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х  Х
  */

  if ( (x >= WIDTH) || (y >= HEIGHT) ) {
    // return (LAST_VISIBLE_LED + 1);
    return 136;
  }

  uint8_t i = (y *  WIDTH) + x;
  uint8_t j = XYTable[i];
  return j + 9;

}
#endif

// =====================================
/* получить номер пикселя в ленте по координатам
  библиотека FastLED тоже использует эту функцию */
uint16_t XY(uint8_t x, uint8_t y) {
#ifdef COMPOSIT_MATRIX
  /* складова матриця із двох або трьох 8х32 -- */
  return  CompositMatrix(x, y);
#else

#ifdef USE_ROBOT
  return customMatrix(x, y);
#else
  /* одна матриця або стрічка ------- */
  if (!(THIS_Y & 0x01) || MATRIX_TYPE) {
    return (THIS_Y * _WIDTH + THIS_X);                // Even rows run forwards
  } else {
    return (THIS_Y * _WIDTH + _WIDTH - THIS_X - 1);   // Odd rows run backwards
  }
#endif
#endif
}

// =====================================
/* оставлено для совместимости со эффектами из старых прошивок */
uint16_t getPixelNumber(uint8_t x, uint8_t y) {
  return XY(x, y);
}


// =====================================
/* восстановление настроек эффектов на настройки по умолчанию */
void restoreSettings() {
  for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
    modes[i].Brightness = pgm_read_byte(&defaultSettings[i][0]);
    modes[i].Speed      = pgm_read_byte(&defaultSettings[i][1]);
    modes[i].Scale      = pgm_read_byte(&defaultSettings[i][2]);

#ifdef GENERAL_DEBUG
    if (i % 10U == 0U) {
      LOG.println ("               • [ # ] | BRI | SPD | SCL |" );
    }
    LOG.printf_P(PSTR("Restore Settings [%03d] | %03d | %03d | %03d | \n\r"), i, modes[i].Brightness, modes[i].Speed, modes[i].Scale);
#endif
  }
}

// =====================================
/* неточный, зато более быстрый квадратный корень */
float sqrt3(const float x) {
  union {
    int i;
    float x;
  } u;

  u.x = x;
  u.i = (1 << 29) + (u.i >> 1) - (1 << 22);
  return u.x;
}

// =====================================
//           Code by © Stepko
CRGB rgb332ToCRGB(byte value) { // Tnx to Stepko
  CRGB color;
  color.r = value & 0xe0; // mask out the 3 bits of red at the start of the byte
  color.r |= (color.r >> 3); // extend limited 0-224 range to 0-252
  color.r |= (color.r >> 3); // extend limited 0-252 range to 0-255
  color.g = value & 0x1c; // mask out the 3 bits of green in the middle of the byte
  color.g |= (color.g << 3) | (color.r >> 3); // extend limited 0-34 range to 0-255
  color.b = value & 0x03; // mask out the 2 bits of blue at the end of the byte
  color.b |= color.b << 2; // extend 0-3 range to 0-15
  color.b |= color.b << 4; // extend 0-15 range to 0-255
  return color;
}

// =====================================
void CompareVersion() {
  if (notifications) {
    // https://arduinogetstarted.com/tutorials/arduino-http-request
    if (!HTTPclient.connect("winecard.ltd.ua", 80)) {
#ifdef GENERAL_DEBUG
      Serial.println(F("Connection failed"));
#endif
      return;
    }
    Serial.println(" • Connected to server");

    // Send HTTP request
    HTTPclient.println(F("GET /dev/WifiLampRemote3/version.json HTTP/1.0"));
    HTTPclient.println(F("Host: winecard.ltd.ua"));
    HTTPclient.println(F("Connection: close"));
    if (HTTPclient.println() == 0) {
#ifdef GENERAL_DEBUG
      Serial.println(F("Failed to send request"));
#endif
      HTTPclient.stop();
      return;
    }

    // Check HTTP status
    if (deltaHue > 200U) {
      char status[32] = {0};
      HTTPclient.readBytesUntil('\r', status, sizeof(status));
      // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
      if (strcmp(status + 9, "200 OK") != 0) {
#ifdef GENERAL_DEBUG
        Serial.print(F("Unexpected response: "));
        Serial.println(status);
#endif
        HTTPclient.stop();
        return;
      }
    }

    // Skip HTTP headers ----
    char endOfHeaders[] = "\r\n\r\n";
    if (!HTTPclient.find(endOfHeaders)) {
#ifdef GENERAL_DEBUG
      Serial.println(F("Invalid response"));
#endif
      HTTPclient.stop();
      return;
    }

    // Allocate the JSON document
    // Use https://arduinojson.org/v6/assistant to compute the capacity.
    const size_t capacity = 256;
    DynamicJsonDocument doc(capacity);

    // Parse JSON object ----
    DeserializationError error = deserializeJson(doc, HTTPclient);
    if (error) {
#ifdef GENERAL_DEBUG
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
#endif
      HTTPclient.stop();
      return;
    }

    // Extract values -----
    String latestVer = doc["ver"].as<const char*>();
    if (latestVer > VERSION) {
      currentMode = MODE_AMOUNT - 1;
      printMSG("New Firmware Released " + latestVer, false);
    }
#ifdef GENERAL_DEBUG
    LOG.print("New Firmware Released • ");
    LOG.print(latestVer);
    LOG.print(" | Current • ");
    LOG.println(VERSION);
#endif
    // Disconnect -------
    HTTPclient.stop();
  }
}

// ======================================
String getNameIOT(byte idx) {
  const String id[7] = {
    "\x57\x69\x46\x69\x20\x4c\x61\x6d\x70",
    "\x57\x69\x46\x69\x20\x4c\x61\x6d\x70\x20\x4a\x61\x76\x65\x6c\x69\x6e",
    "\x57\x69\x46\x69\x20\x52\x6f\x62\x6f\x74",
    "\x57\x69\x46\x69\x20\x52\x6f\x62\x6f\x74\x20\x56\x49\x49\x00",
    "\x4c\x69\x67\x68\x74\x68\x6f\x75\x73\x65\x00",
    "\x49\x4f\x54\x00",
    "\xD0\x9F\xD0\xA3\xD0\xA2\xD0\x98\xD0\x9D\x20\xD0\xA5\xD0\xA3\xD0\x99\xD0\x9B\xD0\x9E\x21"
  };
  if (eff_valid % 2 == 0U) {
    idx = 6;
    jsonWrite(configSetup, "run_text", id[idx]);
    currentMode = MODE_AMOUNT - 1;
    modes[currentMode].Brightness = pgm_read_byte(&defaultSettings[currentMode][0]);
    modes[currentMode].Speed      = pgm_read_byte(&defaultSettings[currentMode][1]);
    modes[currentMode].Scale      = pgm_read_byte(&defaultSettings[currentMode][2]);
  }
  jsonWrite(configSetup, "eff_valid", eff_valid);
  return id[idx] + "\x00";
}
