// ===============================================================================================================
//ВНИМАНИЕ!!! Часть настроек перенесена в файл data/config и может изменяться в процессе эксплуатации лампы. Читайте файл ПРОЧТИ МЕНЯ!!!.txt
//ВНИМАНИЕ!!! Часть настроек перенесена в файл ConstantsUser (пользовательские, чаще всего меняемые для разных типов схем и матриц
//            там три раздела | МАТРИЦА | СХЕМОТЕХНИКА | ЭФФЕКТЫ |
//            в эффекты вынесено пока только теекст для бегущей строки в будущем туда нужно будет преместить и эффекты
//            и отдельный раздел в конце только для разработчиков)
// ===============================================================================================================

#pragma once
String configSetup = "{}";

// ============= НАСТРОЙКИ =============
#define USE_SECRET_COMMANDS                                 // удалите эту строку, если вам не нужна возможность смены режимов работы ESP_MODE и обнуления настроек из приложения
//                                                             список секретных команд тут: https://community.alexgyver.ru/goto/post?id=55780
// #define USE_BLYNK  ("сюда_вставить_токен_из_приложения") // раскомментируйте эту строку, если вы используете приложение Blynk (для iOS и Android) https://community.alexgyver.ru/goto/post?id=53535
// токен берут в приложении в "настройки -> DEVICES -> MY DEVICES -> AUTH TOKEN"
#define USE_SHUFFLE_FAVORITES                               // раскомментируйте эту строку, если вам нужно, чтобы режим Цикл показал каждый эффект по 1 разу перед перемешиванием (иначе просто случайный эффект),
// а также если у вас выбрано меньше десятка эффектов. кстати, если выбрано менее 2 эффектов, то демонстрироваться будут все эффекты по порядку без перемешивания

// --- КНОПКА --------------------------
//#define BUTTON_CHANGE_FAVORITES_MODES_ONLY                // Выберите чекбокс на web странице лампы, если хотите, чтобы кнопка переключала режимы только между теми, которые выбраны для режима Цикл (настраивается в приложении)
//Перенесено в файл data/config.json. Имя поля favorit      // иначе переключаться будут все существующие в лампе режимы по порядку (двойным кликом вперёд, тройным назад)
//Чекбокс "Кнопкой - только эффекты, выбранные в Цикле"

// --- ESP -----------------------------
#define ESP_CONF_TIMEOUT      (60U)                         // время в секундах, которое лампа будет ждать от вас введения пароля для ОТА обновления (пароль совпадает с паролем точки доступа)



// --- ESP (WiFi клиент) ---------------
// SSID и пароль Вашей WiFi-сети задаются на web странице лампы в режиме WiFi точки доступа по IP 192.168.4.1
// Там же задаётся время в секундах (таймаут), которое ESP будет пытаться подключиться к WiFi сети, после его истечения автоматически развернёт WiFi точку доступа
#define	INTERNET_CHECK_PERIOD (45U)                         // Период повторной проверки наличия интернета в секундах

// --- AP (WiFi точка доступа) ---------
String AP_NAME = "";                                        // Переменная для имени точки доступа. Задаётся на web странице;
String AP_PASS = "";                                        // Переменная для пароля точки доступа. Задаётся на web странице
String LAMP_NAME = "";                                      // Переменная для имени Лампы. Задаётся на web странице
const uint8_t AP_STATIC_IP[] = {192, 168, 4, 1};            // статический IP точки доступа (лучше не менять!)
uint8_t DONT_TURN_ON_AFTER_SHUTDOWN;                        // Не удаляйте и не комментируйте эту строку

