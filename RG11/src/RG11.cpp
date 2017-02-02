#include <util/atomic.h>
#include <avr/wdt.h>
#include <SPI.h>
#include <Ethernet.h>
#include "Arduino.h"

#include "checksum_fletcher16.h"
#include "RG11.h"

byte mac[] = {0x02, 0xFA, 0xC7, 0x00, 0x00, 0x10};
IPAddress ip(10, 0, 100, 155);

volatile unsigned long dropCounter;
volatile unsigned long dropPulseLength;

const int RG11_Pin = 3;
const unsigned long time_between_message_updates_in_ms = 10UL*1000UL;

void countDrops ()
{
    static unsigned long dropStartTime = 0;
    if (digitalRead(RG11_Pin) == HIGH) {
        dropStartTime = micros();
    }
    else {
        dropPulseLength += micros() - dropStartTime;
        dropCounter++;
    }
}

EthernetServer server(80);
message_t msg;


void setup() {
    Ethernet.begin(mac, ip);
    server.begin();

    pinMode(RG11_Pin, INPUT);
    attachInterrupt(
        digitalPinToInterrupt(RG11_Pin),
        countDrops,
        CHANGE
    );

    wdt_enable(WDTO_8S);
}



/* Check if a client connected via ethernet and requested a status message
 *  if so:
 *    - send out one status message
 *    - reset the watchdog timer
 *    - close connection to signal end of message
 *
 *  Note: If watchdog timer is not reset within 8sec
 *        The watchdog will reset the MCU.
 *        So in case of network issues, which make it impossible to
 *        have a client connecting to us, there is a chance for this MCU reset
 *        to nicely reset the wiznet chip and allow a client to connect again.
 *
 *  Note: in order to request a status message, any valid HTTP request is okay
 *        any valid HTTP request ends in a "double enter" == "\n\n"
 *        so we just look for a "\n\n" in order to send a status message.
*/
void ethernet_communication() {

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
    unsigned long current_millis = millis();
    static unsigned long time_of_last_message_update = 0;

    if (current_millis - time_of_last_message_update > time_between_message_updates_in_ms)
    {
        msg.payload.current_millis = current_millis;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            msg.payload.dropCounter = dropCounter;
            msg.payload.dropPulseLength = dropPulseLength;
            dropCounter = 0;
            dropPulseLength = 0;
        }

        msg.checksum = checksum_fletcher16(
            (byte *)&(msg.payload),
            sizeof(message_payload_t)
        );
        time_of_last_message_update = current_millis;
    }

    ethernet_communication();
}
