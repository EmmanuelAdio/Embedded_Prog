#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <EEPROM.h>
#include <avr/eeprom.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();


#define len 5

////////////////
extern unsigned int __heap_start;
extern void *__brkval;

/*
 * The free list structure as maintained by the
 * avr-libc memory allocation routines.
 */
struct __freelist
{
  size_t sz;
  struct __freelist *nx;
};

/* The head of the free list structure */
extern struct __freelist *__flp;


/* Calculates the size of the free list */
int freeListSize()
{
  struct __freelist* current;
  int total = 0;
  for (current = __flp; current; current = current->nx)
  {
    total += 2; /* Add two bytes for the memory block's header  */
    total += (int) current->sz;
  }

  return total;
}

int freeMemory()
{
  int free_memory;
  if ((int)__brkval == 0)
  {
    free_memory = ((int)&free_memory) - ((int)&__heap_start);
  }
  else
  {
    free_memory = ((int)&free_memory) - ((int)__brkval);
    free_memory += freeListSize();
  }
  return free_memory;
}
////////////////

//make some arrows
byte upArrow[] = {
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100,
  B00000,
};

byte downArrow[] = {
  B00000,
  B00100,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100,
};

//important global variables
unsigned long lastchecked;
uint8_t pressedButton;
enum State {
  SYNCHRONISATION,
  NORMAL_DISPLAY,
  ONLY_ON,
  ONLY_OFF,
  DISPLAY_STUDENT_ID,
}state,pState;

//this is the device class used to create the device objects.
class Device{
  private:
    char ID[4] = "ZZZ";
    char location[16];
    char type;
    char state[4] = "OFF";
    int power = 0;
    int temperature = 9;
    bool EPFlag = 0;

  public:
    //getter methods
    String getID();
    String getLocation();
    char getType();
    String getState();
    int getPower();
    int getTemperature();
    bool getEPFlag();

    //setter methods
    void setID(String ID);
    void setLocation(String location);
    void setType(char type);
    void setState(String state);
    void setPower(int power);
    void setTemperature(int temperature);
    void setEPFlag(bool flag);

};
String Device::getID() {
  return String(ID);
}

String Device::getLocation() {
  return String(location);
}

char Device::getType() {
  return type;
}

String Device::getState() {
  return String(state);
}

int Device::getPower() {
  return power;
}

int Device::getTemperature() {
  return temperature;
}

bool Device::getEPFlag() {
  return EPFlag;
}


void Device::setID(String ID){
  int str_len = ID.length() + 1;
  ID.toCharArray(this->ID, str_len);
}

void Device::setLocation(String location){
  int str_len = location.length() + 1;
  location.toCharArray(this->location, str_len);
}

void Device::setType(char type){
  this->type = type;
}

void Device::setState(String state){
  int str_len = state.length() + 1;
  
  //this is the validation neede dwhen setting the state of a device.
  if ((state.equals("ON")) || (state.equals("OFF"))){
    //this condtion is for formating the stae on the lcd
    if (state.equals("ON")){
    state = state + " ";
    }
    state.toCharArray(this->state, str_len);
    
  } else {
    Serial.print("Invalid State, please enter either ON or OFF");
  }
    
}

void Device::setPower(int power){
  this->power = power;
}

void Device::setTemperature(int temperature){
  if ((0 <= power) && (power <= 100)){
    this->temperature = temperature;
  }
}

void Device::setEPFlag(bool flag){
  this->EPFlag = flag;
}

//the array of devices and  copy of that array that will be used in HCI extention
//changes will only be made to the  original devices array then they will be copied over to the copy of the array.
Device sDevices[len];
Device sDevicesCopy[len];

//important pointers
int start = 0;
int end;
int display = 0;

