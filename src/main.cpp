#include <Arduino.h>
#include <LiquidCrystal.h>
#include <gspswitch.h>
#include <gspserialresponse.h>
#include <gspquad.h>
#include "math.h"

bool encoderButton = false;
int mark; // shows where the cursor is in the likes of ADF etc
int nmark = 0;

uint8_t activeMode=0;
char blank = 255;
int encButtonDebounce = 0;

void btnEncoder();
void btnFlip();
void setModeCom1();
void setModeCom2();
void setModeNav1();
void setModeNav2();
void setModeAdf();
void setModeDme();
void setModeXpd();
void encLower(int _dif);
void encHigher(int _dif);

void fsrCom1(String);
void fsrCom2(String);
void fsrCom1sb(String);
void fsrCom2sb(String);
void fsrNav1(String);
void fsrNav2(String);
void fsrNav1sb(String);
void fsrNav2sb(String);
void fsrAdf(String);
void fsrXpd(String);
void fsrDme1(String);
void fsrDme2(String);

LiquidCrystal lcd (7, 6, 5, 4, 3, 2);

gspQuad quad1(8, 9, encLower, encHigher);

gspSwitch encoderBtn(10,btnEncoder);
gspSwitch coBtn(11,btnFlip);
gspSwitch modeCOM1(12,setModeCom1,1);
gspSwitch modeCOM2(14,setModeCom2,1);
gspSwitch modeNAV1(16,setModeNav1,1);
gspSwitch modeNAV2(15,setModeNav2,1);
gspSwitch modeADF(17,setModeAdf,1);
gspSwitch modeDME(18,setModeDme,1);
gspSwitch modeXPD(19,setModeXpd,1);

gspSerialResponse srCom1("=A",7,fsrCom1);
gspSerialResponse srCom1sb("=B",7,fsrCom1sb);
gspSerialResponse srCom2("=C",7,fsrCom2);
gspSerialResponse srCom2sb("=D",7,fsrCom2sb);
gspSerialResponse srNav1("=E",6,fsrNav1);
gspSerialResponse srNav1sb("=F",6,fsrNav1sb);
gspSerialResponse srNav2("=G",6,fsrNav2);
gspSerialResponse srNav2sb("=H",6,fsrNav2sb);
gspSerialResponse srAdf("=I",6,fsrAdf);
gspSerialResponse srXpd("=J",4,fsrXpd);
gspSerialResponse srDme1("=K",5,fsrDme1);
gspSerialResponse srDme2("=L",5,fsrDme2);

void setup()
{
  Serial.begin(115200);

  gspGrouped::register_instance(&quad1);
  gspGrouped::register_instance(&encoderBtn);
  gspGrouped::register_instance(&coBtn);
  gspGrouped::register_instance(&modeCOM1);
  gspGrouped::register_instance(&modeCOM2);
  gspGrouped::register_instance(&modeNAV1);
  gspGrouped::register_instance(&modeNAV2);
  gspGrouped::register_instance(&modeADF);
  gspGrouped::register_instance(&modeDME);
  gspGrouped::register_instance(&modeXPD);
  gspGrouped::register_instance(&srCom1);
  gspGrouped::register_instance(&srCom2);
  gspGrouped::register_instance(&srCom1sb);
  gspGrouped::register_instance(&srCom2sb);
  gspGrouped::register_instance(&srNav1);
  gspGrouped::register_instance(&srNav2);
  gspGrouped::register_instance(&srNav1sb);
  gspGrouped::register_instance(&srNav2sb);
  gspGrouped::register_instance(&srAdf);
  gspGrouped::register_instance(&srXpd);
  gspGrouped::register_instance(&srDme1);
  gspGrouped::register_instance(&srDme2);


  lcd.begin(16, 2); // set up the LCD's number of columns and rows:
  lcd.clear();

  mark = 10;

  gspQuad::startCheckAll();
}

void LCDPrint(String element, int x, int y) {
  lcd.setCursor(x, y);
  delay(15);
  lcd.print(element);
  delay(15);
}

