#include<EEPROM.h>
struct EEDataIndex
{
  int _isValid:2;       // -1 = uninitialized/invalid, +1 = valid, 0 = full
  unsigned int _index:9;  
};
struct EEDataIndex eeDataIndex;
const int eeIndexAdress  = 0;
void setup() {
  // put your setup code here, to run once:
  eeDataIndex._isValid = 1;
  eeDataIndex._index = 1;

  EEPROM.put(eeIndexAdress, eeDataIndex);
}

void loop() {
  // put your main code here, to run repeatedly:

}
