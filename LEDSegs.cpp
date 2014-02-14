#include "LEDSegs.h"
#include <Adafruit_NeoPixel.h>

/*______________
LEDSegsInit:Common constructor code
*/

void LEDSegs::LEDSegsInit(short nLEDs, short pinData, short ledType) {
  unsigned short iBand;
  
  segCurrentIndex = 0;
  segMaxDefinedIndex = -1;

  //Noise values for each spectrum band (0..1023). Determined by experimentation. YMMV
  nNoiseFloor[0] =  90;
  nNoiseFloor[1] =  90;
  nNoiseFloor[2] =  90;
  nNoiseFloor[3] = 100;
  nNoiseFloor[4] = 100;
  nNoiseFloor[5] = 110;
  nNoiseFloor[6] = 120;
    
  //Initialize the max level seen for each band.
  for (iBand = 0; iBand < cSegNumBands; iBand++) {maxBandValue[iBand] = cInitialMaxBandValue;}

  //Create an LED strip object. Either SPI or digital pins

  objPxlStrip = new Adafruit_NeoPixel(nLEDs, pinData, ledType);  
  
  nLEDsInStrip = nLEDs;
  
  //Setup pins to drive the spectrum analyzer. 
  pinMode(cSpectrumReset, OUTPUT);
  pinMode(cSpectrumStrobe, OUTPUT);

  //Init spectrum analyzer to start reading from lowest band
  digitalWrite(cSpectrumStrobe,LOW);
    delay(1);
  digitalWrite(cSpectrumReset,HIGH);
    delay(1);
  digitalWrite(cSpectrumStrobe,HIGH);
    delay(1);
  digitalWrite(cSpectrumStrobe,LOW);
    delay(1);
  digitalWrite(cSpectrumReset,LOW);
    delay(5);

  //Init this guy
  ResetStrip();
}

/*____________________
LEDSegs::DefineSegment
Set the properties of the current LED segment. A -1 value indicates that the corresponding property should not be changed.
Return value is the segment index.
*/

short LEDSegs::DefineSegment(short FirstLED, short nLEDs, short Action, uint32_t ForeColor, short Bands) {

  //Move to next segment (if no segments yet, start with #0)
  if (segMaxDefinedIndex < 0) {SetSegmentIndex(0);} else {SetSegmentIndex(segCurrentIndex + 1);}
  
  //Set the segment properties passed in
  SetSegment_FirstLED(FirstLED);
  SetSegment_NumLEDs(nLEDs);
  SetSegment_Action(Action);
  SetSegment_ForeColor(ForeColor);
  SetSegment_Bands(segCurrentIndex, Bands);
  
  //Segment defaults
  SetSegment_BackColor(RGBOff);
  SetSegment_Spacing(0);
  SetSegment_Options(segCurrentIndex, 0);
  SetSegment_DisplayRoutine(segCurrentIndex, NULL);

  //Track the highest segment index defined. This speeds the refresh loop a bit.
  segMaxDefinedIndex = max(segMaxDefinedIndex, segCurrentIndex);

  //Return the segment index that was updated, before incrementing it
  return segCurrentIndex;
};

/*______________________
LEDSegs::DisplaySpectrum
Sample and display according to the defined segments
*/

void LEDSegs::DisplaySpectrum(bool doLeft, bool doRight) { 
  ReadSpectrum(doLeft, doRight);
  MapBandsToSegments();
  ShowSegments();
};

/*_________________________
LEDSegs::MapBandsToSegments
Convert spectrum band samples into LED strip segment values in the range 0 .. (# LEDs in that segment).
We average all the bands defined for the segment, and then scale the final segment value
*/

