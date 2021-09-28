#include "AceBMS.h"
#include "AceBus.h"
#include "AceDump.h"
#include "AceGrid.h"
#include "AceMPPT.h"
#include "VEDirect.h"

#define kRxInterruptPin (19)
void aceCallback(tinframe_t *frame);
AceBus aceBus(Serial1, kRxInterruptPin, aceCallback);

void mppt2Callback(uint16_t id, int32_t value);
void mppt3Callback(uint16_t id, int32_t value);
VEDirect mppt2(Serial2, mppt2Callback);
VEDirect mppt3(Serial3, mppt3Callback);

uint16_t panelVoltage2 = 0;
uint16_t chargeCurrent2 = 0;
uint16_t panelVoltage3 = 0;
uint16_t chargeCurrent3 = 0;

// void hexDump(char *tag, uint8_t *buffer, int size) {
//   int i = 0;
//   Serial.print(tag);
//   Serial.print(" : ");
//   while (i < size) {
//     Serial.print(buffer[i] >> 4, HEX);
//     Serial.print(buffer[i++] & 0x0F, HEX);
//     Serial.print(" ");
//   }
//   Serial.println();
// }

void setup() {
  Serial.begin(115200);
  mppt2.begin();
  mppt3.begin();
  aceBus.begin();
  mppt2.ping();
}

void loop() {
  mppt2.update();
  mppt3.update();

  if (millis() % 1000 == 0) {
    mppt2.ping();
    mppt3.ping();
  }

  int status = aceBus.update();
  if (status == AceBus_kWriteCollision)
    Serial.println("tinbus/error\t\"collision\"");
  if (status == AceBus_kWriteTimeout)
    // Serial.println("TX Timeout");
    Serial.println("tinbus/error\t\"timeout\"");
  // if (status == AceBus_kWriteComplete) Serial.println("TX Complete");
}

static sig_name_t sigNames[] = {ACEBMS_NAMES, ACEMPPT_NAMES,
                                ACEDUMP_NAMES, ACEGRID_NAMES};
static const int sigCount = (sizeof(sigNames) / sizeof(sig_name_t));
static int16_t values[sigCount];

void mqttPublish(msg_t *msg) {
  int index = 0;
  while (index < sigCount) {
    int16_t value;
    fmt_t format = sig_decode(msg, sigNames[index].sig, &value);
    if (format != FMT_NULL) {
      if(value != values[index]){
        values[index] = value;
        Serial.print("tinbus/");
        Serial.print(sigNames[index].name);
        Serial.print("\t");
        char valueBuffer[FMT_MAXSTRLEN] = {0};
        sig_toString(msg, sigNames[index].sig, valueBuffer);
        Serial.println(valueBuffer);
      }
    }
    index++;
  }
}

void aceCallback(tinframe_t *rxFrame) {
  // if (status == AceBus_kReadDataReady) {
  //   tinframe_t rxFrame;
  //   aceBus.read(&rxFrame);
  // hexDump("RX ", (unsigned char*)&rxFrame, tinframe_kFrameSize);
  msg_t *msg = (msg_t *)(rxFrame->data);

  mqttPublish(msg);

  int16_t value;
  if (sig_decode(msg, ACEBMS_VBAT, &value) != FMT_NULL) {
    uint32_t senseVoltage = (value + 5) / 10 - 5;
    int16_t balance = (int16_t)chargeCurrent2 - (int16_t)chargeCurrent3;
    if (balance > 8)
      balance = 8;
    if (balance < -8)
      balance = -8;
    // balance /= 16;
    mppt2.set(VEDirect_kBatterySense, senseVoltage + balance);
    mppt3.set(VEDirect_kBatterySense, senseVoltage);
  }
  if (sig_decode(msg, ACEBMS_RQST, &value) != FMT_NULL) {
    uint16_t frameSequence = value;
    if ((frameSequence & 0x03) == 0x03) {
      tinframe_t txFrame;
      msg_t *txMsg = (msg_t *)txFrame.data;
      sig_encode(txMsg, ACEMPPT_VPV2, panelVoltage2);
      sig_encode(txMsg, ACEMPPT_ICH2, chargeCurrent2);
      sig_encode(txMsg, ACEMPPT_VPV3, panelVoltage3);
      sig_encode(txMsg, ACEMPPT_ICH3, chargeCurrent3);
      aceBus.write(&txFrame);
      mqttPublish(txMsg);
    }
  }
}

void mppt2Callback(uint16_t id, int32_t value) {
  if (id == VEDirect_kPanelVoltage) {
    panelVoltage2 = value / 100;
  }
  if (id == VEDirect_kChargeCurrent) {
    chargeCurrent2 = value / 10;
  }
}

void mppt3Callback(uint16_t id, int32_t value) {
  if (id == VEDirect_kPanelVoltage) {
    panelVoltage3 = value / 100;
  }
  if (id == VEDirect_kChargeCurrent) {
    chargeCurrent3 = value / 10;
  }
}
