// Include necessary libraries
#include <SoftwareSerial.h>    // Library for Software Serial Communication
#include <mbed_mktime.h>       // Library for mktime function
#include <math.h>              // Library for math operations
#include <Adafruit_Thermal.h>  // Library for Adafruit Thermal Printer


// Create an instance of the Adafruit_Thermal class for the thermal printer
Adafruit_Thermal printer(&Serial4);

// Define the states for the state machine
enum State {
  STATE_IDLE,
  STATE_COLLECT_DATA,
  STATE_CALCULATE_DATA,
  STATE_PRINT,
  STATE_SAVE_DATA
};

// Define the states for the display state machine
enum DisplayState {
  DISPLAY_IDLE,
  DISPLAY_COLLECTING_DATA,
  DISPLAY_CALCULATING_DATA,
  DISPLAY_PRINTING,
  DISPLAY_SAVING_DATA
};

// Boolean flags to indicate new card and weight data
bool isNewCardData = false;
bool isNewWeightData = false;

// Initialize the current state for the state machine
State currentState = STATE_IDLE;
DisplayState currentDisplayState = DISPLAY_IDLE;

// Variables for correctly processed data
unsigned long cardNumber = 0;   // Valid card number obtained from the card reader
float weight = 0.0;             // Mass as a floating-point number calculated from weight data
float transactionAmount = 0.0;  // Floating-point number calculated from mass and price, rounded accordingly
byte clubMembershipNumber;      // Club membership number
byte priceType;                 // Price type
char userName[20];              // User's name from the database
bool isAuthorized = false;      // Variable to determine if someone has an active account
float accountBalance = 0.0;     // User's account balance, updated with each transaction

// Temporary variables for data processing, consider moving these as local variables if possible
char cardData[8] = "1234567";
char weightString[7];
char priceString[7];
char amountString[7];
char transactionType[11];
byte blockIndex;

const byte numCharsWeight = 32;
char weightMessage[numCharsWeight];
const byte numCharsCard = 13;
char cardMessage[numCharsCard];
byte numReceived = 0;

const byte biscuitPrice = 10;
const byte icingPrice1 = 22;
const byte icingPrice2 = 20;
const byte sharpPrice = 25;

void setup() {
  // Initialize software serial ports
  Serial3.begin(9600);   // Weight port
  Serial4.begin(19200);  // Printer port
  Serial2.begin(9600);   // Card reader port

  // Initialize and set up the display state
  initializeDisplay();

  // Begin communication with the printer
  printer.begin();
}

void loop() {
  // Store the current time
  unsigned long currentMillis = millis();
  randomSeed(currentMillis);

  // Execute the current state in the state machine
  executeState(currentState);
}

// Input/Output functions

void receiveCardData() {
  static bool isReceiving = false;
  static byte dataIndex = 0;
  char receivedChar;

  while (Serial2.available() > 0 && !isNewCardData) {
    receivedChar = Serial2.read();

    if (isReceiving) {
      cardMessage[dataIndex] = receivedChar;
      dataIndex++;
      if (dataIndex >= numCharsCard) {
        dataIndex = numCharsCard - 1;
        cardMessage[dataIndex] = '\0';  // Terminate the string
        isReceiving = false;
        dataIndex = 0;
        isNewCardData = true;
      }
    } else {
      isReceiving = true;
    }
  }
}

void receiveWeightData() {
  static bool isReceiving = false;
  static byte dataIndex = 0;
  char startMarker = '\n';
  char endMarker = '\r';
  char receivedChar;

  while (Serial3.available() > 0 && !isNewWeightData) {
    receivedChar = Serial3.read();

    if (isReceiving) {
      if (receivedChar != endMarker) {
        weightMessage[dataIndex] = receivedChar;
        dataIndex++;
        if (dataIndex >= numCharsWeight) {
          dataIndex = numCharsWeight - 1;
        }
      } else {
        weightMessage[dataIndex] = '\0';  // Terminate the string
        isReceiving = false;
        dataIndex = 0;
        isNewWeightData = true;
      }
    } else if (receivedChar == startMarker) {
      isReceiving = true;
    }
  }
}

// Data processing functions

void processWeightData() {
  // Extract relevant parts of the weight message
  strncpy(weightString, weightMessage + 5, 5);
  strncpy(priceString, weightMessage + 13, 5);
  strncpy(amountString, weightMessage + 21, 5);

  // Convert strings to floating-point and integer values
  weight = atof(weightString);
  priceType = atoi(priceString);
  transactionAmount = atof(amountString);

  // Round the calculated value
  transactionAmount = roundAmount(transactionAmount);

  // Determine the type of transaction based on price
  determineTransactionType(priceType);
}

float roundAmount(float originalAmount) {
  // Round the value and ensure it's at least 2
  float roundedAmount = round(originalAmount);
  if (roundedAmount <= 2) {
    roundedAmount = 2;
  }
  return roundedAmount;
}

void determineTransactionType(byte priceType) {
  // Implement this function to determine the type of transaction based on the price
  // You can set the transactionType variable here
}

// User-related functions

void identifyUser() {
  // Extract card number from the message
  strncpy(cardData, cardMessage + 3, 7);
  cardNumber = strtoul(cardData, NULL, 16);

  // Find the user's index, name, balance, and check if the account is active
  blockIndex = findUserIndex(cardNumber);
  getUserName(cardNumber);
  accountBalance = checkAccountBalance(cardNumber);
  isAuthorized = checkAuthorization(cardNumber, accountBalance);
}

int findUserIndex(unsigned long cardNumber) {
  // Implement this function to find the index corresponding to the card number
}

bool checkAuthorization(unsigned long cardNumber, float accountBalance) {
  // Implement this function to check if the account is authorized
}