void scrollLocation(class Device device){
  /*
  This funcion will be used to make the location of the device scroll on screen, it will be triggered when the length of the location is longer than 11 charecters
  */
  //Serial.print("Scrolling!");
  String location = device.getLocation();
  String displayedLocation = location;
  unsigned long timer = millis();

  lcd.print(displayedLocation);
  while(true){
    pressedButton = lcd.readButtons();

    if ((pressedButton != 0) || ((Serial.available() > 0))){
      if (((state == ONLY_ON) && (pressedButton & BUTTON_LEFT)) 
      || ((state == ONLY_OFF) && (pressedButton & BUTTON_RIGHT))){
        //skip
      }else{
        break;
      }
    }

    //shift the Locations string every 2 seconds.
    if((timer + 1500) <= millis()){
      timer = millis();

      //refresh the location when scrolling has completed.
      if (displayedLocation.length() == 1){
        displayedLocation = location;

        lcd.setCursor(5,0);
        lcd.print(location);
      } else {
        //remove the first charecter from the location string.
        displayedLocation.remove(0, 1);

        //clear the line when location has been shifted.
        lcd.setCursor(5, 0);
        for(int i = 0; i < 11; i++){
          lcd.print(" ");
        }

        lcd.setCursor(5,0);
        lcd.print(displayedLocation);
      }
    }
  }
}

void displayDevice(Device DeviceArray[], int index){
  /*
  this is where we will write how the device information is displayed on the arduino.

  it takes an array of devices and the index of the device in the array
  */
  //Serial.print("Display device")
  //create the up down arrow charecters 
  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);


  lcd.setCursor(0,0);
  if (!(DeviceArray[index].getID().equals("ZZZ"))){
    //display the device ID and up arrow  ->  first line of the lcd screen.
    (DeviceArray[index].getID() != DeviceArray[start].getID())?lcd.write(byte(0)):lcd.print(" ");
    lcd.print(DeviceArray[index].getID());lcd.print(" ");
    
    lcd.setCursor(0,1);
    //display down arrow, device type, device state ->  second line of lcd screen.
    (((DeviceArray[index].getID() != DeviceArray[end].getID())))?lcd.write(byte(1)):lcd.print(" ");
    lcd.print(DeviceArray[index].getType());lcd.print(" ");lcd.print(DeviceArray[index].getState());lcd.print(" ");
    
    //create the conditions for displaying the device power output.
    if ((DeviceArray[index].getType() == 'L') || (DeviceArray[index].getType() == 'S')){
      lcd.print(DeviceArray[index].getPower());lcd.print("%");
    } else if (DeviceArray[index].getType() == 'T'){
      lcd.print(DeviceArray[index].getTemperature());lcd.print(char(223));lcd.print("C");
    }

    //create the conditions for the backlight colour on the lcd. 
    if(DeviceArray[index].getState().equals("ON")){
      lcd.setBacklight(2);
    }else{
      lcd.setBacklight(3);
    }

    //display the device location ->  first line lcd
    //if the length of location is greater than 11 charecters run the scrollLocation function
    if (DeviceArray[index].getLocation().length() > 11){
      lcd.setCursor(5,0);
      //lcd.print(DeviceArray[index].getLocation());
      scrollLocation(DeviceArray[index]);
    }else{
      lcd.setCursor(5,0);
      lcd.print(DeviceArray[index].getLocation());
    }

  } else {
    //this condtion will run when the array is empty in any state.
    if(state == ONLY_ON){
      lcd.setBacklight(2);
      lcd.print("NOTHING'S ON");
    }else if(state == ONLY_OFF){
      lcd.setBacklight(3);
      lcd.print("NOTHING'S OFF");
    }else{
      lcd.setBacklight(7);
      lcd.clear();
    }
  }
}

void setTheEnd(Device deviceArray[]){
  /*
  this function will collect the index of the last device in the array passeed into it and sets that index to the variable end

  allowing end to be be the index of the last device in the device array pased into the function
  */
  //Serial.print("Set End");
  for(int i = 0; i < len; i++){
    if(deviceArray[i].getID().equals("ZZZ")){
      end = i-1;
      break;
    }
  }
}

