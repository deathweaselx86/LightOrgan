#ifndef _LEDSEGS_H
#define _LEDSEGS_H

#if ARDUINO >= 100
 #include "Arduino.h"
 #include "Print.h"
#else
 #include "WProgram.h"
#endif

#include "Adafruit_NeoPixel.h"

//Total spectrum analyzer shield bands and max value for a band read. Do not change this.
const short cSegNumBands=7;

//Frequency band index values bit values. Bit 0 (0x1) is the lowest freq., bit 7 (0x40) is highest.
//These are fixed by the spectrum analyzer chip used on the shield, and the number of bands cannot be changed.
//For most applications it is recommended not to use bands 1 and 7.

const short cSegBand1 = 0x01;  //63Hz center - I recommend omitting this band in most applications
const short cSegBand2 = 0x02;  //160Hz
const short cSegBand3 = 0x04;  //400Hz
const short cSegBand4 = 0x08;  //1.0KHz
const short cSegBand5 = 0x10;  //2.5KHz
const short cSegBand6 = 0x20;  //6.25KHz - Think about omitting this (6KHz is a pretty high "audible" freq.)
const short cSegBand7 = 0x40;  //16KHz - I REALLY recommend omitting this one, just noise energy.

//Software gain control constants. This provides a simple 'fast attack'/'slow decay' AGC for the input.
//InitialMax is the lowest spectrum band value to which AGC processing will apply. (AGC is applied
//separately to each of the seven spectrum bands.) As each sample is read, a fixed, assumed noise
//value is subtracted, then the max value seen for that channel is updated, but only when it is above MaxBandValue.
//Then the sampled value is scaled to the range (0..MaxSegmentLevel) using the actual max seen for that channel
//so far. The decay constant is subtracted from each channel's max on each display cycle. This sample normalization
//is applied before any custom display routine is called.

const static short cInitialMaxBandValue = 200; //Lowest max value allowed (0..1023) Normalized, after noise deduction.
const static short cMaxBandValueDecay = 2;     //Subtracted from detected max on each sample cycle
const short cMaxSegmentLevel = 1023;           //Normalized max sample value coming out of MapBandsToSegments()

//LEDSegs Segment actions. See DefineSegment and SetSegment_Action.

const short cSegActionNone = 0;        //Do nothing (undefined or do-nothing segment)
const short cSegActionFromBottom = 1;  //Fill LEDs from the first LED up, based on the current averaged band level vs the max possible
const short cSegActionFromTop = 2;     //Fill LEDs from the last down
const short cSegActionFromMiddle = 3;  //Fill LEDs from the middle out
const short cSegActionStatic = 4;      //Fill all LEDs in segment. Do not look at spectrum value.
const short cSegActionRandom = 5;      //Illuminate foreground color randomly throughout the segment

//Segment Options

const short cSegOptNoOffOverwrite = 0x01;
const short cSegOptModulateSegment = 0x02;
const short cSegOptInvertLevel = 0x04;

//Max # of segments that can be defined for a strip. Segments are "written" to the strip in index order.
//So higher-index segments can overwrite part or all of an lower-index segment.

#ifndef cMaxSegments
  #define cMaxSegments 100
#endif

//The prototype for a pointer to a segment display customizing routine that can be defined for any segme
//and will be called just before each strip refresh

typedef void (*SegmentDisplayRoutine) (short iSegment);

//Our LED strip class.

class LEDSegs {
  
  public:

    //Constructor and destructor
    LEDSegs(short nLEDs, short ledType) {LEDSegsInit(nLEDs, 6, ledType);}  //Constructor with default data
    LEDSegs(short nLEDs, short pinData, short ledType) {LEDSegsInit(nLEDs, pinData, ledType);}  //Constructor with explicit data
    ~LEDSegs() {delete[] objPxlStrip;}
    void LEDSegsInit(short, short, short);  //Common constructor code
    
    void DisplaySpectrum(bool, bool);
    void ResetStrip();
    void ResetRandom();
    
    void SetSegmentIndex(short Idx) {segCurrentIndex = constrain(Idx, 0, cMaxSegments - 1);}
    short GetSegmentIndex() {return segCurrentIndex;}

    //The SetSegment_xxx routines are overloaded. The segment # parameter can be omitted and defaults to the current index
    
