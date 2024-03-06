#include "AceBMS.h"
// #include "AceBus.h"
#include "AceDump.h"
#include "AceMPPT.h"
#include "AcePump.h"
#include "MCP2515.h"
#include "VEDirect.h"

#include "NTC.h"

#define CELL_COUNT (8)
#define BALANCE_TIMEOUT (5 * 10)

#define VSETPOINT (26700)
#define ILIMIT (40000)

void mpptCallback_0(uint16_t id, int32_t value);
void mpptCallback_1(uint16_t id, int32_t value);

VEDirect mppt_0(Serial2, mpptCallback_0);
VEDirect mppt_1(Serial3, mpptCallback_1);

uint16_t panelVoltage[2] = {0};
uint16_t chargeCurrent[2] = {0};

uint8_t frameSequence = 0;
struct can_frame canMsg;
MCP2515 mcp2515(53);

#define CANBUS_CAN_ID_SHUNT 40
#define CANBUS_CAN_ID_BMS 300

typedef struct {
    int16_t cellVoltage[CELL_COUNT];
    int16_t temperature[2];
    int32_t chargeMilliAmps;
    int32_t chargeMilliAmpSeconds;
    int16_t balanceVoltage;
    int16_t balanceTimeout;
} bms_t;

static bms_t bms = {0};

int16_t cellVoltage[CELL_COUNT];
uint32_t senseVoltage = 0;
uint16_t dumpSetPoint = VSETPOINT - 50;
uint16_t chargeSetPoint = VSETPOINT + 25;

unsigned long debugTimer = 0;

void setup() {
    Serial.begin(9600);

    mppt_0.begin();
    mppt_0.restart();

    mppt_1.begin();
    mppt_1.restart();

    SPI.begin();
    mcp2515.reset();
    mcp2515.setBitrate(CAN_250KBPS, MCP_8MHZ);
    mcp2515.setNormalMode();
}

void process(void) {
    // called 4 times per second, triggered by current shunt canbus message
    static int8_t count = 0;

    uint16_t cellMin = bms.cellVoltage[0];
    uint16_t cellMax = cellMin;
    uint16_t cellSum = 0;

    uint8_t i = CELL_COUNT;
    while (i--) {
        uint16_t v = bms.cellVoltage[i];
        if (v > cellMax)
            cellMax = v;
        if (v < cellMin)
            cellMin = v;
        cellSum += v;
    }

    if (cellMin) {
        // full set of cell voltage data received
        bms.balanceTimeout = 0;
        senseVoltage = cellSum;
        uint16_t cellAvg = cellSum / CELL_COUNT;
        bms.balanceVoltage = cellAvg + 2;
        if (bms.balanceVoltage < 3200) {
            bms.balanceVoltage = 3200;
        }
        int i = CELL_COUNT;
        while (i--) {
            cellVoltage[i] = bms.cellVoltage[i];
            bms.cellVoltage[i] = 0; // reset cell voltages
        }
    }

    if (++frameSequence >= (4 * 60)) {
        frameSequence = 0; // force rollover on minute boundary
        // bms.balanceVoltage = 0; // reset balance voltage
    }

    msg_t msg;
    if (frameSequence == SIG_MSG_ID(ACEBMS_CEL1)) {
        sig_encode(&msg, ACEBMS_CEL1, cellVoltage[0]);
        sig_encode(&msg, ACEBMS_CEL2, cellVoltage[1]);
        sig_encode(&msg, ACEBMS_CEL3, cellVoltage[2]);
        sig_encode(&msg, ACEBMS_CEL4, cellVoltage[3]);
    } else if (frameSequence == SIG_MSG_ID(ACEBMS_CEL5)) {
        sig_encode(&msg, ACEBMS_CEL5, cellVoltage[4]);
        sig_encode(&msg, ACEBMS_CEL6, cellVoltage[5]);
        sig_encode(&msg, ACEBMS_CEL7, cellVoltage[6]);
        sig_encode(&msg, ACEBMS_CEL8, cellVoltage[7]);
    } else if (frameSequence == SIG_MSG_ID(ACEBMS_VBAL)) {
        sig_encode(&msg, ACEBMS_VBAL, bms.balanceVoltage);
        sig_encode(&msg, ACEBMS_CHAH,
                   ((bms.chargeMilliAmpSeconds >> 8L) * 466L) >> 16L);
        sig_encode(&msg, ACEBMS_BTC1, bms.temperature[0]);
        sig_encode(&msg, ACEBMS_BTC2, bms.temperature[1]);
    } else if (frameSequence == SIG_MSG_ID(ACEMPPT_ICH_0)) {
        sig_encode(&msg, ACEMPPT_ICH_0, chargeCurrent[0]);
        sig_encode(&msg, ACEMPPT_ICH_1, chargeCurrent[1]);
        sig_encode(&msg, ACEMPPT_VPV_0, panelVoltage[0]);
        sig_encode(&msg, ACEMPPT_VPV_1, panelVoltage[1]);
    } else {
        sig_encode(&msg, ACEBMS_VBAT, SIG_DIVU16BY10(cellSum + 5));
        sig_encode(&msg, ACEBMS_IBAT,
                   SIG_DIVS16BY10(bms.chargeMilliAmps + 5));
    }
    logMessage(&msg);
}

