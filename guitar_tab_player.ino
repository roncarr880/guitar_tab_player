
// Just playing with the teensy design tool and audio library to get aquainted with the teensy.
// Possible improvements would be to parse the tab out of a text file on SD card and
// maybe use an array of pointers to the guitar strings to avoid duplicate code.

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "pipeline.h"     // our song as a big char array

#define MIXER_GAIN 0.20
#define END_BAR  -5
#define MUTE     -4   // mute
#define RING     -3   // ring


 #define MAX_BAR 160

// some integer arrays to hold our local copy of which fret to play
int s1[MAX_BAR];
int s2[MAX_BAR];
int s3[MAX_BAR];
int s4[MAX_BAR];
int s5[MAX_BAR];
int s6[MAX_BAR];

int tab_pos;

 int tempo = 50;   // probably should parse this value out of the tab
 
// GUItool: begin automatically generated code
// original idea was an extra A string to play the 2nd guitar part, but changed the plan
AudioSynthKarplusStrong  string2;        //xy=104,175
AudioSynthKarplusStrong  string3;        //xy=105,222
AudioSynthKarplusStrong  string4;        //xy=105,271
AudioSynthKarplusStrong  string1;        //xy=106,127
AudioSynthKarplusStrong  string5;        //xy=109,322
AudioSynthKarplusStrong  string6;        //xy=112,369
// AudioSynthKarplusStrong  string7;        //xy=113,420
AudioMixer4              mixer2;         //xy=281,286
AudioMixer4              mixer1;         //xy=282,204
AudioMixer4              mixer3;         //xy=403,239  // guess could have used just two mixers
AudioOutputI2S           i2s1;           //xy=529,234
AudioConnection          patchCord1(string2, 0, mixer1, 1);
AudioConnection          patchCord2(string3, 0, mixer1, 2);
AudioConnection          patchCord3(string4, 0, mixer1, 3);
AudioConnection          patchCord4(string1, 0, mixer1, 0);
AudioConnection          patchCord5(string5, 0, mixer2, 0);
AudioConnection          patchCord6(string6, 0, mixer2, 1);
// AudioConnection          patchCord7(string7, 0, mixer2, 2);
AudioConnection          patchCord8(mixer2, 0, mixer3, 1);
AudioConnection          patchCord9(mixer1, 0, mixer3, 0);
AudioConnection          patchCord10(mixer3, 0, i2s1, 0);
AudioConnection          patchCord11(mixer3, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=271,49
// GUItool: end automatically generated code



void setup() {
  // put your setup code here, to run once:
 AudioMemory(15);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.6);
  mixer1.gain(0, MIXER_GAIN);
  mixer1.gain(1, MIXER_GAIN);
  mixer1.gain(2, MIXER_GAIN * 1.3);
  mixer1.gain(3, MIXER_GAIN * 1.3);
  mixer2.gain(0, MIXER_GAIN * 1.3);
  mixer2.gain(1, MIXER_GAIN);
  mixer2.gain(2, 0.0);
  mixer2.gain(3, 0.0);    // no connection
  mixer3.gain(0, 0.5);
  mixer3.gain(1, 0.5);
  mixer3.gain(2, 0.0);
  mixer3.gain(3, 0.0);

  Serial.begin(9600);
  tab_pos = load_bar_local(0);        // load initial frets information to local arrays 
 
}

