/* Differential Controller to control the circulation loop between
 *  Phil's heat pump storage tank and his DHW preheat tank.
*/

// Constants that map Trim Pot range to values
#define MIN_DELTA_T      0.0     // deg F
#define MAX_DELTA_T      8.0     // deg F
#define MIN_DEADBAND     1.0     // deg F
#define MAX_DEADBAND     4.0     // deg F

// ON and OFF constants
#define ON    1
#define OFF   0

#define LED_PIN      7  // LED signifying whether relay is On or Off
#define RELAY_PIN   10  // AC Output Relay 

// Trim Pot Pins
#define DELTA_T_PIN    8  // Wiper Pin for Pot that sets On Delta-T
#define DEADBAND_PIN   9  // Wiper Pin for Pot that sets deadband

// Thermistor Pins
#define THOT_PIN    0
#define TCOLD_PIN   1

// Sets the Relay and indicator LED according to 'state', which is either
// ON or OFF.
void set_relay(int state) {
  if (state == ON) {
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  } else {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
  }
}

// Returns temperature in deg F from a thermistor on pin 'pinNum'
float readTemp(int pinNum) {
  const int ITER = 64;          // # of averaging reads
  // Steinhart coefficients for BAPI 10K-3 thermistor
  const float C1 = 0.001028172;
  const float C2 = 0.0002392811;
  const float C3 = 1.5611865E-07;
  int i;   // loop counter
  float val = 0.0;      // read from analog pin
  for(i=0; i<ITER; i++) {
    val += analogRead(pinNum);    // read the input pin
    delay(1);
  }
  val /= (float)ITER;
  if (val > 1022.0) val = 1022.0;     // probably an open circuit. restrict to 1022.
  float resis = val * 4990.0 / (1023.0 - val);
  float lnR = log(resis);
  return (1.8 / (C1 + C2 * lnR + C3 * lnR * lnR * lnR)) - 459.67;
}

void setup() {

  pinMode(LED_PIN, OUTPUT);  // Set 3 mm LED as an output
  
  // Relay is an output
  pinMode(RELAY_PIN, OUTPUT);
  
  Serial.begin(9600); //This pipes to the serial monitor
  delay(300);
  Serial.println("Reboot...");

}

float on_delta_t;       // On delta-T setting, deg F
float deadband;         // Deadband setting, deg F
float t_hot;            // temperature of hot source, deg F
float t_cold;           // temperature of cold sink, deg F
float cur_delta_t;      // current delta-T between Hot and Cold temps

int relay_state = OFF;    // current state of the relay

void loop() {

  // Read Pots and Sensors
  on_delta_t = analogRead(DELTA_T_PIN) / 1023.0 * (MAX_DELTA_T - MIN_DELTA_T) + MIN_DELTA_T;
  deadband = analogRead(DEADBAND_PIN) / 1023.0 * (MAX_DEADBAND - MIN_DEADBAND) + MIN_DEADBAND;
  t_hot = readTemp(THOT_PIN);
  t_cold = readTemp(TCOLD_PIN);
  cur_delta_t = t_hot - t_cold;

  // Determine relay state
  if (relay_state == OFF) {
    if (cur_delta_t >= on_delta_t) relay_state = ON;
  } else {
    if (cur_delta_t < on_delta_t - deadband) relay_state = OFF;
  }
  set_relay(relay_state);

  // Report inputs and outputs
  Serial.print(on_delta_t);
  Serial.print(" ");
  Serial.print(deadband);
  Serial.print(" ");
  Serial.print(t_hot);
  Serial.print(" ");
  Serial.print(t_cold);
  Serial.print(" ");
  Serial.println(relay_state);
  
  delay(1000);
  
}