void loop() {
    mppt_0.update();
    mppt_1.update();

    static uint8_t dsecs = 0;
    static unsigned long time = 0;
    unsigned long now = micros();
    if (now - time >= 200000L) { //  run every 200 ms
        time = now;
        if (bms.balanceTimeout++ > BALANCE_TIMEOUT) {
            bms.balanceTimeout = 0;
            bms.balanceVoltage = 0; // reset balance voltage
            int i = CELL_COUNT;
            while (i--) {
                cellVoltage[i] = 0;
            }
        }
        if (senseVoltage != 0) {
            mppt_0.set(VEDirect_kBatterySense, SIG_DIVU16BY10(senseVoltage));
            mppt_1.set(VEDirect_kBatterySense, SIG_DIVU16BY10(senseVoltage));
            senseVoltage = 0;
        } else {
            dsecs++; // round robin of messages
            if (dsecs > 2)
                dsecs = 0;
            if (dsecs == 0) {
                mppt_0.set(VEDirect_kNetworkMode,
                           VEDirect_kExternalControlMode);
                mppt_1.set(VEDirect_kNetworkMode,
                           VEDirect_kExternalControlMode);
            }
            if (dsecs == 1) {
                mppt_0.set(VEDirect_VoltageSetpoint,
                           SIG_DIVU16BY10(chargeSetPoint));
                mppt_1.set(VEDirect_VoltageSetpoint,
                           SIG_DIVU16BY10(chargeSetPoint));
            }
            if (dsecs == 2) {
                mppt_0.set(VEDirect_CurrentLimit, SIG_DIVU16BY100(ILIMIT));
                mppt_1.set(VEDirect_CurrentLimit, SIG_DIVU16BY100(ILIMIT));
            }
        }
    }

    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
        if (canMsg.can_id == (CANBUS_CAN_ID_SHUNT | CAN_EFF_FLAG)) {
            if (canMsg.can_dlc == 8) {
                int32_t current = (int32_t)(((uint32_t)canMsg.data[0] << 16) +
                                            ((uint32_t)canMsg.data[1] << 8) +
                                            (uint32_t)canMsg.data[2]) -
                                  8388608L;
                bms.chargeMilliAmps = current;
                bms.chargeMilliAmpSeconds += current / 4;

                process();

                // send can message
                canMsg.can_id = (CANBUS_CAN_ID_BMS | CAN_EFF_FLAG);
                canMsg.can_dlc = 2;
                canMsg.data[0] = bms.balanceVoltage >> 8;
                canMsg.data[1] = bms.balanceVoltage;
                mcp2515.sendMessage(&canMsg);
            }
        }
        if (canMsg.can_id == ((CANBUS_CAN_ID_BMS + 1) | CAN_EFF_FLAG)) {
            if (canMsg.can_dlc == 8) {
                // update cell voltages
                bms.cellVoltage[0] =
                    ((uint16_t)canMsg.data[0] << 8) + (uint16_t)canMsg.data[1];
                bms.cellVoltage[1] =
                    ((uint16_t)canMsg.data[2] << 8) + (uint16_t)canMsg.data[3];
                bms.cellVoltage[2] =
                    ((uint16_t)canMsg.data[4] << 8) + (uint16_t)canMsg.data[5];
                bms.cellVoltage[3] =
                    ((uint16_t)canMsg.data[6] << 8) + (uint16_t)canMsg.data[7];
            }
        }
        if (canMsg.can_id == ((CANBUS_CAN_ID_BMS + 2) | CAN_EFF_FLAG)) {
            if (canMsg.can_dlc == 8) {
                // update cell voltages
                bms.cellVoltage[4] =
                    ((uint16_t)canMsg.data[0] << 8) + (uint16_t)canMsg.data[1];
                bms.cellVoltage[5] =
                    ((uint16_t)canMsg.data[2] << 8) + (uint16_t)canMsg.data[3];
                bms.cellVoltage[6] =
                    ((uint16_t)canMsg.data[4] << 8) + (uint16_t)canMsg.data[5];
                bms.cellVoltage[7] =
                    ((uint16_t)canMsg.data[6] << 8) + (uint16_t)canMsg.data[7];
            }
        }
        if (canMsg.can_id == ((CANBUS_CAN_ID_BMS + 4) | CAN_EFF_FLAG)) {
            if (canMsg.can_dlc == 8) {
                // update temperatures
                bms.temperature[0] = (int16_t)canMsg.data[0] - 40;
                bms.temperature[1] = (int16_t)canMsg.data[1] - 40;
            }
        }
    }
}

