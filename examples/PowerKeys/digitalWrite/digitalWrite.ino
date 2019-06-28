// Данный пример поочерёдно включает и выключает каналы модуля.     // * Строки со звёздочкой являются необязательными.
                                                                    //
#include <Wire.h>                                                   // * Подключаем библиотеку для работы с аппаратной шиной I2C.
#include <iarduino_I2C_Relay.h>                                     //   Подключаем библиотеку для работы с реле и силовыми ключами.
iarduino_I2C_Relay pwrkey(0x09);                                    //   Объявляем объект pwrkey для работы с функциями и методами библиотеки iarduino_I2C_Relay, указывая адрес модуля на шине I2C.
                                                                    //   Если объявить объект без указания адреса (iarduino_I2C_Relay pwrkey;), то адрес будет найден автоматически.
void setup(){                                                       //
    delay(500);                                                     // * Ждём завершение переходных процессов связанных с подачей питания.
//  Готовим модуль к работе:                                        //
    pwrkey.begin();                                                 //   Инициируем работу с модулем.
    pwrkey.digitalWrite(ALL_CHANNEL,LOW);                           // * Выключаем все каналы модуля.
}                                                                   //
                                                                    //
void loop(){                                                        //
//  Включаем и выключаем каналы модуля:                             //
    pwrkey.digitalWrite(1,HIGH); pwrkey.digitalWrite(4,LOW);        //   Включаем 1 канал и выключаем 4.
    delay(500);                                                     //   Ждём   500 мс.
    pwrkey.digitalWrite(2,HIGH); pwrkey.digitalWrite(1,LOW);        //   Включаем 2 канал и выключаем 1.
    delay(500);                                                     //   Ждём   500 мс.
    pwrkey.digitalWrite(3,HIGH); pwrkey.digitalWrite(2,LOW);        //   Включаем 3 канал и выключаем 2.
    delay(500);                                                     //   Ждём   500 мс.
    pwrkey.digitalWrite(4,HIGH); pwrkey.digitalWrite(3,LOW);        //   Включаем 4 канал и выключаем 3.
    delay(500);                                                     //   Ждём   500 мс.
}                                                                   //