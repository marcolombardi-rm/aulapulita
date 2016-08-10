String ver = "AULAPULITA v0.1a";

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>

#include "DHT.h"
#define DHTPIN 2     // what digital pin we"re connected to
#define DHTTYPE DHT22   // DHT 22

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

RTC_PCF8523 rtc;

DHT dht(DHTPIN, DHTTYPE);

// include the SD library:
#include <SPI.h>
#include <SD.h>

// set up variables using the SD utility library functions:
char filename[] = "00000000.CSV";

void getFileName(){
  DateTime now = rtc.now();
  sprintf(filename, "%02d%02d%02d.CSV", now.year(), now.month(), now.day());
}

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
const int chipSelect = 4;

int CO2ppm = 0;
int CO2alarm = 0;

String devstatus[5];

void setup () {
  Serial.begin(9600);
  
  Serial.println(ver);
  
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW);
  pinMode(A1, OUTPUT);
  digitalWrite(A1, LOW);
  pinMode(4, OUTPUT);
  digitalWrite(4,1);

  // initialize the LCD
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print(ver);

  Serial.println("ready to go!");

  delay(10000);

  dht.begin();

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("!SDC don't work!");
    lcd.setCursor(0,0);
    lcd.print("!SDC don't work!");
    devstatus[0] = "SDC: KO";
    digitalWrite(A0, HIGH);
    delay(1000);
  }
  Serial.println("card initialized.");
  devstatus[0] = "SDC: OK";
  CO2alarm = CO2limit();
  Serial.print("co2limit set to:");
  Serial.println(CO2alarm);
}

void loop () {
  
    if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      while (1);
    }

    if (! rtc.initialized()) {
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
    
    DateTime now = rtc.now();
    
    if (now.month() == 165 || now.day() == 165 || now.hour() == 165 || now.minute() == 165 || now.second() == 165) {
      Serial.println("!RTC don't work!");
      lcd.setCursor(0,1);
      lcd.print("!RTC don't work!");
      devstatus[1] = "RTC: KO";
      digitalWrite(A1, HIGH);
      delay(1000);
    } else {
      devstatus[1] = "RTC: OK";
      if (devstatus[2] == "DHT: OK" && devstatus[3] == "K30: OK") {
        digitalWrite(A1, LOW);
      }
    }
    
    getFileName();
    File file = SD.open(filename, FILE_WRITE);

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds "old" (its a very slow sensor)
    int h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    int t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    int f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (h == 0) {
      Serial.println("!DHT don't work!");
      lcd.setCursor(0,0);
      lcd.print("!DHT don't work!");
      devstatus[2] = "DHT: KO";
      digitalWrite(A1, HIGH);
      delay(1000);
    } else {
      devstatus[2] = "DHT: OK";
      if (devstatus[1] == "RTC: OK" && devstatus[3] == "K30: OK") {
        digitalWrite(A1, LOW);
      }
    }

    // Get CO2 value from sensor
    CO2ppm = GetCO2(0x68); // default address for K-30 CO2 sensor is 0x68
    // account for dropped values
    if (CO2ppm > 0) {
      devstatus[3] = "K30: OK";
      if (devstatus[1] == "RTC: OK" && devstatus[2] == "DHT: OK") {
        digitalWrite(A1, LOW);
      }
      if (CO2ppm >= CO2alarm ){
        tone(8, 1000, 1000);
      } else if (CO2ppm < CO2alarm ) {
        noTone(8);
      }
    } else {
      Serial.println("!K30 don't work!");
      lcd.setCursor(0,0);
      lcd.print("!K30 don't work!");
      devstatus[3] = "K30: KO";
      digitalWrite(A1, HIGH);
      delay(1000);
      return;
    }

    //serial monitor
    Serial.print("T:");
    Serial.print(t);
    Serial.print("*C");
    Serial.print(" ");
    Serial.print("H:");
    Serial.print(h);
    Serial.print("%");
    Serial.print(" ");
    Serial.print("CO2:");
    Serial.print(CO2ppm);
    Serial.print("ppm");
    Serial.print(" ");
    Serial.print(now.year(), DEC);
    Serial.print("/");
    if (now.month()<10){
      Serial.print("0");
      Serial.print(now.month(), DEC);
    }
    else if (now.month()>=10){
      Serial.print(now.month(), DEC);
    }
    Serial.print("/");
    if (now.day()<10){
      Serial.print("0");
      Serial.print(now.day(), DEC);
    }
    else if (now.day()>=10){
      Serial.print(now.day(), DEC);
    }
    Serial.print(" ");
    if (now.hour()<10){
      Serial.print("0");
      Serial.print(now.hour(), DEC);
    }
    else if (now.hour()>=10){
      Serial.print(now.hour(), DEC);
    }
    Serial.print(":");
    if (now.minute()<10){
      Serial.print("0");
      Serial.print(now.minute(), DEC);
    }
    else if (now.minute()>=10){
      Serial.print(now.minute(), DEC);
    }
    Serial.print(":");
    if (now.second()<10){
      Serial.print("0");
      Serial.print(now.second(), DEC);
    }
    else if (now.second()>=10){
      Serial.print(now.second(), DEC);
    }
    Serial.println();
    //lcd monitor
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(t);
    lcd.setCursor(2,0);
    lcd.print((char)223);
    lcd.setCursor(3,0);
    lcd.print("C");
    lcd.setCursor(4,0);
    lcd.print("-");
    lcd.setCursor(5,0);
    lcd.print(h);
    lcd.setCursor(7,0);
    lcd.print("%");
    lcd.setCursor(8,0);
    lcd.print("-");
    lcd.setCursor(9,0);
    lcd.print(CO2ppm);
    lcd.setCursor(13,0);
    lcd.print("ppm");
    
    lcd.setCursor(0,1);
    lcd.print(now.year(), DEC);
    lcd.setCursor(4,1);
    lcd.print("/");
    if (now.month()<10){
      lcd.setCursor(5,1);
      lcd.print("0"); 
      lcd.setCursor(6,1);
      lcd.print(now.month(), DEC);
    }
    else if (now.month()>=10){
      lcd.setCursor(5,1);
      lcd.print(now.month(), DEC);
    }
    lcd.setCursor(7,1);
    lcd.print("/");
    if (now.day()<10){
      lcd.setCursor(8,1);
      lcd.print("0");
      lcd.setCursor(9,1);
      lcd.print(now.day(), DEC);
    }
    else if (now.day()>=10){
      lcd.setCursor(8,1);
      lcd.print(now.day(), DEC);
    }
    lcd.setCursor(10,1);
    lcd.print("-");
    if (now.hour()<10){
      lcd.setCursor(11,1);
      lcd.print("0");
      lcd.setCursor(12,1);
      lcd.print(now.hour(), DEC);
    }
    else if (now.hour()>=10){
      lcd.setCursor(11,1);
      lcd.print(now.hour(), DEC);
    }
    lcd.setCursor(13,1);
    lcd.print(":");
    if (now.minute()<10){
      lcd.setCursor(14,1);
      lcd.print("0");
      lcd.setCursor(15,1);
      lcd.print(now.minute(), DEC);
    }
    else if (now.minute()>=10){
      lcd.setCursor(14,1);
      lcd.print(now.minute(), DEC);
    }
    //sd datalogger
    file.print(t);
    file.print(",");
    file.print(h);
    file.print(",");
    file.print(CO2ppm);
    file.print(",");
    file.print(now.year(), DEC);
    file.print("/");
    if (now.month()<10){
      file.print("0");
      file.print(now.month(), DEC);
    }
    else if (now.month()>=10){
      file.print(now.month(), DEC);
    }
    file.print("/");
    if (now.day()<10){
      file.print("0");
      file.print(now.day(), DEC);
    }
    else if (now.day()>=10){
      file.print(now.day(), DEC);
    }
    file.print(",");
    if (now.hour()<10){
      file.print("0");
      file.print(now.hour(), DEC);
    }
    else if (now.hour()>=10){
      file.print(now.hour(), DEC);
    }
    file.print(":"); 
    if (now.minute()<10){
      file.print("0");
      file.print(now.minute(), DEC);
    }
    else if (now.minute()>=10){
      file.print(now.minute(), DEC);
    }
    file.print(":"); 
    if (now.second()<10){
      file.print("0");
      file.print(now.second(), DEC);
    }
    else if (now.second()>=10){
      file.print(now.second(), DEC);
    }
    file.print("\n");
    file.close();
    
    delay(10000);
}