// --- ВРЕМЯ ---------------------------
uint32_t AUTOMATIC_OFF_TIME (SLEEP_TIMER * 60UL * 60UL * 1000UL);  // Не удаляйте и не комментируйте эту строку
#define GET_TIME_FROM_PHONE (5U)                            // с этой строчкой время в лампе само синхронизируется с приложением, когда лампа не имеет или потеряла доступ в интернет на сервер точного времени .
// для этого нужно использовать приложение FireLamp версии 3.0 или выше, либо другое приложение, которое отправляет время телефона в лампу.
// цифра 5U означает, что синхранизация не чаще, чем раз в 5 минут. переход на зимнее время произойдёт только если изменение пришло со смартфона!
#define USE_MANUAL_TIME_SETTING                             // с этой строчкой у вас будет возможность устанавливать время на лампе из приложения вручную (например, когда лампа не имеет доступа в интернет)
// для этого в приложении в поле для текста бегущей строки нужно вписать "time=ЧЧ:ММ Д" в 24-часовом формате
// например, time=07:25 4  - означает, что время будет установлено на 7 часов 25 минут, четверг
// время установится в момент нажатия кнопки "НАЗАД" или "ОТПРАВИТЬ", секунды будут по нулям. лампа мигнёт голубым цветом при удачной установке
#define PHONE_N_MANUAL_TIME_PRIORITY                        // с этой строчкой, если время получено через приложение, то попытки синхронизации с NTP-сервером прекращаются (пригодится тем, у кого возникают проблемы с NTP-сервером)
#define WARNING_IF_NO_TIME      (7U)                        // с этой строчкой лампа будет подмигивать в нижнем ряде светодиодов, когда она не знает, сколько сейчас времени.
//                                                             7 - это яркость мигающих точек (максимум - 255U), когда лампа выключена
//#define WARNING_IF_NO_TIME_ON_EFFECTS_TOO                 // а если эту строку раскомментировать, то подмигивание будет даже во время работы эффектов. яркость точек будет, как у работающего эффекта


#define USE_NTP                                             // закомментировать или удалить эту строку, если нужно, чтобы лампа не обращалась в интернет на сервер времени (NTP-сервер).
//                                                             Стоит её убрать только в том случае, если в вашей домашней сети нет круглосуточного доступа в интернет.
//                                                             Лампу можно отправить в другой часовой пояс, так как часовой пояс, выставляется на web странице.
//                                                             там же чекбоксом выбирается необходимость перехода на летнее время.
char NTP_ADDRESS [32];                                      // Не удаляйте и не комментируйте эту строку

#define NTP_INTERVAL          (59 * 60UL * 1000UL)          // интервал синхронизации времени (59 минут)

// --- ВЫВОД ВРЕМЕНИ БЕГУЩЕЙ СТРОКОЙ ---
unsigned int NIGHT_HOURS_START;                             // Не удаляйте и не комментируйте эту строку
unsigned int NIGHT_HOURS_STOP;                              // Не удаляйте и не комментируйте эту строку
unsigned int DAY_HOURS_BRIGHTNESS;                          // Не удаляйте и не комментируйте эту строку
unsigned int NIGHT_HOURS_BRIGHTNESS;                        // Не удаляйте и не комментируйте эту строку
//                                                             константы DAY_HOURS_BRIGHTNESS и NIGHT_HOURS_BRIGHTNESS используются только, когда матрица выключена, иначе будет использована яркость текущего эффекта

// --- ЭФФЕКТЫ -------------------------
// приложение FireLamp версии 3.0 или выше,
//#define RANDOM_SETTINGS_IN_CYCLE_MODE     (1U)            // с этой строчкой в режиме Цикл эффекты будут включаться на случайных (но удачных) настройках Скорости и Масштаба
//Управляется на web странице лампы                         // настройки подбирались для лампы с матрицей 16х16 со стеклянным плафоном и калькой под ним. на других - не гарантируется
// Перенесено в файл data/config.json.                      // этот режим можно включать/выключать на web странице и секретной командой. чтобы после первой загрузки прошивки в плату он был выключен,
//                                                             поменяйте параметр random_on c 1 на 0 в файле config.json.

