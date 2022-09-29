/*
 * PCF8575 GPIO Port Expand
 * https://www.mischianti.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Renzo Mischianti www.mischianti.org All right reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "PCF8575.h"
#include <Wire.h>

PCF8575::PCF8575(){

};

/**
 * Constructor
 * @param address: i2c address
 */
PCF8575::PCF8575(uint8_t address, TwoWire *wire){
	_address = address;
	_wire = wire;
};

/**
 * Construcor
 * @param address: i2c address
 * @param interruptPin: pin to set interrupt
 * @param interruptFunction: function to call when interrupt raised
 */
PCF8575::PCF8575(uint8_t address, uint8_t interruptPin,  void (*interruptFunction)() ){
	//_wire = &Wire;

	_address = address;
	_interruptPin = interruptPin;
	_interruptFunction = interruptFunction;
	_usingInterrupt = true;
};



/**
 * Set if fin is OUTPUT or INPUT
 * @param pin: pin to set
 * @param mode: mode, supported only INPUT or OUTPUT (to semplify)
 */
void PCF8575::pinMode(uint8_t pin, uint8_t mode){
	DEBUG_PRINT("Set pin ");
	DEBUG_PRINT(pin);
	DEBUG_PRINT(" as ");
	DEBUG_PRINTLN(mode);

	if (mode == OUTPUT){
		writeMode = writeMode | bit(pin);
		readMode =  readMode & ~bit(pin);
//		DEBUG_PRINT("writeMode: ");
//		DEBUG_PRINT(writeMode, BIN);
//		DEBUG_PRINT("readMode: ");
//		DEBUG_PRINTLN(readMode, BIN);

	}else if (mode == INPUT){
		writeMode = writeMode & ~bit(pin);
		readMode =   readMode | bit(pin);
//		DEBUG_PRINT("writeMode: ");
//		DEBUG_PRINT(writeMode, BIN);
//		DEBUG_PRINT("readMode: ");
//		DEBUG_PRINTLN(readMode, BIN);
	}else if(mode == INPUT_PULLUP){
		writeMode = writeMode & ~bit(pin);
		readMode =   readMode | bit(pin);
	}else{
		DEBUG_PRINTLN("Mode non supported by PCF8575")
	}
	DEBUG_PRINT("Write mode: ");
	DEBUG_PRINTLN(writeMode, BIN);

};

/**
 * Read value from i2c and bufferize it
 * @param force
 */
void PCF8575::readBuffer(uint8_t address, bool force){
	if (millis() > PCF8575::lastReadMillis+READ_ELAPSED_TIME || _usingInterrupt || force){
		_wire->requestFrom(address,(uint8_t)2);// Begin transmission to PCF8575 with the buttons
		lastReadMillis = millis();
		if(_wire->available())   // If uint16_ts are available to be recieved
		{
			uint16_t iInput = _wire->read();// Read a uint16_t
			iInput |= _wire->read() << 8;// Read a uint16_t
			if ((iInput & readMode)>0){
				byteBuffered = byteBuffered | (uint16_t)iInput;
			}
		}
	}
}

