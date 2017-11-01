/*
 * header for the_clapper
 */

struct button {
  int pin;
  bool state;
  bool last_read;
  unsigned long debounce_time;
};


int sev_seg_array[10][7] = {  { 0,0,0,0,0,0,1 },    // 0
                              { 1,0,0,1,0,0,0 },    // 1
                              { 1,1,0,0,1,1,1 },    // 2
                              { 1,1,0,1,1,0,1 },    // 3
                              { 1,0,0,1,0,1,1 },    // 4
                              { 1,1,0,0,1,1,1 },    // 5
                              { 1,1,1,0,1,1,1 },    // 6
                              { 1,0,0,1,1,0,0 },    // 7
                              { 1,1,1,1,1,1,1 },    // 8
                              { 1,1,0,1,1,1,1 }};   // 9

void set_sev_segment(int digit, int first_pin) {
  int pin = first_pin;
  for (int i = 0; i < 7; i++) {
    digitalWrite(pin, sev_seg_array[digit][i]);
    pin++;
  }
}

// Get current debounced state of a button. Should be called many times in short succession to be effective.
bool get_button_state(button *but) {
  int set_debounce_time = 50;
  bool reading = digitalRead(but->pin);

  // start debounce timer
  if (reading != but->last_read) {
    but->debounce_time = millis();
  }

  // check debounce timer
  if ((millis() - but->debounce_time) > set_debounce_time) {
    if (reading != but->state) {
      but->state = reading;
    }
  }

  but->last_read = reading;
  
  return but->state;
}