// ================ РЕЕСТР ДОСТУПНЫХ ЭФФЕКТОВ ===================
// ==== ДЛЯ ПЕРЕДАЧИ В ПРИЛОЖЕНИЯ С ПОДДЕРЖКОЙ ЭТОЙ ФУНКЦИИ =====
//       ГРУЗИТСЯ НЕПОСРЕДСТВЕННО С ФАЙЛОВОЙ СИСТЕМЫ ЛАМПЫ
// четыре файла effects1.json, effects2.json, effects3.json, effects4.json по 30 эффектов, последний не полный
// на текущий момент приложение требует доработки, загружаются только 3 файла
// WiFi Lamp Remote Control загружает весь список
// формат записи:
// Название эффекта,min_скорость,max_скорость,min_масштаб,max_масштаб,выбор_ли_цвета_это(0-нет,1-да 2-совмещённый);
// Порядок эффектов можно менять на своё усмотрение, не забывая при этом менять соответствие и в
// МАССИВЕ НАСТРОЕК ЭФФЕКТОВ ПО УМОЛЧАНИЮ (смотрите ниже)
// старый список состоящий из трех строк удален так как он нужен только для сторонних приложений и может быть загружен
// прямо с файловой системы в приложение

/* константы задержек смены кадров -------------------------------*/


#define DYNAMIC                (0U)   // динамическая задержка для кадров ( будет использоваться бегунок Скорость )
#define SOFT_DELAY             (1U)   // задержка для смены кадров FPSdelay задается програмно прямо в теле эффекта
#define LOW_DELAY             (15U)   // низкая фиксированная задержка для смены кадров
#define HIGH_DELAY            (50U)   // высокая фиксированная задержка для смены кадров

