// The code of a simple node SHT-20 for mysensors.
// Inspired by the works of Andrey Berk and Alexei Bogdanov 
// Working code but not yet fully tested for stability in different modes. Also, I am just start with the SrFatcat library.
// I never tire of saying thanks to Alexey for his work.
// (https://bitbucket.org/SrFatcat/mysefektalib/src/master/).

#include "main.h"


uint32_t sleepingPeriod = 1 * 60 * 1000;  //первое число - минуты
uint8_t counterBattery = 0;              
uint8_t counterBatterySend = 60;          // Интервал отправки батареи и RSSI. 60 раз в час
float tempThreshold = 0.3;             //  Интервал изменения для отправки    
float humThreshold = 0.5;
float temp;
float hum;
float old_temp;
float old_hum;

DFRobot_SHT20    sht20;


void preHwInit() {
   // pinMode(PIN_BUTTON, INPUT);
    pinMode(BLUE_LED, OUTPUT);
    digitalWrite(BLUE_LED, HIGH);
}

void before() {
NRF_POWER->DCDCEN = 1; //включение режима оптимизации питания, расход снижается на 40%, но должны быть установленны емкости (если нода сделана на модуле https://a.aliexpress.com/_mKN3t2f то нужно раскомментировать эту строку)
//NRF_NFCT->TASKS_DISABLE = 1; //останавливает таски, если они есть
NRF_NVMC->CONFIG = 1; //разрешить запись
//NRF_UICR->NFCPINS = 0; //отключает nfc и nfc пины становятся доступными для использования
NRF_NVMC->CONFIG = 0; //запретить запись
#ifdef SERIAL_PRINT
    NRF_UART0->ENABLE = 1;  
#else
    NRF_UART0->ENABLE = 0;  
#endif
  happyInit();
}

void setup() {    
    happyConfig();
    sht20.initSHT20();      // Init SHT20 Sensor
    blink(1);
    addDreamPin(PIN_BUTTON, NRF_GPIO_PIN_NOPULL, CDream::NRF_PIN_HIGH_TO_LOW);  // Добавление пробуждения от PIN_BUTTON 
    interruptedSleep.init();
        
}

void happyPresentation() {
    happySendSketchInfo("HappyNode nRF52811 test", "V1.0");
    happyPresent(CHILD_ID_TEMP, S_TEMP, "Temperature");
    happyPresent(CHILD_ID_HUM, S_HUM, "Humidity");
}

void loop() {

    happyProcess();

    int8_t wakeupReson = dream(sleepingPeriod);
    if (wakeupReson == MY_WAKE_UP_BY_TIMER){
    SHT_read();
    
    }

    if (PIN_BUTTON) {  // Заглушка на обработку кнопки
        blink(3);
    }

    
}


void receive(const MyMessage & message){
    if (happyCheckAck(message)) return;

}

void SHT_read() {
    hum = sht20.readHumidity();                  // Read Humidity
    temp = sht20.readTemperature();               // Read Temperature

    if ((int)hum < 0) {
    hum = 0.0;
    }
    if ((int)hum > 99) {
    hum = 99.9;
    }

    if ((int)temp < 0) {
    temp = 0.0;
    }
    if ((int)temp > 99) {
    temp = 99.9;
    }

    SHT_send();

    if (counterBattery == 0) {
    happyNode.sendBattery(MY_SEND_BATTERY);
    happyNode.sendSignalStrength(MY_SEND_RSSI);
    blink(1);
    }
    counterBattery ++;
    if (counterBattery == counterBatterySend) {
        counterBattery = 0;
    }
}

void blink (uint8_t flash) {
  for (uint8_t i = 1; i <= flash; i++) {
    digitalWrite(BLUE_LED, LOW);
    wait(50);
    digitalWrite(BLUE_LED, HIGH);
    wait(50);
  }

}

void SHT_send() {
    if (abs(temp - old_temp) >= tempThreshold) {
    old_temp = temp;
    happySend(msgTemp.set(temp, 1));
    blink(1);
    }

    if (abs(hum - old_hum) >= humThreshold) {
    old_hum = hum;
    happySend(msgHum.set(hum, 1));
    blink(1);
    }

}