#include "uartWiFi.h"

#include <Arduino.h>
#include <Array.h>
#include <logging.h>

#define _uartCell Serial

#define READ_TIMEOUT 10000

static char *OK_STR = (char *)"OK";
static char *ERR_STR = (char *)"ERROR";

/**
 * @brief Flush the UART Serial buffer.
 *
 */
void _flushUARTCellRead()
{
    _uartCell.flush();

    while (_uartCell.available())
        _uartCell.read();
}

/**
 * @brief Resets the WiFi module.
 *
 */
void UARTWifi::reset(void)
{
    _flushUARTCellRead();
    _uartCell.println("AT+RST");
    _uartCell.flush();

    if (!_uartCell.find(OK_STR))
    {
        LogErr("Failed to reset WiFi module");
        while (1)
            ;
    }

    char *ready_str = (char *)"ready";

    if (!_uartCell.find(ready_str))
    {
        LogErr("Failed to reset WiFi module");
        while (1)
            ;
    }
}

/**
 * @brief Initializes the WiFi module.
 *
 */
void UARTWifi::begin(void)
{
    _uartCell.begin(115200);
    _uartCell.setTimeout(10000);

    reset();

    _uartCell.println("AT");
    _uartCell.flush();

    if (_uartCell.find("OK"))
    {
        LogInfo("WiFi module initialized");
        return;
    }

    LogErr("No response from module - UART Wifi initialization failed");

    while (1)
        ;
}

/**
 * @brief Connects to the WiFi network as a station.
 *
 * @param ssid The SSID of the WiFi network.
 * @param pwd The password of the WiFi network.
 * @return true if the connection was successful.
 * @return false if the connection was not successful.
 */
bool UARTWifi::connectAP(const char *ssid, const char *password)
{
    char buffer[512];
    sprintf(buffer, "AT+CWJAP=\"%s\",\"%s\"", ssid, password);

    _flushUARTCellRead();
    _uartCell.println(buffer);
    _uartCell.flush();

    char *connected_str = (char *)"CONNECTED";

    if (!_uartCell.find(connected_str))
    {
        LogErr("Failed to connect to WiFi network");
        return false;
    }

    if (!_uartCell.find(OK_STR))
    {
        return false;
    }

    LogDebug("Connected to WiFi network");

    return true;
}

/**
 * @brief Lists the available WiFi networks.
 *
 * @return An array of ACCESS_POINT structures.
 */
void UARTWifi::scan(Array<ACCESS_POINT, MAX_SCAN_RESULT_COUNT> *scan_results)
{
    _flushUARTCellRead();
    _uartCell.println("AT+CWLAP");
    _uartCell.flush();

    unsigned long start = millis();

    while (millis() - start < READ_TIMEOUT)
    {
        String line = Serial.readStringUntil('\n');

        line.trim();

        if (scan_results->full())
        {
            LogWarn("Too many scan results");
            return;
        }

        if (line.equals(OK_STR))
        {
            LogInfo("Scan completed");
            return;
        }

        if (line.equals(ERR_STR))
        {
            LogErr("Scan failed");
            return;
        }

        ACCESS_POINT ap;
        int mode;
        sscanf(line.c_str(), "+CWLAP:%d,%[^,],%d,%[^,],%d", &mode, &ap.ssid, &ap.rssi, &ap.mac, &ap.channel);
        ap.authMode = (WIFI_AUTH_MODE)mode;

        scan_results->push_back(ap);

        LogTrace("Found AP: %s, %d, %s, %d, %d", ap.ssid, ap.rssi, ap.mac, ap.channel, ap.authMode);
    }

    LogWarn("No response from the module.");

    return;
}

/**
 * @brief Gets the current access point configured in the Wifi module as a station.
 *
 * @return The SSID of the current access point.
 */
bool UARTWifi::getAccessPoint(JOIN_AP_CONFIG *ap)
{
    _flushUARTCellRead();

    unsigned long start = millis();

    _uartCell.println("AT+CWJAP?");
    _uartCell.flush();

    while (millis() - start < READ_TIMEOUT)
    {
        String line = Serial.readStringUntil('\n');
        line.trim();

        if (line.equals(OK_STR))
        {
            LogInfo("Completed");
            return false;
        }

        if (line.startsWith("+CWJAP:"))
        {
            sscanf(line.c_str(), "+CWJAP:%[^,],%[^,],%d,%d", &ap->ssid, &ap->mac, &ap->channel, &ap->rssi);
            return true;
        }
    }

    return false;
}