    void SetSegment_Action(short nSegment, short Action) {if (Action >= 0) {SegmentData[nSegment].segAction = Action;};}
    void SetSegment_Action(short Action) {SetSegment_Action(segCurrentIndex, Action);}
    void SetSegment_BackColor(short nSegment, uint32_t BackColor) {if (BackColor != 0xFFFFFFFF) {SegmentData[nSegment].segBackColor = BackColor;};}
    void SetSegment_BackColor(uint32_t BackColor) {SetSegment_BackColor(segCurrentIndex, BackColor);}
    void SetSegment_Bands(short nSegment, short Bands) {if (Bands >= 0) {SegmentData[nSegment].segBands = Bands;};}
    void SetSegment_Bands(short Bands) {SetSegment_Bands(segCurrentIndex, Bands);}
    void SetSegment_DisplayRoutine(short nSegment, SegmentDisplayRoutine Routine) {SegmentData[nSegment].segDisplayRoutine = *Routine;}
    void SetSegment_DisplayRoutine(SegmentDisplayRoutine Routine) {SetSegment_DisplayRoutine(segCurrentIndex, Routine);}
    void SetSegment_FirstLED(short nSegment, short FirstLED) {if (FirstLED >= 0) {SegmentData[nSegment].segFirstLED = FirstLED;};}
    void SetSegment_FirstLED(short FirstLED) {SetSegment_FirstLED(segCurrentIndex, FirstLED);}
    void SetSegment_ForeColor(short nSegment, uint32_t ForeColor) {if (ForeColor != 0xFFFFFFFF) {SegmentData[nSegment].segForeColor = ForeColor;};}
    void SetSegment_ForeColor(uint32_t ForeColor) {SetSegment_ForeColor(segCurrentIndex, ForeColor);}
    void SetSegment_Level(short nSegment, short level) {SegmentData[nSegment].segLevel = level;}
    void SetSegment_Level(short level) {SetSegment_Level(segCurrentIndex, level);}
    void SetSegment_NumLEDs(short nSegment, short nLEDs) {if (nLEDs >= 0) {SegmentData[nSegment].segNumLEDs = nLEDs;};}
    void SetSegment_NumLEDs(short nLEDs) {SetSegment_NumLEDs(segCurrentIndex, nLEDs);}
    void SetSegment_Options(short nSegment, short Options) {if (Options >= 0) {SegmentData[nSegment].segOptions = Options;};}
    void SetSegment_Options(short Options) {SetSegment_Options(segCurrentIndex, Options);}
    void SetSegment_Spacing(short nSegment, short Spacing) {if (Spacing >= 0) {SegmentData[nSegment].segSpacing = Spacing;};}
    void SetSegment_Spacing(short Spacing) {SetSegment_Spacing(segCurrentIndex, Spacing);}

    short    GetSegment_Action(short nSegment)    {return SegmentData[nSegment].segAction;}
    uint32_t GetSegment_BackColor(short nSegment) {return SegmentData[nSegment].segBackColor;}
    short    GetSegment_Bands(short nSegment)     {return SegmentData[nSegment].segBands;}
    short    GetSegment_FirstLED(short nSegment)  {return SegmentData[nSegment].segFirstLED;}
    uint32_t GetSegment_ForeColor(short nSegment) {return SegmentData[nSegment].segForeColor;}
    short    GetSegment_Level(short nSegment)     {return SegmentData[nSegment].segLevel;}
    short    GetSegment_NumLEDs(short nSegment)   {return SegmentData[nSegment].segNumLEDs;}
    short    GetSegment_Options(short nSegment)   {return SegmentData[nSegment].segOptions;}
    short    GetSegment_Spacing(short nSegment)   {return SegmentData[nSegment].segSpacing;}

    //Initialize a new segment and return the index # of the segment defined.
    //You can set spectrum bands to -1 to include all bands, or 0 to not modulate according to audio level at all
    short DefineSegment(
        short     /* First LED in segment (0 origin) */
      , short     /* # LEDs in segment*/
      , short     /* cSegActionXXX value - defines how the segment works */
      , uint32_t  /* Foreground color */
      , short     /* Bitmask of cSegBandN spectrum band specs, to be averaged together to make this segment's value */
    );
 
    //Methods that match Adafruit_NeoPixel member functions, except declared static and does not set the high bit.  Return value is packed RGB value in long int.
    static uint32_t Color(byte r, byte g, byte b) {
      //return ((uint32_t)(g) << 16) | ((uint32_t)(r) <<  8) | b;
      return ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | b;
    }
    
    //Get the r/g/b components of a color into byte values (remember value is GRB, not RGB), return array [0..2]
    //is ordered RGB
    static void Colorvals(uint32_t Color, byte rgbvals[]) {
      rgbvals[0] = ((Color >> 16) & 0x7F);
      rgbvals[1] = ((Color >> 8) & 0x7F);
      rgbvals[2] = (Color & 0x7F);
    }

  private:
    const static short cSpectrumReset=5;
    const static short cSpectrumStrobe=4;
    
    struct stripSegment {
      short segFirstLED;    //The first LED in the segment from the beginning (0-origin)
      short segNumLEDs;  //The number of LEDs in the segment
      uint32_t segForeColor;     //The base color of the segment's LEDs
      uint32_t segBackColor;     //Background color
      short segBands;       //The spectrum bands that are averaged together to make up the value for the segment
      short segAction;      //The way the LEDs in the segment are populated
      short segSpacing;     //Spacing between LEDs that are illuminated in the segment (0 default = no spacing)
      short segOptions;     //Options for the segment (cSegOpt...)
      SegmentDisplayRoutine segDisplayRoutine;  //Optional routine to call just before each display cycle
      short segLevel, segMaxLevel;       //Normalized & max level -- output from MapBandsToSegments
    };

