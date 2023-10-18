#include <SoftwareSerial.h>
#include <EEPROM.h>

#include <math.h>

#include <RTClib.h>

RTC_PCF8523 rtc;

#include <Adafruit_Thermal.h>

SoftwareSerial portDrukarki(8, 9);
Adafruit_Thermal printer(&portDrukarki);

SoftwareSerial portWagi(13, 12);

SoftwareSerial portKarty(10, 11);

#include <LiquidCrystal.h>

const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// #include <anyrtttl.h>
// #include <binrtttl.h>
// #include <pitches.h>


// zmienne stanów do maszyny stanów :D
boolean stanGotowosci = true;
boolean stanWazenia = false;
boolean stanObliczen = false;
boolean stanDrukowania = false;

// zmienne do wyświetlacza
boolean ruszLCD = false;
boolean zmienLCD = true;

unsigned long previousMillis = 0;
const long interval = 500;
boolean wlewo = true;
int tykacz = 01;
int dlugoscTekstu;

// zmienne zarezerwowane dla wiadomości z czytników
unsigned long numerkarty = 0;

const byte numCharsWagi = 32;
char wiadomoscWagi[numCharsWagi];

const byte numCharsKarty = 13;
char wiadomoscKarty[numCharsKarty];
byte numReceived = 0;

boolean newData = false;

// zmienne dla przerabianych danych z czytników
char stringKarty[8] = "1234567";

char chindeks[4];

char chmasa[7];
float fmasa;

char chcena[7];
byte icena;

char chnaleznosc[7];
float fnaleznosc;

char typWypalu[11];

byte indeks;
byte blokindeks;

char imie[20];

byte autoryzacja = 0;

unsigned long currentMillis;


// ##################################################################################
// #################       dane do zmieniania w razie potrzeby        ###############
// ##################################################################################

const byte cenaBiskwit = 10;
const byte cenaSzkliwo1 = 22;
const byte cenaSzkliwo2 = 20;
const byte cenaOstro = 25;

const unsigned long numeryKart[] PROGMEM = {
0,
0,
4702923,
4513885,
5194419,
4638770,
5134615,
5135464,
0,
5134211,
12900311,
4515086,
5194628,
4704116,
4703529,
13110325,
5152408,
5202389,
0,
5174777,
4639725,
0,
4704544,
4636079,
4636088,
4637781,
5173278,
4636089,
4670919,
4637346,
4679537,
4693904,
5135566,
13109378,
5207545,
0,
5201859,
0,
4637361,
0,
0,
4636477,
13040960,
0,
5195908,
4636078,
13099850,
5193273,
5210403,
5142143,
4636918,
5200631,
0,
4583687,
5203571,
5155288,
5212895,
5173528,
4693556,
4651585,
0,
5195149,
0,
4639280,
4638303,
0,
5183751,
0,
5212599,
5134275,
5174279,
4713127,
4711823,
5155198,
0,
4697763,
5212665,
4710684,
4646640,
4710333,
4712946,
5212335,
0,
0,
0,
5133872,
0,
4692360,
5211639,
5204951,
5184254,
4678989,
5134677,
5195819,
4636485,
5199725,
5135032,
0,
0,
0,
5206519,
4710482,
4640238,
5166034,
4636919,
4711129,
5175526,
4713298,
0,
5193743,
5211895,
4709409,
5213455,
4584027,
0,
4650451,
0,
0,
4585499,
4586641,
5213495,
4586250,
4649929,
0,
4712410,
4649827,
5136986,
5175775,
0,
5191358,
5209529,
5160896,
5175817,
5184379,
5208098,
5183648,
5183050,
5209956,
5211994,
4585891,
4703377,
5209978,
4703970,
5173528,
4672406,
4667077,
4667252,
4669297,
4668733,
4670836,
4667805,
4671660,
4649306,
4650551,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
4637322,
5208554,
5209065,
0,
0,
0,
0,
0,
0,
4673779,
5212331,
5213496,
5213717,
5213700,
5193957,
4643643
};

