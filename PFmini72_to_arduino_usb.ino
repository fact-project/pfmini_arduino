#include <SPI.h>

const int rel_hum_pin = A0;
const int temperature_pin = A1;

int rel_hum_adc = 0;
int temperature_adc = 0;

float rel_hum = 0.;
float temperature = 0.;


void setup() {
  analogReference(INTERNAL);
  // initialize serial communications at 9600 bps:
  Serial.begin(115200);  
}

void loop() {
  
  rel_hum_adc = analogRead(rel_hum_pin);
  temperature_adc = analogRead(temperature_pin);
  
  rel_hum = rel_hum_adc/1024. * 1.1* 100; // in %
  temperature = (temperature_adc / 1024. * 1.1 ) * 100. - 20.; // in deg C

  Serial.print("rel_hum = ");
  Serial.print(rel_hum);
  Serial.print("% \t temperature_adc = ");
  Serial.print(temperature);
  Serial.println("deg C");

  delay(2000);

}