void sortDevices(Device deviceArray[]){
  /*
  This is the sort Function. An essential function in the project that allows for so many other features to work.

  it takes an array of devices as its parameters and sorts that array.

  just a noraml bubble sort was used.
  */
  //Serial.print("sort array");
  for(int i = 0; i < (len-1); i++){
    for(int j = 0; j < (len-i-1); j++){
      if ((deviceArray[j].getID().compareTo(deviceArray[j+1].getID())) > 0){
        Device temp;
        
        temp.setID(deviceArray[j].getID());
        temp.setLocation(deviceArray[j].getLocation());
        temp.setType(deviceArray[j].getType());
        temp.setState(deviceArray[j].getState());
        temp.setPower(deviceArray[j].getPower());
        temp.setTemperature(deviceArray[j].getTemperature());
        temp.setEPFlag(deviceArray[j].getEPFlag());
        
        
        deviceArray[j].setID(deviceArray[j+1].getID());
        deviceArray[j].setLocation(deviceArray[j+1].getLocation());
        deviceArray[j].setType(deviceArray[j+1].getType());
        deviceArray[j].setState(deviceArray[j+1].getState());
        deviceArray[j].setPower(deviceArray[j+1].getPower());
        deviceArray[j].setTemperature(deviceArray[j+1].getTemperature());
        deviceArray[j].setEPFlag(deviceArray[j+1].getEPFlag());

        deviceArray[j+1].setID(temp.getID());
        deviceArray[j+1].setLocation(temp.getLocation());
        deviceArray[j+1].setType(temp.getType());
        deviceArray[j+1].setState(temp.getState());
        deviceArray[j+1].setPower(temp.getPower());
        deviceArray[j+1].setTemperature(temp.getTemperature());
        deviceArray[j+1].setEPFlag(temp.getEPFlag());
      }
    }
  }
  //print all device IDs...
}

void addDevice(String *device){
  /*
  this is the function that allows the user to add a devices to the array.

  Takes a String reference of the user's input (device) and then extracts the inforamtion needed for the add.
  */
   //Serial.print("Adding devices works");
  //check if the ID is already being used for another device. and if it is we will be replacing that devices information.
  bool inside = false;
  int index;
  for(int i = 0; i < len;i++){
    String tempID = (*device).substring(2,5);
    if (tempID.equals(sDevices[i].getID())){
      inside = true;
      index = i;
      break;
    }
  }

  if(inside){
    //this is teh conditon that will run if teh ID has already been used.
    sDevices[index].setLocation((*device).substring(8,23));
    sDevices[index].setState("OFF");
    sDevices[index].setEPFlag(false);

    if (((*device).substring(6,7)).equals("S")){
      sDevices[index].setType('S');
      sDevices[index].setPower(0);

    } else if (((*device).substring(6,7)).equals("L")){
      sDevices[index].setType('L');
      sDevices[index].setPower(0);

    } else if (((*device).substring(6,7)).equals("T")){
      sDevices[index].setType('T');
      sDevices[index].setTemperature(9);

    } else if (((*device).substring(6,7)).equals("O")){
      sDevices[index].setType('O');

    } else if (((*device).substring(6,7)).equals("C")){
      sDevices[index].setType('C');

    }
  }else{
    //this is the condtition that will run if teh ID has not been used.
    if ((end+1) == len){
      //error message that will run when there is not more list in the array. and the program stops the user frpm adding the device.
      Serial.print("Array Full : No More Space available, please delete some devices.");
    } else {
      sDevices[end+1].setID((*device).substring(2,5));
      sDevices[end+1].setLocation((*device).substring(8,23));
      sDevices[end+1].setEPFlag(false);

      if (((*device).substring(6,7)).equals("S")){
        sDevices[end+1].setType('S');

        end++;
      } else if (((*device).substring(6,7)).equals("L")){
        sDevices[end+1].setType('L');

        end++;
      } else if (((*device).substring(6,7)).equals("T")){
        sDevices[end+1].setType('T');

        end++;
      } else if (((*device).substring(6,7)).equals("O")){
        sDevices[end+1].setType('O');

        end++;
      } else if (((*device).substring(6,7)).equals("C")){
        sDevices[end+1].setType('C');

        end++;
      }
    }
  }
  lcd.clear();
}

void changeDeviceState(String *device){
  /*
  This is the function that will be used to change a devices state.

  Takes a String reference of the user's input (device) and then extracts the state from that input.

  validation is done in the setter method for the devices state.
  */
   //Serial.print("changing state works");
  String ID = (*device).substring(2,5);
  String state = (*device).substring(6);
  //making it uppercase allows validation to be easier.
  state.toUpperCase();
  
  for(int i = 0; i <= len;i++){
    if (sDevices[i].getID() == ID){
      sDevices[i].setState(state);
    }
  }
}

