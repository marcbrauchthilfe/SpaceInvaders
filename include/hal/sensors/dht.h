// based on https://github.com/vmilea/pico_dht/
#ifndef DHT_H
#define DHT_H

#include <hardware/pio.h>
#include <stdint.h>

typedef enum dht_model_t
{
    DHT11,
    DHT12,
    DHT21,
    DHT22,
} dht_model_t;

typedef struct dht_t
{
    PIO pio;
    uint8_t model;
    uint8_t pio_program_offset;
    uint8_t sm;
    uint8_t dma_chan;
    uint8_t data_pin;
    uint8_t data[5];
    uint32_t start_time;
} dht_t;

typedef enum dht_result_t
{
    DHT_RESULT_OK,           // No error
    DHT_RESULT_TIMEOUT,      // DHT sensor not reponding
    DHT_RESULT_BAD_CHECKSUM, // Sensor data doesn't match checksum
} dht_result_t;

void dht_init(dht_t *dht, dht_model_t model, PIO pio, uint8_t data_pin, bool pull_up);
void dht_deinit(dht_t *dht);
void dht_start_measurement(dht_t *dht);
dht_result_t dht_finish_measurement_blocking(dht_t *dht, float *humidity, float *temperature_c);

#endif // DHT_H
