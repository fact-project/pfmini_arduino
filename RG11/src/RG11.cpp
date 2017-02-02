#include <avr/wdt.h>
#include <SPI.h>
#include <Ethernet.h>
#include "Arduino.h"

#include "checksum_fletcher16.h"
#include "RG11.h"

const int RG11_1_Pin = 3;
const int debugging_Pin = 7;

byte mac[] = {0x02, 0xFA, 0xC7, 0x00, 0x00, 0x10};
IPAddress ip(10, 0, 100, 155);

volatile unsigned long dropCounter;
volatile unsigned long dropPulseLength;
volatile unsigned long dropStartTime = 0;

EthernetServer server(80);
message_t msg;  // we use this instance to fill it with up to date data.


void countRg1Drops ()
{
    if (digitalRead(RG11_1_Pin) == HIGH) {
        digitalWrite(debugging_Pin, HIGH);
        dropStartTime = micros();
    }
    else {
        digitalWrite(debugging_Pin, LOW);
        dropPulseLength += micros() - dropStartTime;
        dropCounter++;
    }
}

void setup() {
    Ethernet.begin(mac, ip);
    server.begin();

    pinMode(RG11_1_Pin, INPUT);
    pinMode(debugging_Pin, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(RG11_1_Pin), countRg1Drops, CHANGE);

    //wdt_enable(WDTO_8S);
    Serial.begin(115200);

    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());
}

unsigned long current_millis;
unsigned long time_of_last_message = 0;

/* Check if a client connected via ethernet:
 *  if so:
 *    - send out one status message
 *    - reset the watchdog timer
 *
 *  Note: If watchdog timer is not reset within 8sec
 *        The watchdog will reset the MCU.
 *        So in case of network issues, which make it impossible to
 *        have a client connecting to us, there is a chance for this MCU reset
 *        to nicely reset the wiznet chip and allow a client to connect again.
*/
void ethernet_comminucation() {

    EthernetClient client = server.available();
    if (client) {
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();

                if (c == '\n' && currentLineIsBlank) {
                    wdt_reset();
                    client.write((byte*)&msg, sizeof(message_t));
                    break;
                }
                if (c == '\n') {
                    currentLineIsBlank = true;
                }
                else if (c != '\r') {
                    currentLineIsBlank = false;
                }
            }
        }
        delay(1);
        client.stop();
    }

}

void loop() {
    current_millis = millis();

    if (time_of_last_message - current_millis > 1000){
        msg.payload.current_millis = current_millis;
        //noInterrupts();
        msg.payload.dropCounter = dropCounter;
        msg.payload.dropPulseLength = dropPulseLength;
        //interrupts();
        msg.checksum = checksum_fletcher16(
            (byte *)&(msg.payload),
            sizeof(message_payload_t)
        );
        time_of_last_message = current_millis;
        Serial.print("millis:");
        Serial.println(msg.payload.current_millis);

        Serial.print("dropCounter:");
        Serial.println(msg.payload.dropCounter);

        Serial.print("dropPulseLength:");
        Serial.println(msg.payload.dropPulseLength);
        dropCounter = 0;
        dropPulseLength = 0;

    }

    ethernet_comminucation();
    delay(2500); // TODO: get rid of this line, its just here during debugging so Serial.print does not flood us.
}
