#ifndef VIRTMEM_SERRAM_ALLOC_H
#define VIRTMEM_SERRAM_ALLOC_H

#include <Arduino.h>
#include "alloc.h"
#include "serram_utils.h"

// UNDONE: settings below
struct SSerRAMMemAllocProperties
{
    static const uint8_t smallPageCount = 4, smallPageSize = 32;
    static const uint8_t mediumPageCount = 4, mediumPageSize = 64;
    static const uint8_t bigPageCount = 4;
    static const uint16_t bigPageSize = 1024;
    static const uint32_t poolSize = 1024 * 1024; // 1024 kB
};

template <typename IOStream, typename TProperties=SSerRAMMemAllocProperties>
class CSerRAMVirtMemAlloc : public CVirtMemAlloc<TProperties, CSerRAMVirtMemAlloc<IOStream, TProperties> >
{
    uint32_t baudRate;
    IOStream *stream;

    void doStart(void)
    {
        SerramUtils::init(baudRate, TProperties::poolSize);
    }

    void doSuspend(void) { } // UNDONE
    void doStop(void) { }

    void doRead(void *data, TVirtPtrSize offset, TVirtPtrSize size)
    {
        uint32_t t = micros();
        SerramUtils::sendReadCommand(SerramUtils::CMD_READ);
        SerramUtils::writeUInt32(offset);
        SerramUtils::writeUInt32(size);
        SerramUtils::waitForCommand(SerramUtils::CMD_READ);
        Serial.print("read(1): "); Serial.print(size); Serial.print("/"); Serial.println(micros() - t);
        t = micros();
        SerramUtils::readBlock((char *)data, size);
        t = micros() - t;
        Serial.print("read(2): "); Serial.print(size); Serial.print("/"); Serial.println(t);
    }

    void doWrite(const void *data, TVirtPtrSize offset, TVirtPtrSize size)
    {
        const uint32_t t = micros();
        SerramUtils::sendWriteCommand(SerramUtils::CMD_WRITE);
        SerramUtils::writeUInt32(offset);
        SerramUtils::writeUInt32(size);
        SerramUtils::writeBlock((const char *)data, size);
        Serial.print("write: "); Serial.print(size); Serial.print("/"); Serial.println(micros() - t);
    }

public:
    SerramUtils::CSerialInput input;

    CSerRAMVirtMemAlloc(uint32_t baud=115200) : stream(&Serial), baudRate(baud) { }

    // only works before start() is called
    void setBaudRate(uint32_t baud) { baudRate = baud; }
};

template <typename, typename> class CVirtPtr;
template <typename T> struct TSerRAMVirtPtr { typedef CVirtPtr<T, CSerRAMVirtMemAlloc<typeof(Serial)> > type; };


#endif // VIRTMEM_SERRAM_ALLOC_H
