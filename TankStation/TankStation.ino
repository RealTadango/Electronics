#include <LiquidCrystal.h>
#include <EEPROM.h>

#define PIN_FLOWSENSOR 2
#define PIN_MOTOR_PWM 5
#define PIN_MOTOR_DIRECTION 6
#define PIN_PRESSURE A0
#define PIN_BATT A1

#define PRES_MIN 20
#define PRES_SCNT 20 //was 50, testing....
#define PRES_DELAY 5000
#define PRES_BUFCNT 30
#define PRES_BUFTIME 80 //50
#define SOFT_DETECTTIMEOUT 10000

#define MODE_IDLE 0
#define MODE_WAIT 1
#define MODE_CAL 2
#define MODE_FILL 3
#define MODE_REVERSE 4
#define MODE_DONE 5
#define MODE_EMPTY 6
#define MODE_ERROR 7
#define MODE_LIMIT 8
#define MODE_BATTLOW 9

#define currentConfig config[config_item]

char* modeText[] = { "Idle",
                      "Wait",
                      "Cali",
                      "Fill",
                      "Revr",
                      "Done",
                      "Empt",
                      "Erro",
                      "Limi",
                      "Batt" };

#define MENU_START 0
#define MENU_CONFIG 1

#define P_BUFFER_SIZE 20

struct ConfigItem{
  char* group;
  char* name; //display name
  int value; //default value
  int minvalue; //min value
  int maxvalue; //max value
  char* unit; //display unit
  byte display_factor; //display factor (100 = 0.00 ect)
  byte step; //step for adjustment in config
  char **options;
};

#define TYPE_HARD 0
#define TYPE_SOFT 1

char *typeOpts[] = { "Hard", "Soft" };
char *autoStartOpts[] = { "No", "Yes" };

ConfigItem config[] = {
  {"", "Type", 1, 0, 1, "", 1, 1, typeOpts},
  {"Soft tank", "Limit", 0, 0, 9999, "ml", 1, 50, nullptr},
  {"Soft tank", "StartPwr", 50, 0, 100, "%", 1, 1, nullptr},
  {"Soft tank", "Power", 100, 30, 100, "%", 1, 1, nullptr},
  {"Soft tank", "Detect", 50, 0, 500, "ml", 1, 1, nullptr},
  {"Soft tank", "Trigger", 2, 1, 20, "^", 1, 1, nullptr},
  {"Soft tank", "TrigDelay", 0, 0, 500, "ml", 1, 10, nullptr},
  {"Soft tank", "Reverse", 5, 0, 100, "sec", 10, 1, nullptr},
  {"Hard tank", "Limit", 0, 0, 9999, "ml", 1, 50, nullptr},
  {"Hard tank", "StartPwr", 50, 0, 100, "%", 1, 1, nullptr},
  {"Hard tank", "Power", 100, 30, 100, "%", 1, 1, nullptr},
  {"Hard tank", "Trigger", 4, 1, 20, "^", 1, 1, nullptr},
  {"Hard tank", "TrigDelay", 150, 0, 500, "ml", 1, 10, nullptr},
  {"Hard tank", "Reverse", 20, 0, 100, "sec", 10, 1, nullptr},
  {"Common", "AutoStart", 0, 0, 1, "", 1, 1, autoStartOpts},
  {"Common", "BattLow", 105, 0, 200, "V", 10, 1, nullptr},
  {"Common", "Flow", 476, 1, 2000, "p", 100, 1, nullptr},
  {"Common", "FlowRev", 241, 1, 2000, "p", 100, 1, nullptr},
};

