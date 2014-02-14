//An exmaple of using LEDSegs to drive a Christmas display

#include <LEDSegs.h>

//Our LED strip instance pointer
LEDSegs* strip;

const short nTotalLEDs = 30; //Total number of LEDs in the strip (160 for a 5-meter 32/meter strip uncut)
const short nFirstLED = 0; //First LED to turn on (0-origin)
const short nLastLED = nTotalLEDs - 1; //Max LED index to illuminate. Must be < nTotalLEDs
const unsigned long refreshDelayMS = 35UL; //Min time between strip update cycles (in milliseconds)
const unsigned long segmentSetDisplayTimeMS = 20000UL; //Amount of time to display each segment set
unsigned static long waitforSegmentTimeMS, thisSegmentSet; //Keeps track of which segment set we're doing and how long

//This is an array of segment display setup subroutines that are selected by the
//four toggle switches. When the state of the switches changes, the current strip setup
//is cleared, and the corresponding setup routine here is called to change the display

typedef void (*SegmentSetupRoutine) ();
static SegmentSetupRoutine SegmentSetups[] = {
  SegmentProgramChristmas1,
  SegmentProgramChristmas5, 
  SegmentProgramChristmas2,
  SegmentProgramChristmas3,
  SegmentProgramChristmas4,
  SegmentProgramChristmas5, 
  SegmentProgramChristmas6,
  SegmentProgramChristmas7,
  SegmentProgramChristmas8,
  SegmentProgramChristmas9,
};

//Total number of segment sets (segment definition routines) in the above array

const short nSegmentSets = sizeof(SegmentSetups) / sizeof(SegmentSetups[0]);

/* Routine just used to tune colors */
/*
void SegmentProgram1StaticColor() {
  static short ColorIndex = 0;
  static uint32_t SegColors[] = {RGBPurple, RGBPurpleWhite, RGBPurpleDim, RGBPurpleVeryDim, RGBOff};
  
  if (ColorIndex >= SIZEOF_ARRAY(SegColors)) {ColorIndex = 0;}
  strip->DefineSegment(0, nTotalLEDs, cSegActionStatic, SegColors[ColorIndex], 0);
  ColorIndex++;
}
*/

/*
SegmentProgramChristmas1: 5 simple segments
*/

void SegmentProgramChristmas1() {
  short nLEDsPerSegment, iSegment;
  const short nSegments = 5;
  const short bands[nSegments] = {cSegBand2, cSegBand3, cSegBand4, cSegBand5, cSegBand6};
  uint32_t Colors5[nSegments] = {RGBRed, RGBGold, RGBPurple, RGBGreen, RGBBlue};

  //segments of equal # of LEDs.
  nLEDsPerSegment = (nLastLED - nFirstLED + 1) / nSegments;

  //Define the active segments
   for (iSegment = 0; iSegment < nSegments; iSegment++) {
    strip->DefineSegment((iSegment*nLEDsPerSegment)+nFirstLED, nLEDsPerSegment, cSegActionRandom, Colors5[iSegment], bands[iSegment]);
  }
}

/*
SegmentProgramChristmas2: Pulsing solid color all spectra -- Choose a new color each call
*/

void SegmentProgramChristmas2() {
  short nLEDs;
  const short nColors = 6;
  uint32_t foreColors[nColors] = {RGBRed, RGBGreen, RGBBlue, RGBGold, RGBPurple, RGBWhiteDim};
  static short thisColor = -1;

  thisColor = (thisColor + 1) % nColors;  
  nLEDs = nLastLED - nFirstLED + 1;

  strip->DefineSegment(nFirstLED, nLEDs, cSegActionStatic, foreColors[thisColor], 0x0E);
  strip->SetSegment_Options(cSegOptModulateSegment);
  strip->SetSegment_DisplayRoutine(&DisplayRoutineModulateHelper);
}

