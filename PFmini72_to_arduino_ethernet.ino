#include <avr/wdt.h>
#include <SPI.h>
#include <Ethernet.h>

const int rel_hum_pin = A0;
const int temperature_pin = A1;

const int message_length_in_words = 3;
const int payload_length_in_words = 2; // 3rd word is checksum, not part of payload
uint16_t message[message_length_in_words];

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0x02, 0xFA, 0xC7, 0x00, 0x00, 0x01
};
IPAddress ip(10, 0, 130, 140);


uint16_t checksum_fletcher16( byte const *data, uint16_t bytes ) {
    uint16_t sum1 = 0xff, sum2 = 0xff;

    while (bytes) {
            uint16_t tlen = bytes > 20 ? 20 : bytes;
            bytes -= tlen;
            do {
                    sum2 += sum1 += *data++;
            } while (--tlen);
            sum1 = (sum1 & 0xff) + (sum1 >> 8);
            sum2 = (sum2 & 0xff) + (sum2 >> 8);
    }
    /* Second reduction step to reduce sums to 8 bits */
    sum1 = (sum1 & 0xff) + (sum1 >> 8);
    sum2 = (sum2 & 0xff) + (sum2 >> 8);
    return sum2 << 8 | sum1;
}


// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  // this means the ADC has 1.1V reference voltage. 
  // so 4095 corresponds to 1.1V
  analogReference(INTERNAL);
  // initialize serial communications at 9600 bps:
  Serial.begin(115200);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();

  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  
  wdt_enable(WDTO_8S);
}

void loop() {
  
  const uint16_t hum_adc_value = analogRead(rel_hum_pin);
  const uint16_t temp_adc_value = analogRead(temperature_pin);

  message[0] = hum_adc_value;
  message[1] = temp_adc_value;
  message[2] = checksum_fletcher16((byte*)message, sizeof(message[0]) * payload_length_in_words );
  
  //rel_hum = rel_hum_adc/1024. * 1.1* 100; // in %
  //temperature = (temperature_adc / 1024. * 1.1 ) * 100. - 20.; // in deg C

  delay(2);

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          
          wdt_reset();
          
          client.write((byte*)message, sizeof(message[0]) * message_length_in_words);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }

}
