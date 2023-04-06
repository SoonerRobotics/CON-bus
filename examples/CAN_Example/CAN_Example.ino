/*
 * CAN_Example
 * 
 * Implements CONBus over CANBus using the ACAN2515 CAN library.
 * 
 * created 2023-04-06
 * by Noah Zemlin
 */

#include <CONBus.h> 
#include <ACAN2515.h>

// Setup CANBus using ACAN2515
static const byte MCP2515_SCK = 10;   // SCK input of MCP2515
static const byte MCP2515_MOSI = 11;  // SDI input of MCP2515
static const byte MCP2515_MISO = 8;   // SDO output of MCP2515

static const byte MCP2515_CS = 9;   // CS input of MCP2515
static const byte MCP2515_INT = 7;  // INT output of MCP2515

ACAN2515 can(MCP2515_CS, SPI1, MCP2515_INT);
static const uint32_t QUARTZ_FREQUENCY = 8UL * 1000UL * 1000UL;  // 8 MHz

// Setup CONBus variables
CONBus::CONBus conbus;

static const uint8_t DEVICE_ID = 42; // CONBus Device ID

bool useOven = false;
int numberOfOvenRacks = 0;
float ovenTemperatureSetpoint = 100;

// Static variables for us in loop/interrupts
CANMessage frame;
CANMessage outFrame;

bool awaiting_read_response = false;
uint8_t register_to_fetch = 0;

bool awaiting_write_response = false;

typedef struct{
    uint8_t registerAddress;
} CAN_readRegisterMessage;

CAN_readRegisterMessage readRegisterMessage;

typedef struct{
    uint8_t registerAddress;
    uint8_t length;
    uint8_t reserved_;
    uint8_t value[4];
} CAN_readRegisterResponseMessage;

CAN_readRegisterResponseMessage readRegisterResponseMessage;

typedef struct{
    uint8_t registerAddress;
    uint8_t length;
    uint8_t reserved_;
    uint8_t value[4];
} CAN_writeRegisterMessage;

CAN_writeRegisterMessage writeRegisterMessage;

typedef struct{
    uint8_t registerAddress;
    uint8_t length;
    uint8_t reserved_;
    uint8_t value[4];
} CAN_writeRegisterResponseMessage;

CAN_writeRegisterResponseMessage writeRegisterResponseMessage;

void setup() {

  while (!Serial) {
    // Wait for a Serial connection...
  }

  Serial.begin(115200);

  // Setup SPI controller (using Raspberry Pi Pico)
  SPI1.setSCK(MCP2515_SCK);
  SPI1.setTX(MCP2515_MOSI);
  SPI1.setRX(MCP2515_MISO);
  SPI1.setCS(MCP2515_CS);
  SPI1.begin();

  Serial.println("Configure ACAN2515");

  ACAN2515Settings settings(QUARTZ_FREQUENCY, 100UL * 1000UL);  // CAN bit rate 100 kb/s
  settings.mRequestedMode = ACAN2515Settings::NormalMode ; // Select Normal mode
  const uint16_t errorCode = can.begin(settings, onCanRecieve );

  if (errorCode == 0) {
    Serial.print("Actual bit rate: ");
    Serial.print(settings.actualBitRate());
  } else {
    Serial.print(errorCode);
  }

  // Create CONBus registers
  // The addresses can be anything from 0 to 255
  conbus.addRegister(0, &useOven);
  conbus.addRegister(4, &numberOfOvenRacks);
  conbus.addRegister(21, &ovenTemperatureSetpoint);
}

void loop() {
  if (awaiting_read_response) {
    awaiting_read_response = false;

    readRegisterResponseMessage.registerAddress = register_to_fetch;
    conbus.readRegisterBytes(register_to_fetch, readRegisterResponseMessage.value, readRegisterResponseMessage.length);

    outFrame.id = 1100 + DEVICE_ID;
    outFrame.len = sizeof(readRegisterResponseMessage);
    memcpy(outFrame.data, &readRegisterResponseMessage, sizeof(readRegisterResponseMessage));

    const bool ok = can.tryToSend(outFrame);
  }

  if (awaiting_write_response) {
    awaiting_write_response = false;

    outFrame.id = 1300 + DEVICE_ID;
    outFrame.len = sizeof(writeRegisterResponseMessage);
    memcpy(outFrame.data, &writeRegisterResponseMessage, sizeof(writeRegisterResponseMessage));

    const bool ok = can.tryToSend(outFrame);
  }
}

void onCanRecieve() {
  can.isr();
  can.receive(frame);  

  // CONBus read register
  if (frame.id == (1000 + DEVICE_ID)) {
    awaiting_read_response = true;

    readRegisterMessage = *(CAN_readRegisterMessage*)frame.data;
    register_to_fetch = readRegisterMessage.registerAddress;
  }

  // CONBus write register
  if (frame.id == (1200 + DEVICE_ID)) {
    awaiting_write_response = true;

    writeRegisterMessage = *(CAN_writeRegisterMessage*)frame.data;
    conbus.writeRegisterBytes(writeRegisterMessage.registerAddress, writeRegisterMessage.value, writeRegisterMessage.length);
  
    memcpy(&writeRegisterResponseMessage, &writeRegisterMessage, sizeof(writeRegisterResponseMessage));
  }
}