#define CFG_TYPE 0
#define CFG_SOFT_LIMIT 1
#define CFG_SOFT_STARTPWR 2
#define CFG_SOFT_PWR 3
#define CFG_SOFT_DETECT 4
#define CFG_SOFT_TRIGGER 5
#define CFG_SOFT_TRIGGERDELAY 6
#define CFG_SOFT_REVERSE 7
#define CFG_HARD_LIMIT 8
#define CFG_HARD_STARTPWR 9
#define CFG_HARD_PWR 10
#define CFG_HARD_TRIGGER 11
#define CFG_HARD_TRIGGERDELAY 12
#define CFG_HARD_REVERSE 13
#define CFG_AUTOSTART 14
#define CFG_BATTLOW 15
#define CFG_FLOW 16
#define CFG_FLOWREVERSE 17

#define MOTOR_OFF 0
#define MOTOR_FILL 1
#define MOTOR_EMPTY 2

byte config_item = 0;
byte menu_item = MENU_START;
byte mode = MODE_IDLE;

int pres_offset = 0;
int pres_buffer[PRES_BUFCNT];
long pres_buf_time = 0;
long tank_start = 0;
int count = 0;
int count_raw = 0;

byte motorSetPower = 0;
byte motorPower = 0;
byte motorDirection = MOTOR_OFF;

long disp_prevdraw = millis();

int p_index = 0;
int p_buffer[P_BUFFER_SIZE];

long btn_last = 0;
long btn_lastupdate = 0;
int btn_delay = 0;
long lastDraw = 0;
long lastFlowCount = 0;
long lastFlowTime = 0;
int flow;

LiquidCrystal lcd(13, 8, 12, 9, 11, 10);

void setup() {
  pinMode(PIN_MOTOR_PWM, OUTPUT);
  pinMode(PIN_MOTOR_DIRECTION, OUTPUT);
  setMotor();

  lcd.begin(16, 2);
  lcd.clear();
  setupCharacters();

  pinMode(PIN_BATT, INPUT);
  pinMode(PIN_PRESSURE, INPUT);
  pinMode(PIN_FLOWSENSOR, INPUT_PULLUP);

  delay(300);

  pres_offset = getPressure(false);
  loadData();

  attachInterrupt(digitalPinToInterrupt(PIN_FLOWSENSOR), flowTick, FALLING);

  if(config[CFG_AUTOSTART].value == 1) {
    mode = MODE_WAIT;
    setMotorStartPower();
  }
}

void setMotorStartPower() {
  switch(config[CFG_TYPE].value) {
    case TYPE_HARD: {
    motorSetPower = config[CFG_HARD_PWR].value;
    motorPower = config[CFG_HARD_STARTPWR].value;
      break;
    }
    case TYPE_SOFT: {
    motorSetPower = config[CFG_SOFT_PWR].value;
    motorPower = config[CFG_SOFT_STARTPWR].value;
      break;
    }
  }
}


void checkMode_Hard(int delta) {
  if (mode == MODE_WAIT) {
    motorDirection = MOTOR_FILL;
    mode = MODE_CAL;
  } else if (mode == MODE_CAL) {
    if (tank_start == 0) {
      tank_start = millis();
    } else if (millis() > (tank_start + PRES_DELAY)) {
      mode = MODE_FILL;
    }
  } else if (mode == MODE_FILL) {
    if (delta < -2 - config[CFG_HARD_TRIGGER].value) {
        motorDirection = MOTOR_OFF;
        mode = MODE_ERROR;
    } else if (count > config[CFG_HARD_TRIGGERDELAY].value && config[CFG_HARD_TRIGGER].value > 0 && delta > config[CFG_HARD_TRIGGER].value) {
      if (config[CFG_HARD_REVERSE].value > 0) {
        //Allow H-bridge to idle
        motorDirection = MOTOR_OFF;
        setMotor();
        delay(100);
        motorDirection = MOTOR_EMPTY;
        setMotor();
        mode = MODE_REVERSE;
        tank_start = millis();
      } else {
        mode = MODE_DONE;
        motorDirection = MOTOR_OFF;
      }
    } else if (config[CFG_HARD_LIMIT].value > 0 && count >= config[CFG_HARD_LIMIT].value) {
      mode = MODE_LIMIT;
      motorDirection = MOTOR_OFF;
    }
  } else if (mode == MODE_REVERSE) {
    if (millis() > tank_start + (config[CFG_HARD_REVERSE].value * 100)) {
      mode = MODE_DONE;
      motorDirection = MOTOR_OFF;
    }
  }
}