// Routine that read the DIP Switch configuration
int bin2int(int numvalues, ...)
{
  int total = 0;
  va_list values;
  va_start(values, numvalues);

  for (; numvalues > 0; numvalues--)
    if (!(analogRead(va_arg(values, int))) )
      total += powint(2, numvalues - 1);

     va_end(values);
    return total;
}

int powint(int base, int exponent)
  {
    int result = 1;
    for (; exponent > 0; exponent--)
      result *= base;
    return result;
  }

int GetCO2(int address)
{
  byte recieved[4] = {0,0,0,0}; // create an array to store bytes received from sensor
  
  Wire.beginTransmission(address);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x08);
  Wire.write(0x2A);
  Wire.endTransmission();
  delay(20); // give delay to ensure transmission is complete
  
  Wire.requestFrom(address,4);
  delay(10);
  
  byte i=0;
  while(Wire.available())
  {
    recieved[i] = Wire.read();
    i++;
  }
  
  byte checkSum = recieved[0] + recieved[1] + recieved[2];
  CO2ppm = (recieved[1] << 8) + recieved[2];
  
  if(checkSum == recieved[3])
    return CO2ppm;
  else
    return -1;
}  

int CO2limit()
{
  int config = 0;
  int limit = 0;

  config = bin2int(2, A2, A3);
  // Check each available configuration and play
  // Here I check the result as binary to make it easier to understand
  if (config == B00)
    limit = 0;
  if (config == B01)
    limit = 1000;
  if (config == B11)
    limit = 1250;
  if (config == B10)
    limit = 1500;

  return limit;
}
