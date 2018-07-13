/*
  TEJ4M1-01 Computer Interfacing Final Project
  M&M Color Sorter
  Arduino Code
  by Jerry Chen
  June 15, 2018
*/

// include required libraries
#include <LiquidCrystal.h>
#include <Servo.h>

// initialize an instance of the LiquidCrystal class and the interface pins
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

// initialize an instance of the Servo class to control servos
Servo servo1;
Servo servo2;

// set pins for color sensor
#define S0 12
#define S1 11
#define sensorOut 10
#define S2 9
#define S3 8

// set servo values
#define load 101
#define scan 75
#define drop 36

// variables for color sensor frequency
int R = 0;
int G = 0;
int B = 0;

// variable to store scanned color
int color = 0;

// candy counter
int red = 0;
int blue = 0;
int green = 0;
int brown = 0;
int orange = 0;
int yellow = 0;
int other = 0;
int total = 0;

// boolean to set manual mode
bool manualMode = false;

// boolean to detect button press
bool startPressed = false;

//boolean to detect eStop press
bool eStopPressed = false;


void setup() {

  // set pin mode for color sensor
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  // set frequency scaling of color sensor to 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  // set up the LCD's number of columns and rows
  lcd.begin(16, 2);

  // assign pin 22 to servo 1
  servo1.attach(22);

  // assign pin 24 to servo 2
  servo2.attach(24);

  // rotate servo 1 to scanning position
  servo1.write(scan);

  // set pin mode for interface buttons
  pinMode (20, INPUT); // start/end button
  pinMode (28, INPUT); // manual/automatic setting switch
  pinMode (30, OUTPUT); // jam preventing motor
  pinMode (18, INPUT); // eStop detection
  pinMode (32, OUTPUT); // reset pin
  digitalWrite (32, LOW); // set pin to HIGH to prevent reset
  pinMode(34, INPUT); // next button
  pinMode (19, INPUT); //pause detection
  pinMode (36, OUTPUT); //piezo


  // interupt detection - emergency stop
  attachInterrupt(digitalPinToInterrupt(18), changeEStop, CHANGE);

  //interupt detection - pause
  attachInterrupt (digitalPinToInterrupt(19), pause, LOW);

  // start button detection
  attachInterrupt(digitalPinToInterrupt(20), changeStart, LOW);

  // begins serial communication at 9600 bud
  Serial.begin(9600);

  // send start up text to serial monitor and LCD
  Serial.println("M&M Color Sorter by Jerry Chen");
  lcdPrint(0, 0, false, "M&M Color Sorter");
  lcdPrint(0, 1, false, "by Jerry Chen");

  // 5 second delay to show text
  delay(5000);

  // clear start up text
  lcd.clear();
}