void loop() {
  // put your main code here, to run repeatedly:
static int i;
float note;

   if( s6[i] == END_BAR ){     // need to load more from the tab
       tab_pos = load_bar_local( tab_pos );
       if( tab_pos == 0 ){   // end of the song was reached
          delay(5000);
          tab_pos = load_bar_local( 0 );   // start song again
          if( tab_pos == 0 ) while(1);     // hang on error
       }
       i = 0;
   }

   note = get_note( 6, s6[i] );
   if( note > 20.0 ) string6.noteOn( note, 0.9);
   else if( s6[i] == MUTE ) string6.noteOff( 0.9);
   
   note = get_note( 5, s5[i] );
   if( note > 20.0 ) string5.noteOn( note, 0.9);
   else if( s5[i] == MUTE ) string5.noteOff( 0.9);
   
   note = get_note( 4, s4[i] );
   if( note > 20.0 ) string4.noteOn( note, 0.9);
   else if( s4[i] == MUTE ) string4.noteOff( 0.9);
   
   note = get_note( 3, s3[i] );
   if( note > 20.0 ) string3.noteOn( note, 0.9);
   else if( s3[i] == MUTE ) string3.noteOff( 0.9);
   
   note = get_note( 2, s2[i] );
   if( note > 20.0 ) string2.noteOn( note, 0.9);
   else if( s2[i] == MUTE ) string2.noteOff( 0.9);
   
   note = get_note( 1, s1[i] );
   if( note > 20.0 ) string1.noteOn( note, 0.9);
   else if( s1[i] == MUTE ) string1.noteOff( 0.9);

   delay ( tempo );
   ++i;
}


float get_note( int string, int fret ){

const float base_note[] = { 0.0, 329.63, 246.94, 196.00, 146.83, 110.00, 82.41 };
float note;

  if( fret == RING || fret == MUTE ) return 0.0;
  if( fret < 0 || fret > 30 ) return 0.0;   // just in case an error makes it into the array
  
  note = base_note[string];
  while( fret-- ) note *= 1.059463094358;  // 12th root of 2.  Freq doubled on 12th fret.
  // Serial.println( note );
  return note;
}

// parse the .h char file and convert to internal integer storage
int load_bar_local( int pos ){

// our sample tab has two guitar parts.  We will merge both and our player will play both
// parts at the same time on one guitar.  He is very clever.
   clear_bar_local();
   pos= load_bar_local1(pos);
   if( pos != 0 ) pos= load_bar_local1(pos);   // merge 2nd part.  Comment out for tab with one guitar part.

   return pos;
}

int load_bar_local1( int pos ){
int i;
int *str;
char c, c2;

   i= 0;   str = &s1[0];   // assume  e string is first for safety

   while( (c = tab[pos++]) ){
     c2 = tab[pos];                    // look ahead may be needed
     if( c2 == 'e' && i > 0 ) break;   // normal done condition when part way through the tab
     if( c == 'Z' ) break;             // end of tab array character
     if( i >= MAX_BAR - 3 ) break;     // line is too long. Y3 - need extra place for END_BAR
                                       // and last parsed may be a number 10-19 which takes two
                                       // if off by one it is on the conservative side.
     switch( c ){
        case 'e':  str = &s1[0]; i = 0;  break;
        case 'B':  str = &s2[0]; i = 0;  break;
        case 'G':  str = &s3[0]; i = 0;  break;
        case 'D':  str = &s4[0]; i = 0;  break;
        case 'A':  str = &s5[0]; i = 0;  break;
        case 'E':  str = &s6[0]; i = 0;  break;
        case '|':  break;   // ignore these 
        case '-':  if( str[i] == END_BAR ) str[i] = RING;
                   ++i;   break;   // merge rather than overwrite
        case 'x':  str[i++] = MUTE; break;           
        case '0':  str[i++] = 0;  break;     // get this one out of the way
        
        case '1':    // special case as it may be followed by another number for frets 10 to 19
                     // no code written for frets above 19
            if( c2 >= '0' && c2 <= '9' ){
              str[i++] = 10 + ( c2 - '0' );
              str[i++] = RING;   // keep the tempo correct as extra number would normally be a '-'
              pos++;             // skip past the 2nd number in tab file
            }
            else str[i++] = 1;
        break;

        default:
           if( c >= '2' && c <= '9' ) str[i++] = c - '0';
        break;
                      
     }   // end case
     
   }  // end while
   
   if( c == 'Z' && i == 0 ) return 0;
   if( c == 0 ) return 0;
   return pos;
}

void clear_bar_local(){
//  put something in the arrays to show no strings are to be played
int i;

    for( i = 0; i < MAX_BAR; ++i ){
      s1[i] =  END_BAR;   
      s2[i] =  END_BAR;
      s3[i] =  END_BAR;
      s4[i] =  END_BAR;
      s5[i] =  END_BAR;
      s6[i] =  END_BAR;
    } 
}