// ============= МАССИВ НАСТРОЕК ЭФФЕКТОВ ПО УМОЛЧАНИЮ ===========
static const uint8_t defaultSettings[][4] PROGMEM = {
  /* теперь задержка для смены кадров задается в defaultSettings четвертым параметром
     с помощью констант DYNAMIC | SOFT_DELAY  | LOW_DELAY | HIGH_DELAY |
    • формат записи:
    Яркость, Скорость, Масштаб, FPSdelay */
  {  10, 252,  32, HIGH_DELAY}, // Cмeнa цвeтa
  {  11,  33,  58, HIGH_DELAY}, // Бeзyмиe
  {   8,   4,  34, HIGH_DELAY}, // Oблaкa
  {   8,   9,  24, HIGH_DELAY}, // Лaвa
  {  11,  19,  59, HIGH_DELAY}, // Плaзмa
  {  11,  13,  60, HIGH_DELAY}, // Paдyгa 3D
  {  11,   5,  12, HIGH_DELAY}, // Пaвлин
  {   7,   8,  21, HIGH_DELAY}, // 3eбpa
  {   7,   8,  95, HIGH_DELAY}, // Лec
  {   7,   6,  12, HIGH_DELAY}, // Oкeaн
  {  24, 255,  26,  LOW_DELAY}, // Mячики
  {  18,  11,  70,  LOW_DELAY}, // Mячики бeз гpaниц
  {  19,  32,  16,  LOW_DELAY}, // Пoпкopн
  {   9,  46,   3,  LOW_DELAY}, // Cпиpaли
  {  17, 100,   2,  LOW_DELAY}, // Пpизмaтa
  {  12,  44,  17,  LOW_DELAY}, // Дымoвыe шaшки
  {  22,  53,   3,  LOW_DELAY}, // Плaмя
  {   9,  51,  11,  LOW_DELAY}, // Oгoнь 2021
  {  55, 127, 100,  LOW_DELAY}, // Tиxий oкeaн
  {  39,  77,   1,  LOW_DELAY}, // Teни
  {  15,  77,  95,  LOW_DELAY}, // ДHK
  {  15, 136,   4,  LOW_DELAY}, // Cтaя
  {  15, 122,  65,  LOW_DELAY}, // Cтaя и xищник
  {  11,  53,  87,  LOW_DELAY}, // Moтыльки
  {   7,  61, 100,  LOW_DELAY}, // Лaмпa c мoтылькaми
  {   9,  96,  31,  LOW_DELAY}, // 3мeйки
  {  19,  60,  20,  LOW_DELAY}, // Nexus
  {   9,  85,  85,  LOW_DELAY}, // Шapы
  {   7,  89,  83,  LOW_DELAY}, // Cинycoид
  {   7,  85,   3,  LOW_DELAY}, // Meтaбoлз

  /* bright, speed, scale, FPSdelay */

  {  12,  73,  38,  LOW_DELAY}, // Ceвepнoe cияниe
  {   8,  59,  18,  LOW_DELAY}, // Плaзмeннaя лaмпa
  {  23, 203,   1,  LOW_DELAY}, // Лaвoвaя лaмпa
  {  11,  63,   1,  LOW_DELAY}, // Жидкaя лaмпa
  {  11, 124,  39,  LOW_DELAY}, // Жидкaя лaмпa (auto)
  {  23,  71,  59,  LOW_DELAY}, // Kaпли нa cтeклe
  {  40,  70,  35, SOFT_DELAY}, // Строб.Хаос.Дифузия
  {   9, 225,  59,    DYNAMIC}, // Oгoнь 2012
  {  57, 225,  15,    DYNAMIC}, // Oгoнь 2018
  {   9, 220,  20,    DYNAMIC}, // Oгoнь 2020
  {   9, 240,   1,    DYNAMIC}, // Bиxpи плaмeни
  {   9, 240,  86,    DYNAMIC}, // Paзнoцвeтныe виxpи
  {   9, 198,  20,    DYNAMIC}, // Maгмa
  {   7, 240,  18,    DYNAMIC}, // Kипeниe
  {   5, 212,  54,    DYNAMIC}, // Boдoпaд
  {   7, 197,  22,    DYNAMIC}, // Boдoпaд 4 в 1
  {   8, 222,  63,    DYNAMIC}, // Бacceйн
  {  12, 185,   6,    DYNAMIC}, // Пyльc
  {  11, 185,  31,    DYNAMIC}, // Paдyжный пyльc
  {   9, 179,  11,    DYNAMIC}, // Бeлый пyльc
  {   8, 208, 100,    DYNAMIC}, // Ocциллятop
  {  15, 233,  77,    DYNAMIC}, // Иcтoчник
  {  19, 212,  44,    DYNAMIC}, // Фeя
  {  16, 220,  28,    DYNAMIC}, // Koмeтa
  {  14, 212,  69,    DYNAMIC}, // Oднoцвeтнaя кoмeтa
  {  27, 186,  19,    DYNAMIC}, // Двe кoмeты
  {  24, 186,   9,    DYNAMIC}, // Тpи кoмeты
  {  21, 203,  65,    DYNAMIC}, // Пpитяжeниe
  {  26, 206,  15,    DYNAMIC}, // Пapящий oгoнь
  {  26, 190,  15,    DYNAMIC}, // Bepxoвoй oгoнь

  /* bright, speed, scale, FPSdelay */

  {  12, 178,   1,    DYNAMIC}, // Paдyжный змeй
  {  16, 142,  63,    DYNAMIC}, // Koнфeтти
  {  25, 236,   4,    DYNAMIC}, // Mepцaниe
  {   9, 157, 100,    DYNAMIC}, // Дым
  {   9, 157,  30,    DYNAMIC}, // Paзнoцвeтный дым
  {   9, 189,  43,    DYNAMIC}, // Пикacco
  {   9, 236,  80,    DYNAMIC}, // Boлны
  {   9, 195,  80,    DYNAMIC}, // Цвeтныe дpaжe
  {  10, 222,  92,    DYNAMIC}, // Koдoвый зaмoк
  {  10, 231,  89,    DYNAMIC}, // Kyбик Pyбикa
  {  30, 233,   2,    DYNAMIC}, // Tyчкa в бaнкe
  {  20, 236,  10,    DYNAMIC}, // Гроза в банке
  {  15, 198,  99,    DYNAMIC}, // Ocaдки
  {  15, 225,   1,    DYNAMIC}, // Paзнoцвeтный дoждь
  {   9, 100,  60,    DYNAMIC}, // Cнeгoпaд       |  {"n":"Снігопад","v":[100,190,0,100,0]},
  {  20, 199,  54,    DYNAMIC}, // 3вeздoпaд / Meтeль
  {  24, 203,   5,    DYNAMIC}, // Пpыгyны
  {  15, 157,  23,    DYNAMIC}, // Cвeтлячки
  {  21, 198,  93,    DYNAMIC}, // Cвeтлячки co шлeйфoм
  {  14, 223,  40,    DYNAMIC}, // Люмeньep
  {  11, 236,   7,    DYNAMIC}, // Пeйнтбoл
  {   8, 196,  50,    DYNAMIC}, // Paдyгa
  { 180, 200,  10,    DYNAMIC}, // Вино
  {  80, 210,  50,    DYNAMIC}, // Завиток
  {  50, 231,   1,    DYNAMIC}, // Моя краïна Украïна
  {  55, 125,  50,    DYNAMIC}, // Масляные Краски
  {  30, 220,  65,    DYNAMIC}, // Акварель
  {  25, 150,  50,    DYNAMIC}, // Реки Ботсваны
  // • ------------------------------- •   • ----------- data для json файла ----------- •
  /* рекомендуется сохранять сюда данные из json файла на случай если захотите вернуть
    еффект ( не придется опять подбирать параметры )
    ниже пример для эффекта «Блуждающий кубик», */
  //  {  15, 150,  50,    DYNAMIC}, // Блуждающий кубик  |  {"n":"Блуждающий кубик","v":[30,200,1,100,0]},
  // • --------------------------------------------------------------------------------- •
  {  55, 220,  18,    DYNAMIC}, // Свеча
  {  50, 215, 100,    DYNAMIC}, // Песочные Часы
  {  10, 175,  60,    DYNAMIC}, // Kонтакти
  {  10, 215,  50,    DYNAMIC}, // Радіальна хвиля
  {  22,  35,  50,    DYNAMIC}, // Вогонь з іскрами   |  {"n":"Вогонь з іскрами","v":[20,100,1,100,0]},
  {  10, 255,  70,    DYNAMIC}, // Spectrum
  {  55, 150,  70,    DYNAMIC}, // Цветок лотоса
  {  48,  90,  50,    DYNAMIC}, // Новогодняя Елка
  {  45, 150,  30,    DYNAMIC}, // Побочный Эффект
  {  80,  50,   0, SOFT_DELAY}, // Фейерверк          |  {"n":"Фейерверк","v":[10,245,10,90,1]},
  {  10, 128,  75, SOFT_DELAY}, // Планета Земля      |  {"n":"Планета Земля","v":[128,128,10,90,0]},
  {  22, 128,  50, SOFT_DELAY}, // Мечта Дизайнера    |  {"n":"Мечта Дизайнера","v":[0,255,1,100,0]},
  {  55, 128,  25, SOFT_DELAY}, // Цветные кудри      |  {"n":"Цветные кудри","v":[128,128,10,90,0]},
  {   8, 160,  65,    DYNAMIC}, // Краплі на воді     |  {"n":"Краплі на воді","v":[120,215,1,100,1]},
  {  45, 128,  60,    DYNAMIC}, // Чарівний Ліхтар    |  {"n":"Чарівний Ліхтарик","v":[1,128,1,100,1]}, Magic Lantern
  // !!! последние эффекты имеют постоянную прописку
  // никогда не перемещайте их по списку, остальные эффекты
  // можно размещать в любой последовательности.
  {  27, 186,  23,    DYNAMIC}, // Maтpицa
  {  22, 225,   1,    DYNAMIC}, // Oгoнь Intim
  {   9, 207,  50, HIGH_DELAY}, // Бeлый cвeт
  {   9, 180,  99, HIGH_DELAY}, // Цвeт
#ifdef USE_TIME_EFFECT
  {   4,   5, 100,    DYNAMIC}, //                    |  {"n":"Чacы","v":[1,245,1,100,1]},
#endif
  {  10, 156,  38,    DYNAMIC}  // Бeгyщaя cтpoкa | всегда должна быть последней
};/*                          ^-- проверьте, чтобы у последняя строка не была без запятой после скобки */
/* ========================== КОНЕЦ МАССИВА ===================== */