void changeDevicePower(String *device){
  /*
  This is the function that will be used to chane the power output of a device.

  Takes a String reference of the user's input (device) and then extracts the power from that input.
  */
   //Serial.print("changing power works");
  String ID = (*device).substring(2,5);
  int power = (*device).substring(6).toInt();
  
  
  for(int i = 0; i <= len;i++){
    if (sDevices[i].getID() == ID){
      if (sDevices[i].getType() == 'T'){
        if ((9 <= power) && (power <= 32)){
        sDevices[i].setTemperature(power);
        } else {
          Serial.print("ERROR : value is outside of temperature range (9 - 32).");
        } 
      } else if ((sDevices[i].getType() == 'L') || (sDevices[i].getType() == 'S')){
        if ((0 <= power) && (power <= 100)){
        sDevices[i].setPower(power);
        } else {
          Serial.print("ERROR : value is outside of power range (0 - 100)");
        }
      } else {
        Serial.print("ERROR : Device does not support power output.");
      }
    }
  }
}

void removingDevice(String *device){
  /*
  This is the remove function that will be used to "remove" a device from the array.
  */
  //Serial.print("Removing devices works");
  String ID = (*device).substring(2,5); 
  
  for(int i = 0; i <= 5;i++){
    if (sDevices[i].getID() == ID){
      sDevices[i].setID("ZZZ");
      sDevices[i].setLocation("");
      sDevices[i].setPower(0);
      sDevices[i].setTemperature(0);
      sDevices[i].setState("OFF");
      sDevices[i].setEPFlag(false);
      end--;
      if (display > end){
        display = end;
      }
      break;
    }
  }
}

void stateCheck(){
  /*
  This function checks the state that the dveice is currently in and creates a condition that willl perform either the ONLY_OFF or ONLY_ON routine depending on which state
  the device is in.
  */
  if(state == ONLY_OFF){
    only_off_state();
    setTheEnd(sDevicesCopy);
  } else if (state == ONLY_ON){
    only_on_state();
    setTheEnd(sDevicesCopy);
  }
}

bool checkID(String *inputID){
  /*
  This function is used to check that the user enters a valid ID

  takes in a string reference of the ID that the user entered.
  */
  bool valid = true;
  if (((*inputID).length() != 3)){
    valid = false;
    Serial.print("Invalid ID :  ID needs to be 3 charecters long.");
    return valid;
  } else {
    for(int i = 0; i < (*inputID).length(); i++){
      if (((int) (*inputID)[i] < 65) || ((int) (*inputID)[i] > 90)){
        valid = false;
        Serial.print("Invalid ID : ID needs to only contain letters between A and Z");
        return valid;
      }
    }
    return valid;
  }
}

bool checkLocation(String *inputLocation){
  /*
  This function is used to check that the user enters a valid Location

  takes in a string reference of the location that the user entered.
  */
  bool valid = true;
  if (((*inputLocation).length() <= 0)){
    valid = false;
    Serial.print("Invalid Location : Please enter a Location!");
    return valid;
  } else {
    for (int i = 0; i < ((*inputLocation).length()); i++){
      if ( ((((int) (*inputLocation)[i]) >= 97) && (((int) (*inputLocation)[i]) <= 122)) 
      || ((((int) (*inputLocation)[i]) >= 0) && (((int) (*inputLocation)[i]) <= 9)) 
      || ((((int) (*inputLocation)[i]) >= 65) && (((int) (*inputLocation)[i]) <= 90)) ){
        //do nothing cause the charecter falls within range of valid charecters to use for location
        valid = true;
      } else {
        valid = false;
        Serial.print("Invalid Location : Please only enter charecters from A-Z or 0-9, and no Spaces.");
        return valid;
      }
    }
    return valid;
  }
}

bool checkType(char type){
  /*
  This function is used to check that the user enters a valid Type in the user input.

  takes in a teh charecter for the device type.
  */
  bool valid = true;
  if ((type == 'S') || (type == 'O') || (type == 'L') || (type == 'T') || (type == 'C')){
    return valid;
  } else {
    Serial.print("Invalid type : please enter either; 'S', 'O', 'L', 'T' or 'C'");
    valid = false;
    return valid;
  }
}

