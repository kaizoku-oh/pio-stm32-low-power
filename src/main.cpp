/*-----------------------------------------------------------------------------------------------*/
/* Includes                                                                                      */
/*-----------------------------------------------------------------------------------------------*/
#include <Arduino.h>
#include <cmsis_os.h>
#include <STM32RTC.h>
#include <STM32LowPower.h>

/*-----------------------------------------------------------------------------------------------*/
/* Private function prototypes                                                                   */
/*-----------------------------------------------------------------------------------------------*/
static void appThreadHandler(const void *argument);
static void onAlarmWakeUpCallback(void *argument);
static void onButtonPressCallback(void);

/*-----------------------------------------------------------------------------------------------*/
/* Private constants                                                                             */
/*-----------------------------------------------------------------------------------------------*/
// UART baudrate
static const uint32_t SERIAL_BAUDRATE = 115200;

// Application stack memory
static const uint32_t APP_THREAD_STACK_SIZE = 1024;

/*-----------------------------------------------------------------------------------------------*/
/* Private Variables                                                                             */
/*-----------------------------------------------------------------------------------------------*/
// OS objects
static osThreadId thread = nullptr;

// App global variables and objects
static STM32RTC& rtc = STM32RTC::getInstance();

/*-----------------------------------------------------------------------------------------------*/
/* Exported functions                                                                            */
/*-----------------------------------------------------------------------------------------------*/
/**************************************************************************************************
  * @brief      Entry point executed at startup from inside the Arduino framework
  * @return     Nothing
  ********************************************************************************************** */
void setup() {
  // Setup UART for logging
  Serial.begin(SERIAL_BAUDRATE);

  // Define and create the application thread
  osThreadDef(app, appThreadHandler, osPriorityNormal, 1, APP_THREAD_STACK_SIZE);
  thread = osThreadCreate(osThread(app), nullptr);

  // Start RTOS scheduler
  osKernelStart();
}

/**************************************************************************************************
  * @brief      Executed from inside the OS idle hook callback
  * @return     Nothing
  ********************************************************************************************** */
void loop() {}

/*-----------------------------------------------------------------------------------------------*/
/* Private functions                                                                             */
/*-----------------------------------------------------------------------------------------------*/
/**************************************************************************************************
  * @brief      Application thread handler
  * @param      argument argument pointer to be passed to thread handler
  * @return     Nothing
  ********************************************************************************************** */
static void appThreadHandler(const void *argument) {
  (void)argument;

  Serial.println("Starting thread...");
  osDelay(3000);

  // Setup LED pin
  pinMode(LED_BUILTIN, OUTPUT);

  // Setup button pin and attach external pin interrupt callback
  pinMode(USER_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(USER_BTN), onButtonPressCallback, CHANGE);

  // Initialize RTC and set initial time and date
  rtc.begin();
  rtc.setTime(8, 30, 58);
  rtc.setDate(21, 4, 24);

  // Initialize low power mode
  LowPower.begin();
  // Wake up from RTC alarm
  LowPower.enableWakeupFrom(&rtc, onAlarmWakeUpCallback);
  // Wake up from external interrupt
  LowPower.attachInterruptWakeup(USER_BTN, onButtonPressCallback, CHANGE, SLEEP_MODE);
  // Configure first alarm in 5 second then it will be done in the rtc callback
  rtc.setAlarmEpoch(rtc.getEpoch() + 5);

  while (true) {
    // Enter in running mode
    Serial.println("Running...");
    digitalWrite(LED_BUILTIN, HIGH);
    osDelay(3000);

    // Enter in STOP2 mode until period elapses or an interrupt gets triggered
    Serial.println("Sleeping...");
    digitalWrite(LED_BUILTIN, LOW);
    osDelay(3000);
    LowPower.deepSleep();
  }
}

/**************************************************************************************************
  * @brief      RTC alarm interrupt callback
  * @param      argument Optional user data
  * @return     Nothing
  ********************************************************************************************** */
static void onAlarmWakeUpCallback(void *argument) {
  (void)argument;
}

/**************************************************************************************************
  * @brief      Button interrupt callback
  * @return     Nothing
  ********************************************************************************************** */
static void onButtonPressCallback(void) {
}