void checkMode_Soft(int delta) {
  if (mode == MODE_WAIT) {
    motorDirection = MOTOR_EMPTY;
    if (tank_start == 0) {
      tank_start = millis();
    } else if (millis() > (tank_start + SOFT_DETECTTIMEOUT)) {
      mode = MODE_ERROR;
      motorDirection = MOTOR_OFF;
    } else if (config[CFG_SOFT_DETECT].value == 0) {
      motorDirection = MOTOR_FILL;
      mode = MODE_CAL;
    } else if ((0 - count) > config[CFG_SOFT_DETECT].value) {
      mode = MODE_CAL;
      motorDirection = MOTOR_OFF;
      setMotor();
      delay(100);
      motorPower = config[CFG_SOFT_STARTPWR].value;
      motorDirection = MOTOR_FILL;
      setMotor();
      tank_start = 0;
    }
  } else if (mode == MODE_CAL) {
    if (tank_start == 0) {
      tank_start = millis();
    } else if (millis() > (tank_start + PRES_DELAY)) {
      mode = MODE_FILL;
    }
  } else if (mode == MODE_FILL) {
    if (delta < -2 - config[CFG_SOFT_TRIGGER].value) {
        motorDirection = MOTOR_OFF;
        mode = MODE_ERROR;
    } else if (count > config[CFG_SOFT_TRIGGERDELAY].value && config[CFG_SOFT_TRIGGER].value > 0 && delta > config[CFG_SOFT_TRIGGER].value) {
      if (config[CFG_SOFT_REVERSE].value > 0) {
        //Allow H-bridge to idle
        motorDirection = MOTOR_OFF;
        setMotor();
        delay(100);
        motorDirection = MOTOR_EMPTY;
        setMotor();
        mode = MODE_REVERSE;
        tank_start = millis();
      } else {
        mode = MODE_DONE;
        motorDirection = MOTOR_OFF;
      }
    } else if (config[CFG_SOFT_LIMIT].value > 0 && count >= config[CFG_SOFT_LIMIT].value) {
      mode = MODE_LIMIT;
      motorDirection = MOTOR_OFF;
    }
  } else if (mode == MODE_REVERSE) {
    if (millis() > tank_start + (config[CFG_SOFT_REVERSE].value * 100)) {
      mode = MODE_DONE;
      motorDirection = MOTOR_OFF;
    }
  }
}

void checkMode(double batt, int delta) {
  if (mode == MODE_IDLE) {
    tank_start = 0;
    lastFlowCount = 0;
    lastFlowTime = 0;
    motorDirection = MOTOR_OFF;
  } else if (batt < ((double)config[CFG_BATTLOW].value / 10)) {
    mode = MODE_BATTLOW;
    motorDirection = MOTOR_OFF;
  } else if (mode == MODE_EMPTY) {
    motorDirection = MOTOR_EMPTY;
    if (tank_start == 0) tank_start = millis();
    else if (millis() - tank_start > PRES_DELAY && flow >= -1) {
      mode = MODE_DONE;
      motorDirection = MOTOR_OFF;
    } 
  } else {
    switch(config[CFG_TYPE].value)
    {
      case TYPE_HARD:
      {
        checkMode_Hard(delta);
        break;
      }
      case TYPE_SOFT:
      {
        checkMode_Soft(delta);
        break;
      }
    }
  }
}

