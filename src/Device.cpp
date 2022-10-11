#include <Arduino.h>

#include <knx.h>
#include "KnxHelper.h"
#include "ModbusGateway.h"
#include "ModbusRead.h"
#include "ModbusMaster.h"
#include "Modbus.h"
#include "wiring_private.h" // pinPeripheral() function
#include "S0Function.h"
#include "Device.h"
#include "LED_Statusanzeige.h"
#include "Logic.h"
#include "S0_Master.h"

S0_Master* S0_Master::instance = NULL ;
S0_Master S0[MAX_S0_CHANNELS] ;


uint32_t heartbeatDelay = 0;
uint32_t gStartupDelay = 0;
Logic gLogic;

uint8_t usedModbusChannels = 0;

bool modbusReady = false;
bool S0Ready = false;

uint16_t getPar(uint16_t PAR, uint8_t CH)
{
    return  MOD_ParamBlockOffset + (CH * MOD_ParamBlockSize) + PAR;
}

uint16_t getCom(uint16_t COM, uint8_t CH)
{
    return  MOD_KoOffset + (CH * MOD_KoBlockSize) + COM;
}

// Functions
void ProcessHeartbeat()
{
    // the first heartbeat is send directly after startup delay of the device
    if (heartbeatDelay == 0 || delayCheck(heartbeatDelay, getDelayPattern(LOG_HeartbeatDelayBase)))
    {
        // we waited enough, let's send a heartbeat signal
        knx.getGroupObject(LOG_KoHeartbeat).value(true, getDPT(VAL_DPT_1));
        heartbeatDelay = millis();
        // debug entry point
        gLogic.debug();
    }
}

void ProcessReadRequests() {
    // this method is called after startup delay and executes read requests, which should just happen once after startup
    static bool sCalledProcessReadRequests = false;
    if (!sCalledProcessReadRequests)
    {
        // put here own readRequest coding
        sCalledProcessReadRequests = true;
    }
    gLogic.processReadRequests();
}

// true solange der Start des gesamten Moduls verzögert werden soll
bool startupDelay()
{
    return !delayCheck(gStartupDelay, getDelayPattern(LOG_StartupDelayBase, true));
}

bool processDiagnoseCommand()
{
    char *lBuffer = gLogic.getDiagnoseBuffer();
    bool lOutput = false;
    if (lBuffer[0] == 'v')
    {
        // Command v: retrun fimware version, do not forward this to logic,
        // because it means firmware version of the outermost module
        // sprintf(lBuffer, "VER [%d] %d.%d", cFirmwareMajor, cFirmwareMinor, cFirmwareRevision);
        lOutput = true;
    }
    else
    {
        // let's check other modules for this command
        // put here local diagnose coding
        if (!lOutput) lOutput = gLogic.processDiagnoseCommand();
    }
    return lOutput;
}

void ProcessDiagnoseCommand(GroupObject &iKo)
{
    // this method is called as soon as iKo is changed
    // an external change is expected
    // because this iKo also is changed within this method,
    // the method is called again. This might result in
    // an endless loop. This is prevented by the isCalled pattern.
    static bool sIsCalledProcessDiagnoseCommand = false;
    if (!sIsCalledProcessDiagnoseCommand)
    {
        sIsCalledProcessDiagnoseCommand = true;
        //diagnose is interactive and reacts on commands
        gLogic.initDiagnose(iKo);
        if (processDiagnoseCommand())
            gLogic.outputDiagnose(iKo);
        sIsCalledProcessDiagnoseCommand = false;
    }
};

void processInputKo(GroupObject &iKo)
{
  // Compute modbus channel number
  int channel = (iKo.asap() - MOD_KoOffset - MOD_KoGO_BASE_) /  MOD_KoBlockSize;
  if (channel >= 0 && channel < MOD_ChannelCount) {
#ifdef Serial_Debug_Modbus
    SERIAL_DEBUG.print("KO: ");
    SERIAL_DEBUG.println(channel + 1);
#endif
    // Get slave number
    int slave = knx.paramByte(getPar(MOD_CHModbusSlaveSelection, channel));
    if (slave >= 1 && slave < MaxCountSlaves) {
      Slave[slave-1].ReadyToSendModbus(channel);
    }
  }
}

void ProcessKoCallback(GroupObject &iKo)
{
    // check if we evaluate own KO
    if (iKo.asap() == LOG_KoDiagnose)
    {
        ProcessDiagnoseCommand(iKo);
    }
    else
    {
        processInputKo(iKo);
        // always dispatch to Logicmodule, too
        gLogic.processInputKo(iKo);
    }
}

/**
 * @return upper border of used channels
 */
uint8_t getUsedModbusChannels()
{
  uint8_t countChannels = 0;
  for (int channel = 0; channel < MOD_ChannelCount; channel++)
  {
    if (knx.paramByte(getPar(MOD_CHModbusSlaveSelection, channel)) != 0) // wenn ein Slave gewählt wurde
    {
      if (knx.paramByte(getPar(MOD_CHModBusDptSelection, channel)) != 0) // und zusätzlich auch ein DPT
      {
        countChannels = channel + 1;
      }
    }
  }
  return countChannels;
}

bool setupModbus()
{
#ifdef LED_YELLOW_PIN
  pinMode(MAX485_RE_NEG, OUTPUT);
#endif
  pinMode(MAX485_DE, OUTPUT);
// Init in receive mode
#ifdef LED_YELLOW_PIN
  digitalWrite(MAX485_RE_NEG, 0);
#endif
  digitalWrite(MAX485_DE, 0);

  modbusInitSerial();

  modbusInitSlaves();

#ifdef Serial_Debug_Modbus
  SERIAL_DEBUG.println("Modbus Setup Done");
#endif

  return true;
}

