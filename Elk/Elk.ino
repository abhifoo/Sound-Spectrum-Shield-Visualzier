//For spectrum analyzer shield, these three pins are used.
//You can move pinds 4 and 5, but you must cut the trace on the shield and re-route from the 2 jumpers.
#define RESET 5
#define STROBE 4
#include <Adafruit_NeoPixel.h>  //Library to simplify interacting with the LED strip
#ifdef __AVR__
#include <avr/power.h>   //Includes the library for power reduction registers if your chip supports them. 
#endif                   //More info: http://www.nongnu.org/avr-libc/user-manual/group__avr__power.htlm

//Constants (change these as necessary)
#define LED_PIN   2  //Pin for the pixel strip. Can be analog or digital.
#define LED_TOTAL 50  //Change this to the number of LEDs in your strip.
#define SPEC 7
//int spectrumAnalog = 1; //0 for left channel, 1 for right.

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_TOTAL, LED_PIN, NEO_GRB + NEO_KHZ800);

// Spectrum analyzer read values will be kept here.
double Lmax[SPEC] = {300}, Rmax[SPEC] = {300};
float Lspectrum[SPEC], Rspectrum[SPEC];
float avgMag = 300;

int count = 600;
long unsigned int secs = 0;
void setup() {
  Serial.begin(9600);

  //Setup pins to drive the spectrum analyzer.
  pinMode(RESET, OUTPUT);
  pinMode(STROBE, OUTPUT);

  //Init spectrum analyzer
  digitalWrite(STROBE, LOW);    delay(1);
  digitalWrite(RESET, HIGH);    delay(1);
  digitalWrite(STROBE, HIGH);   delay(1);
  digitalWrite(STROBE, LOW);    delay(1);
  digitalWrite(RESET, LOW);     delay(5);
  // Reading the analyzer now will read the lowest frequency.

  strip.begin();
  strip.clear();
  strip.show();

  for (int j = 0; j < Rain(-1); j += 3) {
    for (int i = 1; i < strip.numPixels(); i++) {
      float val = double(Rain(-1)) * (float(i) / float(strip.numPixels() - 1));
      strip.setPixelColor(i, Rain(val + j));
    }
    strip.show();
  }
}

void loop() {
  showSpectrum();
  strip.show();
  fade(.925);

  if (round(fmod(millis(), 6)) == 0) count++;
  if (count > Rain(-1))  count = 0;

  //delay(20);
}

// Read 7 band equalizer.
void readSpectrum()
{
  // Band 0 = Lowest Frequencies.
  for (byte band = 0; band < SPEC; band++)  {
    Lspectrum[band] = (analogRead(0) + analogRead(0) ) >> 1; //Read twice and take the average by dividing by 2

    if (Lspectrum[band] > Lmax[band]) Lmax[band] = Lspectrum[band];

    Rspectrum[band] = (analogRead(1) + analogRead(1) ) >> 1;

    if (Rspectrum[band] > Rmax[band]) Rmax[band] = Rspectrum[band];

    digitalWrite(STROBE, HIGH);
    digitalWrite(STROBE, LOW);


  }
}