void loop() {
  int pressure = getPressure(true);

  if (millis() > pres_buf_time + PRES_BUFTIME) {
    pres_buf_time = millis();
    for (int i = 0; i < PRES_BUFCNT - 1; i++) {
      pres_buffer[i] = pres_buffer[i + 1];
    }

    pres_buffer[PRES_BUFCNT - 1] = pressure;
  }

  int delta = pres_buffer[PRES_BUFCNT - 1] - pres_buffer[0];
  double batt = getBatt();

  if(count_raw < 0) {
    count = (double)count_raw / (config[CFG_FLOWREVERSE].value / (double)100);
  }
  else {
    count = (double)count_raw / (config[CFG_FLOW].value / (double)100);
  }

  if (millis() > (lastFlowTime + 1000)) {
    lastFlowTime = millis();
    flow = count - lastFlowCount;
    lastFlowCount = count;
  }
  
  int prevMode = mode;
  checkMode(batt, delta);
  readBtn();

  lastDraw = millis();
  drawLcd(pressure, batt, delta);

  if (motorPower < motorSetPower) {
    motorPower++;
  } else if (motorPower > motorSetPower) {
    motorPower--;
  }

  setMotor();
}

bool doBtn(bool repeat) {
  long diff = millis() - btn_last;
  btn_last = millis();

  if (diff > 150) {
    btn_delay = 300;
    btn_lastupdate = millis();
    return true;
  }

  if (!repeat) {
    return false;
  }

  if (millis() - btn_lastupdate > btn_delay) {
    if (btn_delay > 150) {
      btn_delay = 150;
    }

    if (btn_delay > 20) {
      btn_delay = btn_delay - 5;
    }

    if (btn_delay > 0) {
      btn_delay = btn_delay - 1;
    }

    btn_lastupdate = millis();
    return true;
  }

  return false;
}

void btnUp() {
  if (doBtn(true)) {
    if (mode == MODE_EMPTY || mode == MODE_FILL) {
      motorSetPower++;
      if (motorSetPower > 100) motorSetPower = 100;
    } else if (menu_item == MENU_START && mode == MODE_IDLE) {
      count_raw = 0;
      setMotorStartPower();
      mode = MODE_WAIT;
    } else if(menu_item == MENU_CONFIG) {
      currentConfig.value += currentConfig.step;

      if(currentConfig.value > currentConfig.maxvalue) {
        currentConfig.value = currentConfig.maxvalue;
      }

      saveData();
    }
  }
}

void btnDown() {
  if (doBtn(true)) {
    if (mode == MODE_EMPTY || mode == MODE_FILL) {
      motorSetPower--;
      if(motorSetPower < 30) motorSetPower = 30;
    } else if (menu_item == MENU_START && mode == MODE_IDLE) {
      count_raw = 0;
      setMotorStartPower();
      mode = MODE_EMPTY;
    } else if(menu_item == MENU_CONFIG) {
      currentConfig.value -= currentConfig.step;

      if(currentConfig.value < currentConfig.minvalue) {
        currentConfig.value = currentConfig.minvalue;
      }

      saveData();
    }
  }
}

void btnEnt() {
  if (doBtn(false)) {
    if (mode == MODE_IDLE) {
      if (menu_item == MENU_START) {
        menu_item = MENU_CONFIG;
        config_item = 1;
      } else if (menu_item == MENU_CONFIG) {
        int configMax = sizeof(config) / sizeof(ConfigItem);
        config_item++;
        if(config_item >= configMax) {
          menu_item = MENU_START;
        }
      }
    } else {
      mode = MODE_IDLE;
    }
  }
}

void btnEnt2() {
  if (doBtn(false)) {
    if (menu_item == MENU_CONFIG) {
      menu_item = MENU_START;
    } else {
      if(mode != MODE_IDLE) {
        mode = MODE_IDLE;
      } else {
        config[CFG_TYPE].value++;
        if(config[CFG_TYPE].value > config[CFG_TYPE].maxvalue) {
          config[CFG_TYPE].value = config[CFG_TYPE].minvalue;
        }

        saveData();
      }
    }
  }
}

