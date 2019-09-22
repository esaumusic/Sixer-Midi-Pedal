/* Simple Teensy DIY USB-MIDI controller.
  6tomp (A 6 button midi footpedal with 1 led, midi clock in, and alternate button hold functionality)
  Created by Jacob Hernandez on 05-21-19
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
// detectButtonPress
// Use millis to detect a short and long button press
// See baldengineer.com/detect-short-long-button-press-using-millis.html for more information
// Created by James Lewis

#define PRESSED LOW
#define NOT_PRESSED HIGH

// led pin
const byte led = 12;
// the MIDI channel number to send messages
const int midi_channel = 1;
// button press millisecond thresholds
const unsigned long shortPress = 25;
const unsigned long  longPress = 750;

// for led blinking, probably delete this.
// unsigned long blinkInterval = 500;
// unsigned long previousBlink = 0;
// bool ledState = true;
// bool blinkState = true;




// typedef struct here structures our data so we can use dot notation to access related values
// explained here: https://www.norwegiancreations.com/2017/10/getting-started-with-programming-part-8-typedef-and-structs/
typedef struct {
    byte pin;
    byte midi_note_a;
    byte midi_note_b;
    byte midi_note_c;
    unsigned long counter=0;
    bool prevState = NOT_PRESSED;
    bool currentState;
} Button;

//// DEBOUNCE TIME
const int debounce_time = 20;

//// BUTTON VARIABLES & ARRAYS
const int total_buttons = 6;

//// CREATE A BUTTON VARIABLE TYPE
Button button[total_buttons];

//// MIDI CLOCK VARIABLES
int play_flag = 0;
byte data;
int clock_counter = 0;
int clock_counter_db = 0;
unsigned long currentTime = 0;



void setup() {
  Serial.begin(38400);

  // ------------
  // ------------
  // ------------
  //BUTTON SETUP
  button[0].pin = 0;
  button[0].midi_note_a = 36;
  button[0].midi_note_b = 42;

  button[1].pin = 1;
  button[1].midi_note_a = 37;
  button[1].midi_note_b = 43;

  button[2].pin = 2;
  button[2].midi_note_a = 38;
  button[2].midi_note_b = 44;

  button[3].pin = 3;
  button[3].midi_note_a = 39;
  button[3].midi_note_b = 45;

  button[4].pin = 4;
  button[4].midi_note_a = 40;
  button[4].midi_note_b = 46;

  button[5].pin = 5;
  button[5].midi_note_a = 41;
  button[5].midi_note_b = 47;

  for (int i = 0; i < total_buttons; i++)
  {
    pinMode(button[i].pin, INPUT_PULLUP);
  }

  // ------------
  // ------------
  // ------------
  //LED SETUP
  pinMode(led, OUTPUT);

} // END SETUP()




void loop() {
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
        digitalWrite(led,LOW);
        clock_counter = 0;
      }
      // If playing from pause state
      else if(data == usbMIDI.Continue) {
        play_flag = 1;
        digitalWrite(led,LOW);
      }
      // If stopping
      else if(data == usbMIDI.Stop) {
        digitalWrite(led,LOW);
        clock_counter = 0;
        play_flag = 0;
      }
      // If clock tick message receive (24 ticks per quarter note)
      // and we are in play state
      else if((data == usbMIDI.Clock) && (play_flag == 1)) {
        clock_counter++;
        if(clock_counter == 1) {
          digitalWrite(led,HIGH);
        }
        // turn off led after 4 tick messages
        else if(clock_counter == 3 && clock_counter_db != 0) {
          digitalWrite(led,LOW);
        }
        // downbeat - turn off led after 12 tick messages
        else if(clock_counter == 12 && clock_counter_db == 0) {
          digitalWrite(led,LOW);
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
    //BUTTON DIGITAL PIN LOOP

    for (int i = 0; i < total_buttons; i++) {
      // check the button
      button[i].currentState = digitalRead(button[i].pin);
    }

    for (int i = 0; i < total_buttons; i++) {
      // has it changed?
      if (button[i].currentState != button[i].prevState) {
          delay(debounce_time);
          // update status in case of bounce
          button[i].currentState = digitalRead(button[i].pin);
          if (button[i].currentState == PRESSED) {
              // a new press event occured
              // record when button went down
              button[i].counter = millis();

              // turn led on only if clock is stopped or not receiving
              if(play_flag == 0) {
                digitalWrite(led,HIGH);
              }
          }

          if (button[i].currentState == NOT_PRESSED) {
              // but no longer pressed, how long was it down?
              unsigned long currentMillis = millis();
              //if ((currentMillis - button[i].counter >= shortPress) && !(currentMillis - button[i].counter >= longPress)) {
              if ((currentMillis - button[i].counter >= shortPress) && !(currentMillis - button[i].counter >= longPress)) {
                  // short press detected.
                  handleShortPress(i);

              }
              if ((currentMillis - button[i].counter >= longPress)) {
                  // the long press was detected
                  handleLongPress(i);
              }
              // turn led off if clock is stopped or not receiving
              if(play_flag == 0) {
              digitalWrite(led,LOW);
                }
          }
          // used to detect when state changes
          button[i].prevState = button[i].currentState;
      }
    }

    // blinkLED();
}

void handleShortPress(byte x) {
    usbMIDI.sendNoteOn (button[x].midi_note_a, 110, midi_channel);
    delay(10);
    usbMIDI.sendNoteOff(button[x].midi_note_a, 0, midi_channel);
    // blinkState = true;
    // ledState = true;
    // blinkInterval = blinkInterval / 2;
    // if (blinkInterval <= 50)
    //     blinkInterval = 500;
    // turn led off if clock is stopped or not receiving
    if(play_flag == 0) {
    digitalWrite(led,LOW);
      }
}

void handleLongPress(byte x) {
    usbMIDI.sendNoteOn (button[x].midi_note_b, 110, midi_channel);
    delay(10);
    usbMIDI.sendNoteOff(button[x].midi_note_b, 0, midi_channel);
    // blinkState = false;
    // ledState = false;

}

// void blinkLED() {
//     // blink the LED (or don't!)
//     if (blinkState) {
//         if ( (millis() - previousBlink) >= blinkInterval) {
//             // blink the LED
//             ledState = !ledState;
//             previousBlink = millis();
//         }
//     } else {
//         ledState = false;
//     }
//     digitalWrite(led, ledState);
// }

