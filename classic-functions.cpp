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

float zaokragl(float true_fnaleznosc) {
  float nowa_fnaleznosc = round(true_fnaleznosc);
  if (nowa_fnaleznosc <= cena_minimalna){
    nowa_fnaleznosc = cena_minimalna;
  }
  return nowa_fnaleznosc;
}

void checkOrCreateFiringsListCSV(const char* currentDate) {
  // Check if the SD card is available
  if (!SD.begin(10)) {
    Serial.println("SD card initialization failed");

    ErrorType error = SDError;
    errorHandling(error);
    return;
  }

  // Construct the filename based on the current date
  char filename[11];
  sprintf(filename, "%s.csv", currentDate);

  // Check if the file exists
  if (SD.exists(filename)) {
    // File already exists
    Serial.println("CSV file already exists.");
    return;
  }

  // If the file doesn't exist, create it
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    // File created successfully
    Serial.println("CSV file created.");

    // Write header to the file
    dataFile.println(firingsListFileHeader);
    dataFile.close();
  } else {
    // Error creating file
    Serial.println("Error creating CSV file.");

    ErrorType error = createFileError;
    errorHandling(error);
  }
}

void errorHandling(int errorType) {
  switch (errorType) {
    case SDError:
      Serial.println("SD card error");
      break;
    case createFileError:
      Serial.println("Error creating file");
      break;
    default:
      Serial.println("Unknown error");
      break;
  }
}
