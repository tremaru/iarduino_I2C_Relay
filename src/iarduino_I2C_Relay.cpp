#include "iarduino_I2C_Relay.h"																							//
																														//
//		Инициализация расширителя выводов:																				//	Возвращаемое значение: результат инициализации.
bool	iarduino_I2C_Relay::begin			(void){																		//	Параметр: отсутствует
		//	Инициируем работу с шиной I2C:																				//
			objI2C->begin(100);																							//	Инициируем передачу данных по шине I2C на скорости 100 кГц.
		//	Если адрес не указан, то ищим модуль на шине I2C:															//
			if(valAddrTemp==0){																							//
				for(int i=1; i<127; i++){																				//	Проходим по всем адресам на шине I2C
					if( objI2C->checkAddress(i)									){										//	Если на шине I2C есть устройство с адресом i, то ...
					if( _checkModel(objI2C->readByte(i,REG_MODEL))				){										//	Если у модуля с адресом i в регистре «MODEL»   хранится значение прошедшее проверку на идентификато модуля, то ...
					if( objI2C->readByte(i,REG_CHIP_ID)    == DEF_CHIP_ID_FLASH	){										//	Если у модуля с адресом i в регистре «CHIP_ID» хранится значение DEF_CHIP_ID_FLASH (идентификатор модулей), то ...
					if( objI2C->readByte(i,REG_ADDRESS)>>1 == i					){	valAddrTemp=i; i=128;	}else		//	Если у модуля с адресом i в регистре «ADDRESS» хранится значение i                 ( адрес + младший бит ), то cчитаем что модуль обнаружен, сохраняем значение i как найденный адрес и выходим из цикла.
					if( objI2C->readByte(i,REG_ADDRESS)    == 0xFF				){	valAddrTemp=i; i=128;	}			//	Если у модуля с адресом i в регистре «ADDRESS» хранится значение 0xFF              ( адрес не задавался  ), то cчитаем что модуль обнаружен, сохраняем значение i как найденный адрес и выходим из цикла.
					}																									//
					}																									//
					}																									//
				}																										//
			}																											//
		//	Если модуль не найден, то возвращаем ошибку инициализации:													//
			if( valAddrTemp == 0													){return false;}					//
		//	Проверяем наличие модуля и значения его регистров:															//
			if( objI2C->checkAddress(valAddrTemp)            == false				){return false;}					//	Если на шине I2C нет устройств с адресом valAddrTemp,                   то возвращаем ошибку инициализации
			if( !_checkModel(objI2C->readByte(valAddrTemp,REG_MODEL))				){return false;}					//	Если значение регистра «MODEL»   не совпадает идентификаторами моделей, то возвращаем ошибку инициализации
			if( objI2C->readByte(valAddrTemp,REG_CHIP_ID)    != DEF_CHIP_ID_FLASH	){return false;}					//	Если значение регистра «CHIP_ID» не совпадает со значением DEF_CHIP_ID_FLASH, то возвращаем ошибку инициализации
			if( objI2C->readByte(valAddrTemp,REG_ADDRESS)>>1 != valAddrTemp			){									//	Если значение регистра «ADDRESS» не совпадает с адресом модуля,         то ...
			if( objI2C->readByte(valAddrTemp,REG_ADDRESS)    != 0xFF				){return false;}}					//	Если значение регистра «ADDRESS» не совпадает со значением 0xFF,        то возвращаем ошибку инициализации
		//	Определяем значения переменных:																				//	
			valAddr=valAddrTemp;																						//	Сохраняем адрес модуля на шине I2C.
			valVers=objI2C->readByte(valAddr,REG_VERSION);																//	Сохраняем версию прошивки модуля.
		//	Перезагружаем модуль устанавливая его регистры в значение по умолчанию:										//
			reset();																									//	Выполняем программную перезагрузку.
			delay(5);																									//
			return true;																								//	Возвращаем флаг успешной инициализаии.
}																														//
																														//