const char string0[] PROGMEM = "0";
const char string1[] PROGMEM = "Robo";
const char string2[] PROGMEM = "Agnieszka Gó.";
const char string3[] PROGMEM = "Martka";
const char string4[] PROGMEM = "Alek G.";
const char string5[] PROGMEM = "Marta G.";
const char string6[] PROGMEM = "Aneta R.";
const char string7[] PROGMEM = "Anna B.";
const char string8[] PROGMEM = "";
const char string9[] PROGMEM = "Anna K.";
const char string10[] PROGMEM = "Dorota Chm.";
const char string11[] PROGMEM = "Ewa P.";
const char string12[] PROGMEM = "Ewelina ł.";
const char string13[] PROGMEM = "Małgorzata Z.";
const char string14[] PROGMEM = "Hania G.";
const char string15[] PROGMEM = "Iga S.";
const char string16[] PROGMEM = "Ilona D.";
const char string17[] PROGMEM = "Anna S.";
const char string18[] PROGMEM = "";
const char string19[] PROGMEM = "Marta M.";
const char string20[] PROGMEM = "Karolina R.";
const char string21[] PROGMEM = "";
const char string22[] PROGMEM = "Kasia O.";
const char string23[] PROGMEM = "Katarzyna B.";
const char string24[] PROGMEM = "Katarzyna K.";
const char string25[] PROGMEM = "Katarzyna O.";
const char string26[] PROGMEM = "Ewa Z.";
const char string27[] PROGMEM = "Magda Z.";
const char string28[] PROGMEM = "Ula W.";
const char string29[] PROGMEM = "Marta C.";
const char string30[] PROGMEM = "Marta Goź.";
const char string31[] PROGMEM = "Kamil R.";
const char string32[] PROGMEM = "Marta M.";
const char string33[] PROGMEM = "Mateusz M.";
const char string34[] PROGMEM = "Natalia P.";
const char string35[] PROGMEM = "";
const char string36[] PROGMEM = "Agnieszka R.";
const char string37[] PROGMEM = "";
const char string38[] PROGMEM = "Olga B.";
const char string39[] PROGMEM = "";
const char string40[] PROGMEM = "";
const char string41[] PROGMEM = "Paulina W.";
const char string42[] PROGMEM = "Sabina B.";
const char string43[] PROGMEM = "";
const char string44[] PROGMEM = "Tala M.";
const char string45[] PROGMEM = "Tetiana Z.";
const char string46[] PROGMEM = "Weronika T.";
const char string47[] PROGMEM = "Weronika W.";
const char string48[] PROGMEM = "Ania Ma.";
const char string49[] PROGMEM = "Zuzia W.";
const char string50[] PROGMEM = "Agnieszka Go.";
const char string51[] PROGMEM = "Ewelina Le.";
const char string52[] PROGMEM = "";
const char string53[] PROGMEM = "Agata K.";
const char string54[] PROGMEM = "Bernadetta W.";
const char string55[] PROGMEM = "Joanna G.";
const char string56[] PROGMEM = "Paula D.";
const char string57[] PROGMEM = "Marta K.";
const char string58[] PROGMEM = "Agata An.";
const char string59[] PROGMEM = "Martyna W.";
const char string60[] PROGMEM = "";
const char string61[] PROGMEM = "";
const char string62[] PROGMEM = "";
const char string63[] PROGMEM = "Katarzyna W.";
const char string64[] PROGMEM = "Marta L.";
const char string65[] PROGMEM = "";
const char string66[] PROGMEM = "Kasia Ba.";
const char string67[] PROGMEM = "";
const char string68[] PROGMEM = "Teresa D.";
const char string69[] PROGMEM = "Michał Byk.";
const char string70[] PROGMEM = "Ilona Rz.";
const char string71[] PROGMEM = "Karolina K.";
const char string72[] PROGMEM = "Filip L.";
const char string73[] PROGMEM = "Tomasz O.";
const char string74[] PROGMEM = "";
const char string75[] PROGMEM = "Katarzyna L.";
const char string76[] PROGMEM = "Martyna Ra.";
const char string77[] PROGMEM = "Ev B.";
const char string78[] PROGMEM = "Maria Ż,";
const char string79[] PROGMEM = "Marianna K.";
const char string80[] PROGMEM = "Patrycja Mi.";
const char string81[] PROGMEM = "Michał G.";
const char string82[] PROGMEM = "";
const char string83[] PROGMEM = "";
const char string84[] PROGMEM = "";
const char string85[] PROGMEM = "Zuza N.";
const char string86[] PROGMEM = "";
const char string87[] PROGMEM = "Emilia M.";
const char string88[] PROGMEM = "Sylwia O.";
const char string89[] PROGMEM = "";
const char string90[] PROGMEM = "Wiktoria M.";
const char string91[] PROGMEM = "Agnieszka B.";
const char string92[] PROGMEM = "Patrycja J.";
const char string93[] PROGMEM = "Zosia H.";
const char string94[] PROGMEM = "Paulina T.";
const char string95[] PROGMEM = "Helena H.";
const char string96[] PROGMEM = "Ania D.";
const char string97[] PROGMEM = "";
const char string98[] PROGMEM = "";
const char string99[] PROGMEM = "";
const char string100[] PROGMEM = "Justyna G.";
const char string101[] PROGMEM = "Magda Ch.";
const char string102[] PROGMEM = "Anna Brysz.";
const char string103[] PROGMEM = "Klaudia W.";
const char string104[] PROGMEM = "Paulina Tymosz.";
const char string105[] PROGMEM = "Filip Z.";
const char string106[] PROGMEM = "Maria Sz.";
const char string107[] PROGMEM = "Maria B.";
const char string108[] PROGMEM = "";
const char string109[] PROGMEM = "Paulina C.";
const char string110[] PROGMEM = "Katarzyna De.";
const char string111[] PROGMEM = "Patrycja Gr.";
const char string112[] PROGMEM = "Polina I.";
const char string113[] PROGMEM = "Emilia G.";
const char string114[] PROGMEM = "";
const char string115[] PROGMEM = "Piotr O.";
const char string116[] PROGMEM = "";
const char string117[] PROGMEM = "";
const char string118[] PROGMEM = "Urszula Gr.";
const char string119[] PROGMEM = "Maria Sza.";
const char string120[] PROGMEM = "Wojtek B.";
const char string121[] PROGMEM = "Antek K.";
const char string122[] PROGMEM = "Basia N. ";
const char string123[] PROGMEM = "Ola Ł.";
const char string124[] PROGMEM = "Martyna Ga.";
const char string125[] PROGMEM = "Ewa H.";
const char string126[] PROGMEM = "Martyna No.";
const char string127[] PROGMEM = "Arek N.";
const char string128[] PROGMEM = "Agnieszka J.";
const char string129[] PROGMEM = "Ola K.";
const char string130[] PROGMEM = "Dominika W.";
const char string131[] PROGMEM = "Greta K.";
const char string132[] PROGMEM = "Agnes";
const char string133[] PROGMEM = "Gosia S.";
const char string134[] PROGMEM = "Dorota Os.";
const char string135[] PROGMEM = "Małgorzata S.";
const char string136[] PROGMEM = "Maria N.";
const char string137[] PROGMEM = "Monika P.";
const char string138[] PROGMEM = "Paula K.";
const char string139[] PROGMEM = "Tolek L.";
const char string140[] PROGMEM = "Weronika Pe.";
const char string141[] PROGMEM = "Paulina Pa.";
const char string142[] PROGMEM = "Kaja G.";
const char string143[] PROGMEM = "Nina P.";
const char string144[] PROGMEM = "Emilia Mę.";
const char string145[] PROGMEM = "Marta R.";
const char string146[] PROGMEM = "Paula L.";
const char string147[] PROGMEM = "Natalia M.";
const char string148[] PROGMEM = "Urszula I.";
const char string149[] PROGMEM = "Thai";
const char string150[] PROGMEM = "Zuzanna B.";
const char string151[] PROGMEM = "Monika Sz.";
const char string152[] PROGMEM = "Aleksandra B.";
const char string153[] PROGMEM = "Karolina T. ";
const char string154[] PROGMEM = "";
const char string155[] PROGMEM = "";
const char string156[] PROGMEM = "";
const char string157[] PROGMEM = "";
const char string158[] PROGMEM = "";
const char string159[] PROGMEM = "";
const char string160[] PROGMEM = "";
const char string161[] PROGMEM = "";
const char string162[] PROGMEM = "";
const char string163[] PROGMEM = "";
const char string164[] PROGMEM = "";
const char string165[] PROGMEM = "";
const char string166[] PROGMEM = "";
const char string167[] PROGMEM = "";
const char string168[] PROGMEM = "";
const char string169[] PROGMEM = "";
const char string170[] PROGMEM = "";
const char string171[] PROGMEM = "";
const char string172[] PROGMEM = "";
const char string173[] PROGMEM = "";
const char string174[] PROGMEM = "";
const char string175[] PROGMEM = "";
const char string176[] PROGMEM = "";
const char string177[] PROGMEM = "";
const char string178[] PROGMEM = "";
const char string179[] PROGMEM = "";
const char string180[] PROGMEM = "";
const char string181[] PROGMEM = "";
const char string182[] PROGMEM = "";
const char string183[] PROGMEM = "";
const char string184[] PROGMEM = "";
const char string185[] PROGMEM = "";
const char string186[] PROGMEM = "";
const char string187[] PROGMEM = "";
const char string188[] PROGMEM = "";
const char string189[] PROGMEM = "";
const char string190[] PROGMEM = "KF";
const char string191[] PROGMEM = "KF";
const char string192[] PROGMEM = "KF";
const char string193[] PROGMEM = "KF";
const char string194[] PROGMEM = "";
const char string195[] PROGMEM = "";
const char string196[] PROGMEM = "";
const char string197[] PROGMEM = "";
const char string198[] PROGMEM = "";
const char string199[] PROGMEM = "Sławomira W.";
const char string200[] PROGMEM = "Krystian P";
const char string201[] PROGMEM = "Teresa P";
const char string202[] PROGMEM = "Renata R";
const char string203[] PROGMEM = "Alina O";
const char string204[] PROGMEM = "Magda P";
const char string205[] PROGMEM = "Joasia K";

