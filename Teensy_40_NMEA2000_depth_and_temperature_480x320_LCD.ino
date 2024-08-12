
#include "skp_lvgl.h"
#include <lvgl.h>
#include <NMEA2000_CAN.h>
#include <N2kMessages.h>
#include <N2kMessagesEnumToStr.h>

extern lv_obj_t * label_water_depth; 
extern lv_obj_t * gauge1;
extern lv_obj_t * lvneedle;
extern lv_obj_t * label_temperature;

char buff[20];

typedef struct {
  unsigned long PGN;
  void (*Handler)(const tN2kMsg &N2kMsg); 
} tNMEA2000Handler;

void SystemTime(const tN2kMsg &N2kMsg);
void Depth(const tN2kMsg &N2kMsg);
void Temperature(const tN2kMsg &N2kMsg);
void TemperatureA(const tN2kMsg &N2kMsg);

// Spec for Airmar DT800:
// https://airmartechnology.com/uploads/SpecApps/NMEA%202000%20Smart%20Sensors%20_%20ALL.pdf
// There is some ambiguity about which PGN is reporting water temp in
// my device. Testing multiples for now, will delete the others when I
// figure out which one it is.
tNMEA2000Handler NMEA2000Handlers[]={
  {126992L,&SystemTime},
  {130310L,&Temperature},
  {130311L,&TemperatureA},
  {128267L,&Depth},
  {0,0}
};

Stream *OutputStream;

void HandleNMEA2000Msg(const tN2kMsg &N2kMsg);

void setup() {
  delay(1000);

  Serial.begin(115200); 
  Serial.println("Teensy 4.0 NMEA2000 display www.skpang.co.uk 12/20, modded 7/24");
  
  OutputStream=&Serial;
   
  // NMEA2000.SetN2kCANReceiveFrameBufSize(50);
  // Do not forward bus messages at all
  NMEA2000.SetForwardType(tNMEA2000::fwdt_Text);
  NMEA2000.SetForwardStream(OutputStream);
  // Set false below, if you do not want to see messages parsed to HEX within library
  NMEA2000.EnableForward(false);
  NMEA2000.SetMsgHandler(HandleNMEA2000Msg);
  //  NMEA2000.SetN2kCANMsgBufSize(2);
  NMEA2000.Open();
  OutputStream->print("Running...");

  Serial.print(CrashReport);

  skp_lvgl_init();   // Start LVGL
  
}

//*****************************************************************************
template<typename T> void PrintLabelValWithConversionCheckUnDef(const char* label, T val, double (*ConvFunc)(double val)=0, bool AddLf=false ) {
  OutputStream->print(label);
  if (!N2kIsNA(val)) {
    if (ConvFunc) { OutputStream->print(ConvFunc(val)); } else { OutputStream->print(val); }
  } else OutputStream->print("not available");
  if (AddLf) OutputStream->println();
}

//*****************************************************************************
void SystemTime(const tN2kMsg &N2kMsg) {
    unsigned char SID;
    uint16_t SystemDate;
    double SystemTime;
    tN2kTimeSource TimeSource;
    
    if (ParseN2kSystemTime(N2kMsg,SID,SystemDate,SystemTime,TimeSource) ) {
      PrintLabelValWithConversionCheckUnDef("System time: ",SID,0,true);
      PrintLabelValWithConversionCheckUnDef("  days since 1.1.1970: ",SystemDate,0,true);
      PrintLabelValWithConversionCheckUnDef("  seconds since midnight: ",SystemTime,0,true);
                        OutputStream->print("  time source: "); PrintN2kEnumType(TimeSource,OutputStream);
    } else {
      OutputStream->print("Failed to parse PGN: "); OutputStream->println(N2kMsg.PGN);
    }
}