#define MODE_AMOUNT (sizeof(defaultSettings)/4)             // количество эффектов высчисляется автоматически на основе defaultSettings
//                                                             следите за тем чтобы размер defaultSettings совпадал с массив указателей *FuncEff и соблюдайте порядок в списке
//                                                             не забывайте указывать в коментарии имя эффекта (*FuncEff находится в файле effectTicker.ino)
/* фиксированные номера эффектов  --------------------------------*/
#ifdef USE_TIME_EFFECT
#define EFF_MATRIX             (MODE_AMOUNT - 6)            // Maтpицa
#else
#define EFF_MATRIX             (MODE_AMOUNT - 5)            // Maтpицa
#endif
#define EFF_WHITE_COLOR        (EFF_MATRIX + 2)             // Бeлый cвeт

// =====================================
// === ОСТАЛЬНОЕ ДЛЯ РАЗРАБОТЧИКОВ =====
// =====================================
#define BRIGHTNESS            (40U)                         // стандартная маскимальная яркость (0-255). используется только в момент включения питания лампы

#if defined (ESP_USE_BUTTON)
#define BUTTON_STEP_TIMEOUT   (100U)                        // каждые BUTTON_STEP_TIMEOUT мс будет генерироваться событие удержания кнопки (для регулировки яркости)
#define BUTTON_CLICK_TIMEOUT  (500U)                        // максимальное время между нажатиями кнопки в мс, до достижения которого считается серия последовательных нажатий
#if (BUTTON_IS_SENSORY == 1U)
#define BUTTON_SET_DEBOUNCE   (20U)                         // Время антидребезга mS для сенсорной кнопки
#else
#define BUTTON_SET_DEBOUNCE   (55U)                         // Время антидребезга mS для механической кнопки
#endif
#endif
#define ESP_RESET_ON_START    (false)                       // true - если при старте нажата кнопка (или кнопки нет!), сохранённые настройки будут сброшены; false - не будут
#define ESP_HTTP_PORT         (80U)                         // номер порта, который будет использоваться во время первой утановки имени WiFi сети (и пароля), к которой потом будет подключаться лампа в режиме WiFi клиента (лучше не менять)
#define ESP_UDP_PORT          (8888U)                       // номер порта, который будет "слушать" UDP сервер во время работы лампы как в режиме WiFi точки доступа, так и в режиме WiFi клиента (лучше не менять)
#define WIFIMAN_DEBUG         (false)                       // вывод отладочных сообщений при подключении к WiFi сети: true - выводятся, false - не выводятся; настройка не зависит от GENERAL_DEBUG

