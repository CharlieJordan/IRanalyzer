/*
 *  File....... IRanalyzer.pde 
 *  Purpose.... Records up to 128 signal changes
 *  Author..... Walter Anderson 
 *  E-mail..... wandrson@walteranderson.us 
 *  Started.... 18 May 2007 
 *  Updated.... 18 May 2007 
 * 
 *
 */ 
#include <avr/interrupt.h>
#include <avr/io.h>

#include <myIR.h>


#define TIMER_RESET  TCNT1 = 0
#define SAMPLE_SIZE  64

//int IRrxPin = 8;
unsigned int TimerValue[SAMPLE_SIZE];
char direction[SAMPLE_SIZE];
byte change_count;
long time;
long last;
long elapsed;

void setup() {
  Serial.begin(9600);
  Serial.println("Analyze IR Remote");
  TCCR1A = 0x00;          // COM1A1=0, COM1A0=0 => Disconnect Pin OC1 from Timer/Counter 1 -- PWM11=0,PWM10=0 => PWM Operation disabled
  // ICNC1=0 => Capture Noise Canceler disabled -- ICES1=0 => Input Capture Edge Select (not used) -- CTC1=0 => Clear Timer/Counter 1 on Compare/Match
  // CS12=0 CS11=1 CS10=1 => Set prescaler to clock/64
  TCCR1B = 0x03;          // 16MHz clock with prescaler means TCNT1 increments every 4uS
  // ICIE1=0 => Timer/Counter 1, Input Capture Interrupt Enable -- OCIE1A=0 => Output Compare A Match Interrupt Enable -- OCIE1B=0 => Output Compare B Match Interrupt Enable
  // TOIE1=0 => Timer 1 Overflow Interrupt Enable
  TIMSK1 = 0x00;          
  pinMode(IRrxPIN, INPUT);
}

void loop()
{
  Serial.println("Waiting...");
  change_count = 0;
  while(digitalRead(IRrxPIN) == HIGH) {}                                 
  TIMER_RESET;
  TimerValue[change_count] = TCNT1;
  direction[change_count++] = '0';
  while (change_count < SAMPLE_SIZE) {
    if (direction[change_count-1] == '0') {
      while(digitalRead(IRrxPIN) == LOW) {}
      TimerValue[change_count] = TCNT1;
      direction[change_count++] = '1';
    } else {
      while(digitalRead(IRrxPIN) == HIGH) {}
      TimerValue[change_count] = TCNT1;
      direction[change_count++] = '0';
    }
  }
  Serial.println("Bit stream detected!");
  Serial.println(" cummulative\telapsed\tbit-value");
  change_count = 0;
  last = 0;
  time = (long) TimerValue[change_count] * 4;
  while (change_count < SAMPLE_SIZE-1) {
    time = (long) TimerValue[++change_count] * 4;
    elapsed = time - last;
    Serial.print("\t");
    Serial.print(time);
    Serial.print("\t");
    Serial.print(elapsed);
    Serial.print("\t");
    if( elapsed < 400 ) Serial.print("error - interval less than 400 microseconds!");
    else if( elapsed >= 400 && elapsed <= 800 ) Serial.print("0");
    else if( elapsed >= 1000 && elapsed <= 1400 ) Serial.print("1");
    else if( elapsed > 1600 && elapsed < 3000) Serial.print("Probably a start bit.");
    Serial.println();
    last = time;
  }
  Serial.println("Bit stream end!");
  delay(2000);
}