#ifndef PCF8575_LOW_MEMORY
	/**
	 * Read value of all INPUT pin
	 * Debounce read more fast than 10millis, non managed for interrupt mode
	 * @return
	 */
	PCF8575::DigitalInput PCF8575::digitalReadAll(uint8_t address){
		DEBUG_PRINTLN("Read from buffer");
		_wire->requestFrom(address,(uint8_t)2);// Begin transmission to PCF8575 with the buttons
		lastReadMillis = millis();
		if(_wire->available())   // If uint16_ts are available to be recieved
		{
			  DEBUG_PRINTLN("Data ready");
			  uint16_t iInput = _wire->read();// Read a uint16_t
				iInput |= _wire->read() << 8;// Read a uint16_t

			  if ((iInput & readMode)>0){
				  DEBUG_PRINT("Input ");
				  DEBUG_PRINTLN((uint16_t)iInput, BIN);

				  byteBuffered = byteBuffered | (uint16_t)iInput;
				  DEBUG_PRINT("byteBuffered ");
				  DEBUG_PRINTLN(byteBuffered, BIN);
			  }
		}

		DEBUG_PRINT("Buffer value ");
		DEBUG_PRINTLN(byteBuffered, BIN);
#ifdef NOT_SEQUENTIAL_PINOUT
		if ((bit(0) & readMode)>0) digitalInput.p00 = ((byteBuffered & bit(0))>0)?HIGH:LOW;
		if ((bit(1) & readMode)>0) digitalInput.p01 = ((byteBuffered & bit(1))>0)?HIGH:LOW;
		if ((bit(2) & readMode)>0) digitalInput.p02 = ((byteBuffered & bit(2))>0)?HIGH:LOW;
		if ((bit(3) & readMode)>0) digitalInput.p03 = ((byteBuffered & bit(3))>0)?HIGH:LOW;
		if ((bit(4) & readMode)>0) digitalInput.p04 = ((byteBuffered & bit(4))>0)?HIGH:LOW;
		if ((bit(5) & readMode)>0) digitalInput.p05 = ((byteBuffered & bit(5))>0)?HIGH:LOW;
		if ((bit(6) & readMode)>0) digitalInput.p06 = ((byteBuffered & bit(6))>0)?HIGH:LOW;
		if ((bit(7) & readMode)>0) digitalInput.p07 = ((byteBuffered & bit(7))>0)?HIGH:LOW;
		if ((bit(8) & readMode)>0) digitalInput.p10 = ((byteBuffered & bit(8))>0)?HIGH:LOW;
		if ((bit(9) & readMode)>0) digitalInput.p11 = ((byteBuffered & bit(9))>0)?HIGH:LOW;
		if ((bit(10) & readMode)>0) digitalInput.p12 = ((byteBuffered & bit(10))>0)?HIGH:LOW;
		if ((bit(11) & readMode)>0) digitalInput.p13 = ((byteBuffered & bit(11))>0)?HIGH:LOW;
		if ((bit(12) & readMode)>0) digitalInput.p14 = ((byteBuffered & bit(12))>0)?HIGH:LOW;
		if ((bit(13) & readMode)>0) digitalInput.p15 = ((byteBuffered & bit(13))>0)?HIGH:LOW;
		if ((bit(14) & readMode)>0) digitalInput.p16 = ((byteBuffered & bit(14))>0)?HIGH:LOW;
		if ((bit(15) & readMode)>0) digitalInput.p17 = ((byteBuffered & bit(15))>0)?HIGH:LOW;
#else
		if ((bit(0) & readMode)>0) digitalInput.p0 = ((byteBuffered & bit(0))>0)?HIGH:LOW;
		if ((bit(1) & readMode)>0) digitalInput.p1 = ((byteBuffered & bit(1))>0)?HIGH:LOW;
		if ((bit(2) & readMode)>0) digitalInput.p2 = ((byteBuffered & bit(2))>0)?HIGH:LOW;
		if ((bit(3) & readMode)>0) digitalInput.p3 = ((byteBuffered & bit(3))>0)?HIGH:LOW;
		if ((bit(4) & readMode)>0) digitalInput.p4 = ((byteBuffered & bit(4))>0)?HIGH:LOW;
		if ((bit(5) & readMode)>0) digitalInput.p5 = ((byteBuffered & bit(5))>0)?HIGH:LOW;
		if ((bit(6) & readMode)>0) digitalInput.p6 = ((byteBuffered & bit(6))>0)?HIGH:LOW;
		if ((bit(7) & readMode)>0) digitalInput.p7 = ((byteBuffered & bit(7))>0)?HIGH:LOW;
		if ((bit(8) & readMode)>0) digitalInput.p8 = ((byteBuffered & bit(8))>0)?HIGH:LOW;
		if ((bit(9) & readMode)>0) digitalInput.p9 = ((byteBuffered & bit(9))>0)?HIGH:LOW;
		if ((bit(10) & readMode)>0) digitalInput.p10 = ((byteBuffered & bit(10))>0)?HIGH:LOW;
		if ((bit(11) & readMode)>0) digitalInput.p11 = ((byteBuffered & bit(11))>0)?HIGH:LOW;
		if ((bit(12) & readMode)>0) digitalInput.p12 = ((byteBuffered & bit(12))>0)?HIGH:LOW;
		if ((bit(13) & readMode)>0) digitalInput.p13 = ((byteBuffered & bit(13))>0)?HIGH:LOW;
		if ((bit(14) & readMode)>0) digitalInput.p14 = ((byteBuffered & bit(14))>0)?HIGH:LOW;
		if ((bit(15) & readMode)>0) digitalInput.p15 = ((byteBuffered & bit(15))>0)?HIGH:LOW;
#endif
		if ((readMode & byteBuffered)>0){
			byteBuffered = ~readMode & byteBuffered;
			DEBUG_PRINT("Buffer hight value readed set readed ");
			DEBUG_PRINTLN(byteBuffered, BIN);
		}
		DEBUG_PRINT("Return value ");
		return digitalInput;
	};
