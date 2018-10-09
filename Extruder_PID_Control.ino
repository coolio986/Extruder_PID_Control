#include <PID_v1.h>


const unsigned int secondsInAMinute = 60;
const unsigned int microSecondsInASecond = 1000000;

const unsigned long serialUpdateInterval=1000;
unsigned long serialUpdatePreviousTime = 0;

//*******Spooler variables*******//
unsigned int spooler_pin_irq = 0; //IRQ that matches to pin 2
const unsigned int spoolerPulsesPerRev = 10;
const unsigned long spoolerUpdateInterval=2500; // time in microseconds
unsigned long spoolerPulseCounts = 0;
double spoolerRPM = 0.00;
unsigned long spoolerStartTime = 0;
double spoolerSetpoint, SpoolerInput, SpoolerOutput;
double Spooler_consKp=1, Spooler_consKi=0.05, Spooler_consKd=0.25;
PID spoolerPID(&SpoolerInput, &SpoolerOutput, &spoolerSetpoint, Spooler_consKp, Spooler_consKi, Spooler_consKd, DIRECT);
//*****************************//


void setup() {
  // put your setup code here, to run once:
  Serial.begin (9600);
  Serial.setTimeout(50);

  //initialize the variables we're linked to
  SpoolerInput = spoolerRPM;
  spoolerSetpoint = 0;

  //turn the PID on
  spoolerPID.SetMode(AUTOMATIC);
  
  //setup ISR for spooler interrupt
  attachInterrupt(spooler_pin_irq, ISR_SpoolerPulseCounter, RISING);
}

void ISR_SpoolerPulseCounter() {
  spoolerPulseCounts++;
}

void loop() {

  ReadSerialData();
  CalculateSpoolerRPMS();
  PrintSerialData();

}

void ReadSerialData(){
  String a = "";

  String Device, Value;
  
  while(Serial.available()) {
    a= Serial.readString();// read the incoming data as string
    //Serial.println(a);

    if (a.length() >= 3){
      int splitPosition = a.indexOf(";");
      Device = a.substring(0, splitPosition);
      Value = a.substring(splitPosition + 1);
      
    }
  }

  if (isNumeric(Value)){
    if (Device.length() >= 1 && Value.length() >= 1){
      if (Device == "EXTRUDER_RPM"){
      
      }
      if(Device == "SPOOLER_RPM"){
        spoolerSetpoint = Value.toFloat();
      }
    }
  }
}

void PrintSerialData(){

  if (millis() - serialUpdatePreviousTime >= serialUpdateInterval) {
    serialUpdatePreviousTime = millis();

    Serial.println(PrintSpoolerRPM());
    Serial.println(PrintSpoolerSetpoint());
    
  }
}

String PrintSpoolerRPM(){
  return "SpoolerRPM = " + (String)spoolerRPM;
}

String PrintSpoolerSetpoint(){
  return "SpoolerSetpoint = " + (String)spoolerSetpoint;
}

void CalculateSpoolerRPMS(){

  if (spoolerStartTime == 0) { //kick start the timing routine
    spoolerStartTime=micros();
  
 }
 
 if (micros() - spoolerStartTime > spoolerUpdateInterval && spoolerPulseCounts > 5) { //PulseCounts > 5 filters out false counts
  
  //RPM= 6*1000000/(micros() - startTime)*PulseCounts; //RPM = ((secondsInAMinute / pulsesPerRev) * microSecondsInASecond)/(currentMicroSeconds - startTime) * numberOfPulses
  spoolerRPM = ((secondsInAMinute / spoolerPulsesPerRev) * microSecondsInASecond) / (micros() - spoolerStartTime) * spoolerPulseCounts;
  spoolerPulseCounts = 0; //reset IRQ counts
  
  spoolerStartTime=0;

  
  
  //Serial.print(F("RPM = "));
  //Serial.print(spoolerRPM);

  SpoolerInput = spoolerRPM;
  spoolerPID.Compute();
  
  //Serial.print("    PID Output Value= " + (String)SpoolerOutput); 
  
 }
}

bool isNumeric(String value){
  int decimalExists;
  if (value.length() == 0){return false;}
   
  for (unsigned int i = 0; i < value.length(); i++){
    if (value.charAt(i) == '.'){
      decimalExists++;
    }
    if (!isDigit(value.charAt(i)) && value.charAt(i) != '.') {
      return false;
    }
    
    if (decimalExists > 1){return false;}
        
  }
  
  
  return true;
}



     

