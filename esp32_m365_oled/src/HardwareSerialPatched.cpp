#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "HardwareSerialPatched.h"

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL)
HardwareSerialPatched PatchedSerial(0);
HardwareSerialPatched PatchedSerial1(1);
HardwareSerialPatched PatchedSerial2(2);
#endif

HardwareSerialPatched::HardwareSerialPatched(int uart_nr) : _uart_nr(uart_nr), _uart(NULL) {}

void HardwareSerialPatched::begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert)
{
    if(0 > _uart_nr || _uart_nr > 2) {
        log_e("Serial number is invalid, please use 0, 1 or 2");
        return;
    }
    if(_uart) {
        end();
    }
    if(_uart_nr == 0 && rxPin < 0 && txPin < 0) {
        rxPin = 3;
        txPin = 1;
    }
    if(_uart_nr == 1 && rxPin < 0 && txPin < 0) {
        rxPin = 9;
        txPin = 10;
    }
    if(_uart_nr == 2 && rxPin < 0 && txPin < 0) {
        rxPin = 16;
        txPin = 17;
    }
    _uart = uartBegin(_uart_nr, baud, config, rxPin, txPin, 256, invert);
}

void HardwareSerialPatched::end()
{
    if(uartGetDebug() == _uart_nr) {
        uartSetDebug(0);
    }
    uartEnd(_uart);
    _uart = 0;
}

void HardwareSerialPatched::setDebugOutput(bool en)
{
    if(_uart == 0) {
        return;
    }
    if(en) {
        uartSetDebug(_uart);
    } else {
        if(uartGetDebug() == _uart_nr) {
            uartSetDebug(0);
        }
    }
}

int HardwareSerialPatched::available(void)
{
    return uartAvailable(_uart);
}
int HardwareSerialPatched::availableForWrite(void)
{
    return uartAvailableForWrite(_uart);
}

int HardwareSerialPatched::peek(void)
{
    if (available()) {
        return uartPeek(_uart);
    }
    return -1;
}

int HardwareSerialPatched::read(void)
{
    if(available()) {
        return uartRead(_uart);
    }
    return -1;
}

void HardwareSerialPatched::flush()
{
    uartFlush(_uart);
}

size_t HardwareSerialPatched::write(uint8_t c)
{
    uartWrite(_uart, c);
    return 1;
}

size_t HardwareSerialPatched::write(const uint8_t *buffer, size_t size)
{
    uartWriteBuf(_uart, buffer, size);
    return size;
}
uint32_t  HardwareSerialPatched::baudRate()

{
	return uartGetBaudRate(_uart);
}
HardwareSerialPatched::operator bool() const
{
    return true;
}
