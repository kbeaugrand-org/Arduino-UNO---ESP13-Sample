#include <uartWiFi.h>
#include <logging.h>

#include "connect_soft_accesspoint.h"

#ifndef ENCRYPTION_TYPE
#define ENCRYPTION_TYPE WIFI_AUTH_OPEN
#endif

extern UARTWifi uartWiFi;

/**
 * @brief Connects the device to the WiFi network.
 *
 * @param ssid
 * @param pwd
 * @param channel
 */
void connect_soft_accesspoint(const char *ssid, const char *pwd, int channel)
{
    uartWiFi.configureWiFiMode(SOFT_ACCESS_POINT);

    if (uartWiFi.startAP(ssid, pwd, channel, ENCRYPTION_TYPE))
    {
        LogInfo("Soft AP started");
    }
    else
    {
        LogErr("Failed to start soft AP");
        while (1)
            ;
    }

    WIFI_MODE mode = uartWiFi.getWiFiMode();

    if (mode != WIFI_MODE::SOFT_ACCESS_POINT)
    {
        LogErr("Wifi mode is not SOFT_ACCESS_POINT");
        while (1)
            ;
    }

    SOFT_ACCESS_POINT_CONF conf;

    if (!uartWiFi.getSoftAP(&conf))
    {
        LogErr("Failed to get soft AP configuration");
        while (1)
            ;
    }

    if (strcmp(conf.ssid, ssid) != 0 ||
        strcmp(conf.pwd, pwd) != 0 ||
        conf.channel != channel)
    {
        LogErr("Soft AP configuration mismatch");
        while (1)
            ;
    }
}