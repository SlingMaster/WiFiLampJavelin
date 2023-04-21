# WiFiLamp «Javelin»
(UA) Прошивка для WiFi лампи «Javelin» та кросплатформне програмне забезпечення для керування лампами
---
(EN) Firmware for WiFi Lamp «Javelin» and cross-platform lamp management software
---

## (EN) 
This is a further development of the WiFi Lamp Remote Control project,
I have added some features to the design of the lamp to give it a unique touch.
Unfortunately, this was caused by the aggression of the Russian Federation against Ukraine, this situation affected the software, it is focused on the inhabitants of Ukraine, and the entire civilized world that supported us, with the exception of the russian federation.
They have no forgiveness for the grief they brought to my country, my beloved city and home.

The firmware can be standard or extended (Javelin).
If you want to replicate my lamp design with something of your own, it's a creative process that helps me survive the war.
While the firmware is only standard, over time I will post drawings and diagrams, as soon as I fully debug the circuit and software, unfortunately the war breaks the usual way of life.
You can follow the development process on my YouTube channel SlingMasterJSC

I would like to thank everyone who has supported us during this difficult time. Happy use.
If anyone wants to support the project
## Donations to support the Armed Forces of Ukraine
can be sent directly from the [web page](https://bank.gov.ua/ua/about/support-the-armed-forces) (multi-currency account)

## (UA) 
Це подальший розвиток проекту WiFi Lamp Remote Control,
я зробив деякі особливості в конструкцію лампи, щоб надати їй унікальні риси.
На жаль, це було викликано агресією мордору проти України, ця ситуація позначилася на програмному забезпеченні, воно орієнтоване на жителів України, і всього цивілізованого світу, який нас підтримав за винятком московії.
Їм немає прощення за те горе, яке вони принесли до моєї країни, мого улюбленого міста та мого дому.

Прошивка може бути стандартною або розширеною (Javelin).
Якщо захочете повторити мою конструкцію лампи, додавши щось своє, це творчий процес який допомагає мені пережити війну.
Поки прошивка тільки стандартна, з часом я викладу креслення та схеми, як тільки повністю налагоджу схему та ПЗ, на жаль війна ламає звичний спосіб життя.
процес розробки можна відстежувати на моєму каналі YouTube SlingMasterJSC

Висловлюю подяку всім, хто нас підтримав у важкий для нас час. Приємного використання.
Якщо у когось виникне бажання заохотити проект
## Донати для підтримки Збройних Сил України
можна надіслати прямо з [web сторінки](https://bank.gov.ua/ua/about/support-the-armed-forces) (рахунок мульти валютний)



---
# Firmware WiFiLamp «Javelin»
# Версія 5.XX 
---
# Можливості прошивки
	• Прошивка для лампи з оригінальною конструкцією «Javelin», сумісна зі схемою gunner47.
	• Робота в режимі WiFi точки доступу або в якості WiFi клієнта для роутера.
	• Вбудована HTML сторінка для першого підключення до WiFi мережі.
	• Авто сканування доступних ламп в мережі.
	• можливість тонкої настройки під свої потреби з урахуванням конструкції лампи:
		а) конструктивні особливості: файл ConstantsUser.h
		b) функціональні вподобання : файл button.ino (закоментовані альтернативи поведінки, які ви можете налаштувати на свій смак).
	• Управління матрицями та стрічками на базі адресних світлодіодів WS 2812B, можливі варіанти:
	  led стрічка до 1024 led, 
	  матриці: 8x8, 8x32, 16x16
	  a також композитні: 
	  b) дві матриці 16х16
	  с) три матриці 16х16 в висоту, 
	  d) чотири матриці 16х16 (потребують доробки функції XY())
	• Підтримка кругового індикатор стану та механічної кнопки для оригінальної конструкції лампи «Javelin».
	• Робота в режимі WiFi точки доступу або в якості WiFi клієнта для роутера.
	• Зовнішне управління лампою з допомогою протоколів HTTP, MQTT, UDP 
	  формат даних json {"cmd":number, "val":namber, "valStr":string}, ключі "val" та "valStr" не обов'язкові для більшості команд.
	  коди команд для лампи знаходяться в файлі Constants.h
	• Рандомне налаштування ефектів.  
	• Автозміна ефектів. 
	• Будильник «Світанок».
	• Управління групою ламп, в складі роботи з програмним забезпеченням WiFi Lamp Remote Control в один клік (до 7 ламп).
	• Таймер вимкнення лампи при тривалому використанні (за замовчкванням 3 години).
	• Таймер вимкнення лампи через 5 хвилин.
	• Підтримка файлових систем SPIFFS та LittleFS	(за замовчуванням SPIFFS).
	• Двомовний список ефектів (український, англійський, в складі роботи з програмним забезпеченням WiFi Lamp Remote Control).
	• OTA (підтримка безпровіного оновлення.
	• Тест Led матриці для контролю вірного підключення та конфігурації.
	• Режим «Javelin» імітація пострілу, та діагнрстики.
	• Управління лампою від сенсорної кнопки та механічної кнопки для оригінальної конструкції. 
	• Cповіщення про вихід оновлення прошивки. 
	


## Дії сенсорної кнопки для цієї прошивки:
	• одноразовий клік – увімкнення або вимкнення світильника;
	• дворазовий клік – наступний ефект;
	• триразовий клік – попередній ефект;
	• чотириразовий клік – запуск відтворення ефеків у циклі;
	• п'ятиразовий клік – запуск ефекту вогонь (інтимна обстановка);
	• шестиразовий клік – запуск таймера вимкнення лампи через 5 хвилин;
	• семиразовий клік – зміна робочого режиму лампи: з WiFi точки доступу на WiFi клієнт чи навпаки;
	• утримання – збільшення чи зменшення "яскравості";
	• одноразовий клік та утримання – збільшення чи зменшення «швидкості»;
	• дворазовий клік та утримання – збільшення чи зменшення «масштабу»;
	• утримання при вимкненій лампі – включає ефект «Біле Світло»;
	
## Дії механічної кнопки для цієї прошивки (оригінальна конструкція Javelin):	
	• одноразовий клік – команда для лампи «Вогонь»
	• утримання - тест лампи «Javelin»
	• дворазовий клік - вихід з режиму «Тест»
	• триразовий клік – управління групою (обовязково потрібно хоч один раз ввімкнути цей режим з додатку під любу платформу, щоб сформувався список доступних лампб тілбки для ламп «Javelin»)
	• чотириразовий клік - зміна камуфляжу лампи за замовчуванням (3 теми по черзі ліс, зима, пустеля)
	• п'ятиразовий клік - скинути налаштунки ефектів за замовчуванням
## Круговий індикатор стану лампи
	• зелена секторальна анімація при старті - підключення до роутера
	• синя секторальна анімація при старті - робота в режимі точки доступу
	• червона анімація по колу - на лампі не встановлений час
	• 3 сектори по 10 led по часовій стрільці (швидкість, яскравість, масштаб)
	• 2 мигаючі фіолетові світлодіоди в тиловій частині кругового індикатора - режим «Автозміна Ефектів»
	• 3 свілодіоди (red, blue, green) між секторами - режим управління групою

---
* у файлі button.ino є закоментовані альтернативи поведінки, які ви можете налаштувати на свій смак.
* (будьте уважні як що використовуєте MOSFET його змінено з (5U D1) на (15U D8) якщо вам потрібно можете повернути назад.

---
# Можливості програмного забезпечення

---
### WiFi Lamp Javelin Remote
1. Кросплатформне керування налаштуванням лампи та режимами по WiFi.
	* Управління лампою коли вона в режимі точки доступу тільки під Windows і Android (SDK 23 - 30 | Android 6 - Android 11).
2. Об'єднання в групу та керування групою ламп в один клік.
3. Зручний інтерфейс програми орієнтований як для простих користувачів і розробників.
4. Статистика використання програмного забезпечення та відображення локації на google map. Программа LampRemoteStats.exe, або прямо з програми WiFiLamp Remote, треба тіки натиснути іконку в нижній частині UI програми під заголовком «Developer Tools»
5. Двомовний інтерфасе (український, англійський)
### !!! Будьте уважні.
### Лампа потребує активації, для цього необхідно запустити програмне забезпечення на вибір: 
1. RemoteControlForWindows\WiFiLampRemote.exe, 
2. Android додаток RemoteControlForAndroid,
3. Або в браузері [за посиланням](http://winecard.ltd.ua/dev/WifiLampRemote3/index.html?ip=192.168.1.1&dev=1&timeout=200&sound=1)
 * для всіх користувачів програмне забезпечення безкоштовне, активація проходить автоматично після натискання кнопки «Слава Україні» крім московії, причина я дусаю вам відоиа.

---
### WiFi Lamp Projector
1. Для переходу до WiFi Lamp Projector треба натиснути помаранчовий квадрат в caption програми
2. Зображення для відтворення повинні бути в папці UI/ani/, щоб потрапити в папку треба тапнути по логотипу WiFi Lamp Projector (сюди можна додавати зображення на свій смак, drag and drop буде працювати тільки з цієї папки)
3. Запускати потрібне зображення можливо декількома шляхами:
 - drag and drop
 - навігація кнопками play control: назад, авто, вперед
4. Потім натиснути кнопку трансфер, вона працює в режимі старт/стоп
 кнопка 24 bit потрібна для зміни color space rgb888, за замовчуванням rgb332 (рекомендовано) в цьому режимі кращий fps
5. Кнопка back - повернення до Lamp Javelin Remote
6. Тап по зображенню змінює розмір до повного зображення без обрізання (щось типу: «з єврея робить українця» ... жарт) 
7. Developer Tool – інструмент для підбору solid кольорів та граієнтів на льоту без перепрошивки лампи 
# Інструкція по використанню
---
  Розпакуйте вміст архіву в кореневу папку на диску (не на робочий стіл, будь ласка)
  і робіть так само, як показав Алекс Гайвер у своєму [відео](https://youtu.be/771-Okf0dYs?t=525). 
  Відмінність від відео – матрицю підключаємо до D3 і кнопку живимо від 3,3 вольта.
  В архіві є файл "Прочитай мене!!.doc. Його потрібно уважно прочитати. 
  Для завантаження файлів з папки data у файлову систему контролера потрібно встановити Uploader. 
  [Відео](https://esp8266-arduinoide.ru/esp8266fs/)
  Версію плати у "Менеджері плат" вибирайте 2.7.4. При першому запуску лампа створить свою WiFi мережу з ім'ям «WiFi Lamp Javelin» пароль у мережі при першому запуску буде 31415926. 
  Після підключення до мережі «WiFi Lamp Javelin» наберіть у браузері 192.168.4.1 і зайдіть на web сторінку лампи. Там можна змінити ім'я лампи (якщо їх кілька у мережі), 
  налаштувати підключення до Вашої домашньої WiFi мережі. Перезавантажити лампу.
  Всі налаштування прошивки знаходяться на вкладці ConstantsUser.h (там без проблем розберетеся) і у файлі data/config.json (там можна нічого не змінювати, все змінюється потім 
  з web-сторінки лампи). Але якщо хочете, щоб лампа відразу підключилася до Вашої WiFi мережі, введіть у файлі data/cofig.json у поля "ssid": та "password": 
  ім'я та пароль Вашої WiFi мережі відповідно. Поле "ESP_mode": змініть з 0 на 1. Збережіть файл на те саме місце та зробіть upload файлової системи. Лампа одразу підключиться до Вашої мережі. 
  Інші налаштування можна зробити зі сторінки лампи.

  На YouTube каналі [«SlingMasterJSC»](https://www.youtube.com/user/SlingMasterJSC) 
  є підбірка відео, про конструкцію лампи та програмне забезпечення два плейлісти Wifi Lamp «Javelin» та Arduino Project які я рекомендую переглянути

---
## Change Log

### Firmware WiFiLamp «Javelin»
### Версія 5.3 | 123 ефект
### Останні зміни :

Firmware
1. Покращено ефект та змінено алгоритм «Квітка Лотоса» © SlingMaster.
2. Додано два нові ефекти «Різнобарвні Kульбаби» © SlingMaster на базі коду від © Less Lam та «Веретено» © SlingMaster

---
* Скетч использует 613120 байт (58%) памяти устройства. Всего доступно 1044464 байт.
Глобальные переменные используют 56248 байт (68%) динамической памяти, оставляя 25672 байт для локальных переменных. Максимум: 81920 байт.


## ПОПЕРЕДНІ ВИПУСКИ:
## Firmware WiFiLamp «Javelin»

### Версія 5.2 | 121 ефект
### Останні зміни :

Firmware
1. Додано механізм сповіщення про вихід оновлення прошивки.
2. Додано новий ефект «Теплові Мережі» © SlingMaster
Це спроба додати лампі більшого функціоналу. Eфект можна використати я к темплейт для «Розумного дому» в моєму випадку поведінка ефекту залежить від даних зчитаних з api.thingspeak.com на якому знаходиться дані мого каналу з даними температури гарячої води, вологості та стану вентилятора. Ефект раз в хвилину зчитує дані та відображає результат на лампі (змінює колір, та емітує обертання вентилятора), також є демо режим.
3. Замінена бібліотека ArduinoJson на ArduinoJson-6.x
### !!! Будьте уважні.
Необхідно вилучити з папки C:\Program Files (x86)\Arduino\libraries 
бібліотеку ArduinoJson і додати ArduinoJson-6.x все необхідне в архіві libraries.zip

Software
1. Замінено мову інтерфейсу налаштувань лампи з російської на солов'їну (також залишається англійська)
2. В Налаштуваннях лампи додано можливісь на вибір користувача включити або виключити повідомлення про вихід оновлень.
3. Додано до програмного забезпечення WiFi Lamp Projector інструмент «Developer Tool» для підбору solid кольорів та градієнтів на льоту без перепрошивки лампи.
  * при виборі кольору  значення кольру копіюеться в буфер обміну
  • одним кліком в форматі HEX типу 0x2F00FF
  • подвійним кліком в форматі RGB типу CRGB(47, 0, 255);
 
---
* Скетч использует 612028 байт (58%) памяти устройства. Всего доступно 1044464 байт.
Глобальные переменные используют 55936 байт (68%) динамической памяти, оставляя 25984 байт для локальных переменных. Максимум: 81920 байт.
 

### Версія 5.1 | 120 ефектів
### Останні зміни :

Firmware
1. Додано підтримку потокової трансляції gif анімації, та відображення зображень формату png, jpg по протоколу UDP.

Software
1. На закладці налаштувань програми Lamp Javelin Remote додано іконку для переходу до Git репозиторію.
2. Додано програмне забезпечення WiFi Lamp Projector для потокової трансляції, як закладка до програми Lamp Javelin Remote.
 * використання
 - для переходу до WiFi Lamp Projector треба натиснути помаранчовий квадрат в caption програми
 - ображення для відтворення повинні бути в папці UI/ani/, щоб потрапити в папку треба тапнути по логотипу WiFi Lamp Projector (сюди можна додавати зображення на свій смак, drag and drop буде працювати тільки з цієї папки)
 - запускати потрібне зображення можливо декількома шляхами:
 • drag and drop
 • навігація кнопками play control: назад, авто, вперед
 - потім натиснути кнопку трансфер, вона працює в режимі старт/стоп
 кнопка 24 bit потрібна для зміни color space rgb888, за замовчуванням rgb332 (рекомендовано) в цьому режимі кращий fps
 - кнопка back - повернення до Lamp Javelin Remote
 - тап по зображенню змінює розмір до повного зображення без обрізання (щось типу: «з єврея робить українця» ... жарт)

---
* Скетч использует 604696 байт (57%) памяти устройства. Всего доступно 1044464 байт.
Глобальные переменные используют 55872 байт (68%) динамической памяти, оставляя 26048 байт для локальных переменных. Максимум: 81920 байт.


### Версія 5.0 | 120 ефектів
### Останні зміни :

1. Додано новий ефект «Міраж» © Stepko.
2. Додано новий ефект «Райдужний Торнадо» © SlingMaster на базі коду від © Stepko and © Sutaburosu.

---
Скетч использует 609040 байт (58%) памяти устройства. Всего доступно 1044464 байт.
Глобальные переменные используют 54220 байт (66%) динамической памяти, оставляя 27700 байт для локальных переменных. Максимум: 81920 байт.

### Версія 5.XX | 118 ефектів
### Останні зміни :

1. Оптимізовано код.
2. Прибрана з прошивки підтримка Blink.
3. Прибрана з прошивки підтримка програми від Kотейка, оскільки вона не підтримується і втратила актуальність.
4. Прибрано дублі функцій роботи з протоколами html, MQTT, UDP зараз використовуєтьс один код сумісний з цими протоколами.
6. Оптимізовано код роботи по протоколу MQTT, який може використовуватись для управління IoT (інтернет речей) в інших проектах, планую винести це в окремий репозиторій. На мою думку MQTT для лампи для більшості корустивачів не актуальний тим не менше може використовуватися для інших проектів IoT.
5. Для управління використовується формат json, усі протоколи використовують один і той самий формат json. Винятком є тільки управління по протоколу UDP у складі групи, залишено для сумісності. (розмір json повинен не перевищувати 255 байт, цього достатньо для вирішення типових задач).
6. Вилучено ефект «Kyбик Pyбикa». (погано выглядає на не квадратній матриці).
7. Виправлена помилка та змінено алгоритм в ефекті Масляні Фарби» © SlingMaster.
8. Додано новий ефект «Геном «UA»» співавтор (© Stepko) © SlingMaster.
9. Додано новий ефект «Креативний Годинник» © SlingMaster.

---
Скетч использует 609040 байт (58%) памяти устройства. Всего доступно 1044464 байт.
Глобальные переменные используют 54220 байт (66%) динамической памяти, оставляя 27700 байт для локальных переменных. Максимум: 81920 байт.


### Версія 4.5 | 117 ефектів
### Останні зміни :

1. Виправлена помилка з записом файла конфігурації.
2. Ефект lumenjer (© SottNick) виправлена помилка (© Сотнег) і адаптовано під матриці більші за 16х16 
3. Змінені алгоритми роботи для ефекту «Акварель».
4. Додано новий ефект «Смак меду» на базі коду від (© Stepko) адаптация © SlingMaster.
5. Додана нова функція глобальна зміна яскравості 
	(щоб зберегти зміни необхідно перемкнути кнопку в програмному забезпеченні).
6. Додана нова функція використання лампи тільки з підключенням до роутера, дуже актуальна з постійним відключенням електроенергії (якщо пароль і ssid лаипа вже запамятала то через деякий час всерівно підключиться до роутера, перехід в режим точки доступу можливий семиразовим кліком по кнопці )
7. Оновлене програмне забезпечення під всі платформи.

---
Скетч использует 609040 байт (58%) памяти устройства. Всего доступно 1044464 байт.
Глобальные переменные используют 54220 байт (66%) динамической памяти, оставляя 27700 байт для локальных переменных. Максимум: 81920 байт.


### Firmware WiFiLamp «Javelin»
### Версія 4.4 | 116 ефектів
### Останні зміни :

1. Виправлена помилка в прошивці для стандартної конструкції лампи.
2. Додано новий ефект «Світлофільтр» (© SlingMaster).

---
Скетч использует 607096 байт (58%) памяти устройства. Всего доступно 1044464 байт.
Глобальные переменные используют 52468 байт (64%) динамической памяти, оставляя 29452 байт для локальных переменных. Максимум: 81920 байт.

### Firmware WiFiLamp «Javelin»
### Версія 4.3 | 115 ефектів
### Останні зміни :
• выбачаюсь за не вірне авторство eфекту «Зірки» в попередньому випуску, воно належить © Marc Merlin.
1. Модифікована функція виводу текста.
2. Виправлена помилка в ефекті «Пісочний годинник».
3. Модифікована функція діагностики для лампи Javelin
4. Додано новий ефект «Новорічна листівка» (© SlingMaster).
5. Оновлені додатки для Remote control (Windows и Android).

---
Скетч использует 606292 байт (58%) памяти устройства. Всего доступно 1044464 байт.
Глобальные переменные используют 52440 байт (64%) динамической памяти, оставляя 29480 байт для локальных переменных. Максимум: 81920 байт.

### Firmware WiFiLamp «Javelin»
### Версія 4.2 | 114 ефектів
### Останні зміни :
1. Виправлена помилка в роботі через UDP протокол.
2. Додані нові ефекти:
	• Tixy Land 26 в 1 (на основі коду з прошивки FireLamp_EmbUI (адаптація © SlingMaster),
	. регулятор швидкості – выбір ефекту | < 5 автозміна эфекту по черзі
	. регулятор масштабу – зміна кольору | > 255 автозміна кольру в циклі
	посилання на джерело :  
	https://github.com/80Stepko08/FireLamp_EmbUI
	https://tixy.land/?code=Math.sin%28y%2F8%2Bt%29%2Bx%2F16-0.5
	
	• Зірки (© Stepko),
	. регулятор масштабу – зміна кількості вершин за замовчуванням 5 
	• Бамбук (© SlingMaster).
---
Скетч использует 604520 байт (57%) памяти устройства. Всего доступно 1044464 байт.
Глобальные переменные используют 52284 байт (63%) динамической памяти, оставляя 29636 байт для локальных переменных. Максимум: 81920 байт.



### Firmware WiFiLamp «Javelin»
### Версія 4.1 | 111 ефектів
### Останні зміни :
1. Виправлена помилка в методі VirtualSnow.
2. Оптимізовані методи gradientHorizontal, gradientVertical.
3. Додані нові ефекти:
	Червона Рута (© Stepko & © Sutaburosu, адаптація © SlingMaster),
	Плазмові Хвилі (© Руслан Ус),
	Опахало (на основі коду від © mastercat42, © SlingMaster).
4. Додано режим пострілу Javelіn - буде працювати на попередніх версіях з деякими обмеженнями.
### • для оригінальної лампи «Javelin» 
5. Додано додаткове керування за допомогою механічної кнопки
6. Додано управління круговим індикатором стану лампи	
---
* Скетч использует 589956 байт (56%) памяти устройства. Всего доступно 1044464 байт.
Глобальные переменные используют 51668 байт (63%) динамической памяти, оставляя 30252 байт для локальных переменных. Максимум: 81920 байт.

7. Оновлено програмне забезпечення для Android та Windows, оновлено додаток статистики
8. Доданий файл з конструкцією лампи WiFiLamp_Construction.pdf папка Schemes. 



### Версія 4.0 | 108 ефектів
### Останні зміни :
1. Це новий проект продовження проекту WiFiLamp-RemoteControl. Прошивка сумісна зі старою схемою, а також підтримує нову схему (папка \Schemes ) в цьому випуску нова конструкція на стадії відладки але цілком робоча

* (будьте уважні як що використовуєте MOSFET його змінено з (5U D1) на (15U D8) якщо вам потрібно можете повернути назад.
2. Перероблені та адаптовані під великі матриці деякі эффекти:
	Снігопад; (нові алгоритми)
	Завиток; (нові алгоритми)
	Річки Ботсвани;
	Свічка;
	Пісочний годинник; (нові алгоритми)
	Spectrum;
	Новорічна ялинка; (нові алгоритми)
	Феєрверк;
	Планета Земля;
3. Додані нові ефекти:
	Вогонь з іскрами (© Stepko з Kostyamat)
	Краплі на воді (© Stepko)
	Чарівний ліхтарик (© SlingMaster)

---
* Скетч использует 579288 байт (55%) памяти устройства. Всего доступно 1044464 байт.
Глобальные переменные используют 51496 байт (62%) динамической памяти, оставляя 30424 байт для локальных переменных. Максимум: 81920 байт.



### Останні зміни :
1.Додана схема лампы, та розпайка монтажної платы:
---

### Програмне забезпечення для Android та Windows Версія 3.0 

1. Android • RemoteControlForAndroid/WiFi Lamp Remote 3.1.0.39.apk
2. Windows • RemoteControlForWindows (не потребує встановлення, для запуску выкористовуйте WiFiLamp Remote.exe)
3. OS або браузер через посилання http://winecard.ltd.ua/dev/WifiLampRemote3/?ip=192.168.1.1&dev=1&timeout=200&sound=1

* програмне забезпечення сумісне з попередніми версіями моїх прошивок.
---	

* в майбутньому я додам нову прошивку зі схемами «WiFi Lamp Javelin» в якому я планую деякі зміни в конструкії лампи (наступне оновлення буде підримувати попередні лампи і буде додано новий функціонал, все це зараз в розробці)
Нажаль війна не дає можливості займатись більше новим проектом, але після перемоги обов'язково цей проект буде виконано. 
Слідкуйте я тут згодом додам схеми, креслення та програмне забезпечення, якщо ви захочете відновити копію лампи. 

---
