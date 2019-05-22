/* Simple Teensy DIY USB-MIDI controller.
  6tomp (A 6 button midi footpedal with bank switching)
  Created by Jacob Hernandez on 05-30-18
  This code is in the public domain.

  You must select MIDI from the "Tools > USB Type" menu for this code to compile.
  To change the name of the USB-MIDI device, edit the STR_PRODUCT define
  in the /Applications/Arduino.app/Contents/Java/hardware/teensy/avr/cores/usb_midi/usb_private.h
  file. You may need to clear your computers cache of MIDI devices for the name change to be applied.
  See https://www.pjrc.com/teensy/td_midi.html for the Teensy MIDI library documentation.

  This pedal has two modes: Note mode and CC mode.
  CC Mode - Holding down button connected to pin 0 when powering on will set pedal in CC mode. Holding down button sends momentary cc values of 127.
    Releasing button sends cc value of 0. Good for triggering momentary effects or full potentiometer values.
  Note Mode - The default mode with nothing held down on power up. pressing a button sends note on,
    while releasing button sends note off.
  Bank toggle - 1 button can be designated as an A/B bank toggle to send alternate B bank values and double
    the number of midi messages you can send.
  LED - If midi clock is received, will blink on every quarter note. Additionally, blinks when any button is pressed.
*/

#include <Bounce.h>

//// BANK BUTTON VARIABLES & arrays
const int total_bank = 0;
// Bank Button Pin Numbers
const int bank_pins[] = {};

//// BUTTON VARIABLES & ARRAYS
const int total_buttons = 6;
// Midi Button Pin Numbers
const int button_pins[] = {0,1,2,3,4,5};

//// LED VARIABLES & arrays
const int total_led = 1;
// LED pin numbers
const int led_pins[] = {12};

// the MIDI channel number to send messages
const int midi_channel = 1;

//Variable that stores the current MIDI bank or mode of the device (what type of messages the push buttons send).
int midi_bank_selected = 0;

// Arrays of midi notes to send
const int midi_note_a[] = {36,37,38,39,40,41};
const int midi_note_b[] = {41,42,43,44,45,46};

boolean cc_mode = false;
// Arrays of midi notes to send
const int midi_cc_a[] = {36,37,38,39,40,41};
const int midi_cc_b[] = {41,42,43,44,45,46};

// for toggling banks later
int midi_selected[total_buttons] = {};

// Create Bounce objects for each button and switch. The Bounce object
// automatically deals with contact chatter or "bounce", and
// it makes detecting changes very simple.
// 5 = 5 ms debounce time which is appropriate for good quality mechanical push buttons.
// If a button is too "sensitive" to rapid touch, you can increase this time.

//button debounce time
const int DEBOUNCE_TIME = 50;

Bounce clean_buttons[total_buttons] =
{
  Bounce (button_pins[0], DEBOUNCE_TIME),
  Bounce (button_pins[1], DEBOUNCE_TIME),
  Bounce (button_pins[2], DEBOUNCE_TIME),
  Bounce (button_pins[3], DEBOUNCE_TIME),
  Bounce (button_pins[4], DEBOUNCE_TIME),
  Bounce (button_pins[5], DEBOUNCE_TIME)
};

// // bounce for bank button
// Bounce clean_banks[total_bank] =
// {
//   Bounce (bank_pins[0], DEBOUNCE_TIME)
// };




// Clock Variables
int play_flag = 0;
byte data;
int clock_counter = 0;
int clock_counter_db = 0;
unsigned long currentTime = 0;

//------------------------ setup only runs once---------------------------------------//
void setup()
{
  Serial.begin(38400);

  // ------------
  // ------------
  // ------------
  //BUTTON SETUP
  for (int i = 0; i < total_buttons; i++)
  {
    pinMode (button_pins[i], INPUT_PULLUP);
  }
  // If first button held down on startup,
  // buttons will send momentary cc values.
  if(digitalRead(button_pins[0]) == LOW) {
    cc_mode = true;
    // setup selected control change values
    for (int i = 0; i < total_buttons; i++) {
      midi_selected[i] = midi_cc_a[i];
    }
  } else {
    // setup selected midi note values
    for (int i = 0; i < total_buttons; i++) {
      midi_selected[i] = midi_note_a[i];
    }
  }

  // ------------
  // ------------
  // ------------
  //BANK SETUP
  // if banks declared
  if (total_bank != 0) {
    // set pinmode for each
    for (int i = 0; i < total_bank; i++)
    {
      pinMode (bank_pins[i], INPUT_PULLUP);

    }
  }

  // ------------
  // ------------
  // ------------
  //LED SETUP
  for (int i = 0; i < total_led; i++)
  {
    pinMode(led_pins[i], OUTPUT);
  }

} // END SETUP()

