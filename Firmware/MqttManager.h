#if USE_MQTT
/*
   Библиотека асинхронных MQTT запросов https://github.com/marvinroger/async-mqtt-client
   Не умеет автоматически восстанавливать разорванное соединение с MQTT брокером, поэтому требует периодической проверки подключения
   Зависит от библиотек:
     ESPAsyncTCP https://github.com/me-no-dev/ESPAsyncTCP
     AsyncTCP https://github.com/me-no-dev/AsyncTCP
   Лампа подписана на топик: LedLamp/LedLamp/xxxxxxxx/cmnd, где xxxxxxxx - ESP.getChipID(); payload - строка, содержащая те же команды, что отправляются приложением (регистр важен):
*/

#include <AsyncMqttClient.h>
#include "pgmspace.h"
#include "Constants.h"
#include "Types.h"

static const char TopicBase[]          PROGMEM = "IoT";                         // базова частина топіків
static const char TopicCmd[]           PROGMEM = "cmd";                         // частина командних топіків (іnput command)
static const char TopicState[]         PROGMEM = "state";                       // частина топіків стану (відповідь від пристрою)


// -------------------------------------
class MqttManager {
  public:
    static uint32_t mqttLastConnectingAttempt;
    static void setupMqtt(AsyncMqttClient* mqttClient, char* lampInputBuffer, SendCurrentDelegate sendCurrentDelegate);
    static void mqttConnect();
    static void onMqttConnect(bool sessionPresent);
    static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    static void onMqttMessage(char *topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    static bool publish(const char *topic, const char *value);
    static void publishState();
    static bool needToPublish;
    static char mqttBuffer[MAX_UDP_BUFFER_SIZE];
  private:
    static char* mqttServer;
    static char* mqttUser;
    static char* mqttPassword;

    static char* topicInput;                                                    // TopicBase + '/' + MqttClientIdPrefix + ESP.getChipId + '/' + TopicCmd
    static char* topicOutput;                                                   // TopicBase + '/' + MqttClientIdPrefix + ESP.getChipId + '/' + TopicState
    static char* clientId;
    static char* lampInputBuffer;                                               // ссылка на inputBuffer для записи в него пришедшей MQTT команды и последующей её обработки лампой
    static AsyncMqttClient* mqttClient;
    static const uint8_t qos = 0U;                                              // MQTT quality of service
    static const uint32_t connectionTimeout = MQTT_RECONNECT_TIME * 1000U;      // период времени для проверки (пере)подключения к MQTT брокеру, 10 секунд
    static char* byteToHex(char *out, uint8_t value);
    static bool allocStr(char **str, const char *src);
    static bool allocStr_P(char **str, PGM_P src);
    static SendCurrentDelegate sendCurrentDelegate;
};

// -------------------------------------
void MqttManager::setupMqtt(AsyncMqttClient* mqttClient, char* lampInputBuffer, SendCurrentDelegate sendCurrentDelegate) {
  allocStr_P(&MqttManager::mqttServer, MqttServer);
  allocStr_P(&MqttManager::mqttUser, MqttUser);
  allocStr_P(&MqttManager::mqttPassword, MqttPassword);

  MqttManager::mqttClient = mqttClient;
  MqttManager::lampInputBuffer = lampInputBuffer;
  MqttManager::sendCurrentDelegate = sendCurrentDelegate;
  MqttManager::mqttClient->setServer(MqttManager::mqttServer, MqttPort);

  char clientIdBuf[sizeof(MqttClientIdPrefix) + 8];
  strcpy_P(clientIdBuf, MqttClientIdPrefix);
  uint32_t chipId = ESP.getChipId();
  for (uint8_t i = 0; i < 4; ++i) {
    byteToHex(&clientIdBuf[i * 2 + sizeof(MqttClientIdPrefix) - 1], chipId >> ((3 - i) * 8));
  }
  allocStr(&clientId, clientIdBuf);
  MqttManager::mqttClient->setClientId(clientId);

  if (MqttManager::mqttUser != NULL) {
    MqttManager::mqttClient->setCredentials(MqttManager::mqttUser, MqttManager::mqttPassword);
  }
  // ----------
  uint8_t topicLength = sizeof(TopicBase) + 1 + strlen(clientId) + 1 + sizeof(TopicCmd) + 1;
  topicInput = (char*)malloc(topicLength);
  sprintf_P(topicInput, PSTR("%s/%s/%s"), TopicBase, clientId, TopicCmd);      // topicInput = TopicBase + '/' + MqttClientIdPrefix + ESP.getChipId + '/' + TopicCmd
  // ----------
  topicLength = sizeof(TopicBase) + 1 + strlen(clientId) + 1 + sizeof(TopicState) + 1;
  topicOutput = (char*)malloc(topicLength);
  sprintf_P(topicOutput, PSTR("%s/%s/%s"), TopicBase, clientId, TopicState);   // topicOutput = TopicBase + '/' + MqttClientIdPrefix + ESP.getChipId + '/' + TopicState
  // ----------

#ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR("MQTT топик для входящих команд: %s\n"), topicInput);
  LOG.printf_P(PSTR("MQTT топик для исходящих ответов лампы: %s\n"), topicOutput);
#endif

  mqttClient->onConnect(onMqttConnect);
  mqttClient->onDisconnect(onMqttDisconnect);
  mqttClient->onMessage(onMqttMessage);
}

// -------------------------------------
void MqttManager::mqttConnect() {
  if ((!mqttLastConnectingAttempt) || (millis() - mqttLastConnectingAttempt >= connectionTimeout)) {
#ifdef GENERAL_DEBUG
    LOG.print(F("Подключение к MQTT брокеру \""));
    LOG.print(MqttManager::mqttServer);
    LOG.print(':');
    LOG.print(MqttPort);
    LOG.println(F("\"..."));
#endif
    mqttClient->disconnect();
    mqttClient->connect();
    mqttLastConnectingAttempt = millis();
  }
}

// -------------------------------------
bool MqttManager::publish(const char *topic, const char *value) {
  if (mqttClient->connected()) {
#ifdef GENERAL_DEBUG
    LOG.print(F("Отправлено MQTT: топик \""));
    LOG.print(topic);
    LOG.print(F("\", значение \""));
    LOG.print(value);
    LOG.println('"');
    LOG.println();
#endif
    return mqttClient->publish(topic, qos, true, value, 0) != 0;
  }
  return false;
}

// -------------------------------------
void MqttManager::onMqttConnect(bool sessionPresent) {
#ifdef GENERAL_DEBUG
  LOG.println(F("Подключено к MQTT брокеру"));
#endif
  mqttLastConnectingAttempt = 0;
  mqttClient->subscribe(topicInput, 1);
  mqttClient->subscribe("IoT/discover", 1);
  publishState();
}

// -------------------------------------
void MqttManager::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
#ifdef GENERAL_DEBUG
  LOG.println(F("Отключено от MQTT брокера"));
#endif
}