//		Перезагрузка модуля:																							//	Возвращаемое значение:	результат перезагрузки.
bool	iarduino_I2C_Relay::reset			(void){																		//	Параметр:				отсутствует.
			if(valAddr){																								//	Если реле/ключ был инициализирован, то ...
			//	Сбрасываем глобальные переменные в значения по умолчанию												//
				valFreqPWM = 490;																						//	Сбрасываем установленную частоту ШИМ в значение по умолчанию.
			//	Устанавливаем бит перезагрузки:																			//
				if(_readBytes(REG_BITS_0,1)==false){return false;}														//	Читаем 1 байт регистра «BITS_0» в массив «data».
				data[0] |= 0b10000000;																					//	Устанавливаем бит «SET_RESET»
				if(_writeBytes(REG_BITS_0,1)==false){return false;}														//	Записываем 1 байт в регистр «BITS_0» из массива «data».
			//	Ждём установки флага завершения перезагрузки:															//
				do{ if(_readBytes(REG_FLAGS_0,1)==false){return false;} }												//	Читаем 1 байт регистра «REG_FLAGS_0» в массив «data».
				while( (data[0]&0b10000000) == 0);																		//	Повторяем чтение пока не установится флаг «FLG_RESET».
				return true;																							//
			}else{																										//	Иначе, если реле/ключ не инициализирован, то ...
				return false;																							//	Возвращаем ошибку
			}																											//
}																														//
																														//
//		Смена адреса модуля:																							//	Возвращаемое значение:	резульат смены адреса.
bool	iarduino_I2C_Relay::changeAddress	(uint8_t newAddr){															//	Параметр:				newAddr - новый адрес модуля (0x07 < адрес < 0x7F).
			if(valAddr){																								//	Если реле/ключ был инициализирован, то ...
			//	Проверяем новый адрес:																					//
				if(newAddr>0x7F){newAddr>>=1;}																			//	Корректируем адрес, если он указан с учётом бита RW.
				if(newAddr==0x00 || newAddr==0x7F){return false;}														//	Запрещаем устанавливать адрес 0x00 и 0x7F.
			//	Записываем новый адрес:																					//
				if(_readBytes(REG_BITS_0,1)==false){return false;}														//	Читаем 1 байт регистра «BITS_0» в массив «data».
				data[0] |= 0b00000010;																					//	Устанавливаем бит «SAVE_ADR_EN»
				if(_writeBytes(REG_BITS_0,1)==false){return false;}														//	Записываем 1 байт в регистр «BITS_0» из массива «data».
				data[0] = (newAddr<<1)|0x01;																			//	Готовим новый адрес к записи в модуль, установив бит «SAVE_FLASH».
				if(_writeBytes(REG_ADDRESS,1)==false){return false;}													//	Записываем 1 байт в регистр «ADDRESS» из массива «data».
				delay(200);																								//	Даём более чем достаточное время для применения модулем нового адреса.
			//	Проверяем наличие модуля с новым адресом на шине I2C:													//
				if(objI2C->checkAddress(newAddr)==false){return false;}													//	Если на шине I2C нет модуля с адресом newAddr, то возвращаем ошибку.
				valAddr = newAddr;																						//	Сохраняем новый адрес как текущий.
				return true;																							//	Возвращаем флаг успеха.
			}else{																										//	Иначе, если реле/ключ не инициализирован, то ...
				return false;																							//	Возвращаем ошибку
			}																											//
}																														//
																														//
//		Установка логического уровня:																					//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Relay::digitalWrite	(uint8_t channel, uint8_t level){											//	Параметры:				номер канала (1,2,3,4,ALL_CHANNEL), логический уровень (LOW/HIGH).
			if( ((channel>0)&&(channel<=4)) || (channel==ALL_CHANNEL) ){												//	Если корректно указан номер канала, то ...
			if( valAddr ){																								//	Если реле/ключ был инициализирован, то ...
			if( valAccess >= 1 ){																						//	Если уровень доступа модуля позволяет чтение/запись цифровых значений, то ...
			//	Если указан один канал, то записываем бит в регистр «REG_REL_DIGITAL_ONE»:								//
				if( channel != ALL_CHANNEL ){																			//	Если указан один канал, то ...
					data[0] = 1 << ((level==HIGH? REL_BIT_WRITE_H:REL_BIT_WRITE_L) + channel - 1);						//	Записываем в массив «data» байт для передачи с установленным битом в старшем «REL_BIT_WRITE_H-X» или младшем «REL_BIT_WRITE_L-X» полубайте.
					_writeBytes(REG_REL_DIGITAL_ONE, 1);																//	Записываем 1 байт данных из массива «data» в регистр «REG_REL_DIGITAL_ONE».
			//	Если указаны все каналы, то записываем байт в регистр «REG_REL_DIGITAL_ALL»:							//
				}else{																									//	Если указаны все каналы, то ...
					data[0] = level==HIGH? 0x0F:0x00;																	//	Записываем в массив «data» байт для передачи.
					_writeBytes(REG_REL_DIGITAL_ALL, 1);																//	Записываем 1 байт данных из массива «data» в регистр «REG_REL_DIGITAL_ALL».
				}																										//
			}																											//
			}																											//
			}																											//
}																														//
																														//