void redrawNMark() {
  if (modeADF.getState() == gspSwitch::On || modeXPD.getState() == gspSwitch::On) {
    LCDPrint("                ", 0, 1);
    if (nmark > 0)
      LCDPrint("-", nmark + 10 , 1);
  } else if (
    modeCOM1.getState() == gspSwitch::On ||
    modeCOM2.getState() == gspSwitch::On ||
    modeNAV1.getState() == gspSwitch::On ||
    modeNAV2.getState() == gspSwitch::On )
  {
    switch (nmark) {
      case 0:
        LCDPrint(".", 12, 1);
        break;
      case 1:
        LCDPrint("<", 12, 1);
        break;
      case 2:
        LCDPrint(">", 12, 1);
        break;
    }
  }
}

void btnEncoder () {
  if (modeADF.getState() == gspSwitch::On) {
      if (++nmark > 5) // only 4 possible options to change ADF frequency
        nmark = 0;
      if (nmark == 4)
        nmark++; // skip over the decimal point in the display
  } else if (modeXPD.getState() == gspSwitch::On) {
    if (++nmark > 4) // only 4 possible options to change xpondr frequency
          nmark = 0;
    } else {
        if (++nmark > 2)
          nmark = 0;
    }
  redrawNMark();
}

void btnFlip() {
  if (modeCOM1.getState() == gspSwitch::On)
    Serial.println("A06");
  if (modeCOM2.getState() == gspSwitch::On)
    Serial.println("A12");
  if (modeNAV1.getState() == gspSwitch::On)
    Serial.println("A18");
  if (modeNAV2.getState() == gspSwitch::On)
    Serial.println("A24");
  nmark = 0;
  redrawNMark();
}

void setModeCom1() {
  if (activeMode != 12) {
    LCDPrint("Com.1           ", 0, 0);
    LCDPrint(srCom1.getLastResponse(), 9, 0);
    LCDPrint("  s/b           ", 0, 1);
    LCDPrint(srCom1sb.getLastResponse(), 9, 1);
    nmark = 0;
    redrawNMark();
    activeMode = 12;
  }
}

void setModeCom2() {
  if (activeMode != 14) {
    LCDPrint("Com.2           ", 0, 0);
    LCDPrint(srCom2.getLastResponse(), 9, 0);
    LCDPrint("  s/b           ", 0, 1);
    LCDPrint(srCom2sb.getLastResponse(), 9, 1);
    nmark = 0;
    redrawNMark();    
    activeMode = 14;
  }
}

void setModeNav1() {
  if (activeMode != 15) {
    LCDPrint("Nav.1           ", 0, 0);
    LCDPrint(srNav1.getLastResponse(), 9, 0);
    LCDPrint("  s/b           ", 0, 1);
    LCDPrint(srNav1sb.getLastResponse(), 9, 1);
    nmark = 0;
    redrawNMark();
    activeMode = 15;
  }  
}

void setModeNav2() {
  if (activeMode != 16) {
    LCDPrint("Nav.2           ", 0, 0);
    LCDPrint(srNav2.getLastResponse(), 9, 0);
    LCDPrint("  s/b           ", 0, 1);
    LCDPrint(srNav2sb.getLastResponse(), 9, 1);
    nmark = 0;
    redrawNMark();
    activeMode = 16;
  }
}

void setModeAdf() {
  if (activeMode != 17) {
    LCDPrint("ADF       ", 0, 0);
    LCDPrint(String(srAdf.getLastResponse()) + "  ", 10, 0);
    nmark = 0;
    redrawNMark();
    activeMode = 17;
  }
}

void setModeDme() {
  if (activeMode != 18) {
    LCDPrint("DME1            ", 0, 0);
    LCDPrint(srDme1.getLastResponse(), 9, 0);
    LCDPrint("DME2            ", 0, 1);
    LCDPrint(srDme2.getLastResponse(), 9, 1);
    activeMode = 18;
  }
}

void setModeXpd() {
  if (activeMode != 19) {
    LCDPrint("Xponder     ", 0, 0);
    LCDPrint(String(srXpd.getLastResponse()) + "    ", 11, 0);
    nmark = 0;
    redrawNMark();
    activeMode = 19;
  }
}

//COM1
void fsrCom1(String szParam) {
  if (modeCOM1.getState()==gspSwitch::On) {
      LCDPrint(szParam, 9, 0);
  }
}

//COM1-standby
void fsrCom1sb(String szParam) {
  if (modeCOM1.getState()==gspSwitch::On) {
      LCDPrint(szParam, 9, 1);
      redrawNMark();
  }
}

//COM2
void fsrCom2(String szParam) {
  if (modeCOM2.getState()==gspSwitch::On) {
      LCDPrint(szParam, 9, 0);
  }
}

