#pragma once
//=======CONFIGURATION=SECTION========
//#define MY_DEBUG

#include <variant.h>
#include "DFRobot_SHT20.h"

#define MY_NODE_ID 12 //здесь задается id ноды, если необходимо что бы id выдавал гейт, то нужно закомментировать данную строку
#define MY_RADIO_NRF5_ESB

int16_t myTransportComlpeteMS = 10000;
#define MY_TRANSPORT_WAIT_READY_MS (myTransportComlpeteMS)


#define CHILD_ID_TEMP 0
#define CHILD_ID_HUM 1
#define MY_SEND_RSSI 100
#define MY_SEND_BATTERY 101
#define MY_SEND_RESET_REASON 105
#define MY_RESET_REASON_TEXT


#define CHILD_ID_VIRT 3   // Нужна виртуальная нода для работы библиотеки
//====================================

#include <MySensors.h>
#include "efektaGpiot.h"
#include "efektaHappyNode.h"

MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgVirt(CHILD_ID_VIRT, V_VAR);

CHappyNode happyNode(100); // Адрес c которого будут храниться пользовательские данные
CDream interruptedSleep(1);  // Добавление пробуждения от пин. В (n) n - количество пинов от которых пробуждается (описывается в setup)

extern DFRobot_SHT20    sht20;

void SHT_read();
void blink (uint8_t flash);
void SHT_send();