void LEDSegs::MapBandsToSegments() {
  short iSegment, iBand, nLEDs, segBands;
  unsigned long maxTotal, sampleTotal;
  SegmentDisplayRoutine thisDisplayRoutine;
  
  //Loop all defined segments to calc the normalized band value. We do this even for ActionNone segments
  //in case a segment display routine wants to change the action
  for (iSegment = 0; iSegment <= segMaxDefinedIndex; iSegment++) {
    nLEDs = SegmentData[iSegment].segNumLEDs;
    segBands = SegmentData[iSegment].segBands;

    //Loop spectrum bands. For any that are mapped into this segment we total both the sample values and the
    //max possible values, in order to do the normalization.
    maxTotal = 0;
    sampleTotal = 0;

    for (iBand = 0; iBand < cSegNumBands; iBand++) {
      if ((segBands >> iBand) & 1) {
        maxTotal += maxBandValue[iBand];
        sampleTotal += SpectrumLevel[iBand];
      }
    }
    if (maxTotal <= 0) {maxTotal = 1;} //Safety for use as divisor
      
    //Normalize the averaged level to 0..1023 and record in the level array element for this segment
    sampleTotal = (sampleTotal * cMaxSegmentLevel) / maxTotal;
    SegmentData[iSegment].segLevel = sampleTotal;
    SegmentData[iSegment].segMaxLevel = maxTotal;
  } //end segments loop
  
  //Now that all the segments are setup, call any segment display routines that are defined
  for (iSegment = 0; iSegment <= segMaxDefinedIndex; iSegment++) {
    thisDisplayRoutine = SegmentData[iSegment].segDisplayRoutine;
    if (thisDisplayRoutine != NULL) {thisDisplayRoutine(iSegment);}
  };  
};  

/*___________________
LEDSegs::ReadSpectrum
Read the spectrum band samples into class array SpectrumLevel[].
"Channels" tells whether to read left, right, or average both channels.
*/
void LEDSegs::ReadSpectrum(bool doLeft, bool doRight) {
  short iBand, thisLevel, bandMax;  //Band 0 is lowest frequencies, Band 6 is the highest.

  //This loop happens nBands times per sample, so keep it quick. It just records the
  //current and max sample values into the band value arrays.
  for(iBand=0; iBand < cSegNumBands; iBand++) {
    
    //Decay the max a little on each sample
    bandMax = maxBandValue[iBand] - cMaxBandValueDecay;
    if (bandMax < cInitialMaxBandValue) bandMax = cInitialMaxBandValue;
    
    //Read the spectrum for this band
    thisLevel = 0;
    if (doLeft) {thisLevel += analogRead(cSegSpectrumAnalogLeft);}
    if (doRight) {thisLevel += analogRead(cSegSpectrumAnalogRight);}
    if (doLeft && doRight) {thisLevel = thisLevel >> 1;} //If both channels, then take average

    //Process out assumed noise floor for this band
    thisLevel -= nNoiseFloor[iBand];
    if (thisLevel < 0) {thisLevel = 0;}

    //Set current and max values for this into their respective segment object array slots
    SpectrumLevel[iBand] = thisLevel;
    if (bandMax < thisLevel) {bandMax = thisLevel;}
    maxBandValue[iBand] = bandMax;
    
    //Toggle to ready for next band
    digitalWrite(cSpectrumStrobe,HIGH);
    digitalWrite(cSpectrumStrobe,LOW);     
  }
}

/*_________________
LEDSegs::ResetStrip
Reset the whole thing
*/

void LEDSegs::ResetStrip() {
  short i;
  
  //Reset segment array
  for (i = 0; i < cMaxSegments; i++) {SegmentData[i].segAction = cSegActionNone;}
  
  objPxlStrip->begin();  //Clear and init the strip
  objPxlStrip->show();  //Update the LED strip display to show off to start
  segCurrentIndex = 0;
  segMaxDefinedIndex = -1;

  //Init the random permutation array (for cSegActionRandom)
  ResetRandom();
}

/*__________________
LEDSegs::ResetRandom
Init the cSegActionRandom levels array
*/

void LEDSegs::ResetRandom() {
  unsigned short i, imax;

  randomSeed(micros());
  imax = SIZEOF_ARRAY(segRandomLevels);
  
  //Init the random permutation array (Knuth shuffle)
  for (i=0; i < imax; i++) {segRandomLevels[i] = random(cMaxSegmentLevel);}
}

/*___________________
LEDSegs::ShowSegments
Display the segment values on the LED strip
*/