const char *const string_table[] PROGMEM = {string0, string1, string2, string3, string4, string5, string6, string7, string8, string9, string10, string11, string12, string13, string14, string15, string16, string17, string18, string19, string20, string21, string22, string23, string24, string25, string26, string27, string28, string29, string30, string31, string32, string33, string34, string35, string36, string37, string38, string39, string40, string41, string42, string43, string44, string45, string46, string47, string48, string49, string50, string51, string52, string53, string54, string55, string56, string57, string58, string59, string60, string61, string62, string63, string64, string65, string66, string67, string68, string69, string70, string71, string72, string73, string74, string75, string76, string77, string78, string79, string80, string81, string82, string83, string84, string85, string86, string87, string88, string89, string90, string91, string92, string93, string94, string95, string96, string97, string98, string99, string100, string101, string102, string103, string104, string105, string106, string107, string108, string109, string110, string111, string112, string113, string114, string115, string116, string117, string118, string119, string120, string121, string122, string123, string124, string125, string126, string127, string128, string129, string130, string131, string132, string133, string134, string135, string136, string137, string138, string139, string140, string141, string142, string143, string144, string145, string146, string147, string148, string149, string150, string151, string152, string153, string154, string155, string156, string157, string158, string159, string160, string161, string162, string163, string164, string165, string166, string167, string168, string169, string170, string171, string172, string173, string174, string175, string176, string177, string178, string179, string180, string181, string182, string183, string184, string185, string186, string187, string188, string189, string190, string191, string192, string193, string194, string195, string196, string197, string198, string199, string200, string201, string202, string203, string204, string205};