//*****************************************************************************
void Depth(const tN2kMsg &N2kMsg) {

   unsigned char SID;
   double DepthBelowTransducer;
   double DepthOffset;
   double DepthRange;

   double DepthAngle_degree;

   if(ParseN2kWaterDepth(N2kMsg, SID, 
                         DepthBelowTransducer, DepthOffset, DepthRange)) 
   {
      Serial.print("Depth ");
      Serial.print(DepthBelowTransducer);
      Serial.print(" ");
   
      // Depth is in meters. Convert to feet.
      DepthBelowTransducer = DepthBelowTransducer * (39.3/12);

      sprintf(buff,"%2.0f", DepthBelowTransducer);
      lv_label_set_text(label_water_depth, buff);    // Update depth on LCD
   
      // Make the needle pin at 80ft. (std::min requires including
      // another library.)
      DepthAngle_degree = (DepthBelowTransducer/80) * 360;
      if (DepthAngle_degree > 357) DepthAngle_degree = 357.0;
      DepthAngle_degree = 900 + DepthAngle_degree * 10;
      Serial.print("Depth direction ");
      Serial.println(DepthAngle_degree);
   
      // Update needle on LCD, units are 0.1 degree for some reason.
      lv_img_set_angle(lvneedle, DepthAngle_degree*10);   
  
    } else {
      OutputStream->print("Failed to parse PGN: "); 
      OutputStream->println(N2kMsg.PGN);
    }
}
//*****************************************************************************
// For PGN 130310
void Temperature(const tN2kMsg &N2kMsg) {
    unsigned char SID;
    double ActualTemperature;
    double AirTemperature;  
    double AirPressure;  
    double temperature;

    if (ParseN2kOutsideEnvironmentalParameters(N2kMsg, SID, 
                                               ActualTemperature,
                                               AirTemperature,
                                               AirPressure) ) 
    {
      // Convert K to C
      //temperature = ActualTemperature - 273.15; 
      // Convert K to F
      temperature = (ActualTemperature - 273.15) * (9/5) + 32; 
        
      Serial.print("Temperature ");
      Serial.println(temperature);
      sprintf(buff,"%2.1f",temperature);
      // Update temperature on LCD
      lv_label_set_text(label_temperature, buff); 

    } else {
      OutputStream->print("Failed to parse PGN: ");  OutputStream->println(N2kMsg.PGN);
    }
}
//*****************************************************************************
// For PGN 130311
void TemperatureA(const tN2kMsg &N2kMsg) {
    unsigned char SID;
    tN2kTempSource TempSource;
    double ActualTemperature;
    tN2kHumiditySource HumiditySource;
    double Humidity;
    double AirPressure;  
    double temperature;

    if (ParseN2kEnvironmentalParameters(N2kMsg, SID,
                                        TempSource, ActualTemperature,
                                        HumiditySource, Humidity,
                                        AirPressure) ) 
    {
      // Convert K to C
      //temperature = ActualTemperature - 273.15; 
      // Convert K to F
      temperature = (ActualTemperature - 273.15) * (9/5) + 32; 
      temperature = -temperature;
        
      Serial.print("TemperatureA ");
      Serial.println(temperature);
      sprintf(buff,"%2.1f",temperature);
      // Update temperature on LCD
      lv_label_set_text(label_temperature, buff); 

    } else {
      OutputStream->print("Failed to parse PGN: ");  OutputStream->println(N2kMsg.PGN);
    }
}
//*****************************************************************************
void printLLNumber(Stream *OutputStream, unsigned long long n, uint8_t base=10)
{
  unsigned char buf[16 * sizeof(long)]; // Assumes 8-bit chars.
  unsigned long long i = 0;

  if (n == 0) {
    OutputStream->print('0');
    return;
  }

  while (n > 0) {
    buf[i++] = n % base;
    n /= base;
  }

  for (; i > 0; i--)
    OutputStream->print((char) (buf[i - 1] < 10 ?
      '0' + buf[i - 1] :
      'A' + buf[i - 1] - 10));
}

//*****************************************************************************
void BinaryStatusFull(const tN2kMsg &N2kMsg) {
    unsigned char BankInstance;
    tN2kBinaryStatus BankStatus;

    if (ParseN2kBinaryStatus(N2kMsg,BankInstance,BankStatus) ) {
      OutputStream->print("Binary status for bank "); OutputStream->print(BankInstance); OutputStream->println(":");
      OutputStream->print("  "); //printLLNumber(OutputStream,BankStatus,16);
      for (uint8_t i=1; i<=28; i++) {
        if (i>1) OutputStream->print(",");
        PrintN2kEnumType(N2kGetStatusOnBinaryStatus(BankStatus,i),OutputStream,false);
      }
      OutputStream->println();
    }
}

//*****************************************************************************
void BinaryStatus(const tN2kMsg &N2kMsg) {
    unsigned char BankInstance;
    tN2kOnOff Status1,Status2,Status3,Status4;

    if (ParseN2kBinaryStatus(N2kMsg,BankInstance,Status1,Status2,Status3,Status4) ) {
      if (BankInstance>2) { // note that this is only for testing different methods. MessageSender.ini sends 4 status for instace 2
        BinaryStatusFull(N2kMsg);
      } else {
        OutputStream->print("Binary status for bank "); OutputStream->print(BankInstance); OutputStream->println(":");
        OutputStream->print("  Status1=");PrintN2kEnumType(Status1,OutputStream,false);
        OutputStream->print(", Status2=");PrintN2kEnumType(Status2,OutputStream,false);
        OutputStream->print(", Status3=");PrintN2kEnumType(Status3,OutputStream,false);
        OutputStream->print(", Status4=");PrintN2kEnumType(Status4,OutputStream,false);
        OutputStream->println();
      }
    }
}



//*****************************************************************************
//NMEA 2000 message handler
void HandleNMEA2000Msg(const tN2kMsg &N2kMsg) {
  int iHandler;
  
  // Find handler
  OutputStream->print("In Main Handler: "); OutputStream->println(N2kMsg.PGN);

  for (iHandler=0; NMEA2000Handlers[iHandler].PGN!=0 && !(N2kMsg.PGN==NMEA2000Handlers[iHandler].PGN); iHandler++);
  
  if (NMEA2000Handlers[iHandler].PGN!=0) {
    NMEA2000Handlers[iHandler].Handler(N2kMsg); 
  }
}

//*****************************************************************************
void loop() 
{ 
  NMEA2000.ParseMessages();
  lv_task_handler(); /* let the GUI do its work */
}