char* getUserName(unsigned long cardNumber) {
  // Implement this function to retrieve the user's name from the database
}

// Display functions

void initializeDisplay() {
  // Implement display initialization here
}

void updateDisplay(DisplayState currentDisplayState) {
  switch (currentDisplayState) {
    case DISPLAY_IDLE:
      // Update display content for idle state
      // You can display relevant information, messages, etc.
      break;
    case DISPLAY_COLLECTING_DATA:
      // Update display content for collectingData state
      break;
      // Add cases for other states
  }
  // Refresh the epaper display with the updated content
  refreshEpaperDisplay();
}

// State machine functions

State nextState(State currentState) {
  switch (currentState) {
    case STATE_IDLE:
      if (isAuthorized) {
        changeDisplay(DISPLAY_COLLECTING_DATA);
        return STATE_COLLECT_DATA;
      } else {
        return STATE_IDLE;
      }
    case STATE_COLLECT_DATA:
      if (isNewWeightData && isAuthorized) {
        isNewWeightData = false;
        return STATE_CALCULATE_DATA;
      } else {
        return STATE_IDLE;
      }
    case STATE_CALCULATE_DATA:
      return STATE_PRINT;
    case STATE_PRINT:
      return STATE_SAVE_DATA;
    case STATE_SAVE_DATA:
      return STATE_IDLE;
    default:
      return STATE_IDLE;  // Default to STATE_IDLE
  }
}

void executeState(State currentState) {
  switch (currentState) {
    case STATE_IDLE:
      Serial.println("State: STATE_IDLE");

      // Add actions for STATE_IDLE here

      portKarty.listen();

      if (isNewCardData) {
        identifyUser();
        isNewCardData = false;
        currentState = nextState(currentState);
        updateDisplay(currentDisplayState);  // Update display based on the new state
      } else {
        receiveCardData();
      }
      break;

    case STATE_COLLECT_DATA:
      Serial.println("State: STATE_COLLECT_DATA");

      // Add actions for STATE_COLLECT_DATA here
      portWagi.listen();

      if (isNewWeightData) {
        currentState = nextState(currentState);
        updateDisplay(currentDisplayState);  // Update display based on the new state
      } else {
        receiveWeightData();
      }
      break;

    case STATE_CALCULATE_DATA:
      Serial.println("State: STATE_CALCULATE_DATA");

      // Add actions for STATE_CALCULATE_DATA here

      processWeightData();
      currentState = nextState(currentState);
      updateDisplay(currentDisplayState);  // Update display based on the new state
      break;

    case STATE_PRINT:
      Serial.println("State: STATE_PRINT");

      // Add actions for STATE_PRINT here

      // Handle the printing process
      printReceipt();

      break;

    case STATE_SAVE_DATA:
      Serial.println("State: STATE_SAVE_DATA");

      // Add actions for STATE_SAVE_DATA here
      break;

    default:
      break;
  }
}

void changeDisplay(DisplayState newDisplayState) {
  currentDisplayState = newDisplayState;
  updateDisplay(currentDisplayState);
}

void printReceipt() {
  // Add actions for printing the receipt here

  void printReceipt() {
    Serial.println("Printing receipt...");

    // Handle the printing process
    printer.wake();
    printer.setDefault();
    printer.setHeatConfig(11, 220, 45);

    DateTime now = rtc.now();

    char data_kod[] = "MMDD";
    char czasdruk[] = "hh:mm";
    now.toString(czasdruk);
    char datadruk[] = "DD.MM.YYYY";
    now.toString(datadruk);
    char masa_kod[6];
    dtostrf(weight, 5, 3, masa_kod);

    char kodkreskowy[15];
    int fnaleznosc_int = transactionAmount;
    float fnaleznosc_frac = transactionAmount - fnaleznosc_int;
    int fnaleznosc_intfrac = trunc(fnaleznosc_frac * 100);

    // Create a barcode
    snprintf(kodkreskowy, 15, "%s%03d%02d%02d%d", data_kod, blockIndex, fnaleznosc_int, fnaleznosc_intfrac, priceType);

    // Print the receipt
    printer.setFont('A');
    printer.setSize('L');
    printer.justify('C');
    printer.doubleHeightOff();
    printer.inverseOn();
    printer.println(F("A M F O R A"));
    printer.inverseOff();
    printer.setBarcodeHeight(110);
    printer.printBarcode(kodkreskowy, CODE128);
    printer.feed(1);
    printer.justify('C');
    printer.setSize('L');
    printer.doubleHeightOff();
    printer.boldOn();
    if (blockIndex == 192) {
      printer.println(F("KLUB SENIORA"));
    } else {
      printer.println(transactionType);
    }
    printer.setSize('M');
    printer.doubleHeightOff();
    printer.print(blockIndex);
    printer.print(F(" "));
    if (blockIndex != 217) {
      strcpy_P(userName, (char*)pgm_read_word(&(string_table[blockIndex])));
      printer.println(userName);
    }
    printer.boldOff();
    printer.print(weight);
    printer.print(F(" kg            "));
    printer.print(transactionAmount);
    printer.println(F(" PLN"));
    printer.setSize('S');
    printer.print(czasdruk);
    printer.print(F("     "));
    printer.println(datadruk);
    printer.justify('L');
    printer.feed(3);

    // Clear the LCD display and provide instructions
    // Assuming you have an LCD display, you can add code here to clear it and display instructions.
    // Replace this comment with your code.

    // Put the printer to sleep after printing
    printer.sleepAfter(1200);

    // Update states and variables
    // Assuming you have variables like zmienLCD, stanGotowosci, etc., you can update them here.
    // Replace this comment with your code.

    Serial.println("Receipt printed.");
  }
}

void refreshEpaperDisplay() {
  // Add actions for refreshing the epaper display here
}
