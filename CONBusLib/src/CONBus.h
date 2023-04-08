#ifndef CONBUS_H
#define CONBUS_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

namespace CONBus{

// Define error codes for all CONBus methods
static const uint8_t SUCCESS = 0;
static const uint8_t ERROR_UNKNOWN = 1;
static const uint8_t ERROR_ADDRESS_ALREADY_USED = 2;
static const uint8_t ERROR_INVALID_LENGTH = 3;
static const uint8_t ERROR_ADDRESS_NOT_REGISTERED = 4;
static const uint8_t ERROR_DIFFERENT_TYPE_THAN_REGISTERED = 5;

// Define CONBus specification constants
static const uint8_t MAX_REGISTER_LENGTH = 4;

class CONBus {
    public:
        CONBus();

        template <typename T>
        uint8_t addRegister(const uint8_t conbus_address, T* register_address);

        template <typename T>
        uint8_t writeRegister(const uint8_t conbus_address, const T value);
        uint8_t writeRegisterBytes(const uint8_t conbus_address, const void* buffer, const uint8_t length);

        template <typename T>
        uint8_t readRegister(const uint8_t conbus_address, T& value);
        uint8_t readRegisterBytes(const uint8_t conbus_address, void* buffer, uint8_t& length);
    private:
        uint8_t length_map_[256];
        void* pointer_map_[256];
};

inline CONBus::CONBus() {
    // Clear length map
    for (int i=0; i<256; i++) {
        length_map_[i] = 0;
    }
}

template <typename T>
inline uint8_t CONBus::addRegister(const uint8_t conbus_address, T* register_address) {
    if (length_map_[conbus_address] > 0) {
        return ERROR_ADDRESS_ALREADY_USED;
    }

    if (sizeof(T) > MAX_REGISTER_LENGTH) {
        return ERROR_INVALID_LENGTH;
    }

    length_map_[conbus_address] = sizeof(T);
    pointer_map_[conbus_address] = register_address;

    return SUCCESS;
}

template <typename T>
inline uint8_t CONBus::writeRegister(const uint8_t conbus_address, const T value) {
    if (length_map_[conbus_address] == 0) {
        return ERROR_ADDRESS_NOT_REGISTERED;
    }

    if (sizeof(T) != length_map_[conbus_address]) {
        return ERROR_DIFFERENT_TYPE_THAN_REGISTERED;
    }

    *(T*)(pointer_map_[conbus_address]) = value;

    return SUCCESS;
}

inline uint8_t CONBus::writeRegisterBytes(const uint8_t conbus_address, const void* buffer, const uint8_t length) {
    if (length_map_[conbus_address] == 0) {
        return ERROR_ADDRESS_NOT_REGISTERED;
    }

    if (length != length_map_[conbus_address]) {
        return ERROR_DIFFERENT_TYPE_THAN_REGISTERED;
    }

    memcpy(pointer_map_[conbus_address], buffer, length);

    return SUCCESS;
}

template <typename T>
inline uint8_t CONBus::readRegister(const uint8_t conbus_address, T& value) {
    if (length_map_[conbus_address] == 0) {
        return ERROR_ADDRESS_NOT_REGISTERED;
    }

    if (sizeof(T) != length_map_[conbus_address]) {
        return ERROR_DIFFERENT_TYPE_THAN_REGISTERED;
    }

    value = *(T*)(pointer_map_[conbus_address]);

    return SUCCESS;
}

inline uint8_t CONBus::readRegisterBytes(const uint8_t conbus_address, void* buffer, uint8_t& length) {
    memcpy(buffer, pointer_map_[conbus_address], length_map_[conbus_address]);
    length = length_map_[conbus_address];

    return SUCCESS;
}

} // end CONBus namespace

#endif