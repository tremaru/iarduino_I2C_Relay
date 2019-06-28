// Данный пример определяет тип модуля реле или силового ключа подключённого к шине I2C.                       // * Строки со звёздочкой являются необязательными.
                                                                                                               //
#include <Wire.h>                                                                                              // * Подключаем библиотеку для работы с аппаратной шиной I2C.
#include <iarduino_I2C_Relay.h>                                                                                //   Подключаем библиотеку для работы с реле и силовыми ключами.
iarduino_I2C_Relay relay;                                                                                      //   Объявляем объект relay для работы с функциями и методами библиотеки iarduino_I2C_Relay.
                                                                                                               //   Если при объявлении объекта указать адрес, например, relay(0xBB), то пример будет работать с тем модулем, адрес которого был указан.
void setup(){                                                                                                  //
    Serial.begin(9600);                                                                                        //
    while(!Serial){;}                                                                                          // * Ждём завершения инициализации шины UART.
    delay(500);                     Serial.print( "На шине I2C " );                                            //
    if( relay.begin() ){            Serial.print( "найден модуль с адресом 0x" );                              //   Инициируем работу с модулем реле или силовыми ключами.
                                    Serial.print( relay.getAddress(), HEX );                                   //   Выводим текущий адрес модуля.
                                    Serial.print( ", который является " );                                     //
        switch( relay.getModel() ){                                                                            //   Сравниваем модель модуля с константами:
            case DEF_MODEL_2RM:     Serial.print( "электромеханическим реле на 2-канала" );           break;   //
            case DEF_MODEL_4RT:     Serial.print( "твердотельным реле на 4-канала" );                 break;   //
            case DEF_MODEL_4NC:     Serial.print( "силовым ключом на 4 N-канала с измерением тока" ); break;   //
            case DEF_MODEL_4PC:     Serial.print( "силовым ключом на 4 P-канала с измерением тока" ); break;   //
            case DEF_MODEL_4NP:     Serial.print( "силовым ключом на 4 N-канала до 10A" );            break;   //
            case DEF_MODEL_4PP:     Serial.print( "силовым ключом на 4 P-канала до 10A" );            break;   //
            default:                Serial.print( "неизвестным силовым ключом или реле" );                     //
        }                           Serial.print( ", версия прошивки модуля: " );                              //
                                    Serial.print( relay.getVersion() );                                        //
                                    Serial.print( ".0." );                                                     //
    }else{                          Serial.print( "нет ни силовых ключей, ни реле!" );                         //   Если метод relay.begin() вернул false, значит библиотеке не удалось найти модуль реле или силовых ключей.
    }                               Serial.print( "\r\n" );                                                    //
}                                                                                                              //
                                                                                                               //
void loop(){                                                                                                   //
}                                                                                                              //