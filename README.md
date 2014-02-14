LightOrgan
==========

This is a port of Arduino code to create sound-based color effects on the Adafruit WS2812 LED
strips, using the Ardunio Due or Mega boards with the spectrum analyzer shield.
Original code is here:
https://github.com/ergodic00/LightOrgan

This should work with other AVR boards provided they have 2K sram and 16K flash, but has not been tested
on any except the Arduino Uno.


To use this code as is, you'll need some specific components:

  1) An Arduino Uno - about $30
     ...Or this MAY work with other AVR boards, see below

  2) ...Stacked with a Bliptronics spectrum analyzer shield
     About $25 from sparkfun.com

  3) ...Any length of Adafruit's WS2812 NeoPixel RGB strip.
     These are up to $60/meter, depending on the density of the pixels. You also need an appropriate 5V supply.

Using this library and the above hardware, you can very quickly get sophisticated "light organ"
effects out of your strip.

The spectrum analyzer shield provides seven, fixed-band audio amplitudes from 0..1023 each time
it's values are read out. The code here drives the LPD8806 LED strip in different-colored
"segments" of LEDs on the strip, using the spectrum samples from the shield. You define the
segments, what spectrum bands they look at, and how they behave.

Any combination of the seven fixed spectrum analyzer "bands" can be averaged into any segment.
Segments have a defined starting position, length, forground/background color and other optional
properties such as spacing, background color, etc.

By default, when you define a segment, the level of the spectrum bands determine how many LEDs in the
segment are illuminated. The higher the level, the more "on" LEDs in the segment are lit up. But
options can be defined for each segment to alter how that segment displays. You can also display
static segments or backgrounds that are not level-dependent.


===============
LEDSegs object:
===============

The LEDSegs library here is written as a C++ class. It exposes methods to make creating effects
very easy. So first, include this library or its source in your code.

  #include "LEDSegs.cpp"

Next, you need to create an "LEDSegs" object so that you can define your display segments.