void loop() {

  Serial.println("Choose the desired mode (manual or auto) and press START when ready.");
  lcdPrint(0, 0, true, "Press START");
  lcdPrint(0, 1, false, "Mode: ");

  bool lastModeState = digitalRead(28);

  // detect if AUTO or MANUAL is selected
  if (lastModeState == HIGH) {
    manualMode = true;
    lcdPrint(6, 1, false, "MAN.");
    Serial.println("Manual mode selected.");
  }
  else {
    manualMode = false;
    lcdPrint(6, 1, false, "AUTO");
    Serial.println("Automatic mode selected.");
  }

  // allows user to select AUTO or MANUAL sorting mode
  while (true) {
    if (digitalRead(28) == !lastModeState) {
      lastModeState = !lastModeState;
      if (lastModeState == HIGH) {
        manualMode = true;
        lcdPrint(6, 1, false, "MAN.");
        Serial.println("Manual mode selected.");
      }
      else if (lastModeState == LOW) {
        manualMode = false;
        lcdPrint(6, 1, false, "AUTO");
        Serial.println("Automatic mode selected.");
      }
    }


    if (startPressed == true) {
      startPressed = false;
      break;
    }
    delay(50);
  }
  // turns on anti-jam motor
  digitalWrite (30, HIGH);

  // sort process, repeats until start/stop button is pressed
  while (true) {
    eStopPressed = false;
    startPressed = false;
    Serial.println();
    Serial.println("***********************");
    Serial.println("Sort is in progress");
    ifEStop();

    loading();
    ifEStop();

    ifManual();
    ifEStop();

    scanning();
    ifEStop();

    ifManual();
    ifEStop();

    dropper();
    ifEStop();

    ifManual();
    ifEStop();

    dropping();
    ifEStop();

    ifManual();
    ifEStop();

    if (startPressed == true) {
      startPressed = false;
      break;
    }
  }

  digitalWrite (30, LOW);

  // display after sort

  Serial.println("***********************");
  Serial.println("SORT COMPLETED");
  Serial.println();

  Serial.print("Total = ");
  Serial.println(total);
  Serial.print("Red = ");
  Serial.println(red);
  Serial.print("Blue = ");
  Serial.println(blue);
  Serial.print("Green = ");
  Serial.println(green);
  Serial.print("Brown = ");
  Serial.println(brown);
  Serial.print("Orange = ");
  Serial.println(orange);
  Serial.print("Yellow = ");
  Serial.println(Yellow);
  Serial.print("Other = ");
  Serial.println(other);



  lcdPrint(0, 0, true, "SORT COMPLETE");
  lcdPrint(0, 1, false, "Total:");
  lcdPrint(7, 1, false, String(total));

  delay (5000);

  // allows user to return to main menu after pressing start
  lcdPrint(0, 0, false, "Press NEXT      ");
  Serial.println();
  Serial.println("Press NEXT to continue");

  while (true) {
    if (digitalRead(34) == HIGH) {
      break;
    }
    delay (50);
  }


}

void readColor() {
  // read RED frequency and storing the value
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  R = pulseIn(sensorOut, LOW);

  // read GREEN frequency and storing the value
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  G = pulseIn(sensorOut, LOW);

  // read BLUE frequency and storing the value
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  B = pulseIn(sensorOut, LOW);

  // prints the RBG values to serial monitor
  printRGB();
}

// steps required to load M&Ms into the arm
void loading() {
  delay(50);
  servo1.write(load); // servo turning

  lcdPrint(0, 0, true, "SORT IN PROGRESS");
  lcdPrint(0, 1, false, "Loading...");
  Serial.println("Loading...");
  tone(36, 1200, 100);
  delay(500);


}

// steps required to scan the M7M's color
void scanning() {
  servo1.write(scan);
  Serial.println("Reading M&M color");
  lcdPrint(0, 0, true, "SORT IN PROGRESS");
  lcdPrint(0, 1, false, "Scanning...");
  delay(200);
  readColor();
  tone(36, 1900, 100);
}

// steps required to rotate servo2 to the correct bin
void dropper() {
  Serial.print("M&M Color is: ");
  lcdPrint(0, 0, true, "SORT IN PROGRESS");
  lcdPrint(0, 1, false, "RESULT:");
  decideColor();
  delay(200);
}

// steps required to drop M&M in the correct slot
void dropping() {
  Serial.println("Dropping M&M");
  lcdPrint(0, 0, true, "SORT IN PROGRESS");
  lcdPrint(0, 1, false, "Dropping in bin");
  servo1.write(drop);
  delay(500);

}

// to print something on the LCD
void lcdPrint(int col, int row, bool lcdClear, String text) {
  if (lcdClear) {
    lcd.clear();
  }
  lcd.setCursor(col, row);
  lcd.print(text);
}

