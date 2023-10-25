# amfora-giga

## amfora.ino

Stara wersja, działająca na Arduino Pro Mini. Podłączona drukarka talonów UART [Adafruit Mini Thermal Receipt Printer](https://learn.adafruit.com/mini-thermal-receipt-printer?view=all), waga przez konwerter RS232-> UART i czytnik kart RFID [seeedstudio RFID reader](https://www.seeedstudio.com/Grove-125KHz-RFID-Reader.html). Numery kart i lista imion wgrywana jest w pamięci programu, do update'u potrzebne jest skompilowanie programu jeszcze raz i wgranie na mikrokontroler. Urządzenie odczytuje numer karty użytkownika, pobiera masę i cenę z wagi, datę i czas z zegara, generuje kod kreskowy i drukuje talon. Talon ląduje w pracy, przy wkładaniu do pieca jest zbierany a potem Robert skanuje kod i wrzuca do excela (google sheets), który rozkodowuje kod kreskowy i podlicza wszystkim wypały. Są też karty "funkcyjne" do na przykład blokowania kont, wtedy if (numer karty = cośtam) to zapisz w EEPROM wartość, że ktoś jest zablokowany.

## amfora-giga.ino

Nowa wersja, przeniosłem większość funkcjonalności. Ma działać na Arduino Giga R1 WiFi. Tym razem oprócz drukarki, czytnika kart i połączenia z wagą, planowany jest czytnik kart SD do wgrywania plików. Może uda się w danych na karcie zapisywać stan konta i odliczać na bieżąco kasę za konkretną pracę, wtedy od razu widać przy ważeniu jaki ma się stan konta itd. W wersji wymarzonej dane synchronizowane są z czymś, może z bazą danych Firebase, i możliwa jest obsługa systemu też przez jakąś apkę. Zegar RTC jest wbudowany w mikrokontroler, jest kilka hardłerowych portów UART zamiast SoftwareSerial jak w starej wersji, od razu jest WiFi, dwa rdzenie procesora - więc można jednocześnie uruchamiać program do obsługi wagi i drugi program synchronizujący dane z chmurą, żeby nie blokować podstawowej funkcjonalności. Jest jeszcze port USB HID, więc można podłączyć klawiaturę na przykład. Planowany ekran to jakiś epaper (już jest kupiony), ale coś czuję, że z tym będzie sporo problemów...

test codeapp