strip = new LEDSegs(#LEDs, dataPin, ledType);

The ledType short consists of pixel type structs constructed of the following:
   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
and
   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
  
The constructor for the LEDSegs object initializes it to a base state with no defined segments.
You can explicitly reset the LEDSegs object at any time with a ResetStrip() call:

  strip->ResetStrip()

=========
Segments:
=========

Well, feelin' pretty good are you? Now all you have to do is define your segments and you're
ready to start jammin' to the tunes!

Define your segments also in the setup() routine. For this, simply make calls to the strip's
DefineSegment() method:

  DefineSegment(start, n, action, color, bands);

There are five arguments to DefineSegment:

  1) The starting LED (0-origin) of the segment (short int). In most cases this will be 0.

  2) The number of LEDs in this segment (short int). For a 5 meter strip this is 160.

  3) The type of basic action you want for the segment:

     cSegActionBottom: illuminates LEDs up from the first LED in the segment
     
     cSegActionTop:    illuminates LEDs down, starting from the end of the segment down
     
     cSegActionMiddle: illuminates in both directions starting from the middle of the segment
     
     cSegActionStatic: illuminates all LEDs irrespective of specrum level.
     
     cSegActionRandom: illuminates LEDs randomly in the segment's range based on level.
         The randomiztion stays fixed until you reinit the LEDSegs object, or you
         can explicity call the ResetRandom() method.
     
     cSegActionNone:   the segment is not displayed. You would only use this with custom
         display routines (below).

  4) The foreground (illumination) color (one of the RGBxxx constants defined below, or an
     LEDSegs::Color(R,G,B) value == where the RGB values are 0..127.

  5) A bitmask of cSegBand# (# from 1..7), representing which bands you want in the segment. I
     recommend you keep to the middle five (cSegBand2...cSegBand6), and avoid cSegBand1
     and cSegBand7 as they are mostly inaudible and/or noise. (Especially cSegBand7 which specs
     an almost-preposterous 16KHz center frequency.)
     
     (The spectrum shield has a farily high inherent noise level and does not have any AGC.
     Thus this code attempts to do some simple noise gating and level normalization. You can
     look at the code if you're interested.)

For example, here is a 3-segment display for a 5-meter/160-LED strip: red, green and blue, using
different frequency bands from the analyzer:

  void setup() {
    strip->new LEDSegs(160);
    strip->DefineSegment( 0,  53, cSegAction_Top,    RGBRed,   cSegBand2 | cSegBand3);
    strip->DefineSegment( 53, 53, cSegAction_Middle, RGBYellow, cSegBand4);
    strip->DefineSegment(106, 53, cSegAction_Bottom, RGBBlue,  cSegBand5 | cSegBand6);
  }

To actually show the segments in time with the audio you need to put this statment in
your loop() routine:

    strip->DisplaySpectrum(true, true);
    
And that's all you need. This will give you an LED strip action like this:

     <<....red*******|   <<..****yellow****..>>   |blue****....>>

The red segment extends down from the 1/3 point on the strip based on the level of low frequencies.
The green maps the center band and expands outward in both directions from the middle of the strip,
and the red segment goes up from the 2/3 point.

You will probably want to add some delay between display update cycles, otherwise the display
may look a little frantic. This is illustrated below.

----------------
Segment Overlap:

Segments can overlap. They are "written" to the strip in index order, so later-defined segments
with default options will overwrite lower-index segments' LEDs.

For example, if you wanted a dim white background (instead of an off level) along the entire strip
where the active segments aren't doing anything, define an initial segment for the above example:

 strip->DefineSegment( 0, 160, cSegActionStatic, RGBWhiteVeryDim, 0);
 ...the segments defined as above...
 
----------------------
Current Segment Index:

Segments are stored in an array. They are displayed in index order and referenced by index.

The "current" index is the segment assumed when you call GetSegment()/SetSegment() methods without
an explicit segment index. The first DefineSegment() call sets the current index to 0.

Each call to DefineSegment() automatically first bumps the current segment index to the next
segment, and then resets the segment's properties. The first segment you define is index 0.
Changing the current segment index does nothing to any segment until you actually call DefineSegment()
or set a segment property.

  strip->GetSegmentIndex();  //returns the current (short integer) segment index
  strip->SetSegmentIndex(n); //sets the current segment index to segment index "n".
  
You can define up to 100 segments. If you want a higher or lower max, use a #define to set
cMaxSegments before including this library.

---------------------------
Get/Set Segment Properties:

All of the properties of a segment can be read in your code and with GetSegment_property() methods.
This is mainly useful for custom display routines. These methods are overloaded and you can call
them without an index to get the value for the current segment. Or you can specify a explicit
segment index. E.g.:

  strip->GetSegment_ForeColor(); //assumes current index
  strip->GetSegment_ForeColor(isegment); //Gets forground color for specified segment isegment

Likewise the SetSegment_property([isegment, ] value) methods allow you to change properties
of a segment.

Here are the Get/Set methods for segment properties. Get methods return a type appropriate
for the item - usually a short int.

    Get/SetSegment_Action
    Get/SetSegment_BackColor
    Get/SetSegment_Bands
        SetSegment_DisplayRoutine (no Get method for this)
    Get/SetSegment_FirstLED
    Get/SetSegment_ForeColor
    Get/SetSegment_Level
    Get/SetSegment_NumLEDs
    Get/SetSegment_Options
    Get/SetSegment_Spacing

Consult the class definition below to see the full list of Get/Set methods for segments.

----------------
Segment Spacing:

Any segment can have an optional segment "spacing." A spacing of zero (the default) means all LEDs
in the segment are controlled during the segment's display.

You can set the segment's spacing to any value n >=0 with SetSegment_Spacing(n). This means that only
every (n+1)th LED in the segment's range will be addressed during a display cyle. LEDs in-between are
skipped over.

A positive spacing allows for several, spaced, overlapping segments that are "interleaved."
Spaced segments should still be defined by their full length irrespective of the spacing.

To interleave spaced segments, use the same spacing value for all, and bump the starting position
of each successive segment by +1. This works for all actions, including Middle. IMPORTANT: the
defined integer length of all the interleaved segments MUST BE IDENTICAL in order for them to
overlap and display correctly.

----------------
Segment Options:

You can define options for the segment using the SetSegment_Options(bitmask) method as follows:

  ___
  cSegNoOffOverwrite:

  This option prevents a background/off LED color from overwriting whatever is already set for
  that LED from an lower-index segment. Instead of writing the background color, it skips that LED.
  But note all LEDs are set OFF (=RGBOff) at the start of each display cycle, so this doesn't
  carry across between display cycles.
  
  This option can be used to "merge" segments over each other with the earlier segment "showing through"
  the off LEDs on the higher-index segment.

  ___
  cSegOptModulateSegment:

  This option modulates the RGB intensity of forground-color LEDs illuminated in the segment between the
  background color and the foreground color based on the spectrum level. Only illuminated (foreground)
  LEDs are affected. RGB values are scaled independently and proprotionately to the level for the segment.
  
  If the background color is RGBOff, then the intensity of the "on" LEDs will vary based on the volume of
  the bands mapped to that segment. If this option is applied to a cSegActionStatic segment, then the
  entire segment will "pulse" according to the volume of the spectrum bands.
  
  ___
  cSegOptInvertLevel:
  
  An inverted level (ie 1023 - actuallevel) is used for the display. So higher levels reduce the number
  of LEDs, rather than increasing them.

===========
Displaying:
===========

Once you have the LEDSegs object created and its segments defined, just call:

  DisplaySpectrum(doLeft, doRight)
  
in the Arduino loop() routine. This queries the current values of the spectrum analyzer, and then does a
display cycle on the LED strip with those values. doLeft and doRight are bool arguments indicating which
channels to query in the spectrum analyzer. If both are true the two channels are averaged.

You may want to put in a delay between refreshes to avoid the display refreshing too frequently. This is
especially true with SPI output where the cycles can happen 1 or 2 ms apart, giving the display an overly
active appearance. About 30 per refresh usually looks about right.

For example, here is a setup() and loop() that uses millis() to keep the time between
display cycles to a minimum of 30ms.

  void setup() {
    strip = new LEDSegs(160);
    strip->DefineSegment(0, 32, cSegActionFromBottom, RGBRed, -1);
    waitforSegmentTimeMS = 0;
  }

  void loop() {
    unsigned long startRefreshMS = millis();
    strip->DisplaySpectrum(true, true);
    while (millis() < (startRefreshMS + 30UL)) {;}
  }

_________________
Display Routines:

The basic segment definitions provide a lot of flexibility, but if you want more advanced
customizations and don't mind a little coding, you can define a "display routine" for a
segment.

To set a display routine for a segment, pass a reference to the routine you want called on each
display cycle for that segment, ie: SetSegment_DisplayRoutine(&routine-name)

A display routine gets called after the spectrum samples are gathered and have been normalized
to 0...cMaxSegmentLevel. But before the actual display processing takes place. It is a void()
procedure that is called with a single short parameter: the index of the segment.

A display routine can inspect and change almost any of the segment properties, including color,
length, starting position, current normalized spectrum level, etc. These will be the values used
during the display processing. Use the SetSegment_xxx and GetSegment_xxx methods for this. It can
also inspect and change other segments as well.

Here is an example. This was for a Christmas display. It moves a "sliding" segment up and down
the strip. The starting position of the segment is based on the total volume level, and the
slider segment's color is decided on each display cycle by which band has the largest amplitude.
There is also a background of soft blue for the whole strip.

/*
    const short nFirstLED = 0;      //First LED to illuminate
    const short nTotalLEDs = 160;   //160 for a 5 meter strip
    const short C7SegLen = 30;      //The length of the "slider" segment
    const short C7SegMinLevel = 80; //The min level needed to show the slider
    short C7ColorSegs[3];           //Records the index of the three 0-length color segments
    uint32_t C7SegColors[3] = {RGBRed, RGBGreen, RGBGold};  //Colors to use based on loudest band
    
    void SegmentProgramChristmas7() {
      //Define a background for the whole strip
      strip->DefineSegment(nFirstLED, nTotalLEDs, cSegActionStatic, RGBBlueVeryDim, 0);
    
      //Define the "slider" segment. Color and starting position are set in the display routine
      strip->DefineSegment(0, C7SegLen, cSegActionStatic, RGBBlue, 0x1E);
      strip->SetSegment_DisplayRoutine(&SegmentDisplayChristmas7);
    
      //Define three zero-length segments just to get the levels for the three bands we want to
      //check to set the color of the slider segment in the display routine
      C7ColorSegs[0] = strip->DefineSegment(0, 0, cSegActionNone, RGBRed, cSegBand2);
      C7ColorSegs[1] = strip->DefineSegment(0, 0, cSegActionNone, RGBGreen, cSegBand3);
      C7ColorSegs[2] = strip->DefineSegment(0, 0, cSegActionNone, RGBBlue, cSegBand4 | cSegBand5);
    }
    
    //Segment's display routine called for each display cycle
    void SegmentDisplayChristmas7(short iSegment) {
      short curLevel, startPos;
      uint32_t thiscolor;
      short colorseg, iseg, maxcolorlevel, thislevel;
      curLevel = strip->GetSegment_Level(iSegment) - C7SegMinLevel;
    
      //Find the color to use based on the "loudest" band
      maxcolorlevel = -1; colorseg = 0;
      for (iseg = 0; iseg < 3; iseg++) {
        thislevel = strip->GetSegment_Level(C7ColorSegs[iseg]);
        if (thislevel > maxcolorlevel) {maxcolorlevel = thislevel; colorseg = iseg;}
      }
    
      //Set the starting position of the strip. We have to scale the current level
      //to the possible starting positions. There's some small rounding error here
      //but it's not meaningful
      if (curLevel < 0) {strip->SetSegment_Action(iSegment, cSegActionNone);}
      else {
        strip->SetSegment_Action(iSegment, cSegActionStatic);
        strip->SetSegment_ForeColor(iSegment, C7SegColors[colorseg]);
        startPos = ((curLevel * (nTotalLEDs - C7SegLen)) /
            (cMaxSegmentLevel - C7SegMinLevel)) + nFirstLED;
        startPos = constrain(startPos, 0, nLastLED); //safety
        strip->SetSegment_FirstLED(iSegment, startPos);
      }
    }
*/