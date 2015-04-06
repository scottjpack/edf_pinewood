
//power on.
//	Unarmed, rotating lights.
//2-second button push while unarmed
//	Move to Armed State
//
//Armed State
//	brake LED, VERY low motor as warning for 5 seconds
//	If distance == 0 (high distance)
//		50% speed
//	Else speed = 1+distance capped at 50
	
//PWM available on 3, 5, 6, 9, 10

#include <NewPing.h>
#include <Servo.h>
#include "Timer.h"

#define TRIGGER_PIN  3  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     5  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define MOTOR_PIN 9
#define MAX_SIGNAL 2000
#define MIN_SIGNAL 700
#define LEFT_LED 14
#define RIGHT_LED 16
#define ARM_BUTTON 8

#define MAX_DUTY_CYCLE 50

// Set up Motor & Sonar
Servo motor;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
boolean armed = false;
Timer t;
boolean led_rotate_status = true;
unsigned long button_push_start = 0;
boolean arm_pushed = false;

void brake(boolean left, boolean right) {
  if (left) {
    digitalWrite(LEFT_LED, HIGH);
    //Serial.println("Left LED HIGH");
  }
  else
  {
    digitalWrite(LEFT_LED, LOW);
    //Serial.println("Left LED LOW");
  }
  if (right) {
    digitalWrite(RIGHT_LED, HIGH);
    //Serial.println("Right LED HIGH");
  }
  else
  {
    digitalWrite(RIGHT_LED, LOW);
    //Serial.println("Right LED LOW");
  }
}

void setMotorSpeed(int duty_cycle) {
  //The way I'm accepting duty cycle while capping it here is dumb.  I need to make it better.
  // IE make 1-100 scale between the min signal and min signal + max duty cycle.
  //Duty cycle accepts 0=100
  if (duty_cycle > MAX_DUTY_CYCLE) duty_cycle = MAX_DUTY_CYCLE;
  if (duty_cycle < 0) duty_cycle = 0;
  //Serial.print("EDF Duty Cycle: " );
  //Serial.println(duty_cycle);
  Serial.print("Signal: " );
  Serial.println(MIN_SIGNAL + ((MAX_SIGNAL-MIN_SIGNAL)/100)*duty_cycle);
  motor.writeMicroseconds(MIN_SIGNAL + ((MAX_SIGNAL-MIN_SIGNAL)/100)*duty_cycle); 
}

void pulse(){
 for(int i = 0; i <= 70; i+=2)
 {
  setMotorSpeed(i);
  delay(5); 
 }
 for(int i = 70; i > 0; i--)
 {
  setMotorSpeed(i);
  delay(5); 
 }
}

int getDistance(){
  unsigned int uS = sonar.ping();
  Serial.print("Ping distance: ");
  Serial.println(uS / US_ROUNDTRIP_CM);
  return (uS / US_ROUNDTRIP_CM); 
}



void setup() {
  pinMode(LEFT_LED, OUTPUT);
  pinMode(RIGHT_LED, OUTPUT);
  pinMode(ARM_BUTTON, INPUT_PULLUP);
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  motor.attach(MOTOR_PIN);
  motor.writeMicroseconds(MIN_SIGNAL);
  
  //Timer to trigger brake light rotation.
  //Function is called every 250ms, but lights only rotate when they not armed.
  t.every(1000, rotate_brakes);
  
}

void intimidate()
{
 //TODO: Write in intimidation code.  Revvvvv....Rev.Rev.....
 Serial.println("Intimidate!!"); 
 pulse();
 delay(800);
 pulse();
 delay(200);
 pulse();
 delay(200);
}

void button_response(unsigned long push_length){
  //Ignore anything under 50ms, since it's likely a state transition bounce.
  //If it's armed and there's a push, disarm.
  //If it's not armed and a push is 2-4s, arm.
  //If it's pushed longer than 4s, intimidate routine.
  if (push_length < 50) {
  }
  else if (armed) {
    armed = false;
    Serial.println("DISARMED");
  }
  else if (!armed && push_length > 2000 && push_length <= 3999) {
   armed = true; 
   Serial.println("ARMED");
   }
  else if (!armed & push_length > 4000) {
     intimidate();
  }
}

void loop() {
  t.update();
  delay(50);
  //If the button is newly read as being down, record the time the button went down.
  if (digitalRead(ARM_BUTTON) == LOW && arm_pushed == false){
    //First reading of button push.
    Serial.println("First Push");
    button_push_start = millis();
    arm_pushed = true;
    }
  else if (digitalRead(ARM_BUTTON) == HIGH && arm_pushed == true){
    //First reading of button release.
    arm_pushed = false;
    button_response(millis() - button_push_start);
  }
   
   //Take action on armed status.
   if (armed) {
     int distance = getDistance();
     if (distance < 1) {
       //Distance of '0' means undeterminable (greater than 200cm) distance
       //Open road, full speed ahead!
       brake(false, false);  
       setMotorSpeed(MAX_DUTY_CYCLE);
     }
     else if (distance < 10) {
       //We're about to smash into something, bring engine back to idle.
       setMotorSpeed(0);
       brake(true, true);
     }
     else {
      //For 20-200cm, go 20-100 duty cycle.
      setMotorSpeed(1 + distance); 
     }
   }
   
   if (!armed) {
     setMotorSpeed(0);
   }
}


void rotate_brakes() {
 //Blink the brakes back and forth if armed.
 if (!armed) {
   //Disarmed.  Rotate LEDs
   Serial.println("Rotate");
   if (led_rotate_status == true) 
   {
     brake(true, false);
     led_rotate_status = false;
   }
   else 
   {
     brake(false, true);
     led_rotate_status = true;
   }
 }
}
