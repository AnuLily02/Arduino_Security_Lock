#include <Keypad.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

const String SECRET_PASSWORD = "1234";  
const byte MAX_ATTEMPTS = 3;
const int SERVO_LOCKED_POS = 0;
const int SERVO_UNLOCKED_POS = 90;

const byte SERVO_PIN = 9; // Moved to D11 to avoid keypad conflict
const byte BUZZER_PIN = 10;

// --- 4x4 Keypad Setup ---
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {2, 3, 4, 5}; // Connect to rows 1-4
byte colPins[COLS] = {6, 7, 8, 11}; // Connect to cols 1-4

Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x20, 16, 2); // Fixed Tinkercad I2C address to 0x20
Servo lockServo;

// --- State Variables ---
String inputPassword = "";
byte failedAttempts = 0;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  
  lockServo.attach(SERVO_PIN);
  lockServo.write(SERVO_LOCKED_POS); // Lock on startup
  
  lcd.init();
  lcd.backlight();
  resetState("Enter PIN:");
}

void loop() {
  char key = customKeypad.getKey();

  if (key) {
    if (key == '#') { 
      // '#' acts as the Submit / Enter key
      checkPassword();
    } else if (key == '*') { 
      // '*' acts as Clear / Reset
      resetState("PIN Cleared");
      delay(1000);
      resetState("Enter PIN:");
    } else {
      // Append entered digit
      if (inputPassword.length() < 8) { // Max length buffer limit
        inputPassword += key;
        lcd.setCursor(0, 1);
        
        // Mask password entry with asterisks
        String mask = "";
        for (unsigned int i = 0; i < inputPassword.length(); i++) {
          mask += "*";
        }
        lcd.print(mask);
        
        // Single quick feedback beep on keypress
        tone(BUZZER_PIN, 1000, 50); 
      }
    }
  }
}

void checkPassword() {
  lcd.clear();
  
  if (inputPassword == SECRET_PASSWORD) {
    // --- CORRECT PIN ---
    lcd.print("Access Granted!");
    failedAttempts = 0;
    
    // Unlock feedback sound
    tone(BUZZER_PIN, 1500, 100);
    delay(120);
    tone(BUZZER_PIN, 2000, 200);
    
    // Drive actuator
    lockServo.write(SERVO_UNLOCKED_POS);
    delay(5000); // Keep unlocked for 5 seconds
    
    // Auto-relock
    lockServo.write(SERVO_LOCKED_POS);
    resetState("Door Locked");
    delay(1500);
    resetState("Enter PIN:");
  } else {
    // --- INCORRECT PIN ---
    failedAttempts++;
    lcd.print("Access Denied!");
    
    // Error sound
    tone(BUZZER_PIN, 400, 400);
    delay(1500);

    if (failedAttempts >= MAX_ATTEMPTS) {
      triggerSecurityAlarm();
    } else {
      resetState("Try Again");
      delay(1000);
      resetState("Enter PIN:");
    }
  }
}

void triggerSecurityAlarm() {
  lcd.clear();
  lcd.print("SYSTEM LOCKEDOUT!");
  
  // Sound alarm pulse loop
  for (int i = 0; i < 10; i++) {
    lcd.setCursor(0, 1);
    lcd.print("Wait " + String(10 - i) + "s...");
    tone(BUZZER_PIN, 800);
    delay(500);
    tone(BUZZER_PIN, 1200);
    delay(500);
  }
  
  noTone(BUZZER_PIN);
  failedAttempts = 0;
  resetState("Enter PIN:");
}

void resetState(String prompt) {
  inputPassword = "";
  lcd.clear();
  lcd.print(prompt);
  lcd.setCursor(0, 1);
}