//The LED intensity isn't linear with level, so this display routine provides segments that are
//defined as static/modulate with better "action"
void DisplayRoutineModulateHelper(short iSegment) {
  const short nCuts = 9;
  const short C2CutLevels[nCuts] = { 40, 150, 225, 400, 500, 600, 700, 800, 950};
  const short C2MapLevels[nCuts] = {  0,   1,  10,  30,  60, 200, 400, 700, cMaxSegmentLevel};
  short iLevel, thisLevel;
  thisLevel = strip->GetSegment_Level(iSegment);
  for (iLevel = 0; iLevel < (nCuts-1); iLevel++) {
    if (thisLevel < C2CutLevels[iLevel]) {break;}
  }
  strip->SetSegment_Level(C2MapLevels[iLevel]);
}

/*
SegmentProgramChristmas3: Two interleaved red/green solid segments for all spectra, green one inverted
*/

void SegmentProgramChristmas3() {
  short nLEDs = nLastLED - nFirstLED + 1;

  strip->DefineSegment(nFirstLED, nLEDs, cSegActionFromBottom, RGBRed, cSegBand2 | cSegBand3);
  strip->SetSegment_Spacing(1);

  strip->DefineSegment(nFirstLED + 1, nLEDs - 1, cSegActionFromTop, RGBGreen, cSegBand4 | cSegBand5 | cSegBand6);
  strip->SetSegment_Spacing(1);
}

/*
SegmentProgramChristmas4: Three segments
*/

void SegmentProgramChristmas4() {
  short nLEDs = nLastLED - nFirstLED + 1;
  short nLEDsPerSegment = nLEDs / 3;

  strip->DefineSegment(nFirstLED, nLEDsPerSegment, cSegActionFromTop, RGBRed, 0x02);
  strip->SetSegment_BackColor(RGBBlueVeryDim);
  strip->SetSegment_Options(cSegOptModulateSegment);

  strip->DefineSegment(nFirstLED + nLEDsPerSegment + 1, nLEDsPerSegment, cSegActionFromMiddle, RGBGold, 0x0C);
  strip->SetSegment_BackColor(RGBBlueVeryDim);
  strip->SetSegment_Options(cSegOptModulateSegment);

  strip->DefineSegment(nFirstLED + (2*nLEDsPerSegment) + 1, nLEDsPerSegment, cSegActionFromBottom, RGBGreen, 0x30);
  strip->SetSegment_BackColor(RGBBlueVeryDim);
  strip->SetSegment_Options(cSegOptModulateSegment);
}

/*
SegmentProgramChristmas5: 5 interleaved segments of different colors (my favorite - this is really awesome)
*/

void SegmentProgramChristmas5() {
  short nLEDsPerSegment, iSegment;
  const short nSegments = 5;
  const short bands[] = {cSegBand2, cSegBand3, cSegBand4, cSegBand5, cSegBand6};
  uint32_t Colors5[] = {RGBRed, RGBYellow, RGBPurple, RGBBlue, RGBGreen};

  nLEDsPerSegment = (nLastLED - nFirstLED + 1);

  //Define the active segments
   for (iSegment = 0; iSegment < nSegments; iSegment++) {
    strip->DefineSegment(nFirstLED + iSegment, nLEDsPerSegment, cSegActionRandom, Colors5[iSegment], bands[iSegment]);
    strip->SetSegment_Spacing(nSegments-1);
  }
}

/*
SegmentProgramChristmas6: Adjacent segments with a custom display routine
*/

const short nSegmentsChristmas6 = 40;
short levelsChristmas6[nSegmentsChristmas6 + 1];
short C6ColorIndex = 0;
uint32_t C6SegColors[] = {RGBBlue, RGBGold, RGBYellow, RGBPurple, RGBOrange, RGBSilver};  //Colors to cycle