const int length = (sizeof(numeryKart) / sizeof(*numeryKart));

// ##################################################################################
// #################              MOJE FUNKCJE                        ###############
// ##################################################################################

int znajdzIndeks(unsigned long numerkarty) {
  Serial.print(F("length = "));
  Serial.println(length);
  unsigned long buforIndeksu;
  for (int x = 0; x <= length; x++) {
    // Serial.print(F("x = "));
    // Serial.println(x);
    if (x < length){
    buforIndeksu = pgm_read_dword(&numeryKart[x]);
    if (numerkarty == buforIndeksu) {
      Serial.print(F("znaleziony indeks: "));
      Serial.println(x);
      buforIndeksu = 0;
      return x;
    }
    }
    else{
      return x +1;
    }
    }
}

int EEPROMznajdzIndeks(unsigned long numerkarty) {
  unsigned long buforIndeksu;
  for (int x = 0; x <= length; x++) {
    // Serial.print(F("x = "));
    // Serial.println(x);
    if (x < length){
    EEPROM.get(x + 255 + sizeof(buforIndeksu), buforIndeksu);
    if (numerkarty == buforIndeksu) {
      Serial.print(F("znaleziony indeks: "));
      Serial.println(x);
      buforIndeksu = 0;
      return x;
    }
    }
    else{
      return x +1;
    }
    }
}

