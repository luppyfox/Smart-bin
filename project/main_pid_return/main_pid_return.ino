#include <Servo.h>
#include <HX711_ADC.h>
#include <EEPROM.h>

#define ENCA 18
#define ENCB 3
#define PWM 5
#define IN2 6
#define IN1 7

#define LIM 8    //setzero
#define LIME 22  //หน้าเครื่องขวา
#define LIMS 23  //หน้าเครื่อง ซ้าย
//ulta หน้าเครื่อง
#define Trig_PIN 12  // Pin connect to Trig pin
#define Echo_PIN 13  // Pin connect to Echo pin
//ultra 1 2
//y24 25 || y26 27
#define Trig_PIN_H1 25  //0-20
#define Echo_PIN_H1 24

#define Trig_PIN_H2 27  //ไม่เต็ม 69  เต็ใม 30 - 13 cm
#define Echo_PIN_H2 26

// loade cell sec
#define HX711_dout 44;
#define HX711_sck 45;
const int calVal_eepromAdress = 0;
unsigned long t = 0;

//#define BTrig_PIN 22
//#define BEcho_PIN 24

#define DIRA1 4
#define DIRA2 11

int checkbm[10] = {};
int before = 100;

// PI Constant
float kp = 0;
float ki = 0;
// long prevT = 0;
// float eprev = 0;
// float eintegral = 0;
int max_pwm;
int min_pwm;
int tolerance;

volatile int pos = 0;
int set_pwm = 0;
int num_bin = 4;          // number of bin
float ppr_dist = 62.832;  //62.832 แก้เป็นระยะที่วัดได้จากโลกจริง เป็นหน่วย มม. !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
float ppr_tig = 290;      //667 290.909 331ค่าที่ปริ้นท์จากโค้ด test_enc !!!!!!!!!!!!!!!!!!!!!!!!!!!
float bin_dist = 296;     //(295)distance between bin to bin, unit = mm

int setz = 0;
int state = 1;
//door open motro
int gate_close = 0;
int gate_start = 0;
int gate_time = 0;
int gate_check = 0;
int e = 0;

//python input
String Pyinput;
int integerValue = 0;

Servo servo1;
// Servo servo2;

int i = 0;

void setup() {
  Serial.begin(9600);



  pinMode(LIM, INPUT);
  pinMode(LIMS, INPUT);
  pinMode(LIME, INPUT);

  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);

  pinMode(Trig_PIN, OUTPUT);
  pinMode(Echo_PIN, INPUT);
  //Trig_PIN_H1
  pinMode(Trig_PIN_H1, OUTPUT);
  pinMode(Echo_PIN_H1, INPUT);

  pinMode(Trig_PIN_H2, OUTPUT);
  pinMode(Echo_PIN_H2, INPUT);

  servo1.attach(10);
  // servo2.attach(9);

  servo1.write(25);
  // servo2.write(110);
  delay(3);

  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);
  //test motor
  //while (true) { setMotor(-1, 100, PWM, IN1, IN2); }
  //test servo
  // while (true){servo1.write(25); servo2.write(110); Serial.println("f");}
  //limit
  //while (true){int limit_DoorS = digitalRead(LIMS); Serial.println(limit_DoorS);}

  //Serial.println("target pos");
}

