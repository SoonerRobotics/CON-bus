#ifndef CANBUS_DRIVER_H
#define CANBUS_DRIVER_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "CONBus.h"

namespace CONBus{

typedef struct{
    uint8_t registerAddress;
} CAN_readRegisterMessage;

typedef struct{
    uint8_t registerAddress;
    uint8_t length;
    uint8_t reserved_;
    uint8_t value[4];
} CAN_readRegisterResponseMessage;

typedef struct{
    uint8_t registerAddress;
    uint8_t length;
    uint8_t reserved_;
    uint8_t value[4];
} CAN_writeRegisterMessage;

typedef struct{
    uint8_t registerAddress;
    uint8_t length;
    uint8_t reserved_;
    uint8_t value[4];
} CAN_writeRegisterResponseMessage;

class CANBusDriver {
    public:
        CANBusDriver(CONBus& conbus, const uint32_t device_id);

        uint8_t readCanMessage(const uint32_t can_id, const void* buffer);
        bool isReplyReady();
        uint8_t getReply(uint32_t& can_id, uint8_t& can_len, void* buffer);

    private:
        CONBus& conbus_;
        const uint8_t device_id_;

        CAN_readRegisterMessage readRegisterMessage_;
        CAN_readRegisterResponseMessage readRegisterResponseMessage_;
        CAN_writeRegisterMessage writeRegisterMessage_;
        CAN_writeRegisterResponseMessage writeRegisterResponseMessage_;

        bool awaiting_read_response_ = false;
        uint8_t register_to_fetch_ = 0;
        bool awaiting_write_response_ = false;
};

inline CANBusDriver::CANBusDriver(CONBus& conbus, const uint32_t device_id) : conbus_(conbus), device_id_(device_id) {}

inline uint8_t CANBusDriver::readCanMessage(const uint32_t can_id, const void* buffer) {
    // CONBus read register
    if (can_id == ((uint32_t)1000 + device_id_)) {
        awaiting_read_response_ = true;

        readRegisterMessage_ = *(CAN_readRegisterMessage*)buffer;
        register_to_fetch_ = readRegisterMessage_.registerAddress;
    }

    // CONBus write register
    if (can_id == ((uint32_t)1200 + device_id_)) {
        awaiting_write_response_ = true;

        writeRegisterMessage_ = *(CAN_writeRegisterMessage*)buffer;
        conbus_.writeRegisterBytes(writeRegisterMessage_.registerAddress, writeRegisterMessage_.value, writeRegisterMessage_.length);

        memcpy(&writeRegisterResponseMessage_, &writeRegisterMessage_, sizeof(writeRegisterResponseMessage_));
    }

    return SUCCESS;
}

inline bool CANBusDriver::isReplyReady() {
    return awaiting_read_response_ || awaiting_write_response_;
}

inline uint8_t CANBusDriver::getReply(uint32_t& can_id, uint8_t& can_len, void* buffer) {
    if (awaiting_read_response_) {
        awaiting_read_response_ = false;

        readRegisterResponseMessage_.registerAddress = register_to_fetch_;
        conbus_.readRegisterBytes(register_to_fetch_, readRegisterResponseMessage_.value, readRegisterResponseMessage_.length);

        can_id = 1100 + device_id_;
        // The readRegisterResponseMessage_ is the full 7 bytes for a 4 byte buffer
        // but if we have a smaller message, we should reduce the size
        can_len = sizeof(readRegisterResponseMessage_) - (4 - readRegisterResponseMessage_.length);

        memcpy(buffer, &readRegisterResponseMessage_, sizeof(readRegisterResponseMessage_));
        return SUCCESS; // end early so we dont overwrite a read response with a write response
    }

    if (awaiting_write_response_) {
        awaiting_write_response_ = false;

        can_id = 1300 + device_id_;
        // Same as above, we have to reduce the size appropriately.
        can_len = sizeof(writeRegisterResponseMessage_) - (4 - writeRegisterResponseMessage_.length);
        memcpy(buffer, &writeRegisterResponseMessage_, sizeof(writeRegisterResponseMessage_));
    }

    return SUCCESS;
}



} // end CONBus namespace

#endif