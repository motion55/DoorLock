#include <SoftwareSerial.h> // for GSM and Bluetooth Serial communication Library
#include <LiquidCrystal.h> // includes the LiquidCrystal Library 
#include <Keypad.h> // includes the Keypad Library 
#include "libraries/SIM900/SIM900.h" //calling the sim900.h library script so we can use the function there
#include "libraries/SIM900/sms.h" // SMS library

SMSGSM sms;// this is naming the gsm for sms function

#define buzzer 37
#define relay 36
#define limitSwitch 2
#define pushButton1 3
#define pushButton2 4

// For GSM setup
boolean started = false; // set to true if GSM communication is okay
char smsbuffer[160]; // storage for SMS message
char number[20]; //who texted (number of who texted) now if someone text our device the message will be stored on smsbuffer variable and the number
//(of the one who txted the device) will be stored on this variable
char authNumber[20] = "+639771728248"; // mobile number to send SMS to (authenticated number) where the device will notify for intruder alert
byte Position; //address for sim card messages
byte led = 13; // LED is on if busy initializing and processing SMS
//--------------------------
int BMC;
int y = 0;
int x = 0;
int t = 0;
int passAttempt = 0; // Password attempt
int lastState = 0;
int screenOffMsg1 = 0;
int screenOffMsg2 = 0;
int screenOffMsg3 = 0;
String password = "1234";
String tempPassword;
boolean systemLock = false;
boolean doorOpen = false;
boolean activated = false; // State of the alarm
boolean isActivated;
boolean enterMode = false;
boolean enterPass = false;
boolean enteredPassword; // State of the entered password to stop the alarm
boolean passChangeMode = false;
boolean passChanged = false;
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keypressed;
//define the cymbols on the buttons of the keypads
char keyMap[ROWS][COLS] = {
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {28, 29, 30, 31}; //Row pinouts of the keypad
byte colPins[COLS] = {32, 33, 34, 35}; //Column pinouts of the keypad
Keypad myKeypad = Keypad( makeKeymap(keyMap), rowPins, colPins, ROWS, COLS);
LiquidCrystal lcd(22, 23, 24, 25, 26, 27); // Creates an LC object. Parameters: (rs, enable, d4, d5, d6, d7)

//SoftwareSerial bluetooth(10, 11);
#define bluetooth Serial1

void setup() {
  bluetooth.begin(9600);
  lcd.begin(16, 2);
  pinMode(buzzer, OUTPUT); // Set buzzer as an output
  pinMode(relay, OUTPUT); // Sets the relay as an Output
  pinMode(limitSwitch, INPUT_PULLUP); // Sets the LimitSwitch as Input to automatically close the door
  pinMode(pushButton1, INPUT_PULLUP);
  pinMode(pushButton2, INPUT_PULLUP);
  // for GSM
  Serial.begin(9600); // Begin Serial Comms
  pinMode(led, OUTPUT); // set status LED pin
  digitalWrite(led, HIGH); // turn on LED GSM initialization
  GSMsetup(); // initialize GSM and connect to network..
  digitalWrite(led, LOW); // turn off LED after initialization
  Serial.println(F("System ready"));
  //after the initialization of GSM. we will write on the serial monitor system ready
  sendMessage(authNumber, "System is Ready");
  // send SMS that system is ready to accept commands now authNumber is declared above
}

void loop () {
int LS = digitalRead(limitSwitch);
int LPB = digitalRead(pushButton1);
int UPB = digitalRead(pushButton2);
receiveMessage();
if (passAttempt == 1) {
  sendMessage(authNumber, "Attempt 5 password, Please Lock the System"); // we notify via sms the authNumber
  passAttempt = 0;
}
if (LPB == 0) {
  systemLock = true;
}
if (UPB == 0) {
  systemLock = false;
  screenOffMsg1 = 0;
  if (screenOffMsg2 == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("A - Enter Pass");
    lcd.setCursor(0, 1);
    lcd.print("B - Change Pass");
    screenOffMsg2 = 1;
    digitalWrite(relay, LOW);
    tone(buzzer, 2000, 500);
  }
}
if (systemLock) {
  if (screenOffMsg1 == 0) {
    tone(buzzer, 2000, 500);
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("The System is");
    lcd.setCursor(5, 1);
    lcd.print("LOCK");
    screenOffMsg1 = 1;
    screenOffMsg2 = 0;
    digitalWrite(relay, LOW);
  }
}
if (!systemLock) {
if (!enterMode) {
bluetoothDevice();
if (screenOffMsg3 == 0 ) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("A - Enter Pass");
  lcd.setCursor(0, 1);
  lcd.print("B - Change Pass");
  screenOffMsg3 = 1;
}
if (LS == 1 ) {
  if (lastState == 0) {
    y++;
    lastState = 1;
  }
}
else {
  lastState = 0;
}
if (y == 2) {
  delay(100);
  doorOpen = false;
  delay(100);
  y = 1;
}
if (doorOpen == true) {
  digitalWrite(relay, HIGH);
}
if (!doorOpen) {
  digitalWrite(relay, LOW);
}
if (enterPass == true) {
  lcd.clear();
  enterPassword();
}
keypressed = myKeypad.getKey();
if (keypressed == 'A') {
  tone(buzzer, 1000, 200);
  enterPass = true;
}
else if (keypressed == 'B') {
lcd.clear();
int i = 1;
tone(buzzer, 2000, 100);
tempPassword = "";
lcd.setCursor(0, 0);
lcd.print("Current Password");
lcd.setCursor(0, 1);
lcd.print(">");
passChangeMode = true;
passChanged = true;
while (passChanged) {
keypressed = myKeypad.getKey();
if (keypressed != NO_KEY) {
  if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
  keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
  keypressed == '8' || keypressed == '9' ) {
    tempPassword += keypressed;
    lcd.setCursor(i, 1);
    lcd.print("*");
    i++;
    tone(buzzer, 2000, 100);
  }
}
if (i > 12 || keypressed == '#') {
tempPassword = "";
i = 1;
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Current Password");
lcd.setCursor(0, 1);
lcd.print(">");
}
if ( keypressed == '*') {
i = 1;
tone(buzzer, 2000, 100);
if (password == tempPassword) {
tempPassword = "";
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Set New Password");
lcd.setCursor(0, 1);
lcd.print(">");
while (passChangeMode) {
keypressed = myKeypad.getKey();
if (keypressed != NO_KEY) {
if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
keypressed == '8' || keypressed == '9' ) {
tempPassword += keypressed;
lcd.setCursor(i, 1);
lcd.print("*");
i++;
tone(buzzer, 2000, 100);
}
}
if (i > 12 || keypressed == '#') {
tempPassword = "";
i = 1;
tone(buzzer, 2000, 100);
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Set New Password");
lcd.setCursor(0, 1);
lcd.print(">");
}
if ( keypressed == '*') {
i = 1;
tone(buzzer, 2000, 100);
password = tempPassword;
passChangeMode = false;
passChanged = false;
screenOffMsg3 = 0;
}
}
}
}
}
}
}
}
}
void enterPassword() {
int k = 5;
tempPassword = "";
activated = true;
lcd.clear();
lcd.setCursor(0, 0);
lcd.print(" *** ENTER *** ");
lcd.setCursor(0, 1);
lcd.print("Pass>");
while (activated) {
keypressed = myKeypad.getKey();
if (keypressed != NO_KEY) {
if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
keypressed == '8' || keypressed == '9' ) {
tempPassword += keypressed;
lcd.setCursor(k, 1);
lcd.print("*");
k++;
tone(buzzer, 2000, 100);
}
}
if (k > 15 || keypressed == '#') {
tempPassword = "";
k = 5;
lcd.clear();
lcd.setCursor(0, 0);
lcd.print(" *** ENTER *** ");
lcd.setCursor(0, 1);
lcd.print("Pass>");
}
if ( keypressed == '*') {
if ( tempPassword == password ) {
enterMode = false;
activated = false;
enterPass = false;
screenOffMsg3 = 0;
noTone(buzzer);
doorOpen = true;
}
else if (tempPassword != password) {
passAttempt++;
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Wrong! Try Again");
tone(buzzer, 2000, 1000);
delay(1000);
noTone(buzzer);
lcd.setCursor(0, 1);
lcd.print("Please Press #");
}
}
}
}
void bluetoothDevice() {
  if (bluetooth.available() > 0 ) {
    BMC = bluetooth.read();
  }
  if (BMC == 49) {
    if (x == 0) {
      tone(buzzer, 2000, 100);
      doorOpen = true;
      x = 1;
  }
  }
  else
  {
    x = 0;
  }
  if (BMC == 50) {
    if (t == 0) {
      tone(buzzer, 2000, 100);
      doorOpen = false ;
      t = 1;
    }
  }
  else
  {
    t = 0;
  }
}