//COM2-standby
void fsrCom2sb(String szParam) {
  if (modeCOM2.getState()==gspSwitch::On) {
      LCDPrint(szParam, 9, 1);
      redrawNMark();
  }
}

//NAV1
void fsrNav1(String szParam) {
  if (modeNAV1.getState()==gspSwitch::On) {
      LCDPrint(szParam, 9, 0);
  }
}

//NAV1-standby
void fsrNav1sb(String szParam) {
  if (modeNAV1.getState()==gspSwitch::On) {
      LCDPrint(szParam, 9, 1);
      redrawNMark();
  }
}

//NAV2
void fsrNav2(String szParam) {
  if (modeNAV2.getState()==gspSwitch::On) {
      LCDPrint(szParam, 9, 0);
  }
}

//NAV2-standby
void fsrNav2sb(String szParam) {
  if (modeNAV2.getState()==gspSwitch::On) {
      LCDPrint(szParam, 9, 1);
      redrawNMark();
  }
}

//ADF
void fsrAdf(String szParam) {
  if (modeADF.getState()==gspSwitch::On) {
      LCDPrint(szParam, 10, 0);
      redrawNMark();
  }
}

//XPDR
void fsrXpd(String szParam) {
  if (modeXPD.getState()==gspSwitch::On) {
      LCDPrint(szParam, 11, 0);
      redrawNMark();
  }
}

//DME1
void fsrDme1(String szParam) {
  if (modeDME.getState()==gspSwitch::On) {
      LCDPrint(szParam, 9, 0);
  }
}

//DME2
void fsrDme2(String szParam) {
  if (modeDME.getState()==gspSwitch::On) {
      LCDPrint(szParam, 9, 1);
  }
}


//This function gets called when the encoder value is reduced
void encLower(int _dif){
  for (int i = 0; i < -_dif; i++) {
    switch (activeMode) {
      case 12:
        if (nmark == 1)
          Serial.println("A02");
        if (nmark == 2)
          Serial.println("A04");
        break;
      case 14:
        if (nmark == 1)
          Serial.println("A08");
        if (nmark == 2)
          Serial.println("A10");
        break;
      case 15:
        if (nmark == 1)
          Serial.println("A14");
        if (nmark == 2)
          Serial.println("A16");
        break;
      case 16:
        if (nmark == 1)
          Serial.println("A20");
        if (nmark == 2)
          Serial.println("A22");
        break;
      case 17:
        switch (nmark) {
          case 1:
            Serial.println("A25");
            break;
          case 2:
            Serial.println("A26");
            break;
          case 3:
            Serial.println("A27");
            break;
          case 5:
            Serial.println("A28");
            break;
        }
        break;
      case 19:
        switch (nmark) {
          case 1:
            Serial.println("A34");
            break;
          case 2:
            Serial.println("A35");
            break;
          case 3:
            Serial.println("A36");
            break;
          case 4:
            Serial.println("A37");
            break;
        }
        break;
    }
  }
}

//This function gets called when the encoder value is increased
void encHigher(int _dif) {
  for (int i = 0; i < _dif; i++) {
    switch (activeMode) {
      case 12:
        if (nmark == 1)
          Serial.println("A01");
        if (nmark == 2)
          Serial.println("A03");
        break;
      case 14:
        if (nmark == 1)
          Serial.println("A07");
        if (nmark == 2)
          Serial.println("A09");
        break;
      case 15:
        if (nmark == 1)
          Serial.println("A13");
        if (nmark == 2)
          Serial.println("A15");
        break;
      case 16:
        if (nmark == 1)
          Serial.println("A19");
        if (nmark == 2)
          Serial.println("A21");
        break;
      case 17:
        switch (nmark) {
          case 1:
            Serial.println("A29");
            break;
          case 2:
            Serial.println("A30");
            break;
          case 3:
            Serial.println("A31");
            break;
          case 5:
            Serial.println("A32");
            break;
        }          
        break;
      case 19:
        switch (nmark) {
          case 1:
            Serial.println("A38");
            break;
          case 2:
            Serial.println("A39");
            break;
          case 3:
            Serial.println("A40");
            break;
          case 4:
            Serial.println("A41");
            break;
        }
        break;
    }
  }
}

void loop() {
  gspSwitch::checkAll();
  //gspQuad::checkAll();
  gspSerialResponse::checkAll();
}