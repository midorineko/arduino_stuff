int    joystick_xmax = 1023;
float  joystick_xmult = 1;
int    joystick_ymax = 1023;
float  joystick_ymult = 1;

static void calibrate()
{
  int cnt = 0;
  int xmax = 0;
  int xmin = 1023;
  int ymax = 0;
  int ymin = 1023;
  while(cnt < 4)
  {
     int jx = joystick_xmax-get_joystick_x();
     int jy = joystick_ymax-get_joystick_y();
     if((cnt % 2 == 0) && jx < 450) 
       cnt++;
     if((cnt % 2 == 1) && jx > 800) 
       cnt++;
     xmax = max(xmax, jx);
     xmin = min(xmin, jx);
     ymax = max(ymax, jy);
     ymin = min(ymin, jy);
  }
  joystick_xmax = xmax;
  joystick_xmult = 1023.0/(xmax - xmin);
  joystick_ymax = ymax;
  joystick_ymult = 1023.0/(ymax - ymin);
}

static unsigned int get_joystick_x()
{
  pinMode(14,INPUT);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  return (unsigned int)max(0, min(1023,((joystick_xmax-analogRead(A0))*joystick_xmult)));
}
static unsigned int get_joystick_y()
{
  pinMode(13,INPUT);
  pinMode(14,OUTPUT);
  digitalWrite(14,LOW);
  return (unsigned int)max(0, min(1023,((joystick_ymax-analogRead(A0))*joystick_ymult)));
}

void setup() {
  // put your setup code here, to run once:
      Serial.begin (115200);


}

void loop() {
    unsigned int x = get_joystick_x();
    unsigned int y = get_joystick_y();
    String buff = "Update    "+String(x)+","+String(y) + "," + String(digitalRead(0)==0) + "," + String(digitalRead(2)==0) + "," + String(digitalRead(12)==0) + "," + String(digitalRead(3)==0);
    Serial.println(buff);
    delay(10);
  // put your main code here, to run repeatedly:

}