bool validate(String *inputString){
  /*
  This function is used to check that the user entered valid input 

  takes in a string reference of the user's input.
  */
  bool valid = true;
  
  switch((*inputString)[0]) {
    case 'A':
      //these are the condtions for a valid Add (A)
      if ((*inputString).length() > 7){
        if ( (((*inputString)[1]) == '-') && (((*inputString)[5]) == '-') && (((*inputString)[7]) == '-') ){
          if ( (checkID(&(*inputString).substring(2,5))) && (checkType((char) (*inputString)[6])) && (checkLocation(&(*inputString).substring(8))) ){
            valid = true;
            return valid;
          } else {
            valid = false;
            return valid;
          }
        }
      } 
      Serial.print("Error : invalid input format -> ");
      Serial.print(*inputString);
      valid = false;
      return valid;

      break;
    case 'S':
      //these are the condtions for a valid change of State of a device (S)

      if ((*inputString).length() > 6){
        if ( (((*inputString)[1]) == '-') && (((*inputString)[5]) == '-')){
          if ((checkID(&(*inputString).substring(2,5)))){
            return valid;
          } else {
            valid = false;
            return valid;
          }
        }
      } else {
        Serial.print("ERROR : Please enter a state");
      }

      Serial.print("ERROR : invalid input format -> ");
      Serial.print(*inputString);
      valid = false;
      return valid; 

      break;
    case 'P':
      //these are the condtions for a valid change Power output of a device (P)

      if ((*inputString).length() > 5){
        if ( (((*inputString)[1]) == '-') && (((*inputString)[5]) == '-')){
          if ((checkID(&(*inputString).substring(2,5)))){
            return valid;
          }  else {
            valid = false;
            return valid;
          }
        }
      } else {
        Serial.print("ERROR : Please enter a power output value.");
      }

      Serial.print("Error : invalid input format -> ");
      Serial.print(*inputString);
      valid = false;
      return valid;

      break;
    case 'R':
      //these are the condtions for a valid Remove (R)

      if ((*inputString).length() == 5){
        if ( (((*inputString)[1]) == '-') ){
          if ((checkID(&(*inputString).substring(2,5)))){
            return valid;
          }  else {
            valid = false;
            return valid;
          }
        }
      }

      Serial.print("Error : invalid input format -> ");
      Serial.print(*inputString);
      valid = false;
      return valid;

      break;
    default:
      Serial.print("Error: Invalid Input! please enter either; 'A','S','P','R' at the start.");
      valid = false;
      return valid;

      break;
  }
}

void checkForInput(){
  /*
  This function will be used to be checking for the user's input and handling it.
  */
   if (Serial.available() > 0){
    //save user's input 
    String input = Serial.readString();
    input.trim();
    if (validate(&input)){
      switch(input[0]) {
        case 'A':
          //procedure when the user wants to add a device.
          addDevice(&input);
          sortDevices(sDevices);
          stateCheck();
          lcd.clear();
          writeToEEPROM();
          break;
        case 'S':
          //procedure when the user wants to change the state of a device.
          changeDeviceState(&input);
          stateCheck();
          lcd.clear();
          writeToEEPROM();
          break;
        case 'P':
          //procedure when the user wants to change the power output of a device.
          changeDevicePower(&input);
          stateCheck();
          lcd.clear();
          writeToEEPROM();
          break;
        case 'R':
          //procedure when the user wants to remove a device.
          removingDevice(&input);
          sortDevices(sDevices);
          setTheEnd(sDevices);
          stateCheck();
          lcd.clear();
          writeToEEPROM();
          break;
      }
    }
  }
}

void displayStudentID_state(){
  //this is the function that countains the routine for when we are in the display student ID state.
  lcd.clear();
  lcd.setBacklight(5);
  lcd.setCursor(0,0);
  lcd.print("F229639");
  lcd.setCursor(0, 1);
  lcd.print("Free SRAM: ");lcd.print(freeMemory());
}

void only_off_state(){
  //this is the function that countains the routine for when we are in the ONLY_OFF state.

  //make a copy of the smart devices array!
  for (int i = 0; i < len; i++){
    sDevicesCopy[i] = sDevices[i];
  }

  //remove all the off devices from sDevices
  for (int i = 0; i < len; i++){
    if (!(sDevicesCopy[i].getState().equals("OFF"))){
      sDevicesCopy[i].setID("ZZZ");
    }
  }
  sortDevices(sDevicesCopy);
  setTheEnd(sDevicesCopy);
}