/**
 * @brief Connects to the WiFi network as an Access Point.
 *
 * @param ssid The SSID of the WiFi network.
 * @param pwd The password of the WiFi network.
 * @return true if the connection was successful.
 * @return false if the connection was not successful.
 */
bool UARTWifi::startAP(const char *ssid, const char *pwd, byte chl = 1, WIFI_AUTH_MODE ecn = WIFI_AUTH_WAP_WAP2_PSK, bool hidden = false)
{
    char buffer[512];
    // AT+CWSAP=<ssid>,<pwd>,<chl>,<ecn>,<max_conn>,<ssid hidden>
    sprintf(buffer, "AT+CWSAP=\"%s\",\"%s\",%d,%u,%u,%u", ssid, pwd, chl, ecn, 4, hidden);

    _flushUARTCellRead();
    _uartCell.println(buffer);
    _uartCell.flush();

    if (!_uartCell.find(OK_STR))
    {
        return false;
    }

    LogDebug("Access Point started");

    return true;
}

/**
 * @brief Configures the Wifi module connection mode (Station, Access Point, Station and Access Point).
 *
 * @param mode The connection mode.
 * @return true if the configuration was successful.
 * @return false  if the configuration was not successful.
 */
bool UARTWifi::configureWiFiMode(WIFI_MODE mode)
{
    _flushUARTCellRead();

    char buffer[15];
    sprintf(buffer, "AT+CWMODE=%d", mode);

    _uartCell.println(buffer);
    _uartCell.flush();

    if (_uartCell.find(OK_STR))
    {
        LogDebug("WiFi mode configured");
        return true;
    }

    LogErr("Failed to configure WiFi mode");
    return false;
}

/**
 * @brief Request for the Wifi module mode.
 *
 * @return true if the request was successful.
 * @return false if the request was not successful.
 */
WIFI_MODE UARTWifi::getWiFiMode(void)
{
    _flushUARTCellRead();

    char buffer[15];
    sprintf(buffer, "AT+CWMODE?");

    unsigned long start = millis();
    _uartCell.println(buffer);
    _uartCell.flush();

    WIFI_MODE mode;

    while (millis() - start < READ_TIMEOUT)
    {
        String line = Serial.readStringUntil('\n');
        line.trim();

        if (line.equals(OK_STR))
        {
            LogInfo("Completed");
            return mode;
        }

        if (line.startsWith("+CWMODE:"))
        {
            sscanf(line.c_str(), "+CWMODE:%d", &mode);
            continue;
        }
    }
}

/**
 * @brief Disconnects from the WiFi network.
 *
 * @return true if the disconnection was successful.
 * @return false if the disconnection was not successful.
 */
bool UARTWifi::quitAP(void)
{
    _flushUARTCellRead();

    Serial.println("AT+CWQAP");

    if (_uartCell.find(OK_STR))
    {
        LogDebug("Disconnected from AP");
        return true;
    }

    _flushUARTCellRead();

    return false;
}

/**
 * @brief Gets the current access point configured in the Wifi module as an Access Point.
 *
 * @return String
 */
bool UARTWifi::getSoftAP(SOFT_ACCESS_POINT_CONF *config)
{
    _flushUARTCellRead();

    Serial.println("AT+CWSAP?");
    _uartCell.flush();

    unsigned long start = millis();

    while (millis() - start < READ_TIMEOUT)
    {
        String line = Serial.readStringUntil('\n');
        line.trim();

        if (line.equals(OK_STR))
        {
            LogInfo("Completed");
            return true;
        }

        if (line.startsWith("+CWSAP:"))
        {
            sscanf(line.c_str(), "+CWSAP:%[^,],%[^,],%d", &config->ssid, &config->pwd, &config->channel);
            continue;
        }
    }
}

/**
 * @brief Creates a TCP server.
 *
 * @param port The port to listen to.
 * @return
 */
bool UARTWifi::createServer(int port)
{
    _flushUARTCellRead();

    // AT+CIPSERVER=<mode>,<port>
    char buffer[25];
    sprintf(buffer, "AT+CIPSERVER=%d,%d", SERVER_MODE::OPEN, port);
    _uartCell.println(buffer);
    _uartCell.flush();

    if (_uartCell.find(OK_STR))
    {
        LogDebug("Server started");
        return true;
    }

    return false;
}

/**
 * @brief Closes the TCP server.
 *
 * @return true if the server was closed.
 * @return false if the server was not closed.
 */