    short segCurrentIndex;    //The "current" (default) index that will be modified
    short segMaxDefinedIndex; //Tracks the highest index defined
    stripSegment SegmentData[cMaxSegments];  //The segment array
    
    //The per-band level from the spectrum analyzer for the current sample (see ReadSpectrum)
    short SpectrumLevel[cSegNumBands];
    
    //Max value seen for each spectrum band so far. Used to implement a simple adaptive AGC.
    short maxBandValue[cSegNumBands];

    //Maximum noise values for each band. A band spectrum value of this or lower cause no illumination
    //These were determined by experimentation.
    short nNoiseFloor[cSegNumBands];
    
    //Spectrum analyzer left/right channels
    const static short cSegSpectrumAnalogLeft=0;  //Left channel
    const static short cSegSpectrumAnalogRight=1; //Right channel

    void MapBandsToSegments();
    void ReadSpectrum(bool, bool);
    void ShowSegments();
    
    //A pointer to the low-level I/O LBD8806 strip object we talk to
    Adafruit_NeoPixel * objPxlStrip;
    short nLEDsInStrip;
    
    //Array of random cutoff levels (for cSegActionRandom)
    unsigned short segRandomLevels[64];  //Changing this requires code changes
};

//Various colors. The bit format of these is defined by the LPD8806 library.
//Assume nothing about the format except they are an unsigned long int and 0..127

const uint32_t RGBOff =         LEDSegs::Color(0, 0, 0);
const uint32_t RGBBlack =       RGBOff;

const uint32_t RGBWhite =       LEDSegs::Color(127, 127, 127); //Watch out - 60mA per LED with this
const uint32_t RGBGold =        LEDSegs::Color( 90,  40,   8);
const uint32_t RGBSilver =      LEDSegs::Color( 15,  30,  60);
const uint32_t RGBYellow =      LEDSegs::Color( 90,  70,   0);
const uint32_t RGBOrange =      LEDSegs::Color( 60,  20,   0);
const uint32_t RGBRed =         LEDSegs::Color(127,   0,   0);
const uint32_t RGBGreen =       LEDSegs::Color(  0, 127,   0);
const uint32_t RGBBlue =        LEDSegs::Color(  0,   0, 127);
const uint32_t RGBPurple =      LEDSegs::Color( 40,   0,  40);

//White-ish versions of the base colors (except white!)

const uint32_t RGBGoldWhite =   LEDSegs::Color(110,  70,  30);
const uint32_t RGBSilverWhite = LEDSegs::Color( 20,  45,  90);
const uint32_t RGBYellowWhite = LEDSegs::Color(127, 100,  15);
const uint32_t RGBOrangeWhite = LEDSegs::Color( 80,  35,   5);
const uint32_t RGBRedWhite =    LEDSegs::Color(100,   3,   5); //aka "Pink"
const uint32_t RGBGreenWhite =  LEDSegs::Color( 20, 127,  20);
const uint32_t RGBBlueWhite =   LEDSegs::Color( 10,  20, 127);
const uint32_t RGBPurpleWhite = LEDSegs::Color( 40,   8,  40);

//Dimmer versions of the base colors
const uint32_t RGBWhiteDim =       LEDSegs::Color( 8, 15, 15);
const uint32_t RGBSilverDim =      LEDSegs::Color( 8, 15, 24);
const uint32_t RGBGoldDim =        LEDSegs::Color( 8,  4,  1);
const uint32_t RGBYellowDim =      LEDSegs::Color(20, 15,  0);
const uint32_t RGBOrangeDim =      LEDSegs::Color(15,  3,  0);
const uint32_t RGBRedDim =         LEDSegs::Color(32,  0,  0);
const uint32_t RGBGreenDim =       LEDSegs::Color( 0,  6,  0);
const uint32_t RGBBlueDim =        LEDSegs::Color( 0,  0, 24);
const uint32_t RGBPurpleDim =      LEDSegs::Color(10,  0, 10);

//Very dim versions of the colors
const uint32_t RGBWhiteVeryDim =   LEDSegs::Color( 1,  2,  2);
const uint32_t RGBSilverVeryDim =  LEDSegs::Color( 1,  2,  4);
const uint32_t RGBGoldVeryDim =    LEDSegs::Color( 4,  2,  1);
const uint32_t RGBYellowVeryDim =  LEDSegs::Color( 4,  3,  0);
const uint32_t RGBOrangeVeryDim =  LEDSegs::Color( 4,  1,  0);
const uint32_t RGBRedVeryDim =     LEDSegs::Color( 1,  0,  0);
const uint32_t RGBGreenVeryDim =   LEDSegs::Color( 0,  1,  0);
const uint32_t RGBBlueVeryDim =    LEDSegs::Color( 0,  0,  1);
const uint32_t RGBPurpleVeryDim =  LEDSegs::Color( 1,  0,  1);

//General macros
#define SIZEOF_ARRAY(ary) (sizeof(ary) / sizeof(ary[ 0 ]))

#endif