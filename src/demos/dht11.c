// based on https://github.com/vmilea/pico_dht/
#include "demos/dht11.h"
#include "hal/sensors/dht.h"
#include "pico/stdlib.h"
#include <stdio.h>

void dht11_demo_execute(void)
{

    dht_t dht;
    dht_init(&dht, DHT11, pio0, 0, true /* pull_up */);

    while (true)
    {
        dht_start_measurement(&dht);

        float humidity;
        float temperature_c;
        dht_result_t result = dht_finish_measurement_blocking(&dht, &humidity, &temperature_c);
        if (result == DHT_RESULT_OK)
        {
            printf("Temperature: %.1f C Humidity: %.1f %%\n", temperature_c, humidity);
        }
        else
        {
            printf("DHT11 measurement error: %d\n", result);
        }
        sleep_ms(2000);
    }
}