void SegmentProgramChristmas6() {
  short nLEDsPerSegment, iSegment, levelRangePerSegment, nLevel;
  nLEDsPerSegment = (nLastLED - nFirstLED + 1) / nSegmentsChristmas6;
  levelRangePerSegment = cMaxSegmentLevel / nSegmentsChristmas6;
  nLevel = 0;
  
  if (C6ColorIndex >= SIZEOF_ARRAY(C6SegColors)) {C6ColorIndex = 0;}
  
  //Define the active segments
   for (iSegment = 0; iSegment < nSegmentsChristmas6; iSegment++) {
    strip->DefineSegment(nFirstLED + (iSegment * nLEDsPerSegment), nLEDsPerSegment, cSegActionStatic, C6SegColors[C6ColorIndex], 0x1E);
    strip->SetSegment_DisplayRoutine(iSegment, &SegmentDisplayChristmas6);
    levelsChristmas6[iSegment] = nLevel;
    nLevel += levelRangePerSegment;
  }
  levelsChristmas6[nSegmentsChristmas6] = cMaxSegmentLevel;
  C6ColorIndex++;
}

void SegmentDisplayChristmas6(short iSegment) {
  short curLevel;
  curLevel = strip->GetSegment_Level(iSegment);
//  strip->SetSegment_Action(iSegment, cSegActionStatic);
  strip->SetSegment_ForeColor(iSegment, C6SegColors[C6ColorIndex - 1]);
  if ((levelsChristmas6[iSegment] >= curLevel) || (levelsChristmas6[iSegment+1] < curLevel)) {
//    strip->SetSegment_Action(iSegment, cSegActionNone);
    strip->SetSegment_ForeColor(iSegment, RGBBlueVeryDim);
  }
}

/*
SegmentProgramChristmas7: A single, sliding segment of fixed length, colors change on each call
*/

short C7ColorIndex = 0;
const short C7SegLen = 40;  //The length of the "slider"
const short C7SegMinLevel = 3; //The min level needed to show the slider
short C7ColorSegs[3];  //Records the index of the three 0-length color segments
uint32_t C7SegColors[] = {RGBRed, RGBGreen, RGBGold, RGBYellow, RGBPurple, RGBOrange, RGBSilver, RGBBlue};  //Colors to cycle

static short C7LastStartPos = 0;
const short C7MaxAttack = 30;
const short C7MaxDecay = 10;

void SegmentProgramChristmas7() {
  
  if (C7ColorIndex >= SIZEOF_ARRAY(C7SegColors)) {C7ColorIndex = 0;}

  //Define the slider segment. It's color and starting position are set in the display routine
  strip->DefineSegment(0, C7SegLen, cSegActionStatic, C7SegColors[C7ColorIndex], 0x1E);
  strip->SetSegment_DisplayRoutine(&SegmentDisplayChristmas7);

  //Define three zero-length segments just to get the levels for the three bands we want to
  //check to set the color of the motion segment in the display routine
  C7ColorSegs[0] = strip->DefineSegment(0, 0, cSegActionNone, RGBRed, cSegBand2);
  C7ColorSegs[1] = strip->DefineSegment(0, 0, cSegActionNone, RGBGreen, cSegBand3);
  C7ColorSegs[2] = strip->DefineSegment(0, 0, cSegActionNone, RGBBlue, cSegBand4 | cSegBand5);

  C7ColorIndex++;
}

void SegmentDisplayChristmas7(short iSegment) {
  short curLevel, startPos;
  uint32_t thiscolor;
  short colorseg, iseg, maxcolorlevel, thislevel;

  curLevel = strip->GetSegment_Level(iSegment) - C7SegMinLevel;
  if (curLevel < 0) {strip->SetSegment_Action(iSegment, cSegActionNone);}
  else {
    strip->SetSegment_Action(iSegment, cSegActionStatic);
    startPos = (
        (((long) curLevel) * ((long) (nTotalLEDs - C7SegLen))) /
        ((long) (cMaxSegmentLevel - C7SegMinLevel))) +
        nFirstLED;
    startPos = constrain(startPos, 0, nLastLED); //safety
/*    
    if (startPos > (C7LastStartPos - C7MaxAttack)) {startPos = C7LastStartPos + C7MaxAttack;}
    else if (startPos < (C7LastStartPos + C7MaxDecay)) {startPos = C7LastStartPos - C7MaxDecay;}
    C7LastStartPos = startPos;
*/    
    strip->SetSegment_FirstLED(iSegment, startPos);
  }
}
/*
SegmentProgramChristmas8: Single color-modulated segment based on sound level
*/