void LEDSegs::ShowSegments() {
  short    iSegment, iLEDinSegment, iLED, LEDIncrement, segval, ledval;
  short    FirstLED, NumberLEDs, Action, Options, segSpacing1, SpacingCount;
  bool     optOffOverwrite, optModulate, notSpacingLED, doled;
  uint32_t thisColor, backColor, foreColor;
  byte     bcRGB[3], fcRGB[3]; //extra byte for long align
  stripSegment *segptr;

  //First, init all LEDs in the strip to off
  for (iLED = 0; iLED < nLEDsInStrip; iLED++) {objPxlStrip->setPixelColor(iLED, RGBOff);}
  
  //Write each defined segment
  for (iSegment = 0; iSegment <= segMaxDefinedIndex; iSegment++) {
      
    segptr = &SegmentData[iSegment];
    Action = segptr->segAction;

    //Process segment if it does something

    if (Action != cSegActionNone) {
 
      /* Set some local vars for fast reference that we'll need */
      FirstLED = segptr->segFirstLED;
      NumberLEDs = segptr->segNumLEDs;
      backColor = segptr->segBackColor;
      foreColor = segptr->segForeColor;
      segSpacing1 = segptr->segSpacing + 1;

      Options = segptr->segOptions;
      optOffOverwrite = (Options & cSegOptNoOffOverwrite) == 0;
      optModulate = (Options & cSegOptModulateSegment) != 0;
      
      //The level coming out of MapBandsToSegments() is normalized to 0..1023. Here we
      //scale to the number of LEDs that means for this segment.
      
      if (Options & cSegOptInvertLevel) {segptr->segLevel = cMaxSegmentLevel - segptr->segLevel;}
      segval = segptr->segLevel;
      segval = (((long) segval) * ((long) (NumberLEDs + 1))) / ((long) (cMaxSegmentLevel + 1));
      segval = constrain(segval, 0, NumberLEDs); //Insure within expected range
      ledval = segval;
      if ((Action == cSegActionStatic) || (Action == cSegActionRandom)) {ledval = NumberLEDs;}

      //If this is a ModulateSegment option segment, then figure the foreground color scaled between
      //backcolor and forecolor according to the segment's spectrum level.
      if (optModulate) {
        Colorvals(backColor, bcRGB);
        Colorvals(foreColor, fcRGB);
        foreColor = LEDSegs::Color(
            bcRGB[0] + (((fcRGB[0] - bcRGB[0]) * segval) / NumberLEDs)
          , bcRGB[1] + (((fcRGB[1] - bcRGB[1]) * segval) / NumberLEDs)
          , bcRGB[2] + (((fcRGB[2] - bcRGB[2]) * segval) / NumberLEDs));
      }
  
      //Get the starting LED index for this segment and an initial increment to get to the next LED
      switch (Action) {
        case cSegActionFromBottom: //bottom, static and random start iLED the same
        case cSegActionRandom:
        case cSegActionStatic:     LEDIncrement = 1;  iLED = FirstLED; break;

        case cSegActionFromTop:    LEDIncrement = -1; iLED = FirstLED + NumberLEDs - 1; break;

        case cSegActionFromMiddle: LEDIncrement = 0;  iLED = FirstLED + ((NumberLEDs - 1) >> 1); break;
      }

      //Init the spacing counter. This counts down from spacing-1 each time it hits 0 (an illuminated LED).
      //The first LED is always a non-spacer, which is why we init to zero.

      SpacingCount = 0;
      
      //Loop across the LEDs in the segment being illuminated.
      //NOTE: Keep this loop TIGHT! It is run for every LED in every defined segment

      for (iLEDinSegment = 0; iLEDinSegment < NumberLEDs; iLEDinSegment++) {

        //Figure the color we want this LED to be and write it to the LED
        thisColor = backColor;
        if (ledval > iLEDinSegment) {thisColor = foreColor;}

        //Determine if this LED is blanked out because of spacing > 1
        notSpacingLED = (SpacingCount == 0);

        //Write the color value to the strip, but not if...
        //  1) This is a spacing LED (when segment's spacing value is > 1), or...
        //  2) The color value is RGBOff and this is a no-off-overwrite-option segment
        //  3) This is an ActionRandom segment and the level is too low based on the randomizer
        
        doled = (notSpacingLED & ((thisColor != RGBOff) | optOffOverwrite));
        if (doled & (Action == cSegActionRandom)) {
          if (segRandomLevels[iLEDinSegment & 0x3F] > segptr->segLevel) {doled = false;}
        }

        if (doled) {objPxlStrip->setPixelColor(iLED, thisColor);}
   
        //Move to next LED. For from-middle, we jump back and forth around the center of the segment, increasing
        //the increment's absolute value by one more each jump.

        if (Action == cSegActionFromMiddle) {
          if (LEDIncrement <= 0) {
            LEDIncrement--;
            if (notSpacingLED) {SpacingCount = segSpacing1;}
            SpacingCount--;
          }
          else {LEDIncrement++;}
          LEDIncrement = -LEDIncrement;
        }
        else {
          if (notSpacingLED) {SpacingCount = segSpacing1;}
          SpacingCount--;
        }

        iLED += LEDIncrement;
      } //LED-in-segment loop
    } //If an action defined
  }  //Segment loop

  //Finally, refresh the strip.
  objPxlStrip->show();
}