#else
	/**
	 * Read value of all INPUT pin in byte format for low memory usage
	 * Debounce read more fast than 10millis, non managed for interrupt mode
	 * @return
	 */
	uint16_t PCF8575::digitalReadAll(void){
		DEBUG_PRINTLN("Read from buffer");
		_wire->requestFrom(_address,(uint8_t)2);// Begin transmission to PCF8575 with the buttons
		lastReadMillis = millis();
		if(_wire->available())   // If uint16_ts are available to be recieved
		{
			  DEBUG_PRINTLN("Data ready");
			  uint16_t iInput = _wire->read();// Read a uint16_t
				iInput |= _wire->read() << 8;// Read a uint16_t

			  if ((iInput & readMode)>0){
				  DEBUG_PRINT("Input ");
				  DEBUG_PRINTLN((uint16_t)iInput, BIN);

				  byteBuffered = byteBuffered | (uint16_t)iInput;
				  DEBUG_PRINT("byteBuffered ");
				  DEBUG_PRINTLN(byteBuffered, BIN);
			  }
		}

		DEBUG_PRINT("Buffer value ");
		DEBUG_PRINTLN(byteBuffered, BIN);

		uint16_t byteRead = byteBuffered;

		if ((readMode & byteBuffered)>0){
			byteBuffered = ~readMode & byteBuffered;
			DEBUG_PRINT("Buffer hight value readed set readed ");
			DEBUG_PRINTLN(byteBuffered, BIN);
		}
		DEBUG_PRINT("Return value ");
		return byteRead;
	};
#endif

uint16_t PCF8575::pcf8575_ReadAll(uint8_t address)
{
   /* Start request, wait for data and receive GPIO values as byte */
	_wire->requestFrom(address, (uint8_t) 0x02);
	while (_wire->available() < 2)
		;
	_PIN = _wire->read(); /* LSB first */
	_PIN |= _wire->read() << 8;	

	return _PIN;
}

uint8_t PCF8575::pcf8575_Read_NEU(uint8_t pin) {

	/* Read GPIO */
	readGPIO();

#ifdef PCF8575_INTERRUPT_SUPPORT
	/* Check for interrupt (manual detection) */
	//checkForInterrupt();
#endif

	/* Read and return the pin state */
	return (_PIN & (1 << pin)) ? HIGH : LOW;
}

void PCF8575::readGPIO() {

	/* Start request, wait for data and receive GPIO values as byte */
	_wire->requestFrom(_address, (uint8_t) 0x02);
	while (_wire->available() < 2)
		;
	_PIN = _wire->read(); /* LSB first */
	_PIN |= _wire->read() << 8;
}

/**
 * Read value of specified pin
 * Debounce read more fast than 10millis, non managed for interrupt mode
 * @param pin
 * @return
 */
