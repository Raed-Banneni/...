#include "main.h"
void success(void) {
  digitalWrite(led, 1);
  delay(50);
  digitalWrite(led, 0);
  delay(50);
}
void fail(void)
{
    digitalWrite(led, 1);
    delay(500);
    digitalWrite(led, 0);
    delay(500);
    digitalWrite(led, 1);
    delay(500);
    digitalWrite(led, 0);
    delay(500);  
    digitalWrite(led, 1);
    delay(500);
    digitalWrite(led, 0);
    delay(500);      digitalWrite(led, 1);
    delay(500);
    digitalWrite(led, 0);
    delay(500);  
 
}