#include "web_server.h"

#include <logging.h>
#include <uartWiFi.h>

extern UARTWifi uartWiFi;

SERVER_CONFIG config;

/**
 * @brief Starts the web server.
 *
 */
void start_web_server()
{
    if (!uartWiFi.createServer(8080))
    {
        LogErr("Failed to start web server");
        while (1)
            ;
    }

    LogInfo("Web server started");

    uartWiFi.getServerStatus(&config);

    if (config.mode == SERVER_MODE::OPEN)
    {
        LogInfo("Server is running");
    }
    else
    {
        LogErr("Server is not running");
    }
}

/**
 * @brief Closes the web server.
 *
 */
void close_web_server()
{
    uartWiFi.closeServer(config.port);
}