void SegmentProgramChristmas8() {
  strip->DefineSegment(0, nTotalLEDs, cSegActionStatic, RGBOff, cSegBand2 | cSegBand3 | cSegBand4 | cSegBand5);
  strip->SetSegment_DisplayRoutine(&SegmentDisplayChristmas8);
}

//Display routine to modulate the color.
void SegmentDisplayChristmas8(short iSegment) {
  const short C8nColors = 3;
  static const short C8ColorLow[C8nColors] = {0, 550, 700};     //RGB
  static const short C8ColorPeak[C8nColors] = {100, 750, 1023};
  static const short C8ColorHigh[C8nColors] = {600, 800, 1023};
  short C8RGBLevels[C8nColors];
  short iColor, seglevel, lowval, peakval, highval;

  seglevel = strip->GetSegment_Level(iSegment);  
  for (iColor = 0; iColor < C8nColors; iColor++) {
    lowval = C8ColorLow[iColor];
    peakval = C8ColorPeak[iColor];
    highval = C8ColorHigh[iColor];
    C8RGBLevels[iColor] = 0;
    if (seglevel > lowval) {
      if (seglevel < peakval) {C8RGBLevels[iColor] = ((seglevel - lowval) * 127) / (peakval - lowval);}
      else if (seglevel < highval) {C8RGBLevels[iColor] = ((highval - seglevel) * 127) / (highval - peakval);}
    }
  }
  strip->SetSegment_ForeColor(iSegment, LEDSegs::Color(C8RGBLevels[0], C8RGBLevels[1], C8RGBLevels[2])); 
}

/*
SegmentProgramChristmas9: Simple single segment from the bottom
*/

short C9ColorIndex = 0;
uint32_t C9SegColors[] = {RGBRed, RGBGreen, RGBGold, RGBBlue};  //Colors to cycle

void SegmentProgramChristmas9() {  
  if (C9ColorIndex >= SIZEOF_ARRAY(C7SegColors)) {C9ColorIndex = 0;}
  strip->DefineSegment(0, nTotalLEDs, cSegActionFromBottom, C9SegColors[C9ColorIndex], 0x1E);
  C9ColorIndex++;
}

/*
The Arduino boot setup routine
*/

void setup() {

  //Create the strip class instance we will use
  strip = new LEDSegs(nTotalLEDs, 6, NEO_GRB + NEO_KHZ800);
  
  //Make sure we see a "change" to start the segment sets cycling
  thisSegmentSet = -1;
  waitforSegmentTimeMS = 0;
#if defined DIAGINITSERIAL
  Serial.begin(9600);
  Serial.println(""); Serial.println("----- Starting Sketch -----");
#endif
}

/*
The Arduino main loop. Just sample and display with a wait
*/

void loop() {
  uint32_t startRefreshMS;       //MS time at the start of this refresh cycle;

  //Get the current time
  startRefreshMS = millis();

  //If time for this segment set has run out, reset thetimer and move to the next segment set (cyclic)
  if (waitforSegmentTimeMS <= startRefreshMS) {
    waitforSegmentTimeMS = startRefreshMS + segmentSetDisplayTimeMS; //Set the time for the upcoming segment set
    thisSegmentSet++; //Move to the next segment set (cycling)
    if (thisSegmentSet >= nSegmentSets) {thisSegmentSet = 0;};
    strip->ResetStrip();
    SegmentSetups[thisSegmentSet]();
  }  
  
  //Do the deed
  strip->DisplaySpectrum(true, true);

  //Wait for the minimum refresh time;
  while (millis() < (startRefreshMS + refreshDelayMS)) {;}
}
