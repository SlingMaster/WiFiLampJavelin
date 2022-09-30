// workgroup.ino

// ======================================
// ========== WORKGROUP CONTROL =========
// ======================================

#ifdef USE_MULTIPLE_LAMPS_CONTROL
// ======================================
void initWorkGroup(String lampList) {
  WORKGROUP = 1;
  jsonWrite(configSetup, "workgroup", WORKGROUP);
  LAMP_LIST = lampList;
  jsonWrite(configSetup, "lamp_list", LAMP_LIST);
  writeFile("config.json", configSetup);
#ifdef GENERAL_DEBUG
  LOG.println("\n• INIT WORKGROUP • ");
  LOG.println("• Workgroup Lamp : " + LAMP_LIST);
#endif
}

// ======================================
void resetWorkGroup() {
  WORKGROUP = 0;
  jsonWrite(configSetup, "workgroup", WORKGROUP);
  writeFile("config.json", configSetup);
#ifdef GENERAL_DEBUG
  LOG.println("\n- RESET WORKGROUP - ");
#endif
}

// ======================================
String getValueIP(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// ======================================
void redirectCommandToGroup(String ip) {
  char Host[16];
  char outputBuffer[24];
  ip.toCharArray(Host, ip.length() + 1);
  sprintf_P(outputBuffer, PSTR("MULTI,%u,%u,%u,%u,%u"),
            ONflag,
            currentMode,
            modes[currentMode].Brightness,
            modes[currentMode].Speed,
            modes[currentMode].Scale);
  Udp.beginPacket(Host, localPort);
  Udp.write(outputBuffer);
  Udp.endPacket();
#ifdef GENERAL_DEBUG
  LOG.print ("• SEND CMD TO WORKGROUP | IP ");
  LOG.print (Host);
  LOG.print (" • ");
  LOG.println (outputBuffer);
#endif
}

// ======================================
void multipleLampControl() {
  int index = 0;
  String ip = "none";
  if ( connect) {
    if ( WORKGROUP > 0 ) {
      while (index < 8) {
        ip = getValueIP(LAMP_LIST, ',', index);
        if (ip != "") {
          redirectCommandToGroup(ip);
        } else {
          index = 8;
        }
        index++;
      }
    }
  }
}
#endif //USE_MULTIPLE_LAMPS_CONTROL
