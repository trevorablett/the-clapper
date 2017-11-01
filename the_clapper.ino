#define LED_PIN 13
#define MIC_PIN 0
#define SWITCH_PIN 2
#define REL_PIN 3
#define BUT_PIN 4
#define SEV_SEG_START 5
#define AVG_SIG_SIZE 50
#define MIN_DIFF_TIME 300
#define MAX_DIFF_TIME 1000

#include "the_clapper.h"

int diff_thresh = 8; // user settable using a button
byte avg_sig[AVG_SIG_SIZE];
byte sig_index = 0;
int total = 0;
int mean_sig = 0;
bool first_fill = true;
bool clap_1 = false;
bool relay = false;
unsigned long clap_1_millis = 0;
unsigned long clap_2_millis = 0;
button thresh_button = {BUT_PIN, true, true, 0};  // def'n in header
bool prev_thresh_but_state = true;  // pullup resistor

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(REL_PIN, OUTPUT);
  pinMode(BUT_PIN, INPUT_PULLUP);

  for (int i = 0; i < AVG_SIG_SIZE; i++) {
    avg_sig[i] = 0;
  }

  // for seven segment
  for (int i = SEV_SEG_START; i < (SEV_SEG_START + 7); i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, 0);
  }
}

void loop() {
  int mic_val = analogRead(MIC_PIN);
  //Serial.println(mic_val);
  delay(1); // todo: consider replacing with timer calls instead

  total = total - avg_sig[sig_index];
  avg_sig[sig_index] = mic_val;
  total = total + avg_sig[sig_index];

  if (sig_index < (AVG_SIG_SIZE-1)) {
    sig_index++;
  }
  else {
    sig_index = 0;
    first_fill = false;
  }
  
  mean_sig = total / AVG_SIG_SIZE;
  //Serial.println(mic_val);

  // detect first clap
  unsigned long time_since_clap_2 = millis() - clap_2_millis;
  if ((abs(mean_sig - mic_val) > diff_thresh) and (!first_fill) and (!clap_1)
      and (time_since_clap_2 > MIN_DIFF_TIME)) {
    Serial.println("clap 1 found!");
    clap_1 = true;
    clap_1_millis = millis();
  }

  // detect second clap, make sure interval is appropriate
  unsigned long clap_diff = millis() - clap_1_millis;
  if ((abs(mean_sig - mic_val) > diff_thresh) and (!first_fill) and (clap_1)) {
    if ((clap_diff > MIN_DIFF_TIME) and (clap_diff <= MAX_DIFF_TIME)) {
      Serial.println("clap 2 found!");
      clap_1 = false;
      clap_2_millis = millis();

      // switch relay
      relay = !relay;
    }
  }

  // reset clap 1 if no clap is found
  if (clap_diff > MAX_DIFF_TIME and clap_1) {
    Serial.println("no clap 2 found...");
    clap_1 = false;
  }

  // turn on light if successful double clap
  if (time_since_clap_2 < MIN_DIFF_TIME) {
    digitalWrite(LED_PIN, 1);
  }
  else {
    digitalWrite(LED_PIN, 0);
  }

  // write relay, or if switch is on, always turn it on
  if (!digitalRead(SWITCH_PIN)) {
    digitalWrite(REL_PIN, 1);
  }
  else {
    digitalWrite(REL_PIN, relay);
  }

  // change difference threshold using button
  bool thresh_but_state = get_button_state(&thresh_button);
  
  // detect rising edge only (aka button release)
  if (thresh_but_state and !prev_thresh_but_state) {
    if (diff_thresh < 9) {
      diff_thresh++;
    }
    else {
      diff_thresh = 3;
    }
    Serial.println(diff_thresh);
  }
  prev_thresh_but_state = thresh_but_state;

  // show current diff_thresh using sev segment
  set_sev_segment(diff_thresh, SEV_SEG_START);
  
}
