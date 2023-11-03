// Include necessary libraries
#include <SoftwareSerial.h>    // Library for Software Serial Communication
#include <mbed_mktime.h>       // Library for mktime function
#include <math.h>              // Library for math operations
#include <Adafruit_Thermal.h>  // Library for Adafruit Thermal Printer

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_EPD.h>

// Define the pins used for the e-paper display
#define SRAM_CS     10
#define EPD_CS      9
#define EPD_DC      8
#define EPD_RESET   7
#define EPD_BUSY    6

// Create an instance of the display
Adafruit_IL0373 display(400, 300, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

// Define the states for the display state machine - to na potem
// dopiero przy działającym ekranie trzeba się będzie zastanowić, jakie ekrany chcemy mieć
//
// enum DisplayState {
//   DISPLAY_IDLE,
//   DISPLAY_COLLECTING_DATA,
//   DISPLAY_CALCULATING_DATA,
//   DISPLAY_PRINTING,
//   DISPLAY_SAVING_DATA
// };

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

// Boolean flags to indicate new card and weight data
bool isNewCardData = false;
bool isNewWeightData = false;

// Initialize the current state for the state machine
State currentState = STATE_IDLE;
// DisplayState currentDisplayState = DISPLAY_IDLE;


const char* transactionTypefilename = "transaction_types.txt";
const char* minPriceFilename = "min_price.txt";
const char* userDataFilename = "user_data.csv"

// Zmienne "końcowe", po wszystkich przeliczeniach i konwersjach, ze stringu wagi do liczb itd.
unsigned long cardNumber = 0;   // Valid card number obtained from the card reader
float weight = 0.0;             // Mass as a floating-point number calculated from weight data
float transactionAmount = 0.0;  // Floating-point number calculated from mass and price, rounded accordingly
byte clubMembershipNumber;      // Club membership number
int transactionPrice;           // Price type
char userName[20];              // User's name from the database
bool isAuthorized = false;      // Variable to determine if someone has an active account
float accountBalance = 0.0;     // User's account balance, updated with each transaction
float minPrice = 2;             // minimal price for firing
char transactionType[11];

// Temporary variables for data processing, consider moving these as local variables if possible
byte userNumber;

const byte numCharsWeight = 32;
char weightMessage[numCharsWeight];
const byte numCharsCard = 13;
char cardMessage[numCharsCard];
byte numReceived = 0;

const byte bisquePrice = 10;
const byte glazePrice1 = 22;
const byte glazePrice2 = 20;
const byte highTempPrice = 25;

void setup() {
  // Initialize software serial ports
  Serial3.begin(9600);   // Weight port
  Serial4.begin(19200);  // Printer port
  Serial2.begin(9600);   // Card reader port

  // Initialize the e-paper display
  display.begin();
  
  // Clear the buffer
  display.clearBuffer();


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

void handleError(const char* errorMessage) {
  // You can implement the error handling logic here, e.g., printing to Serial or taking appropriate actions
  Serial.println(errorMessage);
}

// Input/Output functions


// Ta funkcja jest 1:1 skopiowana z poprzedniej wersji programu, działała więc powinna działać dalej...
// wygląda dziwnie, wiem, ale chyba w taki sposob musi wyglądać pobieranie danych z czynnika karty
// na podstawie kodu producenta, trochę dostosowane z tego co pamiętam
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


// tez działało, podobna zasada jak pobieranie danych z karty.
// na podstawie przykładowego szkicu producenta
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

// wyciąganie masy, ceny i kwoty (masa*cena) ze stringu wysyłanego przez wagę
void processWeightData() {
  char weightString[7];
  char priceString[7];
  char amountString[7];

  // Extract relevant parts of the weight message
  strncpy(weightString, weightMessage + 5, 5);
  strncpy(priceString, weightMessage + 13, 5);
  strncpy(amountString, weightMessage + 21, 5);

  // Convert strings to floating-point and integer values
  weight = atof(weightString);
  transactionPrice = atoi(priceString);
  transactionAmount = atof(amountString);

  // Round the calculated value
  transactionAmount = roundAmount(transactionAmount);

  // Determine the type of transaction based on price
  transactionType = determineTransactionType(transactionPrice);
}

// wiadomo
float roundAmount(float originalAmount) {
  // Round the value and ensure it's at least 2
  float roundedAmount = round(originalAmount);
  minPrice = getMinimalPrice;
  if (roundedAmount <= minPrice) {
    roundedAmount = minPrice;
  }
  return roundedAmount;
}


// pobiera wartość z karty pamięci z pliku min_price.txt
int getMinimalPrice() {
  int minimalPrice = 0;

  // Initialize the SD card
  if (!SD.begin(4)) { // You may need to change the pin number (4) to match your hardware
    handleError("SD initialization failed");
    return minimalPrice;
  }

  // Open the "min_price.txt" file
  File file = SD.open(minPriceFilename);
  if (!file) {
    handleError("Could not open min_price.txt");
    return minimalPrice;
  }

  // Read the integer from the file
  if (file.available()) {
    minimalPrice = file.parseInt();
  } else {
    handleError("No data found in min_price.txt");
  }

  // Close the file
  file.close();

  return minimalPrice;
}

// tutaj w starej wersji było po prostu - jak na wadze wpisana była odpowiednia cena
// to na jej podstawie wybierany był typ wypału - biskwit, szliwo, szkliwo wysza temperatura itd
// ze zmiennych globalnych zapisanych w pamięci programu
// teraz pobierane są dane z pliku na karcie pamięci - nie testowałem tego jeszcze
char* determineTransactionType(int transactionPrice) {
  static char transactionType[50]; // Assuming transaction types won't exceed 50 characters

  // Initialize the SD card
  if (!SD.begin(4)) { // You may need to change the pin number (4) to match your hardware
    handleError("SD initialization failed");
    return transactionType;
  }

  // Open the data file
  File file = SD.open(transactionTypefilename);
  if (!file) {
    handleError("Could not open file");
    return transactionType;
  }

  // Read the file line by line and determine the transaction type
  while (file.available()) {
    String line = file.readStringUntil('\n');
    int separatorIndex = line.indexOf(' ');
    if (separatorIndex != -1) {
      int price = line.substring(0, separatorIndex).toInt();
      if (price == transactionPrice) {
        strcpy(transactionType, line.substring(separatorIndex + 1).c_str());
        file.close();
        return transactionType;
      }
    }
  }

  // Close the file if the transactionPrice is not found
  file.close();
  handleError("Transaction type not found");
  return transactionType;
}

// User-related functions

// nowa funkcja, łączy kilka rzeczy z poprzedniego programu, podzieliłem to na drobniejsze funkcje
// bo taka podobno jest dobra praktyka, ale się nie znam :D
void identifyUser() {
  // Extract card number from the message
  char cardData[8] = "1234567";
  strncpy(cardData, cardMessage + 3, 7);
  cardNumber = strtoul(cardData, NULL, 16);

  // Find the user's index, name, balance, and check if the account is active
  userNumber = findUserIndex(cardNumber);
  userName = getUserName(cardNumber);
  accountBalance = checkAccountBalance(cardNumber);
  isAuthorized = checkAuthorization(cardNumber, accountBalance);
}

// w poprzedniej wersji programu wyszukiwało index po znalezieniu numeru w tablicy.
// tablica była wgrywana z pamięci programu
int findUserIndex(unsigned long cardNumber) {
  File file = SD.open(userDataFilename);
  if (!file) return -1; // Unable to open the file

  char line[100]; // Assuming a line won't be longer than 100 characters
  int index = -1;
  while (file.available()) {
    file.read(line, sizeof(line));
    unsigned long currentCardNumber = strtoul(strtok(line, ","), NULL, 10);
    if (currentCardNumber == cardNumber) {
      index = atoi(strtok(NULL, ","));
      break;
    }
  }
  file.close();
  return index;
}

bool checkAuthorization(unsigned long cardNumber, float accountBalance) {
  File file = SD.open(userDataFilename);
  if (!file) return false; 

  char line[100];
  bool isAuthorized = false;
  while (file.available()) {
    file.read(line, sizeof(line));
    unsigned long currentCardNumber = strtoul(strtok(line, ","), NULL, 10);
    if (currentCardNumber == cardNumber) {
      strtok(NULL, ","); // Skip the membership number
      strtok(NULL, ","); // Skip the user name
      isAuthorized = atoi(strtok(NULL, ",")) ? true : false;
      break;
    }
  }
  file.close();
  return isAuthorized;
}

char* getUserName(unsigned long cardNumber) {
  File file = SD.open(userDataFilename);
  if (!file) return NULL;

  char line[100];
  static char userName[50]; // Assuming a name won't be longer than 50 characters
  while (file.available()) {
    file.read(line, sizeof(line));
    unsigned long currentCardNumber = strtoul(strtok(line, ","), NULL, 10);
    if (currentCardNumber == cardNumber) {
      strtok(NULL, ","); // Skip the membership number
      strcpy(userName, strtok(NULL, ","));
      break;
    }
  }
  file.close();
  return userName;
}

float checkAccountBalance(unsigned long cardNumber){
  File file = SD.open(userDataFilename);
  if (!file) return -1;

  char line[100];
  float balance = -1;
  while (file.available()) {
    file.read(line, sizeof(line));
    unsigned long currentCardNumber = strtoul(strtok(line, ","), NULL, 10);
    if (currentCardNumber == cardNumber) {
      strtok(NULL, ","); // Skip the membership number
      strtok(NULL, ","); // Skip the user name
      strtok(NULL, ","); // Skip the authorization
      balance = atof(strtok(NULL, ","));
      break;
    }
  }
  file.close();
  return balance;
}


// tu na razie pusto, zostawiam takie placeholdery na przyszłość
// Display functions
// void initializeDisplay() {
//   // Implement display initialization here
// }
//
// void updateDisplay(DisplayState currentDisplayState) {
//   switch (currentDisplayState) {
//     case DISPLAY_IDLE:
//       // Update display content for idle state
//       // You can display relevant information, messages, etc.
//       break;
//     case DISPLAY_COLLECTING_DATA:
//       // Update display content for collectingData state
//       break;
//       // Add cases for other states
//   }
//   // Refresh the epaper display with the updated content
//   refreshEpaperDisplay();
// }

// State machine functions

// przełączanie między "trybami" maszyny stanow na podstawie spelnianych warunkow.
// w poprzedniej wersji programu maszyna stanow była zrobiona w przedziwny sposob,
// ale jakoś działało...
State nextState(State currentState) {
  switch (currentState) {
    case STATE_IDLE;
        return STATE_IDLE;
    case STATE_COLLECT_DATA:
        return STATE_IDLE;
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

// workflow całego systemu z funkcjami do odpalenia podczas kazdego stanu maszyny
// trzeba to rozbijać na funkcje? Zeby kazdy stan maszyny miał w zawartości tylko call funkcji
// w stylu void executeSTATE_IDLE {} ?
void executeState(State currentState) {
  switch (currentState) {
    case STATE_IDLE:
      Serial.println("State: STATE_IDLE");

      // Add actions for STATE_IDLE here
      // pobieranie numeru karty z czytnika i identyfikacja danych uzytkownika

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
      // pobieranie danych z wagi, zegara (niezaimplementowane)
      // moze pobieranie danych z bazy? (jakich?)
      // cennik, w stylu biskwit = 10zł/kg, szkliwo 25zł/kg itd
      // moze to tutaj powinno wyladować pobieranie stanu konta uzytkownika, a nie w identifyUser()

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
      // na podstawie wszystkich zebranych danych liczenie... rzeczy :D

      processWeightData();
      currentState = nextState(currentState);
      updateDisplay(currentDisplayState);  // Update display based on the new state
      break;

    case STATE_PRINT:
      Serial.println("State: STATE_PRINT");

      // Add actions for STATE_PRINT here
      // tylko drukowanie rzeczy na drukarce

      // Handle the printing process
      printReceipt();

      break;

    case STATE_SAVE_DATA:
      Serial.println("State: STATE_SAVE_DATA");

      // Add actions for STATE_SAVE_DATA here
      // zapisywanie czego trzeba w plikach, wypały na liście wypałow
      // stan konta w uzytkowniku itd.
      break;

    default:
      break;
  }
}

// void changeDisplay(DisplayState newDisplayState) {
//   currentDisplayState = newDisplayState;
//   updateDisplay(currentDisplayState);
// }


// na razie skopiowane ze starej wersji, muszę to jeszcze uporządkować
// nowe talony będą słuzyły do tego samego co stare, ale mogą mieć dodatkowe informacje
// albo być bardziej kompaktowe, zeby było eko.
// ze względu na to, ze wydrukowanie talonu będzie jednoczesnie odejmowalo kwotę z konta
// talony wydrukowane przez przypadek muszą lądować u Roberta, zeby mogl zwrocić hajs na konto
// (tak to sobie wyobrazaliśmy na razie - bedzie mniej skanowania niz skanowanie wszystkich poprawnych talonow)
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
    snprintf(kodkreskowy, 15, "%s%03d%02d%02d%d", data_kod, userNumber, fnaleznosc_int, fnaleznosc_intfrac, transactionPrice);

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
    if (userNumber == 192) {
      printer.println(F("KLUB SENIORA"));
    } else {
      printer.println(transactionType);
    }
    printer.setSize('M');
    printer.doubleHeightOff();
    printer.print(userNumber);
    printer.print(F(" "));
    if (userNumber != 217) {
      strcpy_P(userName, (char*)pgm_read_word(&(string_table[userNumber])));
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

// void refreshEpaperDisplay() {
//   // Add actions for refreshing the epaper display here
// }