//		Чтение логического уровня:																						//	Возвращаемое значение:	логический уровень.
uint8_t	iarduino_I2C_Relay::digitalRead		(uint8_t channel){															//	Параметр:				номер канала (1,2,3,4,ALL_CHANNEL).
			if( ((channel>0)&&(channel<=4)) || (channel==ALL_CHANNEL) ){												//	Если корректно указан номер канала, то ...
			if( valAddr ){																								//	Если реле/ключ был инициализирован, то ...
			if( valAccess >= 1 ){																						//	Если уровень доступа модуля позволяет чтение/запись цифровых значений, то ...
			//	Читаем значение регистра «REG_REL_DIGITAL_ALL»:															//
				_readBytes(REG_REL_DIGITAL_ALL, 1);																		//	Читаем 1 байт регистра «REG_REL_DIGITAL_ALL» в массив «data».
			//	Если указан один канал, то возвращаемым числом будет значение HIGH или LOW:								//
				if( channel != ALL_CHANNEL ){ data[0] = (data[0]&(1<<(channel-1)))? HIGH:LOW; }							//	Если бит указанного канала установлен, то возвращаем HIGH, иначе LOW.
			//	Возвращаем результат:																					//
				return data[0] & 0x0F;																					//	Возвращаем 4 младших бита.
			}																											//
			}																											//
			}																											//
			return 0;																									//	Возвращаем 0.
}																														//
																														//
//		Установка аналогового уровня:																					//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Relay::analogWrite		(uint8_t channel, uint16_t level){											//	Параметры:				номер канала (1,2,3,4,ALL_CHANNEL), уровень ШИМ (0-4095).
			if( ((channel>0)&&(channel<=4)) || (channel==ALL_CHANNEL) ){												//	Если корректно указан номер канала, то ...
			if( valAddr ){																								//	Если реле/ключ был инициализирован, то ...
			if( valAccess >= 2 ){																						//	Если уровень доступа модуля позволяет чтение/запись цифровых значений и чтение/запись аналоговых значений, то ...
				uint8_t channelMin = channel==ALL_CHANNEL? 1:channel;													//	Определяем номер младшего канала в который записывается ШИМ.
				uint8_t channelMax = channel==ALL_CHANNEL? 4:channel;													//	Определяем номер старшего канала в который записывается ШИМ.
			//	Записываем уровень «level» в регистры «REG_REL_PWM»:													//
				for(uint8_t i=channelMin; i<=channelMax; i++){															//	Проходим по одному или всем каналам.
					data[(i-channelMin)*2  ] =  level & 0xFF;															//	Устанавливаем младший байт значения «level» для регистра «REG_REL_PWM_L».
					data[(i-channelMin)*2+1] = (level>>8);																//	Устанавливаем старший байт значения «level» для регистра «REG_REL_PWM_H».
				}	_writeBytes( REG_REL_PWM + ((channelMin-1)*2) , (channelMax-channelMin+1)*2 );						//	Записываем 2 или 8 байт из массива «data» в модуль начиная с регистра «REG_REL_PWM_L-X».
			}																											//
			}																											//
			}																											//
}																														//
																														//
//		Чтение аналогового уровня:																						//	Возвращаемое значение:	аналоговый уровень.
uint16_t iarduino_I2C_Relay::analogRead		(uint8_t channel){															//	Параметр:				номер канала (1,2,3,4).
			uint16_t level=0;																							//	Определяем переменную для вывода результата.
			if( (channel>0)&&(channel<=4) ){																			//	Если корректно указан номер канала, то ...
			if( valAddr ){																								//	Если реле/ключ был инициализирован, то ...
			if( valAccess >= 2 ){																						//	Если уровень доступа модуля позволяет чтение/запись цифровых значений и чтение/запись аналоговых значений, то ...
			//	Читаем уровень из регистров «REG_REL_PWM»:																//
				_readBytes( REG_REL_PWM + ((channel-1)*2) , 2 );														//	Читаем 2 байта из регистров «REG_REL_PWM_L-X» и «REG_REL_PWM_H-X» в массив «data».
				level=data[1]; level<<=8; level|=data[0];																//	Сохраняем 2 прочитанных байта в переменную «level».
			}																											//
			}																											//
			}																											//
			return level;																								//	Возвращаем результат чтения.
}																														//
																														//
