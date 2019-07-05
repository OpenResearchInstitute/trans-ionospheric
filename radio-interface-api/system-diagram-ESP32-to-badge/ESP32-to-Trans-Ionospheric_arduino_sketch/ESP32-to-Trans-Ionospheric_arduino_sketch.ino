#include "WiFi.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_NeoPixel.h>

// TFT Display Pins
#define TFT_CS    19
#define TFT_DC    22
#define TFT_MOSI  23
#define TFT_CLK   26
#define TFT_RST   21
#define TFT_MISO  25

// NeoPixel Values
#define PIXELPIN   5
#define NUMPIXELS  5
#define pixlux    20  //saturation level for NeoPixels colors

// Audio Buzzer Values
const int buzzerPin = 18;
const int f = 349;
const int gS = 415;
const int a = 440;
const int cH = 523;
const int eH = 659;
const int fH = 698;
const int e6 = 1319;
const int g6 = 1568;
const int a6 = 1760;
const int as6 = 1865;
const int b6 = 1976;
const int c7 = 2093;
const int d7 = 2349;
const int e7 = 2637;
const int f7 = 2794;
const int g7 = 3136;

int muted = true;
int mute_touched = false;
char ssid[]="HackerBoxer_abraxas3d";  //put your handle after the underscore
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXELPIN, NEO_GRB + NEO_KHZ800);
char hackers_found[13][70];
int next_hacker_found = 0;

void setup() 
{
  tft.begin(9341);
  tft.setRotation(3); // rotate 3*(pi/2)
  pixels.begin();
  pinMode(buzzerPin, OUTPUT);
  for (int i; i<13; i++)
    hackers_found[i][0] = 0; //empty array of strings
   touchAttachInterrupt(15, mutebutton, 40);  //threshold 40
}

void loop() 
{
  // cycle some NeoPixel Rainbows
  RainbowLEDcycle(18);
  // scan other SSIDs
  wifiScan2LCD();
  // start broadcating SSID (AP on)
  //(ssid, password, channel, hidden if true, max connection 1 to 8 default 4)
  WiFi.softAP(ssid, NULL, 1, 0, 1);
  // Play Mario Theme on Buzzer
  mute_handler();
  if (!muted)
    MarioTheme();
  // chill here for a while
  delay(5000);
  //diplay list of found hackers tagged
  found2LCD();
  // cycle some NeoPixel Blues
  BlueLEDcycle(18);
  // Play Imperial March on Buzzer
  mute_handler();
  if (!muted)
    ImperialMarch();
  // chill here for a while
  delay(5000);
  // stop broadcating SSID (AP off)  
  WiFi.softAPdisconnect(1);
}

void wifiScan2LCD() 
{
  int netsfound;
  int displaylines=13;
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(4);
  tft.println(" HACKER BOXES");
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(2);

  while (displaylines > 0)
  {
    netsfound = WiFi.scanNetworks();
    if (netsfound==0)
    {
      tft.println(". . .");
      displaylines--;
    }
    for (int i = 0; i < netsfound; ++i) 
    {
      if (WiFi.SSID(i).startsWith("HackerBoxer"))
      {
        WiFi.SSID(i).toCharArray(hackers_found[next_hacker_found],70);
        hackers_found[next_hacker_found][25] = 0;  //truncate string
        next_hacker_found++; 
        if (next_hacker_found == 13)
          next_hacker_found = 0;
      }
      else
      {
        // Print SSID and RSSI for each network found
        tft.print(" [");
        tft.print(WiFi.RSSI(i));
        tft.print("] ");
        tft.print(WiFi.SSID(i).substring(0,17));
        tft.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
        delay(50);
        displaylines--;
      }
    }
    delay(3000); // Wait before scanning again
  }
}

void found2LCD() 
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(4);
  tft.println(" IDs TAGGED");
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(2);
  for (int i=0; i<13; i++)
  {
    tft.print(" ");
    tft.println(hackers_found[i]+12);
  }
}

void RainbowLEDcycle(int cycles)
{
  int i=0;
  while(cycles) 
  { 
    pixels.setPixelColor(i, pixels.Color(pixlux,0,0));
    i = (i==4) ? 0 : (i+1);
    pixels.setPixelColor(i, pixels.Color(pixlux,pixlux,0));
    i = (i==4) ? 0 : (i+1);
    pixels.setPixelColor(i, pixels.Color(0,pixlux,0));
    i = (i==4) ? 0 : (i+1);
    pixels.setPixelColor(i, pixels.Color(0,0,pixlux));
    i = (i==4) ? 0 : (i+1);
    pixels.setPixelColor(i, pixels.Color(pixlux,0,pixlux));
    i = (i==4) ? 0 : (i+1);
    i = (i==4) ? 0 : (i+1);
    pixels.show();
    delay(150);
    cycles--;
  }
}

void BlueLEDcycle(int cycles)
{
  int i=0;
  while(cycles) 
  { 
    pixels.setPixelColor(i, pixels.Color(0,0,pixlux*2));
    i = (i==4) ? 0 : (i+1);
    pixels.setPixelColor(i, pixels.Color(0,0,pixlux/2));
    i = (i==4) ? 0 : (i+1);
    pixels.setPixelColor(i, pixels.Color(0,0,pixlux/2));
    i = (i==4) ? 0 : (i+1);
    pixels.setPixelColor(i, pixels.Color(0,0,pixlux/2));
    i = (i==4) ? 0 : (i+1);
    pixels.setPixelColor(i, pixels.Color(0,0,pixlux/2));
    i = (i==4) ? 0 : (i+1);
    i = (i==4) ? 0 : (i+1);
    pixels.show();
    delay(150);
    cycles--;
  }
}

void beep(int tone, int duration)
{
  for (long i = 0; i < duration * 900L; i += tone * 1)
  {
    digitalWrite(buzzerPin, HIGH);
    delayMicroseconds(tone*(.50));
    digitalWrite(buzzerPin, LOW);
    delayMicroseconds(tone*(.50));
  }
  delay(30);
}
 
void  ImperialMarch()
{
  beep(a, 500);
  beep(a, 500);    
  beep(a, 500);
  beep(f, 350);
  beep(cH, 150);  
  beep(a, 500);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 650);
  delay(500);
  beep(eH, 500);
  beep(eH, 500);
  beep(eH, 500);  
  beep(fH, 350);
  beep(cH, 150);
  beep(gS, 500);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 650);
}

void  MarioTheme()
{
  beep(e7,150);
  beep(e7,150);
  delay(150);
  beep(e7,150);  
  delay(150);
  beep(c7,150);
  beep(e7,150);
  delay(150);
  beep(g7,150);
  delay(450);
  beep(g6,150);
  delay(450);
  beep(c7,150);
  delay(300);
  beep(g6,150);
  delay(300);
  beep(e6,150);
  delay(300);
  beep(a6,150);
  delay(150);
  beep(b6,150);
  delay(150);
  beep(as6,150);
  beep(a6,150);
  delay(150);
  beep(g6,112);
  beep(e7,112); 
  beep(g7,112);
  beep(a6,150);
  delay(150);
  beep(f7,150);
  beep(g7,150);
  delay(150);
  beep(e7,150);
  delay(150); 
  beep(c7,150);
  beep(d7,150);
  beep(b6,150);
}

void mutebutton()
{
  mute_touched = true;
}

void mute_handler()
{
  if (mute_touched)
  {
    if (muted)
    {
      muted = false;
    }
    else
    {
      muted = true;
    }
    mute_touched = false;
  }
}


