#ifndef RG11__H
#define RG11__H

struct message_payload_t {
    unsigned long dropCounter;
    unsigned long dropPulseLength;
    unsigned long current_millis;
};

struct message_t {
    message_payload_t payload;
    uint16_t checksum;
};

#endif // RG11__H