bool UARTWifi::closeServer(int port)
{
    _flushUARTCellRead();

    char buffer[25];
    sprintf(buffer, "AT+CIPSERVER=%d,%d", SERVER_MODE::CLOSED, port);
    _uartCell.println(buffer);
    _uartCell.flush();

    if (_uartCell.find(OK_STR))
    {
        LogDebug("Server closed");
        return true;
    }

    return false;
}

/**
 * @brief Gets the current TCP server status.
 *
 * @return true if the server is running.
 * @return false if the server is not running.
 */
bool UARTWifi::getServerStatus(SERVER_CONFIG *config)
{
    _flushUARTCellRead();

    Serial.println("AT+CIPSERVER?");
    _uartCell.flush();

    unsigned long start = millis();

    while (millis() - start < READ_TIMEOUT)
    {
        String line = Serial.readStringUntil('\n');
        line.trim();

        if (line.equals(OK_STR))
        {
            LogInfo("Completed");
            return true;
        }

        if (line.startsWith("+CIPSERVER:"))
        {
            int mode;
            sscanf(line.c_str(), "+CIPSERVER:%d,%d", &mode, &config->port);
            config->mode = (SERVER_MODE)mode;
            continue;
        }
    }
}

/**
 * @brief Gets the current TCP client status.
 *
 * @return true if the client is connected.
 * @return false if the client is not connected.
 */
bool UARTWifi::getClientStatus(Array<TCP_CHANNEL, MAX_TCP_CLIENT_COUNT> *tcp_clients)
{
    _flushUARTCellRead();

    Serial.println("AT+CIPRECVLEN?");
    _uartCell.flush();

    unsigned long start = millis();

    bool has_client = false;

    while (millis() - start < READ_TIMEOUT)
    {
        String line = Serial.readStringUntil('\n');
        line.trim();

        if (line.equals(OK_STR))
        {
            LogInfo("Completed");
            return has_client;
        }

        if (line.startsWith("+CIPRECVLEN:"))
        {
            TCP_CHANNEL channel;
            sscanf(line.c_str(), "+CIPRECVLEN:%d,%d", &channel.channel, &channel.size);
            tcp_clients->push_back(channel);
            has_client = true;
            continue;
        }
    }

    return has_client;
}

/**
 * @brief Reads the data from the Wifi module.
 * If there is data available, it is read and returned to the buffer.
 * The corresponding channel is returned in the channel parameter.
 * The corresponding size is returned in the size parameter.
 *
 * @param buf The buffer to store the data.
 * @return int The number of bytes read.
 */
bool UARTWifi::readData(char *buf, int chl, int size)
{
    _flushUARTCellRead();

    char buffer[25];
    sprintf(buffer, "AT+CIPRECVDATA=%d,%d", chl, size);
    _uartCell.println(buffer);
    _uartCell.flush();

    unsigned long start = millis();

    size_t a_size;
    int channel;

    while (millis() - start < READ_TIMEOUT)
    {
        String line = Serial.readStringUntil('\n');
        line.trim();

        if (line.startsWith("+CIPRECVDATA:"))
        {
            sscanf(line.c_str(), "+CIPRECVDATA:%d,%d", &channel, &a_size);
            break;
        }
    }

    int offset = 0;

    while ((offset < size) && (millis() - start < READ_TIMEOUT))
    {
        if (Serial.available())
        {
            buf[offset] = Serial.read();

            if (buf[offset] == '\n')
            {
                break;
            }

            offset++;
        }
    }

    buf[offset] = '\0';

    return true;
}

/**
 * @brief Sends the data to the connection channel.
 *
 * @param id The id of the connection channel.
 * @param str The data to send.
 * @return true if the data was sent.
 * @return false if the data was not sent.
 */
bool UARTWifi::send(byte id, char *str)
{
    // AT+CIPSEND=<chan>,<len>
    _flushUARTCellRead();

    char buffer[25];

    sprintf(buffer, "AT+CIPSEND=%d,%d", id, strlen(str) + 1);

    _uartCell.println(buffer);
    _uartCell.flush();

    unsigned long start = millis();

    while (millis() - start < READ_TIMEOUT)
    {
        String line = Serial.readStringUntil('\n');
        line.trim();

        if (line.startsWith(OK_STR))
        {
            break;
        }
    }

    Serial.find(">");

    Serial.println(str);

    if (!Serial.find("SEND OK"))
    {
        LogErr("SEND FAILED");
        return false;
    }

    _flushUARTCellRead();

    return true;
}