/* This data logger is designed to read an input from a Vernier pH meter, and log the first 255 values in EEPROM.
 *  
 * The device is controlled by a set of commands that can be input from the Serial Monitor. The commands are:
 *    DUMP: This prints all of the data recorded since the last reset directly to the Serial Monitor with one entry per line.
 *    RESET: This resets the EEPROM by setting the pointer location to 0. The message "Reset Complete" will be printed to the Serial Monitor when successful.
 *    PRINT: This prints all recorded data to the Serial Monitor formatted as <entry number, value>. The first line of the report states the number of total entries.
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
        for(int i = 1; i <= checkLastLocation(); i++)
        {
          uint8_t data = readMemory(i);
          Serial.println(decompressData(data));
        }
      }
      else if(command.equalsIgnoreCase("RESET"))
      {
        clearEEPROM();
        Serial.println("Reset Complete");
      }
      else if(command.equalsIgnoreCase("PRINT"))
      {
        Serial.println(checkLastLocation());
        for(int i = 1; i <= checkLastLocation(); i++)
        {
          uint8_t data = readMemory(i);
          Serial.print(i);
          Serial.print(" --- ");
          Serial.println(decompressData(data) / 100.0);
        }
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

/*Converts a 32-bit signed integer into an unsigned 8-bit integer through a linear transformation
 * Input: integer in range 700 +/- 255
 * Output: uint8_t in range 0,255
 */
uint8_t compressData(int data)
{
  data -= 445;
  uint8_t compressed = data >> 1;
  return compressed;
}

/* Converts an unsigned 8-bit integer into a 32-bit signed integer by a linear transformation
 * Input: integer in range 0,255
 * Ouptut: integer in range 700 +/- 255
 */
int decompressData(uint8_t data)
{
  int output = data;
  output = output << 1;
  output += 446;
  return output;
}

/* stores an 8-bit number in the next available EEPROM memory space
 * stops recording after 255 numbers have been recorded
 */
void saveData(uint8_t data)
{
  uint8_t address = checkLastLocation();
  if(address < 255)
  {
    address++;
    EEPROM.write(address, data);
    updateLocation(address);
  }
}

/* Gets the location of the last place data was entered
 * Output: last occupied spot in EEPROM memory
 */
uint8_t checkLastLocation()
{
  return EEPROM.read(0);
}

/* Moves the last location pointer to a new value
 * Input: new location
 */
void updateLocation(uint8_t loc)
{
  EEPROM.write(0, loc);
}

/* Reads a location in EEPROM memory
 * Input: address to read
 * Ouput: data contained at the address
 */
uint8_t readMemory(uint8_t address)
{
  return EEPROM.read(address);
}

/* Sets the last location pointer to 0
 */
void clearEEPROM()
{
  updateLocation(0);
}