//------------- loop is where real-time logic, input, output happens indefinitely. -------------//
void loop()
{
  // ------------
  // ------------
  // ------------
  // MIDI CLOCK LOOP
  currentTime = millis();
  // every time a new message comes in
  while (usbMIDI.read()) {
    // check data type of message
    data = usbMIDI.getType();
    // If playing from stop state
    if(data == usbMIDI.Start) {
      play_flag = 1;
      digitalWrite(led_pins[0],LOW);
      clock_counter = 0;
    }
    // If playing from pause state
    else if(data == usbMIDI.Continue) {
      play_flag = 1;
      digitalWrite(led_pins[0],LOW);
    }
    // If stopping
    else if(data == usbMIDI.Stop) {
      digitalWrite(led_pins[0],LOW);
      clock_counter = 0;
      play_flag = 0;
    }
    // If clock tick message receive (24 ticks per quarter note)
    // and we are in play state
    else if((data == usbMIDI.Clock) && (play_flag == 1)) {
      clock_counter++;
      if(clock_counter == 1) {
        digitalWrite(led_pins[0],HIGH);
      }
      // turn off led after 4 tick messages
      else if(clock_counter == 3 && clock_counter_db != 0) {
        digitalWrite(led_pins[0],LOW);
      }
      // downbeat - turn off led after 12 tick messages
      else if(clock_counter == 12 && clock_counter_db == 0) {
        digitalWrite(led_pins[0],LOW);
      }
      // turn on led on 24th tick message
      else if(clock_counter == 24) {
        clock_counter = 0;
        // keep track of downbeat (in 4/4 time)
        if(clock_counter_db < 3) {
          clock_counter_db++;
        } else {
          clock_counter_db = 0; 
        }
      }

    }
  }
  // ------------
  // ------------
  // ------------
  // BANK DIGITAL PIN LOOP


  // if (total_bank != 0) {
  //   for (int i = 0; i < total_bank; i++)
  //   {
  //     // get bank values
  //     clean_banks[i].update();
  //
  //     // if bank is pressed
  //     if (clean_banks[i].fallingEdge())
  //     {
  //       // toggle B to A
  //       if (midi_bank_selected == 1) {
  //         midi_bank_selected = 0;
  //         // set to selected bank values
  //         for (int i = 0; i < total_buttons; i++) {
  //           // in cc mode set control change values
  //           if( cc_mode == true ) {
  //             midi_selected[i] = midi_cc_a[i];
  //           }
  //           // in default send note values
  //           else {
  //             midi_selected[i] = midi_note_a[i];
  //
  //           }
  //         }
  //       }
  //       // toggle A to B
  //       else if (midi_bank_selected == 0) {
  //         midi_bank_selected = 1;
  //         // set to selected bank values
  //         for (int i = 0; i < total_buttons; i++) {
  //           // in cc mode set control change values
  //           if( cc_mode == true ) {
  //             midi_selected[i] = midi_cc_b[i];
  //           }
  //           // in default send note values
  //           else {
  //             midi_selected[i] = midi_note_b[i];
  //
  //           }
  //         }
  //       }
  //     }
  //     //do nothing for rising Edge (bank button release not important)
  //   }
  // }


  // ------------
  // ------------
  // ------------
  //BUTTON DIGITAL PIN LOOP
  for (int i = 0; i < total_buttons; i++)
  {
    clean_buttons[i].update();
  }
  for (int i = 0; i < total_buttons; i++) {
    // Check each button for "falling" edge or when inverted button
    // is pressed and voltage goes from high to low.
    if (clean_buttons[i].fallingEdge()) {
      // if in control change mode, send cc messages
      if(cc_mode == true) {
        usbMIDI.sendControlChange(midi_selected[i], 127, midi_channel);
      }
      // otherwise send note on message
      else {
        usbMIDI.sendNoteOn (midi_selected[i], 110, midi_channel);
      }
      // turn led on only if clock is stopped or not receiving
      if(play_flag == 0) {
        digitalWrite(led_pins[0],HIGH);
      }
    }
    // Check each button for "rising" edge or when inverted button
    // is released and voltage goes from low to high.
    else if (clean_buttons[i].risingEdge()) {
        // if in control change mode, send cc messages
        if(cc_mode == true) {
          usbMIDI.sendControlChange(midi_selected[i], 0, midi_channel);
        }
        // otherwise send note off message
        else {
          usbMIDI.sendNoteOff(midi_selected[i], 0, midi_channel);
        }
      // turn led off if clock is stopped or not receiving
      if(play_flag == 0) {
      digitalWrite(led_pins[0],LOW);
        }
    }
  }

  } // END LOOP()