//		Установка коэффициента усреднения показаний АЦП:																//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Relay::analogAveraging	(uint8_t coefficient){														//	Параметр:				коэффициент усреднения (0-255).
			if( valAddr ){																								//	Если реле/ключ был инициализирован, то ...
			if( valAccess >= 3 ){																						//	Если уровень доступа модуля позволяет чтение/запись цифровых значений, чтение/запись аналоговых значений и чтение тока, то ...
				data[0] = coefficient;																					//	Копируем значение аргумента «coefficient» в первый элемент массива «data».
				_writeBytes(REG_REL_AVERAGING, 1);																		//	Записываем 1 байт из массива «data» в регистр «REG_REL_AVERAGING» модуля.
			}																											//
			}																											//
}																														//
																														//
//		Установка частоты ШИМ:																							//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Relay::freqPWM			(uint16_t frequency){														//	Параметр:				частота в Гц (1-12000).
			if( (frequency>0) && (frequency<12000) ){																	//	Если корректно указана частота, то ...
			if( valAddr ){																								//	Если реле/ключ был инициализирован, то ...
			if( valAccess >= 2 ){																						//	Если уровень доступа модуля позволяет чтение/запись цифровых значений и чтение/запись аналоговых значений, то ...
			//	Готовим два байта для записи:																			//
				data[0]		= uint8_t(frequency & 0x00FF);																//	Устанавливаем младший байт значения «frequency» для регистра «REG_REL_FREQUENCY_L».
				data[1]		= uint8_t(frequency >> 8);																	//	Устанавливаем старший байт значения «frequency» для регистра «REG_REL_FREQUENCY_H».
				valFreqPWM	= frequency;																				//	Сохраняем частоту ШИМ для её сравнения при управлении сервоприводами.
			//	Записываем обновлённые данные в регистр «REG_REL_FREQUENCY_H» и «REG_REL_FREQUENCY_L»:					//
				_writeBytes(REG_REL_FREQUENCY_L, 2);																	//	Записываем 2 байта из массива «data» в модуль, начиная с регистра «REG_REL_FREQUENCY_L».
			}																											//
			}																											//
			}																											//
}																														//
																														//
//		Калибровка номинала шунтирующего резистора:																		//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Relay::currentWrite	(uint8_t channel, float current){											//	Параметры:				номер канала (1,2,3,4), реально измеренная сила тока в А (желательно около 1.0А).
			if( (channel>0)&&(channel<=4) ){																			//	Если корректно указан номер канала, то ...
			if( valAddr ){																								//	Если реле/ключ был инициализирован, то ...
			if( valAccess >= 3 ){																						//	Если уровень доступа модуля позволяет чтение/запись цифровых значений, чтение/запись аналоговых значений и чтение тока, то ...
				currentWrite(channel, current, data[0]);																//	Рассчитываем Rш в мОм и сохраняем его в массив «data».
				if( data[0] ){ _writeBytes( REG_REL_SHUNT + (channel-1) , 1 );}											//	Если «data» не равно 0, то записываем 1 байт из массива «data» в регистр «REG_REL_SHUNT-X» модуля.
			}																											//
			}																											//
			}																											//
}																														//
																														//
//		Расчёт номинала шунтирующего резистора:																			//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Relay::currentWrite	(uint8_t channel, float current, uint8_t& Rsh){								//	Параметры:				номер канала (1,2,3,4), реально измеренная сила тока в А (желательно около 1.0А).
			if( (channel>0)&&(channel<=4) ){																			//	Если корректно указан номер канала, то ...
			if( valAddr ){																								//	Если реле/ключ был инициализирован, то ...
			if( valAccess >= 3 ){																						//	Если уровень доступа модуля позволяет чтение/запись цифровых значений, чтение/запись аналоговых значений и чтение тока, то ...
				uint16_t valADC = 0;																					//	Определяем переменную для хранения значения АЦП.
				float    fltADC = 0;																					//	Определяем переменную для хранения значения АЦП усреднённого.
			//	Читаем значение АЦП в цикле:																			//
				for(uint8_t i=0; i<20; i++){																			//	Считываем силу тока рассчитанную модулем 20 раз подряд.
				//	Читаем значение АЦП из регистров «REG_REL_ADC»:														//
					_readBytes( REG_REL_ADC + ((channel-1)*2) , 2 );													//	Читаем 2 байта из регистров «REG_REL_ADC_L-X» и «REG_REL_ADC_H-X» в массив «data».
					valADC=data[1]; valADC<<=8; valADC|=data[0];														//	Сохраняем 2 прочитанных байта в переменную «valADC».
					fltADC += valADC; delay(10);																		//	Добавляем значение АЦП к переменной «fltADC».
				}	fltADC /= 20.0;																						//	Получаем среднее арифметическое значение АЦП.
			//	Рассчитываем значение шунтирующего резистора: I = 3,3•АЦП / 4095•Rш  =>  Rш = 3,3•АЦП / 4095•I			//
				if( fltADC==0 || current==0 ){ Rsh=0; }else{ Rsh=uint8_t((float)(3.3f * fltADC)/(4.095f * current)); }	//
			}																											//
			}																											//
			}																											//
}																														//
																														//