void showSpectrum() {
  readSpectrum();
  Lspectrum[SPEC - 1] = (Lspectrum[SPEC - 1] + Rspectrum[SPEC - 1]) / 2.0;
  Rspectrum[SPEC - 1] = Lspectrum[SPEC - 1];
  Lmax[SPEC - 1] = (Lmax[SPEC - 1] + Rmax[SPEC - 1]) / 2.0;
  Rmax[SPEC - 1] = Lmax[SPEC - 1];
  Lspectrum[0] = (Lspectrum[0] + Rspectrum[0]) / 2.0;
  Rspectrum[0] = Lspectrum[0];
  Lmax[0] = (Lmax[0] + Rmax[0]) / 2.0;
  Rmax[0] = Lmax[0];

  int jump =  round((float(strip.numPixels()) / 2.0) / float(SPEC));
  float aspec = 0, amax = 0;


  for (int i = 0; i < SPEC; i++) {

    aspec += pow((Lspectrum[i] + Rspectrum[i]) / 2.0, 2);
    amax += pow((Lmax[i] + Rmax[i]) / 2.0, 2);
    float val = double(Rain(-1)) * (float(i) / float(SPEC - 1));

    if (Lmax[i] > 200) {
      Lmax[i] -= .2;
      //if (i == 3) Serial.println(Lmax[i]);
    }
    else continue;
    if (Lspectrum[i] > 90) {
      double Lratio = Lspectrum[i] / Lmax[i];
      Lratio = pow(Lratio, 7.0);
      uint8_t Lpos = (i == 0) ? 5 : (i == 1) ? 1 : pos(5 - (i * jump));
      //Lpos = (Lpos+count)%(strip.numPixels()-1);
      //uint32_t Lcol = Rainbow(val + 600), Lcol2 = strip.getPixelColor(Lpos);
      uint32_t Lcol = Rain(val + count), Lcol2 = strip.getPixelColor(Lpos);

      uint8_t Lcols[3], Lcols2[3];
      for (int j = 0; j < 3; j++) {
        Lcols[j] = split(Lcol, j) * Lratio;
        Lcols2[j] = split(Lcol2, j);
      }

      float Lmag = sqrt( pow(Lcols[0], 2) + pow(Lcols[1], 2) + pow(Lcols[2], 2));
      float Lmag2 = sqrt( pow(Lcols2[0], 2) + pow(Lcols2[1], 2) + pow(Lcols2[2], 2));

      if ( Lmag > Lmag2 && Lmag > 30) {
        Lcol = strip.Color(Lcols[0], Lcols[1], Lcols[2]);
        strip.setPixelColor(Lpos, Lcol);
        bleed(Lpos, jump);
      }
    }

    if (Rmax[i] > 200) Rmax[i] -= .2;
    else continue;
    if (Rspectrum[i] > 90) {
      double Rratio = Rspectrum[i] / Rmax[i];
      Rratio = pow(Rratio, 7.0);
      uint8_t Rpos = (5 + (i * jump)) % strip.numPixels();
      //Rpos = (Rpos+count)%(strip.numPixels()-1);
      //uint32_t Rcol = Rainbow(val + 600), Rcol2 = strip.getPixelColor(Rpos);
      uint32_t Rcol = Rain(val + count), Rcol2 = strip.getPixelColor(Rpos);

      uint8_t Rcols[3], Rcols2[3];
      for (int j = 0; j < 3; j++) {
        Rcols[j] = split(Rcol, j) * Rratio;
        Rcols2[j] = split(Rcol2, j);
      }

      float Rmag = sqrt( pow(Rcols[0], 2) + pow(Rcols[1], 2) + pow(Rcols[2], 2));
      float Rmag2 = sqrt( pow(Rcols2[0], 2) + pow(Rcols2[1], 2) + pow(Rcols2[2], 2));

      if ( Rmag > Rmag2 && Rmag > 30) {
        Rcol = strip.Color(Rcols[0], Rcols[1], Rcols[2]);
        strip.setPixelColor(Rpos, Rcol);
        bleed(Rpos, jump);
      }
    }
  }

  aspec = sqrt(aspec); amax = sqrt(amax);
  float aratio = aspec / amax;
  if (aratio - avgMag >= .4) {
    jump /= 2.0;
    for (int i = 0; i < (SPEC - 1) * 2; i++) {
      uint8_t Lpos = pos(3 - (i * jump));
      uint8_t Rpos = (7 + (i * jump)) % strip.numPixels();
      uint8_t cols[3];
      for (int j = 0; j < 3; j++) {
        cols[j] = ((255 * aratio) + split(strip.getPixelColor(i), j)) / 2.0;
      }
      uint32_t col = strip.Color(cols[0], cols[1], cols[2]);
      strip.setPixelColor(Lpos, col);
      strip.setPixelColor(Rpos, col);
      bleed(Rpos, jump * 2);
      bleed(Lpos, jump * 2);
    }
  }
  avgMag = (avgMag + aratio) / 2.0;
  strip.setPixelColor(0, 0);
}

uint8_t split(uint32_t color, uint8_t i ) {   //0 = Red, 1 = Green, 2 = Blue
  if (i == 0) return color >> 16;
  if (i == 1) return color >> 8;
  if (i == 2) return color >> 0;
  return -1;
}

