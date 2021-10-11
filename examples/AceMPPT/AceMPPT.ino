#include "AceBMS.h"
#include "AceDump.h"
#include "AceGrid.h"
#include "AceMPPT.h"
#include "TinBus.h"
#include "VEDirect.h"

#define VSETPOINT (26700)
#define ILIMIT (25000)

#define kRxInterruptPin (19)
void busCallback(unsigned char *data, unsigned char length);
TinBus tinBus(Serial1, ACEBMS_BAUD, kRxInterruptPin, busCallback);

void mppt2Callback(uint16_t id, int32_t value);
void mppt3Callback(uint16_t id, int32_t value);
VEDirect mppt2(Serial2, mppt2Callback);
VEDirect mppt3(Serial3, mppt3Callback);

uint16_t panelVoltage2 = 0;
uint16_t chargeCurrent2 = 0;
uint16_t panelVoltage3 = 0;
uint16_t chargeCurrent3 = 0;
uint16_t busError = TinBus_kOK;
uint32_t senseVoltage = 0;

unsigned long debugTimer = 0;

void setup() {
  Serial.begin(115200);
  mppt2.begin();
  mppt3.begin();
  tinBus.begin();
  mppt2.restart();
  mppt3.restart();
}

void loop() {
  mppt2.update();
  mppt3.update();

  static uint8_t dsecs = 0;
  static unsigned long time = 0;
  unsigned long now = micros();
  if (now - time >= 200000L) {  //  run every 200 ms
    time = now;
    if (senseVoltage != 0) {
      mppt2.set(VEDirect_kBatterySense, senseVoltage);
      mppt3.set(VEDirect_kBatterySense, senseVoltage);
      senseVoltage = 0;
    } else {
      dsecs++;  // round robin of messages
      if (dsecs > 2)
        dsecs = 0;
      if (dsecs == 0) {
        mppt2.set(VEDirect_kNetworkMode, VEDirect_kExternalControlMode);
        mppt3.set(VEDirect_kNetworkMode, VEDirect_kExternalControlMode);
      }
      if (dsecs == 1) {
        mppt2.set(VEDirect_VoltageSetpoint, SIG_DIVU16BY10(VSETPOINT - 50));
        mppt3.set(VEDirect_VoltageSetpoint, SIG_DIVU16BY10(VSETPOINT));
      }
      if (dsecs == 2) {
        mppt2.set(VEDirect_CurrentLimit, SIG_DIVU16BY100(ILIMIT));
        mppt3.set(VEDirect_CurrentLimit, SIG_DIVU16BY100(ILIMIT));
      }
    }
  }

  int status = tinBus.update();
  if ((status == TinBus_kWriteCollision) || (status == TinBus_kWriteTimeout) ||
      (status == TinBus_kReadOverrun) || (status == TinBus_kReadCRCError)) {
    busError = status;
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

static sig_name_t sigNames[] = {ACEBMS_NAMES, ACEMPPT_NAMES, ACEDUMP_NAMES,
                                ACEGRID_NAMES};
static const int sigCount = (sizeof(sigNames) / sizeof(sig_name_t));

void logMessage(msg_t *msg) {
  int index = 0;
  int count = 0;
  while (index < sigCount) {
    int16_t value;
    fmt_t format = sig_decode(msg, sigNames[index].sig, &value);
    if (format != FMT_NULL) {
      if(count++ == 0)
        Serial.print(":");
      else
        Serial.print(",");
      Serial.print(sigNames[index].name);
      Serial.print("=");
      char valueBuffer[FMT_MAXSTRLEN] = {0};
      sig_toString(msg, sigNames[index].sig, valueBuffer);
      Serial.print(valueBuffer);
    }
    index++;
  }
  if(count)
    Serial.print("\n");
}

void busCallback(unsigned char *data, unsigned char length) {
  // hexDump("msg", data, length);

  msg_t *msg = (msg_t *)data;
  logMessage(msg);

  int16_t value;
  if (sig_decode(msg, ACEBMS_VBAT, &value) != FMT_NULL) {
    senseVoltage = value;
    // int16_t balance = (int16_t)chargeCurrent2 - (int16_t)chargeCurrent3;
    // if (balance > 8) // limit current based balancing adjustment to vsense
    //   balance = 8;
    // if (balance < -8)
    //   balance = -8;
    // mppt2.set(VEDirect_kBatterySense, senseVoltage); // + balance);
    // mppt3.set(VEDirect_kBatterySense, senseVoltage);
  }
  if (sig_decode(msg, ACEBMS_RQST, &value) != FMT_NULL) {
    uint8_t frameSequence = value;
    if ((frameSequence & 0x03) == 0x03) {
      msg_t txMsg;
      sig_encode(&txMsg, ACEMPPT_VPV2, panelVoltage2);
      sig_encode(&txMsg, ACEMPPT_ICH2, chargeCurrent2);
      sig_encode(&txMsg, ACEMPPT_VPV3, panelVoltage3);
      sig_encode(&txMsg, ACEMPPT_ICH3, chargeCurrent3);
      uint8_t size = sig_encode(&txMsg, ACEMPPT_BERR, busError);
      tinBus.write((uint8_t *)&txMsg, size, MEDIUM_PRIORITY);

      // pinMode(2, OUTPUT);
      // digitalWrite(2, HIGH);
      // debugTimer = millis();

      logMessage(&txMsg);
      busError = TinBus_kOK;
    }
  }
}

void mppt2Callback(uint16_t id, int32_t value) {
  if (id == VEDirect_kPanelVoltage) {
    panelVoltage2 = SIG_DIVU16BY100(value);
  }
  if (id == VEDirect_kChargeCurrent) {
    chargeCurrent2 = SIG_DIVS16BY10(value);
  }
}

void mppt3Callback(uint16_t id, int32_t value) {
  if (id == VEDirect_kPanelVoltage) {
    panelVoltage3 = SIG_DIVU16BY100(value);
  }
  if (id == VEDirect_kChargeCurrent) {
    chargeCurrent3 = SIG_DIVS16BY10(value);
  }
}