void receiveKarta() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  //char startMarker = '0';
  char rc;

  while (portKarty.available() > 0 && newData == false) {
    rc = portKarty.read();

    if (recvInProgress == true) {
      // if (rc != endMarker) {
      wiadomoscKarty[ndx] = rc;
      ndx++;
      if (ndx >= numCharsKarty) {
        ndx = numCharsKarty - 1;
        wiadomoscKarty[ndx] = '\0';  // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else {
      recvInProgress = true;
    }
  }
}

float transponuj(float true_fnaleznosc) {
  float korekcja = 0;
  int losowe;
  if (currentMillis % 3 == 0) {
    for (int i = 0; i <= 10; i++) {
      losowe = random(7, 67);
    }
    korekcja = losowe / 100.0;
  }
  float korygowana_fnaleznosc = true_fnaleznosc + korekcja;
  float nowa_fnaleznosc = round(korygowana_fnaleznosc);
  if (nowa_fnaleznosc <= 2){
    nowa_fnaleznosc = 2;
  }
  return nowa_fnaleznosc;
}

float zaokragl(float true_fnaleznosc) {
  float nowa_fnaleznosc = round(true_fnaleznosc);
  if (nowa_fnaleznosc <= 2){
    nowa_fnaleznosc = 2;
  }
  return nowa_fnaleznosc;
}

void receiveWaga() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '\n';
  char endMarker = '\r';
  char rc;

  // if (Serial.available() > 0) {
  while (portWagi.available() > 0 && newData == false) {
    // Serial.println(F("weszlo do while w receiveWaga"));
    rc = portWagi.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        wiadomoscWagi[ndx] = rc;
        ndx++;
        if (ndx >= numCharsWagi) {
          ndx = numCharsWagi - 1;
        }
      } else {
        wiadomoscWagi[ndx] = '\0';  // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}


// ##################################################################################
// #################             SETUP I LOOP                         ###############
// ##################################################################################

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  // Start each software serial port
  portWagi.begin(9600);
  portDrukarki.begin(19200);
  portKarty.begin(9600);

  printer.begin();

  lcd.begin(16, 2);
  lcd.clear();

  portKarty.listen();

  rtc.start();

  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    Serial.flush();
    abort();
  }

  if (!rtc.isrunning()) {
    Serial.println(F("RTC is NOT running, let's set the time!"));
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2021, 5, 14, 0, 5, 0));
  }
}