// -------------------------------------
void MqttManager::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  if (payload != NULL) {                                     // сохраняем пришедшее MQTT сообщение для дальнейшей обработки
    strncpy(lampInputBuffer, payload, len);
    lampInputBuffer[len] = '\0';
    needToPublish = true;
  }

#ifdef GENERAL_DEBUG
  LOG.print(F("Получено MQTT:"));
  LOG.print(F(" топик \""));
  LOG.print(topic);
  LOG.print("\"");
  /*
    LOG.print(F(" qos: "));
    LOG.println(properties.qos);
    LOG.print(F(" dup: "));
    LOG.println(properties.dup);
    LOG.print(F(" retain: "));
    LOG.println(properties.retain);
    LOG.print(F(" len: "));
    LOG.println(len);
    LOG.print(F(" index: "));
    LOG.println(index);
    LOG.print(F(" total: "));
    LOG.println(total);
  */
  LOG.print(F(", значение \""));
  LOG.print(lampInputBuffer);
  LOG.println("\"");
  LOG.println();
#endif
}

// -------------------------------------
void MqttManager::publishState() {
  if (mqttBuffer == NULL || strlen(mqttBuffer) <= 0) {
    sendCurrentDelegate(mqttBuffer);                        // если буфер MQTT ответа не задан, но метод MQTT публикации вызван, закполняем его текущим состоянием лампы
  }
  if (mqttBuffer != NULL && strlen(mqttBuffer) > 0) {
    publish(topicOutput, mqttBuffer);                       // публикация буфера MQTT ответа
    mqttBuffer[0] = '\0';                                   // очистка буфера
    needToPublish = false;                                  // сброс флага для предотвращения повторной публикации
  }
}

// -------------------------------------
char* MqttManager::byteToHex(char *out, uint8_t value) {
  uint8_t b;
  b = value >> 4;
  if (b < 10)  {
    out[0] = '0' + b;
  } else {
    out[0] = 'A' + (b - 10);
  }
  b = value & 0x0F;
  if (b < 10) {
    out[1] = '0' + b;
  } else {
    out[1] = 'A' + (b - 10);
  }
  out[2] = '\0';
  return out;
}

// -------------------------------------
bool MqttManager::allocStr(char **str, const char *src) {
  if (src && *src) {
    if (*str) {
      void *ptr = realloc(*str, strlen(src) + 1);
      if (!ptr) {
        return false;
      }
      *str = (char*)ptr;
    } else {
      *str = (char*)malloc(strlen(src) + 1);
      if (!*str) {
        return false;
      }
    }
    strcpy(*str, src);
  } else {
    if (*str) {
      free(*str);
      *str = NULL;
    }
  }
  return true;
}

// -------------------------------------
bool MqttManager::allocStr_P(char **str, PGM_P src) {
  if (src && pgm_read_byte(src)) {
    if (*str) {
      void *ptr = realloc(*str, strlen_P(src) + 1);
      if (!ptr) {
        return false;
      }
      *str = (char*)ptr;
    } else {
      *str = (char*)malloc(strlen_P(src) + 1);
      if (!*str) {
        return false;
      }
    }
    strcpy_P(*str, src);
  } else {
    if (*str) {
      free(*str);
      *str = NULL;
    }
  }
  return true;
}

#endif