void fade(float damper) {
  for (int i = 0; i < strip.numPixels(); i++) {
    uint32_t col = strip.getPixelColor(i);
    if (col == 0) continue;
    float colors[3]; //Array of the three RGB values
    for (int j = 0; j < 3; j++) colors[j] = split(col, j) * damper;
    strip.setPixelColor(i, strip.Color(colors[0] , colors[1], colors[2]));
  }
}

void bleed(uint8_t Point, uint8_t radius) {
  for (int i = 1; i < radius; i++) {
    int sides[] = {pos(Point - i), pos(Point + i)};
    for (int i = 0; i < 2; i++) {
      int point = sides[i];
      uint32_t colors[] = {strip.getPixelColor(pos(point - 1)), strip.getPixelColor(point), strip.getPixelColor(pos(point + 1))  };
      strip.setPixelColor(point, strip.Color(
                            float( split(colors[0], 0) + split(colors[1], 0) + split(colors[2], 0) ) / 3.0,
                            float( split(colors[0], 1) + split(colors[1], 1) + split(colors[2], 1) ) / 3.0,
                            float( split(colors[0], 2) + split(colors[1], 2) + split(colors[2], 2) ) / 3.0)
                         );
    }
  }
}

uint8_t pos(int pos) {
  if (pos < 1) return (strip.numPixels() - 1) + pos;
  if (pos >= strip.numPixels()) return (pos % (strip.numPixels() - 1));
  return pos;
  //return (strip.numPixels()-1 + pos) % strip.numPixels();
}

/*
  uint32_t Rainbow(int i) {
  const unsigned int LOOP = 1785;
  if (i < 0) return LOOP;
  if (i > LOOP - 1) return Rainbow(i % LOOP);
  if (i > 1529) return strip.Color(255, 0, 255 - (i % 255));     //violet -> red
  if (i > 1274) return strip.Color((i % 255), 0, 255);           //blue -> violet
  if (i > 1019) return strip.Color(0, 255 - (i % 255), 255);     //aqua -> blue
  if (i > 764) return strip.Color(0, 255, (i % 255));            //green -> aqua
  if (i > 509) return strip.Color(255 - (i % 255), 255, 0);      //yellow -> green
  if (i > 255) return strip.Color(255, 128 + (i % 255) / 2, 0);  //orange -> yellow
  return strip.Color(255, i / 2, 0);                             //red -> orange
  }
*/

uint32_t Rain(int i) {
  const unsigned int LOOP = 1020;
  if (i < 0) return LOOP;
  if (i > LOOP - 1) return Rain(i % LOOP);
  if (i > 764) return strip.Color(i % 255, 0, 255 - (i % 255));               //blue -> red
  if (i > 509) return strip.Color(0, 255 - (i % 255), i % 255);               //green -> blue
  if (i > 255) return strip.Color(255 - (i % 255), 128 + (i % 255) / 2, 0);   //orange ->green
  return strip.Color(255, i / 2, 0);                                          //red -> orange

}

uint32_t Sunset(int i) {
  /*
    const unsigned int LOOP = 875;
    if (i < 0) return LOOP;
    if (i > LOOP - 1) return Sunset(i % LOOP);
    if (i > 764) return strip.Color(i % 255, 0, 255);                        //blue -> red
    if (i > 509) return strip.Color(255 - (i % 255), 0, 255);                //purple -> blue
    if (i > 255) return strip.Color(255, 128 - (i % 255) / 2, (i % 255));    //orange -> purple
    return strip.Color(255, i / 2, 0);                                       //red -> orange
  */

  const unsigned int LOOP = 765;
  if (i < 0) return LOOP;
  if (i > LOOP - 1) return Rain(i % LOOP);
  if (i > 509) return strip.Color(i % 255, 0, 255 - (i % 255));                     //blue -> red
  if (i > 255) return strip.Color(255 - (i % 255), 128 - (i % 255) / 2, i % 255);   //orange ->blue
  return strip.Color(255, i / 2, 0);                                                //red -> orange
}
