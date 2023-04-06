#include <CONBus.h>
#include <catch.hpp>

TEST_CASE("Initialize test") {
    CONBus::CONBus conbus;

    bool x;
    uint32_t y;
    float z;

    conbus.addRegister(10, &x);
    conbus.addRegister(20, &y);
    conbus.addRegister(30, &z);
}

TEST_CASE("Write test") {
    CONBus::CONBus conbus;

    bool x = false;
    uint32_t y = 10;
    float z = 42.0f;

    conbus.addRegister(10, &x);
    conbus.addRegister(20, &y);
    conbus.addRegister(30, &z);

    REQUIRE(x == false);
    REQUIRE(y == 10);
    REQUIRE(z == 42.0f);

    conbus.writeRegister(10, true);
    conbus.writeRegister(20, 99);
    conbus.writeRegister(30, 7.8f);

    REQUIRE(x == true);
    REQUIRE(y == 99);
    REQUIRE(z == 7.8f);
}

TEST_CASE("Read test") {
    CONBus::CONBus conbus;

    bool x = false;
    uint32_t y = 10;
    float z = 42.0f;

    conbus.addRegister(10, &x);
    conbus.addRegister(20, &y);
    conbus.addRegister(30, &z);

    conbus.writeRegister(10, true);
    conbus.writeRegister(20, 99);
    conbus.writeRegister(30, 7.8f);

    bool x2;
    uint32_t y2;
    float z2;

    conbus.readRegister(10, x2);
    conbus.readRegister(20, y2);
    conbus.readRegister(30, z2);

    REQUIRE(x2 == true);
    REQUIRE(y2 == 99);
    REQUIRE(z2 == 7.8f);
}

TEST_CASE("Read bytes test") {
    CONBus::CONBus conbus;

    bool x = false;
    uint32_t y = 10;
    float z = 42.0f;

    conbus.addRegister(10, &x);
    conbus.addRegister(20, &y);
    conbus.addRegister(30, &z);

    conbus.writeRegister(10, true);
    conbus.writeRegister(20, 99);
    conbus.writeRegister(30, 7.8f);

    bool x2;
    uint32_t y2;
    float z2;

    uint8_t x2_length = 0;
    uint8_t y2_length = 0;
    uint8_t z2_length = 0;
    conbus.readRegisterBytes(10, &x2, x2_length);
    conbus.readRegisterBytes(20, &y2, y2_length);
    conbus.readRegisterBytes(30, &z2, z2_length);

    REQUIRE(x2 == true);
    REQUIRE(y2 == 99);
    REQUIRE(z2 == 7.8f);

    REQUIRE(x2_length == sizeof(bool));
    REQUIRE(y2_length == sizeof(uint32_t));
    REQUIRE(z2_length == sizeof(float));
}