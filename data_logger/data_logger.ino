int wait = 5; //delay in seconds between readings
int dumpPin = 2; //interrupt pi to trigger data dump

const int ARR_SIZE = 500;
int nums[ARR_SIZE];

boolean dumpMail = false;

int k = 0;
int loopNum = 0;

void setup()
{
  Serial.begin(9600);
  
  pinMode(dumpPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(dumpPin), dumpData, FALLING);
}

void loop()
{
  if(loopNum < ARR_SIZE)
  {
    nums[loopNum] = readData();
    loopNum++;
  }

  for(int i = 0; i < wait; i++)
  {
    delay(10);
  }

  if(dumpMail)
  {
    Serial.println("dump");
    for(int i = 0; i < loopNum; i++)
    {
      Serial.println(nums[i]);
    }
    dumpMail = false;
  }

}

int readData()
{
  k++;
  return k;
}

//opens the serial port then dumps the data to that port
void dumpData()
{
  dumpMail = true;
}

