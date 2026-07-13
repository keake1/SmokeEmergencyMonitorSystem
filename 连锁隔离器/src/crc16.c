#include "crc16.h"

uint16_t Crc16_Modbus(const uint8_t *data, uint8_t len)
{
    uint16_t crc = 0xFFFFU;
    uint8_t i;
    uint8_t bit;

    for (i = 0U; i < len; ++i) {
        crc ^= data[i];
        for (bit = 0U; bit < 8U; ++bit) {
            if ((crc & 0x0001U) != 0U) {
                crc = (uint16_t)((crc >> 1) ^ 0xA001U);
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

uint8_t Crc16_Check(const uint8_t *frame, uint8_t len)
{
    uint16_t crc;

    if ((frame == 0) || (len < 3U)) {
        return 0U;
    }

    crc = Crc16_Modbus(frame, (uint8_t)(len - 2U));
    return ((frame[len - 2U] == (uint8_t)crc) &&
            (frame[len - 1U] == (uint8_t)(crc >> 8))) ? 1U : 0U;
}

void Crc16_Append(uint8_t *frame, uint8_t payload_len)
{
    uint16_t crc = Crc16_Modbus(frame, payload_len);
    frame[payload_len] = (uint8_t)crc;
    frame[payload_len + 1U] = (uint8_t)(crc >> 8);
}

