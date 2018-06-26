int pin_led1 = 0;
int pin_led2 = 1;
int pin_rc = 3;
int offset = 0;
boolean up = true;


void setup()  { 
  pinMode(pin_led1, OUTPUT);
  pinMode(pin_led2, OUTPUT);
  pinMode(pin_rc, INPUT);
} 

void setLed(int led, int value)
{
  if(value > 255)
  {
    value = 255;
  }
  if(value < 0)
  {
    value = 0;
  }

  analogWrite(led, value);
}

void loop()  {
  long rc = pulseIn(pin_rc, HIGH, 50000);
  
  if(rc > 1500)
  {
    up = true;
  }
  else if(rc == 0)
  {
    up = false;
  }
  else
  {
    up = false;
  }
  
  int base = offset;
  
  if(base > 200)
  {
    base = 200;
  }
  
  int delta = base * 1.5;
  
  if(delta < 10)
  {
    delta = 10;
  }

  setLed(pin_led1, base + random(0, delta));    
  setLed(pin_led2, base + random(0, delta));    

  if(!up && offset > -20)
  {
    offset -= 4;
  }

  if(up && offset < 200)
  {
    offset += 3;
  }

  delay(20);
}