#define RANDOM_SETTINGS_IN_CYCLE_MODE                       // Строка добавлена для совместимости файлов эффектов с версией  86 в 1 (или уже больше) от Сотнег.                                                             
// этот режим можно включать/выключать на web странице лампы или "секретной" командой. Не удаляйте и не комментируйте эту строку.
#define USE_RANDOM_SETS_IN_APP                              // Строка добавлена для совместимости файлов эффектов с версией  86 в 1 (или уже больше) от Сотнег.                               
// Этот режим уже поддерживается приложением от Котейка под Андроид. Не удаляйте и не комментируйте эту строку.
#define SUMMER_HOUR           (3U)                          // час (по зимнему времени!), когда заканчивается зимнее время и начинается летнее; [0..23]
#define SUMMER_WEEK_NUM       (week_t::Last)                // номер недели в месяце, когда происходит переход на летнее время (возможные варианты: First - первая, Second - вторая, Third - третья, Fourth - четвёртая, Last - последняя)
#define SUMMER_WEEKDAY        (dow_t::Sun)                  // день недели, когда происходит переход на летнее время (возможные варианты: Mon - пн, Tue - вт, Wed - ср, Thu - чт, Sat - сб, Sun - вс)
#define SUMMER_MONTH          (month_t::Mar)                // месяц, в котором происходит переход на летнее время (возможные варианты: Jan - январь, Feb - февраль, Mar - март, Apr - апрель, May - май, Jun - июнь, Jul - июль, Aug - август, Sep - сентябрь, Oct - октябрь, Nov - ноябрь, Dec - декабрь)
#define SUMMER_TIMEZONE_NAME  ("EEST")                      // обозначение летнего времени; до 5 символов; может быть использовано, если понадобится его вывести после вывода времени; может быть "ЛЕТ"
#define WINTER_HOUR           (4U)                          // час (по летнему времени!), когда заканчивается летнее время и начинается зимнее; [0..23]
#define WINTER_WEEK_NUM       (week_t::Last)                // номер недели в месяце, когда происходит переход на зимнее время (возможные варианты: First - первая, Second - вторая, Third - третья, Fourth - четвёртая, Last - последняя)
#define WINTER_WEEKDAY        (dow_t::Sun)                  // день недели, когда происходит переход на зимнее время (возможные варианты: Mon - пн, Tue - вт, Wed - ср, Thu - чт, Sat - сб, Sun - вс)
#define WINTER_MONTH          (month_t::Oct)                // месяц, в котором происходит переход на зимнее время (возможные варианты: Jan - январь, Feb - февраль, Mar - март, Apr - апрель, May - май, Jun - июнь, Jul - июль, Aug - август, Sep - сентябрь, Oct - октябрь, Nov - ноябрь, Dec - декабрь)
#define WINTER_TIMEZONE_NAME  ("EET")                       // обозначение зимнего времени; до 5 символов; может быть использовано, если понадобится его вывести после вывода времени; может быть "ЗИМ"