//		Чтение силы тока в А на канале силового ключа:																	//	Возвращаемое значение:	сила тока в Амперах.
float	iarduino_I2C_Relay::currentRead		(uint8_t channel){															//	Параметр:				номер канала (1,2,3,4).
			uint16_t level=0;																							//	Определяем переменную для вывода результата.
			if( (channel>0)&&(channel<=4) ){																			//	Если корректно указан номер канала, то ...
			if( valAddr ){																								//	Если реле/ключ был инициализирован, то ...
			if( valAccess >= 3 ){																						//	Если уровень доступа модуля позволяет чтение/запись цифровых значений, чтение/запись аналоговых значений и чтение тока, то ...
			//	Читаем силу тока из регистров «REG_REL_CURRENT»:														//
				_readBytes( REG_REL_CURRENT + ((channel-1)*2) , 2 );													//	Читаем 2 байта из регистров «REG_REL_CURRENT_L-X» и «RREG_REL_CURRENT_H-X» в массив «data».
				level=data[1]; level<<=8; level|=data[0];																//	Сохраняем 2 прочитанных байта в переменную «level».
			}																											//
			}																											//
			}																											//
			return float(level)/1000.0f;																				//	Возвращаем результат чтения, переведя прочитанные мА в А.
}																														//
																														//
//		Установка защиты по току на канале силового ключа:																//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Relay::setCurrentProtection	(uint8_t channel, float current, uint8_t type){						//	Параметры:				номер канала (1,2,3,4,ALL_CHANNEL), сила тока (0.0-25.5А), тип защиты (CURRENT_LIMIT/CURRENT_DISABLE).
			if( ((channel>0)&&(channel<=4)) || (channel==ALL_CHANNEL) ){												//	Если корректно указан номер канала, то ...
			if( (type==CURRENT_LIMIT) || (type==CURRENT_DISABLE) ){														//	Если корректно указан тип защиты, то ...
			if( valAddr ){																								//	Если реле/ключ был инициализирован, то ...
			if( valAccess >= 3 ){																						//	Если уровень доступа модуля позволяет чтение/запись цифровых значений, чтение/запись аналоговых значений и чтение тока, то ...
				uint8_t channelMin = channel==ALL_CHANNEL? 1:channel;													//	Определяем номер младшего канала в который записывается значение ограничения тока.
				uint8_t channelMax = channel==ALL_CHANNEL? 4:channel;													//	Определяем номер старшего канала в который записывается значение ограничения тока.
			//	Записываем ток «current» в регистр(ы) «REG_REL_LIMIT»:													//
				for(uint8_t i=channelMin; i<=channelMax; i++){															//	Проходим по одному или всем каналам.
					data[(i-channelMin)] =  uint8_t(current*10);														//	Сохраняем значение «current» в элемент массива «data».
				}	_writeBytes( REG_REL_LIMIT + (channelMin-1) , (channelMax-channelMin+1) );							//	Записываем 1 или 4 байт из массива «data» в модуль начиная с регистра «REG_REL_LIMIT-X».
			//	Устанавливаем биты в регистре «REG_REL_BIT»:															//
				_readBytes( REG_REL_BIT , 1 );																			//	Читаем 1 байт из регистра «REG_REL_BIT» в массив «data».
				if( channel != ALL_CHANNEL ){																			//	Если указан один канал, то ...
						data[0] |=   1<<((type==CURRENT_DISABLE? REL_BIT_A_OFF:REL_BIT_A_MAX)+channel-1);				//	В байте «data» устанавливаем бит в старшем «REL_BIT_A_MAX-X» или младшем «REL_BIT_A_OFF-X» полубайте.
						data[0] &= ~(1<<((type==CURRENT_DISABLE? REL_BIT_A_MAX:REL_BIT_A_OFF)+channel-1));				//	В байте «data» сбрасываем    бит в старшем «REL_BIT_A_MAX-X» или младшем «REL_BIT_A_OFF-X» полубайте.
				}else{  data[0]  =        type==CURRENT_DISABLE?   0x0F   :   0xF0; }									//	Если указаны все каналы, то записываем в массив «data» байт с 4 установленными битами в старшем «REL_BIT_A_MAX» или младшем «REL_BIT_A_OFF» полубайте.
				_writeBytes(REG_REL_BIT, 1);																			//	Записываем 1 байт данных из массива «data» в регистр «REG_REL_BIT».
			//	Сохраняем тип установленной защиты для функции перезапуска защиты:										//	
				if( channel != ALL_CHANNEL ){valProtect[channel-1] = type;}												//	Сохраняем тип установленной защиты в элемент массива «valProtect» (для функции перезапуска защиты).
				if( channel == ALL_CHANNEL ){valProtect[0]=valProtect[1]=valProtect[2]=valProtect[3]=data[0];}			//	Сохраняем тип установленной защиты во все элементы массива «valProtect» (для функции перезапуска защиты).
			}																											//
			}																											//
			}																											//
			}																											//
}																														//
																														//