void setupS0()
{

  if (knx.paramByte(MOD_DefineS0zaehler1) > 0)
  {
    SERIAL_DEBUG.println("S01 = Aktiv");
    pinMode(S0_CH1, INPUT);
    S0[0].initS0(S0_CH1,S01_LED);

    SERIAL_DEBUG.print("Impulse: ");
    SERIAL_DEBUG.println(setZaehlerImpulse(0, knx.paramWord(MOD_S01Impulse)));

#ifdef Serial_Debug_S0

    SERIAL_DEBUG.print("DefineUnit S02: ");
    SERIAL_DEBUG.print(knx.paramByte(MOD_DefineUnitS02));
    SERIAL_DEBUG.print(" ");
    SERIAL_DEBUG.println(knx.paramByte(MOD_DefineUnitS02), BIN);

    SERIAL_DEBUG.print("min Value S1: ");
    SERIAL_DEBUG.print(knx.paramWord(MOD_DefineMinValueS01));
    SERIAL_DEBUG.print(" ");
    SERIAL_DEBUG.println(knx.paramWord(MOD_DefineMinValueS01), BIN);

    SERIAL_DEBUG.print("min Value S2: ");
    SERIAL_DEBUG.print(knx.paramWord(MOD_DefineMinValueS02));
    SERIAL_DEBUG.print(" ");
    SERIAL_DEBUG.println(knx.paramWord(MOD_DefineMinValueS02), BIN);
#endif

#ifdef Debug_S0_LED
    pinMode(Diag_LED, OUTPUT);
    digitalWrite(Diag_LED, false);
#endif
  }

  if (knx.paramByte(MOD_DefineS0zaehler2) > 0)
  {
    SERIAL_DEBUG.println("S02 = Aktiv");
    pinMode(S0_CH2, INPUT);
    S0[1].initS0(S0_CH2,S02_LED);

    SERIAL_DEBUG.print("Impulse: ");
    SERIAL_DEBUG.println(setZaehlerImpulse(1, knx.paramWord(MOD_S02Impulse)));
  }
}

uint8_t get_PROG_LED_PIN(uint8_t hwID)
{
  switch (hwID)
  {
  case 7: // V1.x
    return 9;
    break;
  case 6: // V2.x
    return 6;
    break;
  default:
    return 255;
    break;
  }
}

uint8_t get_PROG_BUTTON_PIN(uint8_t hwID)
{
  switch (hwID)
  {
  case 7: // V1.x
    return A1;
    break;
  case 6: // V2.x
    return 7;
    break;
  default:
    return 250;
    break;
  }
}

uint8_t get_PROG_LED_PIN_ACTIVE_ON(uint8_t hwID)
{
  switch (hwID)
  {
  case 7: // V1.x
    return HIGH;
    break;
  case 6: // V2.x
    return HIGH;
    break;
  default:
    return HIGH;
    break;
  }
}

uint8_t get_SAVE_INTERRUPT_PIN(uint8_t hwID)
{
  switch (hwID)
  {
  case 7: // V1.x
    return 0;
    break;
  case 6: // V2.x
    return 0;
    break;
  default:
    return 0;
    break;
  }
}

uint8_t get_HW_ID()
{
  uint8_t hw_ID = 0;
  // Set Inputs
  pinMode(ID1, INPUT_PULLUP);
  pinMode(ID2, INPUT_PULLUP);
  pinMode(ID3, INPUT_PULLUP);
  // read Inputs
  bitWrite(hw_ID, 0, digitalRead(ID1));
  bitWrite(hw_ID, 1, digitalRead(ID2));
  bitWrite(hw_ID, 2, digitalRead(ID3));

  return hw_ID;
}

void initHW(uint8_t hwID)
{
  switch (hwID)
  {
  case 7:
    SERIAL_DEBUG.println("HW_ID: V1.x");
    break;
  case 6:
    SERIAL_DEBUG.println("HW_ID: V2.x");
    break;

  default:
    SERIAL_DEBUG.print("HW_ID: ERROR ");
    SERIAL_DEBUG.println(hwID);
    break;
  }

  initI2cStatusLeds();
  setLED_OFF();
  setLED(MODBUS_STATUS, HIGH);
  setLED(MBUS_STATUS, LOW);
  setLED(MODBUS_ERROR, HIGH);

}

void logicCallback(void *iInstance)
{
  gLogic.loop();
}

void appSetup()
{
  // Modbus
  modbusReady = setupModbus();
  // determine count of used channels
  usedModbusChannels = getUsedModbusChannels();
  SERIAL_DEBUG.print("Channel Count:");
  SERIAL_DEBUG.println(usedModbusChannels);

  // S0
  setupS0();

  if (knx.configured())
  {
    if (GroupObject::classCallback() == 0)
      GroupObject::classCallback(ProcessKoCallback);
    gStartupDelay = millis();
    heartbeatDelay = 0;
    Schedule::addCallback(logicCallback, nullptr);
    gLogic.setup(false);
  }
}

void appLoop()
{
  if (!knx.configured())
    return;

  // handle KNX stuff
  if (startupDelay())
    return;

  ProcessHeartbeat();
  ProcessReadRequests();

  //S0[0].process(0);
  //S0[1].process(1);
  //Process_S0(0); //CH = S01
  //Process_S0(1); //CH = S02

  ModbusRead(usedModbusChannels);
  Schedule::loop();
}