void loop() {
  currentMillis = millis();
  randomSeed(currentMillis);
  if (stanGotowosci == true) {
    portKarty.listen();
    if (zmienLCD == true) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Zwaz prace, wpisz cene i zbliz karte"));
      lcd.setCursor(0, 1);
      lcd.print(F("Biskwit-10 Szkliwo-20/22 Na ostro-25"));
      ruszLCD = true;
      zmienLCD = false;
    }
    if (newData == true) {
      Serial.println(F("newData == true"));
      Serial.print(F("wiadomoscKarty = "));
      Serial.println(wiadomoscKarty);
      Serial.print(F("str karty przed: "));
      Serial.println(stringKarty);
      strncpy(stringKarty, wiadomoscKarty + 3, 7);
      Serial.print(F("str karty po: "));
      Serial.println(stringKarty);
      //Serial.println(substr(wiadomoscKarty, 2, 3));
      numerkarty = strtoul(stringKarty, NULL, 16);
      Serial.print(F("numer karty: "));
      Serial.println(numerkarty);
      indeks = znajdzIndeks(numerkarty);
      Serial.print(F("indeks = "));
      Serial.println(indeks);

      autoryzacja = EEPROM.read(indeks);
      if (autoryzacja == 255) {
        stanGotowosci = false;
        stanWazenia = true;
        zmienLCD = true;
        stanObliczen = false;
        stanDrukowania = false;
        newData = false;
      }
      else {
      Serial.println("numer klubowy zablokowany w EEPROM");      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F(" BLOKADA KONTA! "));
      lcd.setCursor(0, 1);
      lcd.print(F("  BRAK SRODKOW  "));
      delay(4000);
      receiveKarta();
      stanGotowosci = true;
      stanWazenia = false;
      zmienLCD = true;
      stanObliczen = false;
      stanDrukowania = false;
      newData = false;
      }
      
    } else {
      receiveKarta();
    }
  }
  if (ruszLCD == true) {
    if (currentMillis - previousMillis >= interval && wlewo == true) {
      lcd.scrollDisplayLeft();
      previousMillis = currentMillis;
    }
  }
  if (stanWazenia == true) {
    if(indeks >= length){
      Serial.println("nie znaleziono tego numeru karty");      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("BRAK KARTY      "));
      lcd.setCursor(0, 1);
      lcd.print(F("W SYSTEMIE      "));
      delay(4000);
      stanGotowosci = true;
      stanWazenia = false;
      stanObliczen = false;
      stanDrukowania = false;
    }
    else{
    portWagi.listen();
    if (zmienLCD == true) {
      lcd.clear();
      lcd.print(F("Poczekaj..."));
      lcd.setCursor(0, 1);
      lcd.print(F("nie zdejmuj prac"));
      dlugoscTekstu = 23;
      ruszLCD = false;
      zmienLCD = false;
    }
    if (newData == true) {
      Serial.println(wiadomoscWagi);
      stanGotowosci = false;
      stanWazenia = false;
      stanObliczen = true;
      stanDrukowania = false;

      newData = false;
    } else {
      receiveWaga();
    }
    }
  }
  if (stanObliczen == true) {
    ruszLCD = false;
    Serial.println(F("weszlo w stanObliczen"));

    strncpy(chindeks, wiadomoscWagi + 12, 3);
    blokindeks = atoi(chindeks);                           // numer klubowy wpisany na wadze

    strncpy(chmasa, wiadomoscWagi + 5, 5);
    Serial.print(F("chmasa = "));
    Serial.println(chmasa);
    //       chmasa = substr(wiadomoscWagi, 8, 6);        //masa
    strncpy(chcena, wiadomoscWagi + 13, 5);
    Serial.print(F("chcena = "));
    Serial.println(chcena);
    //       chcena = substr(wiadomoscWagi, 16, 3);       //cena
    strncpy(chnaleznosc, wiadomoscWagi + 21, 5);
    Serial.print(F("chnaleznosc = "));
    Serial.println(chnaleznosc);
    //       chnaleznosc = substr(wiadomoscWagi, 22, 8);  //naleznosc
    fmasa = atof(chmasa);
    Serial.print(F("fmasa = "));
    Serial.println(fmasa);
    icena = atoi(chcena);
    Serial.print(F("icena = "));
    Serial.println(icena);
    fnaleznosc = atof(chnaleznosc);
    if (indeks == 11) {
      fnaleznosc = transponuj(fnaleznosc);
    }
    else{
      fnaleznosc = zaokragl(fnaleznosc);
    }
    Serial.print(F("fnaleznosc = "));
    Serial.println(fnaleznosc);
    if (indeks == 190) {
      EEPROM.update(blokindeks, 0);
      lcd.clear();
      lcd.print(F("Blokowanie konta"));
      lcd.setCursor(0, 1);
      lcd.print(blokindeks);
      delay(4000);
      stanGotowosci = true;
      stanWazenia = false;
      stanObliczen = false;
      stanDrukowania = false;
      zmienLCD = true;
    } else if (indeks == 191) {
      EEPROM.update(blokindeks, 255);
      lcd.clear();
      lcd.print(F("Aktywacja konta "));
      lcd.setCursor(0, 1);
      lcd.print(blokindeks);
      delay(4000);
      stanGotowosci = true;
      stanWazenia = false;
      stanObliczen = false;
      stanDrukowania = false;
      zmienLCD = true;
    } else if (icena == cenaBiskwit) {
      strcpy(typWypalu, "BISKWIT   ");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Biskwit "));
      lcd.print(cenaBiskwit);
      lcd.print(F("PLN"));
      lcd.setCursor(0, 1);
      lcd.print(F("drukowanie..."));
      stanGotowosci = false;
      stanWazenia = false;
      stanObliczen = false;
      stanDrukowania = true;
    } else if (icena == cenaSzkliwo1) {
      strcpy(typWypalu, "SZKLIWO 22");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Szkliwo "));
      lcd.print(cenaSzkliwo1);
      lcd.print(F("PLN"));
      lcd.setCursor(0, 1);
      lcd.print(F("drukowanie..."));
      stanGotowosci = false;
      stanWazenia = false;
      stanObliczen = false;
      stanDrukowania = true;
    } else if (icena == cenaSzkliwo2) {
      strcpy(typWypalu, "SZKLIWO 20");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Szkliwo "));
      lcd.print(cenaSzkliwo2);
      lcd.print(F("PLN"));
      lcd.setCursor(0, 1);
      lcd.print(F("drukowanie..."));
      stanGotowosci = false;
      stanWazenia = false;
      stanObliczen = false;
      stanDrukowania = true;
    } else if (icena == cenaOstro) {
      strcpy(typWypalu, "NA OSTRO  ");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Na ostro "));
      lcd.print(cenaOstro);
      lcd.print(F("PLN"));
      lcd.setCursor(0, 1);
      lcd.print(F("drukowanie..."));
      stanGotowosci = false;
      stanWazenia = false;
      stanObliczen = false;
      stanDrukowania = true;
    } else if (indeks != 190 && indeks != 191 && icena != cenaBiskwit && icena != cenaSzkliwo1 && icena != cenaSzkliwo2 && icena !=cenaOstro) {
      lcd.clear();
      lcd.print(F("Bledna cena"));
      lcd.setCursor(0, 1);
      lcd.print(F("Wpisz ponownie"));
      delay(1000);
      stanGotowosci = false;
      stanWazenia = true;
      stanObliczen = false;
      stanDrukowania = false;
    }
    newData = false;
  }
  if (stanDrukowania == true) {
    if (indeks == 0){
      Serial.println("nie znaleziono tego numeru karty");      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("BRAK KARTY      "));
      lcd.setCursor(0, 1);
      lcd.print(F("W SYSTEMIE      "));
      delay(4000);
      stanGotowosci = true;
      stanWazenia = false;
      stanObliczen = false;
      stanDrukowania = false;
    } else{
    printer.wake();  // tutaj troche za pozno na rozgrzewanie drukarki
    printer.setDefault();
    printer.setHeatConfig(11, 220, 45);

    DateTime now = rtc.now();

    Serial.print(F("data_kod = "));
    char data_kod[] = "MMDD";
    Serial.println(now.toString(data_kod));

    // Serial.println(data_kod);

    char czasdruk[] = "hh:mm";
    now.toString(czasdruk);
    Serial.print(F("czasdruk = "));
    Serial.println(czasdruk);

    char datadruk[] = "DD.MM.YYYY";
    now.toString(datadruk);
    Serial.print(F("datadruk = "));
    Serial.println(datadruk);

    char masa_kod[6];
    dtostrf(fmasa, 5, 3, masa_kod);
    Serial.print(F("masa_kod: "));
    Serial.println(masa_kod);

    char kodkreskowy[15];

    int fnaleznosc_int = fnaleznosc;                        // Get the integer (678).
    float fnaleznosc_frac = fnaleznosc - fnaleznosc_int;    // Get fraction (0.0123).
    int fnaleznosc_intfrac = trunc(fnaleznosc_frac * 100);  // Turn into integer (123).

    snprintf(kodkreskowy, 15, "%s%03d%02d%02d%d", data_kod, indeks, fnaleznosc_int, fnaleznosc_intfrac, icena);
    Serial.print(F("kodkreskowy = "));
    Serial.println(kodkreskowy);

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
    if (indeks == 192){
      printer.println(F("KLUB SENIORA"));
    }
    else {
    printer.println(typWypalu);
    }
    printer.setSize('M');
    printer.doubleHeightOff();
    printer.print(indeks);
    printer.print(F(" "));
    if (indeks != 217){
      strcpy_P(imie, (char *)pgm_read_word(&(string_table[indeks])));
      printer.println(imie);
    }
    printer.boldOff();

    printer.print(fmasa);
    printer.print(F(" kg            "));
    printer.print(fnaleznosc);
    printer.println(F(" PLN"));

    printer.setSize('S');
    printer.print(czasdruk);
    printer.print(F("     "));
    printer.println(datadruk);
    Serial.print(F("indeks = "));
    Serial.println(indeks);
    // printer.justify('R');
    // printer.println(imie);
    printer.justify('L');


    printer.feed(3);
    
    lcd.clear();
    lcd.print(F("zabierz talon"));
    lcd.setCursor(0, 1);
    lcd.print(F("   "));
    delay(2000);

    printer.sleepAfter(1200);

    zmienLCD = true;
    stanGotowosci = true;
    stanWazenia = false;
    stanObliczen = false;
    stanDrukowania = false;
    indeks = 0;
    }
  }
}