//		Проверка выполнения защиты на канале силового ключа:															//	Возвращаемое значение:	флаг выполнения защиты (true/false).
bool	iarduino_I2C_Relay::getCurrentProtection	(uint8_t channel){													//	Параметр:				номер канала (1,2,3,4,ALL_CHANNEL).
			if( ((channel>0)&&(channel<=4)) || (channel==ALL_CHANNEL) ){												//	Если корректно указан номер канала, то ...
			if( valAddr ){																								//	Если реле/ключ был инициализирован, то ...
			if( valAccess >= 3 ){																						//	Если уровень доступа модуля позволяет чтение/запись цифровых значений, чтение/запись аналоговых значений и чтение тока, то ...
				_readBytes( REG_REL_FLG , 1 );																			//	Читаем 1 байт из регистра «REG_REL_FLG» в массив «data».
				if( data[0] & ((1<<(REL_FLG_A_MAX+channel-1))|(1<<(REL_FLG_A_OFF+channel-1))) ){return true;}			//	Если в прочтённом байте установлен флаг «REL_FLG_A_MAX-X» или «REL_FLG_A_OFF-X», то возвращаем 1.
				if( data[0]>0 && channel==ALL_CHANNEL                                         ){return true;}			//	Если в прочтённом байте установлен любой флаг и проверяется любой канал, то возвращаем 1.
			}																											//
			}																											//
			}                                                                                   return false;			//
}																														//
																														//
//		Отключение защиты на канале силового ключа:																		//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Relay::delCurrentProtection	(uint8_t channel){													//	Параметр:				номер канала (1,2,3,4,ALL_CHANNEL).
			if( ((channel>0)&&(channel<=4)) || (channel==ALL_CHANNEL) ){												//	Если корректно указан номер канала, то ...
			if( valAddr ){																								//	Если реле/ключ был инициализирован, то ...
			if( valAccess >= 3 ){																						//	Если уровень доступа модуля позволяет чтение/запись цифровых значений, чтение/запись аналоговых значений и чтение тока, то ...
			//	Сбрасываем биты в регистре «REG_REL_BIT»:																//
				_readBytes( REG_REL_BIT , 1 );																			//	Читаем 1 байт из регистра «REG_REL_BIT» в массив «data».
				if( channel != ALL_CHANNEL ){ data[0] &= ~((1<<(REL_FLG_A_MAX+channel-1))|(1<<(REL_FLG_A_OFF+channel-1)));}	//	Если указан один канал, то в байте «data» сбрасываем бит «REL_BIT_A_MAX-X» и «REL_FLG_A_OFF-X».
				else                        { data[0]  =  0; }															//	Если указаны все каналы, то сбрасываем все биты в байте «data».
				_writeBytes( REG_REL_BIT, 1 );																			//	Записываем 1 байт данных из массива «data» в регистр «REG_REL_BIT».
			//	Сбрасываем флаги в регистре «REG_REL_FLG»:																//
				if( channel != ALL_CHANNEL ){ data[0]  =  ((1<<(REL_FLG_A_MAX+channel-1))|(1<<(REL_FLG_A_OFF+channel-1)));}	//	Если указан один канал, то cоздаём байт состоящий из установленных флагов «REL_FLG_A_MAX-X» и «REL_FLG_A_OFF-X».
				else                        { data[0]  =  0xFF; }														//	Если указаны все каналы, то cоздаём байт состоящий 8 установленных битов.
				_writeBytes( REG_REL_FLG, 1 );																			//	Записываем 1 байт данных из массива «data» в регистр «REG_REL_BIT».
			}																											//
			}																											//
			}																											//
}																														//
																														//