void only_on_state(){
  //this is the function that countains the routine for when we are in the ONLY_ON state.

  //make a copy of the smart devices array!
  for (int i = 0; i < len; i++){
    sDevicesCopy[i] = sDevices[i];
  }

  //remove all the off devices from sDevices
  for (int i = 0; i < len; i++){
    if (!(sDevicesCopy[i].getState().equals("ON"))){
      sDevicesCopy[i].setID("ZZZ");
    }
  }
  sortDevices(sDevicesCopy);
  setTheEnd(sDevicesCopy);
}

void writeToEEPROM(){
  //this function is going to be used to write the device array onto the EEPROM.
  int eeAddress = 0;

  for(int i = 0; i < len; i++){
    sDevices[i].setEPFlag(true);
    EEPROM.put(eeAddress, sDevices[i]);

    eeAddress += sizeof(sDevices[i]);
  }
}

void readEEPROM(){
  //this function is going to be used to read the devices from the EEPROM.
  end = -1;
  int eeAddress = 0;
  Device eeDevice;

  for(int i = 0; i < len; i++){
    EEPROM.get(eeAddress, eeDevice);

    if ((checkID(&eeDevice.getID())) && !(eeDevice.getID().equals("ZZZ"))){
      sDevices[i].setID(eeDevice.getID());
      sDevices[i].setLocation(eeDevice.getLocation());
      sDevices[i].setType(eeDevice.getType());
      sDevices[i].setState(eeDevice.getState());
      sDevices[i].setPower(eeDevice.getPower());
      sDevices[i].setTemperature(eeDevice.getTemperature());
      sDevices[i].setEPFlag(eeDevice.getEPFlag());

      end++;
    }else{
      break;
    }

    eeAddress += sizeof(eeDevice);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16, 2);

  state =  SYNCHRONISATION;
}