void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0;

  long cm, cm1, cm2;

  //cm = ultrasonic(Trig_PIN , Echo_PIN);
  //cm1 = ultrasonic(Trig_PIN_H1 , Echo_PIN_H1);

  //Serial.println(cm2);

  //limit switch
  int limit_check = digitalRead(LIM);
  int limit_DoorS = digitalRead(LIMS);
  int limit_DoorE = digitalRead(LIME);
  //Serial.println(limit_DoorS);
  //Serial.println(limit_DoorS);
  //Serial.println(limit_check);

  if (setz == 0) {
    setMotor(-1, 80, PWM, IN1, IN2);
    //Serial.println("setz");
    if (limit_check == 1) {
      setMotor(0, 0, PWM, IN1, IN2);
      pos = 0;
      gate_close = 0;
      gate_start = 0;
      state = 1;
      setz = 1;
      e = 0;
    }
    //รับส่งค่า
  } else if (setz == 1) {
    //ขั้นกลางการทำงาน
    //Serial.println("STATE INPUT");

    cm = ultrasonic(Trig_PIN, Echo_PIN);
    cm1 = ultrasonic(Trig_PIN_H1, Echo_PIN_H1);

    // Serial.println(cm1);

    //|| (cm1 < 15 && cm1 > 0)) {
    if ((cm < 20 && cm > 0)) {
      gate_time = 0;
      gate_check = 1;
    }

    if (gate_check == 1) {
      digitalWrite(DIRA1, HIGH);
      digitalWrite(DIRA2, LOW);
      if (limit_DoorS == 1) {
        digitalWrite(DIRA1, LOW);
        digitalWrite(DIRA2, LOW);

        gate_check = 0;
        gate_start = 1;
      }

    } else if (limit_DoorE != 1) {
      digitalWrite(DIRA1, LOW);
      digitalWrite(DIRA2, HIGH);
    } else {
      //Serial.println("STOP");
      digitalWrite(DIRA1, LOW);
      digitalWrite(DIRA2, LOW);
    }


    //เริ่มการทำงาน จับเวลาไม่เจอมือคน ถึงจะเริ่มทำงาน
    if (gate_start == 1 && (limit_DoorS == 0 && limit_DoorE == 1)) {
      gate_time++;
      Serial.println(gate_time);
      if (gate_time == 10) {
        gate_time = 0;   //res time
        gate_close = 1;  //start find object
        gate_check = 0;  //res delay door

        Serial.println("F");  //pyinput
      }
    } else {
      gate_time = 0;
    }


    //ทำการรอ python
    if (gate_close == 1) {
      Pyinput = Serial.readString();
      integerValue = 3;  //Pyinput.toInt(); // Input data from AI by python serial !!!!!!!!!!!!!!!!!!!!!!!!!!!!

      if (integerValue == 1) {
        // Serial.println(integerValue);
        Serial.println("R");

        state = 1;
        setz = 2;
        max_pwm = 100;
        min_pwm = 70;
        kp = 1;
        tolerance = -2;
        //  ki = 0;

      } else if (integerValue == 2) {
        // Serial.println(integerValue);
        Serial.println("R");
        state = 2;
        setz = 2;
        max_pwm = 170;
        min_pwm = 60;
        kp = 0.17;
        tolerance = 0;
        //  ki = 0.006;

      } else if (integerValue == 3) {
        // Serial.println(integerValue);
        Serial.println("R");
        state = 3;
        setz = 2;
        max_pwm = 255;
        min_pwm = 100;
        kp = 0.15;
        tolerance = 0;
        //  ki = 0.006;

      } else if (integerValue == 4) {
        // Serial.println(integerValue);
        Serial.println("R");

        state = 4;
        setz = 2;
        max_pwm = 255;
        min_pwm = 130;
        kp = 0.15;
        tolerance = 0;
        //  ki = 0.006;

      } else if (Pyinput == "") {
        setz = 1;
      }
    }

    //ทำงานttt
  } else if (setz == 2) {
    // Serial.println("GO!!!!!!!!!!!");
    while (setz == 2) {
      // error
      e = pos - pprToGoal(state);

      // ปัญหาไม่ยอมวิ้งกลับ//

      // motor direction
      int dir = 1;
      if (e > 0) {
        dir = -1;
      }

      // // PI Controller
      // // time difference
      // long currT = micros();
      // float deltaT = ((float)(currT - prevT)) / (1.0e6);
      // prevT = currT;

      // // integral
      // eintegral = eintegral + abs(e) * deltaT;

      // // PI Control pwm
      // set_pwm = abs(e) * kp + eintegral * ki;

      // P control pwm
      set_pwm = abs(e) * kp;

      if (set_pwm >= max_pwm) {
        set_pwm = max_pwm;
      } else if (set_pwm <= min_pwm) {
        set_pwm = min_pwm;
      }
      if (e >= tolerance) {
        set_pwm = 0;
        e = 0;
      }

      setMotor(dir, set_pwm, PWM, IN1, IN2);

      // // store previous error
      // eprev = e;

      // For serial print
      Serial.print("Error is ");
      Serial.print(e);
      Serial.print(" Goal is ");
      Serial.print(pprToGoal(state));
      Serial.print(" Position is ");
      Serial.print(pos);
      Serial.print(" Speed value is ");
      Serial.print(set_pwm);
      Serial.println();

      // // For serial plot
      // Serial.print("e:");
      // Serial.print(e);
      // Serial.print(",");
      // Serial.print("pos:");
      // Serial.print(pos);
      // Serial.print(",");
      // Serial.print("set_pwm:");
      // Serial.println(set_pwm);

      if (e == 0 && state >= 1 && state <= 4) {
        Serial.print("State test value is ");
        Serial.println(state);

        setMotor(0, 0, PWM, IN1, IN2);
        delay(1000);
        for (i = 20; i <= 110; i++) {
          servo1.write(i);  // 20 --> 110

          delay(1);
        }

        delay(5000);

        // for (i = 20; i <= 110; i++) {
        //   servo2.write(i);        // 20 --> 110
        //   servo1.write(110 - i);  // 110 --> 20
        //   delay(1);
        // }

        for (i = 115; i >= 20; i--) {
          servo1.write(i);  // 20 --> 110

          delay(1);
        }

        servo1.write(25);
        // servo2.write(110);

        delay(1000);
        ///////////////////////////////////////////////////// หาพื้นที่ในกล่อง

        // for (int i = 0; i < 9; i++) {
        //   checkbm[i] = ultrasonic(Trig_PIN_H2, Echo_PIN_H2);
        // }

        Serial.println("-----------------------------------------");

        while (state >= 2) {

          // error
          int e = pos - pprToGoal(1.5);  // This mean the box will be stop in half of Box No.2

          // motor direction
          int dir = 1;
          if (e > 0) {
            dir = -1;
          }
          set_pwm = abs(e) * 0.07;  //kp


          if (set_pwm >= 255) {  // max_pwm
            set_pwm = 255;
          } else if (set_pwm <= 80) {  //min_pwm
            set_pwm = 80;
          }

          if (e <= 10) {  //tolerance
            set_pwm = 0;
            e = 0;
            break;
          }
          // For serial print
          Serial.print("Error is ");
          Serial.print(e);
          Serial.print(" Goal is ");
          Serial.print(pprToGoal(1.5));
          Serial.print(" Position is ");
          Serial.print(pos);
          Serial.print(" Speed value is ");
          Serial.print(set_pwm);
          Serial.println();

          setMotor(dir, set_pwm, PWM, IN1, IN2);
        }
        Serial.println("E");

        //หาที่น้อยที่สุดจากการหาถัง
        for (int i = 0; i < 9; i++) {

          if (checkbm[i] < before) {
            before = checkbm[i];
          }
        }

        //ส่งค่าเก็บถัง
        Serial.print(before);

        setz = 0;
      }
    }
  }
}

void setMotor(int dir, int pwmVal, int pwm, int in1, int in2) {
  analogWrite(pwm, pwmVal);
  if (dir == 1) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (dir == -1) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

int pprToGoal(float state_num) {
  float target = 0;
  if (state_num == 2) {
    target = (ppr_tig / ppr_dist) * (((bin_dist) * (state_num - 1)) + 25);
  } else if (state_num == 3) {
    target = (ppr_tig / ppr_dist) * (((bin_dist) * (state_num - 1)) - 0);
  }  //22
  else if (state_num == 4) {
    target = (ppr_tig / ppr_dist) * (((bin_dist) * (state_num - 1)) + 40);
  }
  return (target);
}

void readEncoder() {
  int b = digitalRead(ENCB);
  if (b > 0) {
    pos++;
  } else {
    pos--;
  }
}

long ultrasonic(int pinTig, int pinEcho) {
  pinMode(pinTig, OUTPUT);

  digitalWrite(pinTig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTig, LOW);
  pinMode(pinEcho, INPUT);
  long duration = pulseIn(pinEcho, HIGH);
  long cm = microsecondsToCentimeters(duration);

  return cm;
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}