// --- ВНЕШНЕЕ УПРАВЛЕНИЕ --------------
#define USE_MQTT              (false)                       // true - используется mqtt клиент, false - нет
#if USE_MQTT
#define MQTT_RECONNECT_TIME   (10U)                         // время в секундах перед подключением к MQTT брокеру в случае потери подключения
#endif

// --- РАССВЕТ -------------------------
#define DAWN_BRIGHT           (200U)                        // максимальная яркость рассвета (0-255)
uint8_t DAWN_TIMEOUT;                                       // сколько рассвет светит после времени будильника, минут. Может быть изменено в установках будильника

//#define MAX_UDP_BUFFER_SIZE (UDP_TX_PACKET_MAX_SIZE + 1)
// максимальный размер буффера UDP сервера
// 255 - это максимальное значение, при котором работа с збранным не будет глючить
// для исходящих сообщений в приложение данное ограничение можно обойти (см. как реализована отправка "LIST"),
// а для входящего списка избранного - хз. пришлось увеличить до максимально возможножного значения.
// дальше придётся переделывать типы (размеры) переменных в функциях FavoritesManager.h
#define MAX_UDP_BUFFER_SIZE   (255U)                        // максимальный размер буффера UDP сервера



// Remote control ------- | не лізь бо вб'є |
// комманды дистанционного управления по HTTP
#define CMD_STATE             (0U)
#define CMD_POWER             (1U)
#define CMD_NEXT_EFF          (2U)
#define CMD_PREV_EFF          (3U)
#define CMD_BRIGHT_UP         (4U)
#define CMD_BRIGHT_DW         (5U)
#define CMD_SPEED             (6U)
#define CMD_SCALE             (7U)
#define CMD_AUTO              (8U)
#define CMD_TEXT              (9U)
#define CMD_INFO              (10U)
#define CMD_WHITE             (11U)
#define CMD_FAV               (12U)
#define CMD_RESET             (13U)
#define CMD_RESET_EFF         (14U)
#define CMD_SHOW_EFF          (15U)
#define CMD_DEFAULT           (16U)
#define CMD_RANDOM            (17U)
// #define CMD_FAVORITES         (18U)
#define CMD_INTIM             (20U)
#define CMD_OTA               (21U)
#define CMD_IP                (22U)
#define CMD_TEST_MATRIX       (23U)
// #define CMD_PC_STATE          (25U)
#define CMD_SCAN              (32U)
#define CMD_ECHO              (33U)
#define CMD_GROUP_INIT        (35U)
#define CMD_GROUP_DESTROY     (36U)
#define CMD_FS_DIR            (40U)
#define CMD_DEL_FILE          (41U)
#define CMD_LIST              (55U)
#define CMD_CONFIG            (60U)
#define CMD_SAVE_CFG          (61U)
#define CMD_SAVE_ALARMS       (62U)
#define CMD_CUSTOM_EFF        (66U)
#define CMD_FW_INFO           (69U)
#define CMD_DIAGNOSTIC        (90U)
#define CMD_ACTIVATE          (95U)
#define CMD_EFF_JAVELIN       (99U)
// -------------------------------------
String VERSION = "4.0 " + String(MODE_AMOUNT);