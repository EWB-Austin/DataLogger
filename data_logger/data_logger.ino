/* This data logger is designed to read an input from a Vernier pH meter, and log the first 255 values in EEPROM.
 *  
 * The device is controlled by a set of commands that can be input from the Serial Monitor. The commands are:
 *    DUMP: This prints all of the data recorded since the last reset directly to the Serial Monitor with one entry per line.
 *    RESET: This resets the EEPROM by setting the pointer location to 0. The message "Reset Complete" will be printed to the Serial Monitor when successful.
 *    PRINT: This prints all recorded data to the Serial Monitor formatted as <entry number, value>. The first line of the report states the number of total entries.
 *    PYPRINT: Prints data formatted for the pySerial read program
 */


#include <EEPROM.h>
#include "VernierLib.h"
VernierLib Vernier;

int wait = 3; //delay in seconds between readings
/* useful intervals:
 * 1 minute - 60
 * 5 minutes - 300
 * 10 minutes - 600
 * 15 minutes - 900
 * 30 minutes - 1800
 * 1 hour - 3600
 */

void setup()
{
  Serial.begin(9600);

  Vernier.autoID();
}

void loop()
{
  //read and store data into EEPROM
  int pHData = readData();
  saveData(compressData(pHData));

  //Waits until the wait period is over, checks every second for Serial Commands
  for(int i = 0; i < wait; i++)
  {
    delay(1000); //delays one second
    //Checking for commands
    if(Serial.available() > 0) 
    {
      //Read input String and remove newline character on the end
      String command = Serial.readString();
      command = command.substring(0,command.length() - 1);
      Serial.println(command);
      //Clear any remaining data in the buffer
      while(Serial.available() > 0)
        Serial.read();
      //Process commands
      if(command.equalsIgnoreCase("DUMP"))
      {
        for(int i = 1; i*2 < getAddress(); i++)
        {
          uint16_t data = readMemory(i*2);
          Serial.println(data);
        }
      }
      else if(command.equalsIgnoreCase("RESET"))
      {
        clearEEPROM();
        Serial.println("Reset Complete");
      }
      else if(command.equalsIgnoreCase("PRINT"))
      {
        //Serial.println(getAddress());
        for(int i = 1; i*2 < getAddress(); i++)
        {
          uint16_t data = readMemory(i*2);
          Serial.print(i-1);
          Serial.print(" --- ");
          Serial.println(data / 100.0);
        }
      }
      else if(command.equalsIgnoreCase("PYPRINT"))
      {
        Serial.println(wait);
        int numReadings = (getAddress() / 2) - 1;
        Serial.println(numReadings);
        for(int i = 1; i*2 < getAddress(); i++)
        {
          uint16_t data = readMemory(i*2);
          Serial.println(data);
        }
        Serial.println(-1);
      }
      else
      {
        Serial.println("Invalid Command");
      }
    }
  }

}

/* Reads the float value from the Vernier sensor and returns an integer value scaled by 100 (keeps 3 significant digits)
 * Output: Data from the Vernier Sensor as an integer
 */
int readData()
{
  int k = 100 * Vernier.readSensor();
  return k;
}

/* Converts a 32-bit signed integer into an unsigned 16-bit unsigned integer.
 * Integers outside the range of a uint16_t are set to the bound.
 * Input: integer
 * Output: bounded uint16_t
 */
uint16_t compressData(int data)
{
  uint16_t compressed = 0;
  if(data > 0xFFFF){
    compressed = 0xFFFF;
  }else if(data < 0){
    compressed = 0;
  }else{
    compressed = (uint16_t)(data & 0x0000FFFF);
  }
  return compressed;
}

/* stores an 16-bit number in the next available EEPROM memory space
 * stops recording when the memory is full
 */
void saveData(uint16_t data)
{
  uint16_t address = getAddress();
  if(address + 1 < EEPROM.length())
  {
    EEPROM.write(address, (uint8_t)((data & 0xFF00)>>8));
    EEPROM.write(address + 1, (uint8_t)(data & 0x00FF));
    updateLocation(address + 2);
  }
}

/* Gets the location of the last place data was entered
 * Output: first unoccupied spot in EEPROM memory
 */
uint16_t getAddress()
{
  uint16_t address = EEPROM.read(0);
  address = address << 8;
  address += EEPROM.read(1);
  return address;
}

/* Moves the last location pointer to a new value
 * Input: new location
 */
void updateLocation(uint16_t loc)
{
  EEPROM.write(0, (uint8_t)((loc & 0xFF00) >> 8));
  EEPROM.write(1, (uint8_t)(loc & 0x00FF));
}

/* Reads a location in EEPROM memory
 * Input: address to read
 * Ouput: data contained at the address
 */
uint16_t readMemory(int address)
{
  if(address + 1 >= EEPROM.length() || address < 0)
  {
    return 0xFFFF;
  }
  uint16_t data = EEPROM.read(address);
  data = data << 8;
  data += EEPROM.read(address + 1);
  return data;
}

/* Sets the last location pointer to the first available location
 */
void clearEEPROM()
{
  updateLocation(2);
}