void hexDump(char *tag, uint8_t *buffer, int size) {
    int i = 0;
    Serial.print(tag);
    Serial.print(" : ");
    while (i < size) {
        Serial.print(buffer[i] >> 4, HEX);
        Serial.print(buffer[i++] & 0x0F, HEX);
        Serial.print(" ");
    }
    Serial.println();
}

static sig_name_t sigNames[] = {ACEBMS_NAMES, ACEMPPT_NAMES, ACEDUMP_NAMES};
static const int sigCount = (sizeof(sigNames) / sizeof(sig_name_t));

void sendByte(int16_t txChar) {
    static uint16_t checksum = 0;
    if (txChar == EOF) {
        Serial.print(F(",cs=")); // append lrc to message
        Serial.print(checksum);
        Serial.print("\n");
        checksum = 0;
    } else {
        checksum += (uint16_t)(txChar & 0xFF);
        checksum += 0xFF;
        Serial.write((uint8_t)(txChar & 0xFF));
    }
}

void sendString(char *str) {
    while (*str) {
        sendByte((int16_t)((uint16_t)*str++));
    }
}

void logMessage(msg_t *msg) {
    int index = 0;
    int count = 0;
    if (Serial.availableForWrite() < 56)
        return;
    while (index < sigCount) {
        int16_t value;
        fmt_t format = sig_decode(msg, sigNames[index].sig, &value);
        if (format != FMT_NULL) {
            if (count++ == 0)
                sendByte(':');
            else
                sendByte(',');
            sendString(sigNames[index].name);
            sendByte('=');
            char valueBuffer[FMT_MAXSTRLEN] = {0};
            sig_toString(msg, sigNames[index].sig, valueBuffer);
            sendString(valueBuffer);
        }
        index++;
    }
    if (count)
        sendByte(EOF);
}

void mpptCallback_0(uint16_t id, int32_t value) {
    if (id == VEDirect_kPanelVoltage) {
        panelVoltage[0] = SIG_DIVU16BY10(value);
    }
    if (id == VEDirect_kChargeCurrent) {
        chargeCurrent[0] = value;
    }
}

void mpptCallback_1(uint16_t id, int32_t value) {
    if (id == VEDirect_kPanelVoltage) {
        panelVoltage[1] = SIG_DIVU16BY10(value);
    }
    if (id == VEDirect_kChargeCurrent) {
        chargeCurrent[1] = value;
    }
}