//		Перезапуск защиты на канале силового ключа:																		//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Relay::resCurrentProtection	(uint8_t channel){													//	Параметр:				номер канала (1,2,3,4,ALL_CHANNEL).
			if( ((channel>0)&&(channel<=4)) || (channel==ALL_CHANNEL) ){												//	Если корректно указан номер канала, то ...
			if( valAddr ){																								//	Если реле/ключ был инициализирован, то ...
			if( valAccess >= 3 ){																						//	Если уровень доступа модуля позволяет чтение/запись цифровых значений, чтение/запись аналоговых значений и чтение тока, то ...
				uint8_t channelMin = channel==ALL_CHANNEL? 1:channel;													//	Определяем номер младшего канала для которого устанавливается защита.
				uint8_t channelMax = channel==ALL_CHANNEL? 4:channel;													//	Определяем номер старшего канала для которого устанавливается защита.
			//	Проходим по одному или всем каналам.																	//
				for(uint8_t i=channelMin; i<=channelMax; i++){															//	Проходим по одному или всем каналам.
				//	Если ранее была установлена защита текущего канала:													//
					if(valProtect[i-1]!=10){																			//
					//	Отключаем защиту канала:																		//
						delCurrentProtection(i);																		//
					//	Устанавливаем защиту канала:																	//
						_readBytes( REG_REL_LIMIT + (i-channelMin) , 1 );												//	Считываем 1 байт из регистра «REG_REL_LIMIT-X» в массив «data».
						setCurrentProtection(i, float(data[0])/10.0, valProtect[i-1]);									//	Устанавливаем защиту канала i с током «data»/10 и типом защиты «valProtect».
					}																									//
				}																										//
			}																											//
			}																											//
			}																											//
}																														//
																														//
//		Разрешение работы сторожевого таймера модуля:																	//	Возвращаемое значение:	результат разрещения работы сторожевого таймера.
bool	iarduino_I2C_Relay::enableWDT				(uint8_t time){														//	Параметр:				время в секундах от 1 до 254).
			if(time==255){time=254;}																					//	Значение 255 является недопустимым, так как оно отключает сторожевой таймер.
			valWDT  = time;																								//	Сохраняем время сторожевого таймера
			data[0] = valWDT;																							//	Готовим один байт данных для отправки в модуль.
			return _writeBytes( REG_REL_WDT, 1 );																		//	Записываем 1 байт данных из массива «data» в регистр «REG_REL_WDT».
}																														//
																														//
//		Запрет работы сторожевого таймера модуля:																		//	Возвращаемое значение:	результат запрета работы сторожевого таймера.
bool	iarduino_I2C_Relay::disableWDT				(void){																//	Параметр:				отсутствует.
			valWDT  = 0xFF;																								//	Стираем время сторожевого таймера.
			data[0] = valWDT;																							//	Готовим один байт данных для отправки в модуль.
			return _writeBytes( REG_REL_WDT, 1 );																		//	Записываем 1 байт данных из массива «data» в регистр «REG_REL_WDT».
}																														//
																														//
//		Сброс (перезапуск) сторожевого таймера модуля:																	//	Возвращаемое значение:	результат сброса (перезапуска) сторожевого таймера.
bool	iarduino_I2C_Relay::resetWDT				(void){																//	Параметр:				отсутствует.
			if( valWDT==0xFF ){ return false; }																			//	Если в переменной valWDT (время сторожевого таймера) хранится значение 0xFF (таймер отключён), то возвращаем ошибку сброса таймера.
			data[0] = valWDT;																							//	Готовим один байт данных для отправки в модуль.
			return _writeBytes( REG_REL_WDT, 1 );																		//	Записываем 1 байт данных из массива «data» в регистр «REG_REL_WDT».
}																														//
																														//
//		Получение состояния сторожевого таймера модуля:																	//	Возвращаемое значение:	0 - выключён, 1 - включён.
bool	iarduino_I2C_Relay::getStateWDT				(void){																//	Параметр:				отсутствует.
			data[0] = 0xFF;																								//	Готовим ячейку массива «data» для получения одного байта данных.
			_readBytes( REG_REL_WDT, 1 );																				//	Читаем 1 байт данных из массива регистра «REG_REL_WDT» в массив «data».
			return data[0]<0xFF;																						//	Если в ячейке массива «data» значение меньше 0xFF, то возвращаем true (сторожевой таймер работает), иначе возвращаем false.
}																														//
																														//
