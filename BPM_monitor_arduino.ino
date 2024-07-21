#include <SPI.h>
#include <Wire.h>//i2c communication
#include <Adafruit_SSD1306.h>//oled display
#include "MAX30105.h"//heart rate sensor
#include "heartRate.h"

//_______________________Oled display
const int x = 128;
const int y = 64;
const int r = -1;

Adafruit_SSD1306 display(x, y, &Wire, r);

// heart icon
const unsigned char heartRateIcon [] PROGMEM = {
	0x1c, 0x38, 0x3e, 0x7c, 0x63, 0xc6, 0xc1, 0x83, 0xc1, 0x81, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 
	0xc0, 0x03, 0x60, 0x06, 0x30, 0x0c, 0x18, 0x18, 0x0c, 0x30, 0x06, 0x60, 0x03, 0xc0, 0x01, 0x80
};
void createPixel() {
  // display bitmap
  display.drawBitmap(0, 0, heartRateIcon, 16, 16, WHITE);
}

//_______________________Heart rate sensor
MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

const int buffor_dla_bpm = 4;
char buffor_int_to_string[buffor_dla_bpm];//do przechowywania bpm maksymalny rozmiar liczby to 3 plus znak \0 na końcu

int buffor_size = 30;
void setup() {
  Serial. begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 aallocation failed"));
    for(;;);    //Dont proceed. loop forever
  }
  display.clearDisplay();

  for (int i = 0; i < 128; i++)
  {
    display.drawPixel(i, 15, WHITE);
    display.drawPixel(i, 0, WHITE);
  }

  display.drawRect(0, 0, 128, 64, WHITE);
  wyswietl("BPM", 1, 57, 4, 0, 0);  //funkcja wyświetlająca napis BPM
  display.drawBitmap(10, 30, heartRateIcon, 16, 16, WHITE); //wyswietla ikonkę serca
  display.display();


//_______________________Heart rate sensor
// Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    for(;;);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void intToString (int liczba, char* buffor)
{
  int i = 0;

  //ilczby które podaję zawsze są dodatnie, ni emartwię się znakiem minus
  if(liczba == 0)
  {
    buffor[i++] = '0';
  }
  else
  {
    while (liczba > 0)
    {
      buffor[i++] = (liczba % 10) + '0';
      liczba = liczba / 10;
    }
  }
  buffor[i] = '\0';

  //odracanie liczby
  for (int j = 0; j < i/2; j++)
  {
    char temp = buffor[j];
    buffor[j] = buffor[i - j - 1];
    buffor[i - j - 1] = temp;
  }
}

void wyswietl(char* wyraz, int rozmiarCzcionki, int x, int y, int a, int b)
{
  display.fillRect(x, y, a, b, BLACK);
  display.setCursor(x, y);
  display.setTextSize(rozmiarCzcionki);
  display.setTextColor(WHITE);
  display.println(wyraz);
  display.display();
}

void loop() {
  //_______________________Heart rate sensor
    long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.println(beatAvg);

  if (irValue < 50000)
  {
    Serial.print(" No finger?");
    //wyswietl("Nie wykryto", 1, 40, 30, 10, 67);
  }

    //wyswietlanie wyniku na ekranie
    intToString(beatAvg, buffor_int_to_string);
    Serial.println(buffor_int_to_string);
    wyswietl(buffor_int_to_string, 2, 64, 30, 30, 30);

}
