#include "AceBMS.h"
#include "AceBus.h"
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

void setup() {
  Serial.begin(19200);
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
    Serial.println("TX Collision");
  if (status == AceBus_kWriteTimeout)
    Serial.println("TX Timeout");
  // if (status == AceBus_kWriteComplete) Serial.println("TX Complete");
}

void aceCallback(tinframe_t *rxFrame) {
  // if (status == AceBus_kReadDataReady) {
  //   tinframe_t rxFrame;
  //   aceBus.read(&rxFrame);
  // hexDump("RX ", (unsigned char*)&rxFrame, tinframe_kFrameSize);
  msg_t *msg = (msg_t *)(rxFrame->data);
  int16_t value;
  if (sig_decode(msg, ACEBMS_VBAT, &value) != FMT_NULL) {
    uint32_t senseVoltage = (value + 5) / 10 - 5;
    int16_t balance = (int16_t)chargeCurrent2 - (int16_t)chargeCurrent3;
    if (balance > 64)
      balance = 64;
    if (balance < -64)
      balance = -64;
    balance /= 16;
    mppt2.set(VEDirect_kBatterySense, senseVoltage + balance);
    mppt3.set(VEDirect_kBatterySense, senseVoltage);
  }
  if (sig_decode(msg, ACEBMS_RQST, &value) != FMT_NULL) {
    uint8_t frameSequence = value;
    if ((frameSequence & 0x03) == 0x03) {
      tinframe_t txFrame;
      msg_t *txMsg = (msg_t *)txFrame.data;
      sig_encode(txMsg, ACEMPPT_VPV2, panelVoltage2);
      sig_encode(txMsg, ACEMPPT_ICH2, chargeCurrent2);
      sig_encode(txMsg, ACEMPPT_VPV3, panelVoltage3);
      sig_encode(txMsg, ACEMPPT_ICH3, chargeCurrent3);
      aceBus.write(&txFrame);
    }
  }
}

void mppt2Callback(uint16_t id, int32_t value) {
  if (id == VEDirect_kPanelVoltage) {
    panelVoltage2 = value;
  }
  if (id == VEDirect_kChargeCurrent) {
    chargeCurrent2 = value;
  }
}

void mppt3Callback(uint16_t id, int32_t value) {
  if (id == VEDirect_kPanelVoltage) {
    panelVoltage3 = value;
  }
  if (id == VEDirect_kChargeCurrent) {
    chargeCurrent3 = value;
  }
}
