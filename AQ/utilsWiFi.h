#ifndef UTILSWIFI_H /* include guards */
#define UTILSWIFI_H

// Callback methods prototypes
void wifi_connect_cb();
void wifi_watchdog_cb();
void send_data_cb();

void wifi_init();
void wifi_run();

#endif /* UTILSWIFI_H */