//decides which color the M&M is, rotate servo2 and display info for user
void decideColor() {
  if (R<371 & R>359 & G<412 & G>397 & B<300 & B>255 ) {
    color = 1; // Red
    lcdPrint(8, 1, false, "Red");
    Serial.println("Red");
    red++;
    servo2.write(80);
  }
  else if (R<230 & R>115 & G<617 & G>515 & B<100 & B>55) {
    color = 3; // Green
    lcdPrint(8, 1, false, "Green");
    Serial.println("Green");
    green++;
    servo2.write(95);

  }
  else if (R<400 & R>456 & G<417 & G>487 & B<590 & B>542) {
    color = 5; // Brown
    lcdPrint(8, 1, false, "Brown");
    Serial.println("Brown");
    brown++;
    servo2.write(110);

  }
  else if (R<212 & R>193 & G<100 & G>78 & B<400 & B>370) {
    color = 6; // Blue
    lcdPrint(8, 1, false, "Blue");
    Serial.println("Blue");
    blue++;
    servo2.write(125);

  }
  else if (R<314 & R>233 & G<408 & G>388 & B<370 & B>200) {
    color = 7; // Orange
    lcdPrint(8, 1, false, "Orange");
    Serial.println("Orange");
    orange++;
    servo2.write(140);

  }
  else if (R<314 & R>233 & G<408 & G>388 & B<370 & B>200) {
    color = 7; // Yellow
    lcdPrint(8, 1, false, "Yellow");
    Serial.println("Yellow");
    orange++;
    servo2.write(140);

  }
  else {
    color = 8;
    lcdPrint(8, 1, false, "Other");
    Serial.println("Other");
    other++;
    servo2.write(155);

  }

  total++;

}

void printRGB() {
  //prints RGB values detected
  Serial.println("Scanned RGB values: ");
  Serial.print("R= ");
  Serial.print(R);
  Serial.print(" G= ");
  Serial.print(G);
  Serial.print(" B= ");
  Serial.println(B);
}


// activates emergency stop
void eStop() {
  Serial.println("EMERGENCY STOP PRESSED");
  Serial.println("Discarding all remaining M&Ms");
  lcdPrint(0, 0, true, "*EMERGENCY STOP*");
  lcdPrint(0, 1, false, " Discarding M&Ms...");
  servo1.write(drop);
  // dump all remaining M&Ms
  for (int i = 0; i < 50; i++) {
    delay(500);

    servo1.write(load);
    delay(500);
    servo1.write(drop);

    if (digitalRead(34) == HIGH) {
      break;
    }

  }
  Serial.println("All M&Ms have been discarded. Press NEXT to reset.");
  lcdPrint(0, 0, true, "M&Ms discarded");
  lcdPrint(0, 1, false, "Press NEXT");


  while (true) {
    if (digitalRead(34) == HIGH) {
      break;
    }
    delay (50);
  }

  digitalWrite(32, HIGH);
}

// pauses the execution of code until button is not LOW
void pause() {
  Serial.println("PAUSE BUTTON PRESSED");
  Serial.println("Press PAUSE again to continue");
  lcdPrint(0, 0, true, "SORTING PAUSED");
  lcdPrint(0, 1, false, "Press PAUSE");
  while (true) {
    if (digitalRead(19) == HIGH) {
      break;
    }
  }
  Serial.println("PAUSE BUTTON PRESSED");
  Serial.println("Sorting resumed");

  lcdPrint(0, 0, true, "SORTING UNPAUSED");

  lcd.clear();
}

// wait for the NEXT button to be pressed before continuing
void waitNext() {
  Serial.println("MANUAL MODE: Press next to continue.");
  lcdPrint(0, 0, true, "MAN. MODE: Press");
  lcdPrint(0, 1, false, "NEXT to continue");

  while (true) {
    if (digitalRead(34) == HIGH) {
      break;
    }
    delay (50);
  }
  lcd.clear();
}

// detect if manual mode is activated
void ifManual() {
  if (manualMode == true) {
    waitNext();
  }
}

// detect if start is pressed
void changeStart() {
  startPressed = true;
  delay(25);
}

// change value of eStop variable
void changeEStop() {
  eStopPressed = true;
  delay(25);

}

//detect if stop is pressed
void ifEStop() {
  if (eStopPressed == true) {
    eStop();
  }
}