void readBtn() {
  digitalWrite(8, HIGH);

  pinMode(11, INPUT);
  pinMode(9, INPUT);
  pinMode(12, INPUT);

  delay(4);

  bool btn1 = !digitalRead(11);
  bool btn2 = !digitalRead(9);
  bool btn3 = !digitalRead(12);

  if (btn1 && btn2 && !btn3) {
    //btn 4
    btnUp();
  } else if (!btn1 && btn2 && !btn3) {
    //btn 2
    btnEnt();
  } else if (btn1 && !btn2 && !btn3) {
    //btn 1
    btnDown();
  } else if (!btn1 && !btn2 && btn3) {
    //btn 3
    btnEnt2();
  }

  pinMode(11, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(12, OUTPUT);

  digitalWrite(8, LOW);

  //Re init
  lcd.print(" ");
}

void drawFixed(String text, int total) {
  int printed = lcd.print(text);

  for (int i = printed; i < total; i++) {
    lcd.print(" ");
  }
}

String getDisplayValue(ConfigItem configItem) {
    if(configItem.options != nullptr) {
      return configItem.options[configItem.value];
    }

    if(configItem.value == 0) return "Off";

    if(configItem.display_factor == 1) {
      return configItem.value + String(configItem.unit);
    } else {
      double displayValue = (double)configItem.value / (double)configItem.display_factor;
      return displayValue + String(configItem.unit);
    }
}

void drawLcd_Start(int pressure, double batt, int delta) {
  lcd.setCursor(0, 0);

  if(mode == MODE_IDLE) {
    drawFixed(getDisplayValue(config[CFG_TYPE]), 5);      
  } else {
    drawFixed(modeText[mode], 5);
  }

  lcd.setCursor(5, 0);
  drawFixed(String(count) + "ml", 7);

  lcd.setCursor(12, 0);
  if (motorDirection != MOTOR_OFF) {
    drawFixed(String(motorPower) + "%", 4);
  } else {
    drawFixed("Off", 4);
  }

  lcd.setCursor(0, 1);

  if (mode != MODE_IDLE && mode != MODE_BATTLOW) {

    int limit = 0;
    int trigger = 0;

    switch(config[CFG_TYPE].value) {
      case TYPE_HARD: {
        limit = config[CFG_HARD_LIMIT].value;
        trigger = config[CFG_HARD_TRIGGER].value;
        break;
      }
      case TYPE_SOFT: {
        limit = config[CFG_SOFT_LIMIT].value;
        trigger = config[CFG_SOFT_TRIGGER].value;
        break;
      }
    }

    if (limit > 0) {
      int fill = abs(((double)count / (double)limit) * 16);
      for (int i = 0; i < fill; i++) {
        lcd.print("#");
      }
      for (int i = fill; i < 16; i++) {
        lcd.print("_");
      }
    } else if (trigger > 0) {
      int lowest = 0;
      for (int i = 0; i < PRES_BUFCNT - 16; i++) {
        lowest += pres_buffer[i];
      }

      lowest = lowest / (PRES_BUFCNT - 16);

      for (int i = PRES_BUFCNT - 16; i < PRES_BUFCNT; i++) {
        int diff = pres_buffer[i] - lowest;
        diff = 4 + ((double)diff / (double)trigger * 4);

        if (diff > 8) diff = 8;
        else if (diff < 0) diff = 0;

        if (diff == 0) {
          lcd.print(" ");
        } else {
          lcd.write(diff - 1);
        }
      }
    } else {
      drawFixed("Battery " + String(batt) + "V", 16);
    }
  } else {
    drawFixed("Battery " + String(batt) + "V", 16);
  }
}

void drawLcd_Config() {
  lcd.setCursor(0, 0);
  drawFixed("Menu " + String(currentConfig.group), 16);

  lcd.setCursor(0, 1);
  String displayValue = getDisplayValue(currentConfig);
  drawFixed(String(currentConfig.name) + "=" + displayValue, 16);
}

void drawLcd(int pressure, double batt, int delta) {
  if (menu_item == MENU_START) {
    drawLcd_Start(pressure, batt, delta);

  } else if(menu_item == MENU_CONFIG)
  {
    drawLcd_Config();
  }
}

void setupCharacters() {
  lcd.createChar(0, new byte[8]{
                      B00000,
                      B00000,
                      B00000,
                      B00000,
                      B00000,
                      B00000,
                      B00000,
                      B11111,
                    });
  lcd.createChar(1, new byte[8]{
                      B00000,
                      B00000,
                      B00000,
                      B00000,
                      B00000,
                      B00000,
                      B11111,
                      B11111,
                    });
  lcd.createChar(2, new byte[8]{
                      B00000,
                      B00000,
                      B00000,
                      B00000,
                      B00000,
                      B11111,
                      B11111,
                      B11111,
                    });
  lcd.createChar(3, new byte[8]{
                      B00000,
                      B00000,
                      B00000,
                      B00000,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                    });
  lcd.createChar(4, new byte[8]{
                      B00000,
                      B00000,
                      B00000,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                    });
  lcd.createChar(5, new byte[8]{
                      B00000,
                      B00000,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                    });
  lcd.createChar(6, new byte[8]{
                      B00000,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                    });
  lcd.createChar(7, new byte[8]{
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                      B11111,
                    });
}

void writeEEPROMValue(int pos, int value) {
  EEPROM.write(pos * 2, lowByte(value));
  EEPROM.write((pos * 2) + 1, highByte(value));
}

int readEEPROMValue(int pos, int value) {
  if (EEPROM.read(pos * 2) == 255 && EEPROM.read((pos * 2) + 1) == 255) {
    return value;
  } else {
    return EEPROM.read(pos * 2) + (EEPROM.read((pos * 2) + 1) * 256);
  }
}

void saveData() {
  int configMax = sizeof(config) / sizeof(ConfigItem);

  for(int i = 0; i < configMax; i++) {
    writeEEPROMValue(i, config[i].value);
  }
}

void loadData() {
  int configMax = sizeof(config) / sizeof(ConfigItem);

  for(int i = 0; i < configMax; i++) {
    config[i].value = readEEPROMValue(i, config[i].value);
  }
}

int getPressure(bool buffered) {
  double val = analogRead(PIN_PRESSURE);

  for (int i = 0; i < PRES_SCNT; i++) {
    val += (analogRead(PIN_PRESSURE));
  }

  val = (val / PRES_SCNT) - pres_offset;

  p_buffer[p_index] = val;
  p_index++;

  if (p_index >= P_BUFFER_SIZE) {
    p_index = 0;
  }

  double total = 0;

  for (int i = 0; i <= P_BUFFER_SIZE; i++) {
    total += p_buffer[i];
  }

  if (buffered) {
    return total / P_BUFFER_SIZE;
  } else {
    return val;
  }
}

void flowTick() {
  if (motorDirection == MOTOR_EMPTY) {
    count_raw--;
  } else {
    count_raw++;
  }
}

void setMotor() {  
  int mPwr = 255 - ((double)motorPower * 2.55);

  if (motorDirection == MOTOR_FILL) {
    analogWrite(PIN_MOTOR_PWM, mPwr);
    digitalWrite(PIN_MOTOR_DIRECTION, LOW);
  } else if (motorDirection == MOTOR_EMPTY) {
    analogWrite(PIN_MOTOR_PWM, mPwr);
    digitalWrite(PIN_MOTOR_DIRECTION, HIGH);
  } else {
    digitalWrite(PIN_MOTOR_PWM, HIGH);
    digitalWrite(PIN_MOTOR_DIRECTION, HIGH);
  }
}

double getBatt() {
  return (double)analogRead(PIN_BATT) / 50;
}
