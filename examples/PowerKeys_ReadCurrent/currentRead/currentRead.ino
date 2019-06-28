// Данный пример демонстрирует чтение тока с третьего канала.       // * Строки со звёздочкой являются необязательными.
                                                                    //
#include <Wire.h>                                                   // * Подключаем библиотеку для работы с аппаратной шиной I2C.
#include <iarduino_I2C_Relay.h>                                     //   Подключаем библиотеку для работы с реле и силовыми ключами.
iarduino_I2C_Relay pwrkey(0x09);                                    //   Объявляем объект pwrkey для работы с функциями и методами библиотеки iarduino_I2C_Relay, указывая адрес модуля на шине I2C.
                                                                    //   Если объявить объект без указания адреса (iarduino_I2C_Relay pwrkey;), то адрес будет найден автоматически.
uint8_t i = 0;                                                      //
                                                                    //
void setup(){                                                       //
    Serial.begin(9600);                                             //
    while(!Serial){;}                                               // * Ждём завершения инициализации шины UART.
    delay(500);                                                     // * Ждём завершение переходных процессов связанных с подачей питания.
//  Готовим модуль к работе:                                        //
    pwrkey.begin();                                                 //   Инициируем работу с модулем.
    pwrkey.digitalWrite(ALL_CHANNEL,LOW);                           // * Выключаем все каналы модуля.
}                                                                   //
                                                                    //
void loop(){                                                        //
//  Управляем каналом N 3:                                          //
    if( i==0   ){ pwrkey.digitalWrite(3,HIGH); }                    //   Включаем  3 канал если в переменной i хранится значение 0.
    if( i==128 ){ pwrkey.digitalWrite(3, LOW); }                    //   Отключаем 3 канал если в переменной i хранится значение 128.
//  Выводим силу тока проходещего по 3 каналу:                      //
    float j = pwrkey.currentRead(3);                                //   Считываем силу тока с третьего канала в переменную j.
    Serial.println( (String) "Сила тока "+j+" А." );                //   Выводим силу тока в монитор последовательного порта.
//  Приостанавливаем выполнение скетча:                             //
    i+=4; delay(100);                                               //   Увеличиваем счётчик i на 4 и ждем 100 мс, значит до сброса переменной i пройдет 256*100/4 = 6,4 сек.
}                                                                   //
                                                                    //
//  ПРИМЕЧАНИЕ: Третий канал будет включаться и отключаться.        //
//  Если к третьему каналу подключить нагрузку, то в мониторе       //
//  последовательного порта будет отображаться ток потребляемый     //
//  нагрузкой в то время пока канал включён. А пока канал выключен  //
//  (или нагрузка отсутствует) ток будет равен 0.00 А               //