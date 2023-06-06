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
void drawNote(CRGB color) {
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
  FastLED.clear();
  for (uint8_t y = 0U; y < 9; y++) {
    for (uint8_t x = 0U; x < 6; x++) {
      drawPixelXY(posX + x, posY - y, (data[y][x]) ? color : CRGB(0, 0, 0));
    }
  }
  FastLED.show();
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

    /* SPECTR */
    case SPECTR_DEF:
    case SPECTR_SCROL:
      visSpectrum(id_cmd, packetBuffer);
      break;
    case SPECTR_SCROL_V:
      visSpectrumV(id_cmd, packetBuffer);
      break;

    /* ABSTRACT */
    case ABSTRACT_1:
      visHornyPeacock(id_cmd, packetBuffer);
      break;
    case ABSTRACT_2:
      visColoredBlots(id_cmd, packetBuffer);
      break;
    case ABSTRACT_3:
      visDeveloper(id_cmd, packetBuffer);
      break;
    case ABSTRACT_DEV:
      visDeveloper(id_cmd, packetBuffer);
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
    byte val = (HEIGHT / 100.0) * packetBuffer[(x + 6)] * 1.25;
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
          hue = 148U + y;                     // color bar
          hue2 = (y > cur_val) ? 0 : 220 + y;    // brightness 110 + y * STEP
          break;
        case BAR_SPECTR:
          // hue = x * 255 / (WIDTH / 2) + abs(y - HEIGHT / 2) + step;
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
void visSpectrumV(byte id_cmd, char *packetBuffer) {
  dimAll(72U);
  deltaHue ++;
  for (uint8_t y = 0U; y < HEIGHT; y++) {
    byte posY = y;
    float val = ((WIDTH / 200.0) * packetBuffer[(y + 6)]) * 1.25;
    /* color    */ hue = 80 + y * 255 / (HEIGHT) - deltaHue;
    /* saturate */ deltaHue2 = 255U;
    DrawLineF(CENTER_X_MINOR - val, y, CENTER_X_MINOR + val, y,  CHSV(hue, deltaHue2, 255U));
  }
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

// ======================================
/*   Кольорові плями | Colored blots */
// ======================================
void visDeveloper(byte id_cmd, char *packetBuffer) {
  if (hue > 30) {
    hue = 0;
  }
  //  &PartyColors_p,
  //  &OceanColors_p,
  //  &LavaColors_p,
  //  &HeatColors_p,
  //  &WaterfallColors_p,
  //  &CloudColors_p,
  //  &ForestColors_p,
  //  &RainbowColors_p,
  //  &RainbowStripeColors_p

  currentPalette = RubidiumFireColors_p;
  currentPalette[1] = CRGB::Black;
  currentPalette[4] = CRGB::White;
  currentPalette[12] = CRGB::White;
  scale = map(packetBuffer[(hue + 6)], 0, 100, 1U, 30U) ;
  speed = map(packetBuffer[(hue + 7)], 0, 100, 1U, 20U) ;
  colorLoop = 0;
  hue++;

  fillNoiseLED();
  ihue += 1;
}

// ======================================


#endif