uint8_t PCF8575::pcf8575_Read(uint8_t pin){
	uint8_t value = LOW;
	if ((bit(pin) & writeMode)>0){
		DEBUG_PRINTLN("Pin in write mode, return value");
		DEBUG_PRINT("Write data ");
		DEBUG_PRINT(writeByteBuffered, BIN);
		DEBUG_PRINT(" for pin ");
		DEBUG_PRINT(pin);
		DEBUG_PRINT(" bin value ");
		DEBUG_PRINT(bit(pin), BIN);
		DEBUG_PRINT(" value ");
		DEBUG_PRINTLN(value);

		if ((bit(pin) & writeByteBuffered)>0){
			  value = HIGH;
		  }else{
			  value = LOW;
		  }
		return value;
	}

	DEBUG_PRINT("Read pin ");
	DEBUG_PRINTLN(pin);
	// Check if pin already HIGH than read and prevent reread of i2c
	if ((bit(pin) & byteBuffered)>0){
		DEBUG_PRINTLN("Pin already up");
		value = HIGH;
	 }else if ((/*(bit(pin) & byteBuffered)<=0 && */millis() > PCF8575::lastReadMillis+READ_ELAPSED_TIME) /*|| _usingInterrupt*/){
		 DEBUG_PRINTLN("Read from buffer");
		  _wire->requestFrom(_address,(uint8_t)2);// Begin transmission to PCF8575 with the buttons
		  lastReadMillis = millis();
		  if(_wire->available())   // If bytes are available to be recieved
		  {
			  DEBUG_PRINTLN("Data ready");
			  uint16_t iInput = _wire->read();// Read a uint16_t
				iInput |= _wire->read() << 8;// Read a uint16_t

//				Serial.println(iInput, BIN);

			  if ((iInput & readMode)>0){
				  DEBUG_PRINT("Input ");
				  DEBUG_PRINTLN((uint16_t)iInput, BIN);

				  byteBuffered = byteBuffered | (uint16_t)iInput;
				  DEBUG_PRINT("byteBuffered ");
				  DEBUG_PRINTLN(byteBuffered, BIN);

				  if ((bit(pin) & byteBuffered)>0){
					  value = HIGH;
				  }
			  }
		  }
	}
	DEBUG_PRINT("Buffer value ");
	DEBUG_PRINTLN(byteBuffered, BIN);
	// If HIGH set to low to read buffer only one time
	if (value==HIGH){
		byteBuffered = ~bit(pin) & byteBuffered;
		DEBUG_PRINT("Buffer hight value readed set readed ");
		DEBUG_PRINTLN(byteBuffered, BIN);
	}
	DEBUG_PRINT("Return value ");
	DEBUG_PRINTLN(value);
	return value;
};

/**
 * Write on pin
 * @param pin
 * @param value
 */
void PCF8575::pcf8575_Write(uint8_t pin, uint8_t value){
	DEBUG_PRINTLN("Begin trasmission");
	_wire->beginTransmission(_address);     //Begin the transmission to PCF8575
	if (value==HIGH){
		writeByteBuffered = writeByteBuffered | bit(pin);
	}else{
		writeByteBuffered = writeByteBuffered & ~bit(pin);
	}
//	DEBUG_PRINT("Write data ");
//	DEBUG_PRINT(writeByteBuffered, BIN);
//	DEBUG_PRINT(" for pin ");
//	DEBUG_PRINT(pin);
//	DEBUG_PRINT(" bin value ");
//	DEBUG_PRINT(bit(pin), BIN);
//	DEBUG_PRINT(" value ");
//	DEBUG_PRINTLN(value);

//	Serial.print(" --> ");
//	Serial.println(writeByteBuffered);
//	Serial.println((uint8_t) writeByteBuffered);
//	Serial.println((uint8_t) (writeByteBuffered >> 8));

	writeByteBuffered = writeByteBuffered & writeMode;
	_wire->write((uint8_t) writeByteBuffered);
	_wire->write((uint8_t) (writeByteBuffered >> 8));
	DEBUG_PRINTLN("Start end trasmission if stop here check pullup resistor.");

	_wire->endTransmission();
};

void PCF8575::pcf8575_WriteALL(uint16_t value){
	_wire->beginTransmission(_address);     //Begin the transmission to PCF8575
	_wire->write((uint8_t) value);
	_wire->write((uint8_t) (value >> 8));
	_wire->endTransmission();
};

void PCF8575::pcf8575_Clear(){
	_wire->beginTransmission(_address);     //Begin the transmission to PCF8575
	_wire->write(0xFF);
	_wire->write(0xFF);
	_wire->endTransmission();
};