const int RX_pin = 2;
const int TX_pin = 3;
const int GSM_ON_pin = A5;

void GSMsetup() // this is the defining of the function named GSMsetup that we used above.. if you call GSMsetup you are actually doing all these
{
  Serial.println(F("Initializing GSM"));
  
  gsm.SelectHardwareSerial(&Serial1, GSM_ON_pin);
  if (gsm.begin(9600)) // communicate with GSM module at 2400
  {
    Serial.println(F("GSM is ready"));
    started = true; // established communication with GSM module
  }
  else Serial.println(F("GSM is not ready"));
}

void sendMessage (char* phone, char* message) //another user defined function
{
  if (started) // check if we can communicate with the GSM module
  {
    digitalWrite(led, HIGH); // turn on status LED before sending SMS
    if (sms.SendSMS(phone, message))
    {
      Serial.print(F("\nSMS sent to "));
      Serial.println(phone);
    }
    digitalWrite(led, LOW); // turn off LED after sending SMS
  }
}

void receiveMessage()
//first we need to receive the message then process it..
{
  if (started) // check if we can communicate with the GSM module
  {
    Position = sms.IsSMSPresent(SMS_UNREAD); //check location of unread sms
    if (Position != 0) // check if there is an unread SMS message
    {
      sms.GetSMS(Position, number, 20, smsbuffer, 160); // get number and SMS message
      Serial.print(F("Received SMS from "));
      Serial.println(number); // sender number
      Serial.println(smsbuffer); // sender message
      sms.DeleteSMS(Position); // delete read SMS message after getting the message and number we need to delete the sms since we dont need it anymore so that the sim wont get full
      Serial.println(F("Sending to SMS processor")); // sender message
      processSMScommand(); // check if valid SMS command and process --> this is yet another user defined function. we run this to check if the sms received is valid or has
      //a programmed outcome as defined from the following user defined function...
    }
  }
}

void processSMScommand() //this is the user defined function where we can command the arduino via sms check how::
{
  Serial.print(F("Processing SMS from "));
  Serial.println(number); // send number
  Serial.println(smsbuffer); // sender message
  digitalWrite(led, HIGH); // turn on LED before processing SMS message
  // balance inquiry for smart
  if (strcmp(strlwr(smsbuffer), "checkbal") == 0) // checking the balance
  {
    sendMessage("8888", "ASTIGTXT30 BAL");
  }
  if (strcmp(strlwr(smsbuffer), "systemLock") == 0) //--> if the message received is sysytemLock we do the following
  {
    sendMessage(authNumber, "The System will be lock"); // just simple lock the system
    systemLock = true;
  }
  memset(smsbuffer, 0, sizeof(smsbuffer)); // clear SMS buffer
  digitalWrite(led, LOW); // turn off LED // we turn the led low to indicate that the processing is done already....
}