//		Чтение данных из регистров в массив data:																		//	Возвращаемое значение:	результат чтения (true/false).
bool	iarduino_I2C_Relay::_readBytes		(uint8_t reg, uint8_t sum){													//	Параметры:				reg - номер первого регистра, sum - количество читаемых байт.
			bool	result=false;																						//	Определяем флаг       для хранения результата чтения.
			uint8_t	sumtry=10;																							//	Определяем переменную для подсчёта количества оставшихся попыток чтения.
			do{	result = objI2C->readBytes(valAddr, reg, data, sum);													//	Считываем из модуля valAddr, начиная с регистра reg, в массив data, sum байт.
				sumtry--;	if(!result){delay(1);}																		//	Уменьшаем количество попыток чтения и устанавливаем задержку при неудаче.
			}	while		(!result && sumtry>0);																		//	Повторяем чтение если оно завершилось неудачей, но не более sumtry попыток.
			return result;																								//	Возвращаем результат чтения (true/false).
}																														//
																														//
//		Запись данных в регистры из массива data:																		//	Возвращаемое значение:	результат записи (true/false).
bool	iarduino_I2C_Relay::_writeBytes	(uint8_t reg, uint8_t sum, uint8_t num){										//	Параметры:				reg - номер первого регистра, sum - количество записываемых байт, num - номер первого элемента массива data.
			bool	result=false;																						//	Определяем флаг       для хранения результата записи.
			uint8_t	sumtry=10;																							//	Определяем переменную для подсчёта количества оставшихся попыток записи.
			do{	result = objI2C->writeBytes(valAddr, reg, &data[num], sum);												//	Записываем в модуль valAddr начиная с регистра reg, sum байи из массива data начиная с элемента num.
				sumtry--;	if(!result){delay(1);}																		//	Уменьшаем количество попыток записи и устанавливаем задержку при неудаче.
			}	while		(!result && sumtry>0);																		//	Повторяем запись если она завершилась неудачей, но не более sumtry попыток.
			delay(10);																									//	Ждём применения модулем записанных данных.
			return result;																								//	Возвращаем результат записи (true/false).
}																														//
																														//
//		Сравнение идентификатора модели модуля:																			//	Возвращаемое значение:	результат сравнения (true/false).
bool	iarduino_I2C_Relay::_checkModel	(uint8_t val){																	//	Параметры:				val - значение регистра 0x04 «REG_MODEL» проверяемого модуля.
			if( val == DEF_MODEL_2RM ){ valModel = val; valAccess = 1; return true; }									//	Если значение аргумента совпадает с идентификатором модели модуля электромеханических реле на 2-канала,				то сохраняем идентификатор в переменную «valModel», сохраняем уровень доступа в переменную «valAccess» и возвращаем true.
			if( val == DEF_MODEL_4RT ){ valModel = val; valAccess = 1; return true; }									//	Если значение аргумента совпадает с идентификатором модели модуля твердотельных реле на 4-канала,					то сохраняем идентификатор в переменную «valModel», сохраняем уровень доступа в переменную «valAccess» и возвращаем true.
			if( val == DEF_MODEL_4NC ){ valModel = val; valAccess = 3; return true; }									//	Если значение аргумента совпадает с идентификатором модели модуля силовых ключей, 4N-канала 3А с измерением тока,	то сохраняем идентификатор в переменную «valModel», сохраняем уровень доступа в переменную «valAccess» и возвращаем true.
			if( val == DEF_MODEL_4PC ){ valModel = val; valAccess = 3; return true; }									//	Если значение аргумента совпадает с идентификатором модели модуля силовых ключей, 4P-канала 3А с измерением тока,	то сохраняем идентификатор в переменную «valModel», сохраняем уровень доступа в переменную «valAccess» и возвращаем true.
			if( val == DEF_MODEL_4NP ){ valModel = val; valAccess = 2; return true; }									//	Если значение аргумента совпадает с идентификатором модели модуля силовых ключей, 4N-канала 10A,					то сохраняем идентификатор в переменную «valModel», сохраняем уровень доступа в переменную «valAccess» и возвращаем true.
			if( val == DEF_MODEL_4PP ){ valModel = val; valAccess = 2; return true; }									//	Если значение аргумента совпадает с идентификатором модели модуля силовых ключей, 4P-канала 10A,					то сохраняем идентификатор в переменную «valModel», сохраняем уровень доступа в переменную «valAccess» и возвращаем true.
			                            valModel =   0; valAccess = 0; return false;									//	Иначе, если не выполнено ни одно из предыдущих условий,																то обнуляем  значение        переменной «valModel», обнуляем  значение          переменной «valAccess» и возвращаем false.
}																														//
																														//