void loop() {
  // put your main code here, to run repeatedly:
  checkForInput();
  lastchecked = millis();

  switch(state) {
    case SYNCHRONISATION:
      //start writing the initialization stage code
      lcd.setBacklight(5);
      lcd.setCursor(0,0);

      lastchecked = millis();

      while(true){

        if ((lastchecked + 1000) < millis()){
          lastchecked = millis();
          Serial.print('Q');
        }

        if (Serial.available() > 0){
          String input = Serial.readString();
          input.trim();
          input.toUpperCase();
          if (input == "X"){
            break;
          } 
        }
      }

      //when syncronisation is complete.
      lcd.setBacklight(7);
      Serial.print("UDCHARS, EEPROM, FREERAM, HCI, SCROLL\n");

      readEEPROM();
      setTheEnd(sDevices);

      lastchecked = millis();
      state = NORMAL_DISPLAY;

    case DISPLAY_STUDENT_ID:
      //this is the procedure when the device is in the DISPLAY_STUDENT_ID state.
      displayStudentID_state();

      //while loop to keep device in the DISPLAY_STUDENT_ID state
      while (pressedButton & BUTTON_SELECT){
        if (Serial.available() > 0){
          checkForInput();
          displayStudentID_state();
        }

        //make the escape condition for the Display_STUDENT_ID state.
        pressedButton = lcd.readButtons();
        if(!pressedButton & BUTTON_SELECT){

          //this is going to be use to allow the device to return to the previous state it was in before going into the DISPLAY_STUDENT_ID state.
          if (pState == ONLY_ON){
            state = ONLY_ON;
          } else if (pState == ONLY_OFF){
            state = ONLY_OFF;
          } else{
             state = NORMAL_DISPLAY;
          }
         
          lcd.clear();
          //displayDevice(sDevices, display);
        }
      }
      break;

    case ONLY_ON://right
      //this is the procedure when the device is in the ONLY_ON state.
      lcd.clear();
      
      only_on_state();

      displayDevice(sDevicesCopy, display);

      //while loop to keep device in the ONLY_ON state
      while (true){
        if (Serial.available() > 0){
          checkForInput();
          displayDevice(sDevicesCopy, display);
        }

        
        pressedButton = lcd.readButtons();
        if((lastchecked+1000) < millis()){
          //contition to go into the DISPLAY_STUDENT_ID state.
          lastchecked = millis();
          uint8_t pressedButton2 = lcd.readButtons();

          if ((pressedButton & BUTTON_SELECT) && (pressedButton2 == pressedButton)){
            pState = ONLY_ON;
            state = DISPLAY_STUDENT_ID;
            break;
          }

        } else if(pressedButton & BUTTON_RIGHT){
          //this is the condition for escaping the the ONLY_ON state
          pState = ONLY_ON;
          state = NORMAL_DISPLAY;
          break;
        }else if(pressedButton & BUTTON_UP){
          //this is the condition to go up array
          if(sDevices[display].getID() != sDevices[start].getID()){
            display--;
            lcd.clear();
            displayDevice(sDevicesCopy, display);
          }
        }else if((pressedButton & BUTTON_DOWN) && !(end == start)){
          //this is the condition to go down the array
          if(sDevices[display].getID() != sDevices[end].getID()){
            display++;
            lcd.clear();
            displayDevice(sDevicesCopy, display);
          }
        }
      }
      //delay(500);
      lcd.clear();
      
      display = 0;
      setTheEnd(sDevices);

      displayDevice(sDevices, display);
      break;

    //ONLY_OFF state
    case ONLY_OFF://left
      //this is the procedure when the device is in the ONLY_OFF state.
      lcd.clear();

      only_off_state();

      displayDevice(sDevicesCopy, display);
      //while loop to keep device in the ONLY_OFF state
      while (true){
        if (Serial.available() > 0){
          checkForInput();
          displayDevice(sDevicesCopy, display);
        }

        pressedButton = lcd.readButtons();
        if((lastchecked+1000) < millis()){
          //contition to go into the DISPLAY_STUDENT_ID state.
          lastchecked = millis();
          uint8_t pressedButton2 = lcd.readButtons();

          if ((pressedButton & BUTTON_SELECT) && (pressedButton2 == pressedButton)){
            pState = ONLY_OFF;
            state = DISPLAY_STUDENT_ID;
            break;
          }

        } else if(pressedButton & BUTTON_LEFT){
          //this is the condition for escaping the the ONLY_OFF state
          pState = ONLY_OFF;
          state = NORMAL_DISPLAY;
          break;
        }else if(pressedButton & BUTTON_UP){
          //this is the condition to go up array
          if(sDevices[display].getID() != sDevices[start].getID()){
            display--;
            lcd.clear();
            displayDevice(sDevicesCopy, display);
          }
        }else if((pressedButton & BUTTON_DOWN) && !(end == start)){
          //this is the condition to go down the array
          if(sDevices[display].getID() != sDevices[end].getID()){
            display++;
            lcd.clear();
            displayDevice(sDevicesCopy, display);
          }
        }
      }
      lcd.clear();

      display = 0;
      setTheEnd(sDevices);

      displayDevice(sDevices, display);
      break;

    //NORMAL_DISPLAY state
    case NORMAL_DISPLAY:
      //this is the procedure when the device is in the NORMAL_DISPLAY state.
      sortDevices(sDevices);
      setTheEnd(sDevices);
      lcd.clear();
      displayDevice(sDevices, display);

      //while loop to keep device in the NORMAL_DISPLAY state
      while (true){
      while(true){
        if (Serial.available() > 0){
          checkForInput();
          displayDevice(sDevices, display);
        }
        pressedButton = lcd.readButtons();

        if((lastchecked+1000) < millis()){
          //contition to go into the DISPLAY_STUDENT_ID state.
          lastchecked = millis();
          uint8_t pressedButton2 = lcd.readButtons();

          if ((pressedButton & BUTTON_SELECT) && (pressedButton2 == pressedButton)){
          pState = NORMAL_DISPLAY;
          state = DISPLAY_STUDENT_ID;
          break;
          }

        }else if(pressedButton & BUTTON_RIGHT){
          //Condition ot initiate the ONLY_ON state
          pState = NORMAL_DISPLAY;
          state = ONLY_ON;
          display = 0;
          break;

        }else if(pressedButton & BUTTON_LEFT){
          //Condition ot initiate the ONLY_OFF state
          pState = NORMAL_DISPLAY;
          state = ONLY_OFF;
          display = 0;
          break;

        }else if(pressedButton & BUTTON_UP){
          //this is the condition to go up array
          if(sDevices[display].getID() != sDevices[start].getID()){
            display--;
            lcd.clear();
            displayDevice(sDevices, display);
          }
        }else if((pressedButton & BUTTON_DOWN) && !(end == start)){
          //this is the condition to go down the array
          if(sDevices[display].getID() != sDevices[end].getID()){
            display++;
            lcd.clear();
            displayDevice(sDevices, display);
          }
        }
      }
      break;
      }
    }
}
