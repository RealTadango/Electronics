#include <Servo.h> 
Servo arm;
//Servo hoofd;
//Servo stuur;

int pout_arm = 2;
//int pout_hoofd = 3;
//int pout_stuur = 6;

int pin_mode = 5;
//int pin_stuur  = 5;

int arm_beneden = 30;
int arm_boven1 = 100;
int arm_boven2 = 145;

//int hoofd_rechts = 00;
//int hoofd_midden = 60;
//int hoofd_links = 140;

int prog_vertraging = 50;
//int hoofd_prog_stap = 5;
int arm_prog_stap = 20;

int arm_gewenst = arm_beneden;
//int hoofd_gewenst = hoofd_midden;
int zwaai_delay = 200;
int zwaai_init = 2000;

long last = 0;

void setup()
{
  //stuur.attach(pout_stuur);
  arm.attach(pout_arm);
  //hoofd.attach(pout_hoofd);
  pinMode(pin_mode, INPUT);
  //pinMode(pin_stuur, INPUT);

  last = millis();
}

//int last_stuur = 90;
int last_mode = 1500;

void loop()
{  
  int mode = pulseIn(pin_mode, HIGH, 5000);
  //int stuur = pulseIn(pin_stuur, HIGH, 5000);
    
  if(mode > 900 && mode < 2100)
  {
    last_mode = mode;
  }

  //if(stuur > 900 && stuur < 2100)
  //{
  //  last_stuur = stuur;
  //}
  
  if(last_mode < 1300)
  {
    arm_gewenst = arm_beneden;
    //Sturen();
  }
  else if(last_mode < 1700)
  {
    ArmHoog();
  }
  else
  {
    Zwaaien();
  }  

  //UpdateStuur();
  //UpdateHoofd();
  UpdateArm();
  delay(prog_vertraging);
}

/*
void Demo()
{
  Zwaaien();
  //TODO stuur
}

void Sturen()
{
  arm_gewenst = arm_beneden;
  hoofd_gewenst = hoofd_midden;
  
  if(last_stuur < 1450)
  {
    hoofd_gewenst = hoofd_links;
  }
  else if(last_stuur > 1550)
  {
    hoofd_gewenst = hoofd_rechts;
  }
  else
  {
    hoofd_gewenst = hoofd_midden;
  }
}

void Vliegen()
{
  arm_gewenst = arm_beneden;

  long d = millis() - last;
  if(d > 6000)
  {
    last = millis();
    hoofd_gewenst = hoofd_midden;
  }
  else if(d > 4500)
  {
    hoofd_gewenst = hoofd_links;
  }
  else if(d > 3000)
  {
    hoofd_gewenst = hoofd_rechts;
  }
}

*/

void ArmHoog()
{
    arm_gewenst = arm_boven2;
}

void Zwaaien()
{
  long d = millis() - last;
  if(d > zwaai_init + (zwaai_delay * 7))
  {
    last = millis();
    arm_gewenst = arm_beneden;
  }
  else if(d > zwaai_init + (zwaai_delay * 6))
  {
    //hoofd_gewenst = hoofd_midden;
    arm_gewenst = arm_boven1;
  }
  else if(d > zwaai_init + (zwaai_delay * 5))
  {
    arm_gewenst = arm_boven2;
  }
  else if(d > zwaai_init + (zwaai_delay * 4))
  {
    arm_gewenst = arm_boven1;
  }
  else if(d > zwaai_init + (zwaai_delay * 3))
  {
    arm_gewenst = arm_boven2;
  }
  else if(d > zwaai_init + (zwaai_delay * 2))
  {
    arm_gewenst = arm_boven1;
  }
  else if(d > zwaai_init + zwaai_delay)
  {
    arm_gewenst = arm_boven2;
  }
  else if(d > zwaai_init)
  {
    arm_gewenst = arm_boven1;
    //hoofd_gewenst = hoofd_links;
  }
}

void UpdateArm()
{
  int arm_nu = arm.read();
  
  if(abs(arm_nu - arm_gewenst) < arm_prog_stap)
  {
    arm.write(arm_gewenst);
  }
  else
  {
    if(arm_nu > arm_gewenst)
    {
      arm.write(arm_nu - arm_prog_stap);
    }
    else
    {
      arm.write(arm_nu + arm_prog_stap);
    }
  }
}

/*
void UpdateHoofd()
{
  int hoofd_nu = hoofd.read();
  
  if(abs(hoofd_nu - hoofd_gewenst) < hoofd_prog_stap)
  {
    hoofd.write(hoofd_gewenst);
  }
  else
  {
    if(hoofd_nu > hoofd_gewenst)
    {
      hoofd.write(hoofd_nu - hoofd_prog_stap);
    }
    else
    {
      hoofd.write(hoofd_nu + hoofd_prog_stap);
    }
  }
}

void UpdateStuur()
{
  if(last_stuur > 900 && last_stuur < 2100)
  {
    stuur.write(90 + ((last_stuur - 1500) / 10) );
  }
}
*/
