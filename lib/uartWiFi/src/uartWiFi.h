
#ifndef __UARTWIFI_H__
#define __UARTWIFI_H__

#include <Arduino.h>
#include <Array.h>

#define MAX_SCAN_RESULT_COUNT 5
#define MAX_TCP_CLIENT_COUNT 4

/**
 * @brief The WiFi authentication mode.
 *
 */
typedef enum WIFI_AUTH_MODE
{
    WIFI_AUTH_OPEN = 0,
    WIFI_AUTH_WEP,
    WIFI_AUTH_WAP_PSK,
    WIFI_AUTH_WAP2_PSK,
    WIFI_AUTH_WAP_WAP2_PSK
} WIFI_AUTH_MODE;

/**
 * @brief The Server mode.
 *
 */
typedef enum SERVER_MODE
{
    CLOSED = 0,
    OPEN
} SERVER_MODE;

/**
 * @brief The TCP Server mode.
 * 
 */
typedef struct SERVER_CONFIG
{
    SERVER_MODE mode;
    int port;
} SERVER_CONFIG;

/**
 * @brief The WiFi Connection mode.
 *
 */
typedef enum WIFI_MODE
{
    OFF = 0,
    STATION,
    SOFT_ACCESS_POINT,
    STATION_AND_SOFT_ACCESS_POINT,
} WIFI_MODE;

/**
 * @brief Structure representing a WiFi access point.
 *
 */
typedef struct ACCESS_POINT
{
    WIFI_AUTH_MODE authMode;
    char ssid[32];
    int rssi;
    char mac[18];
    int channel;
} ACCESS_POINT;

/**
 * @brief Structure for the WiFi connection when in station mode.
 *
 */
typedef struct JOIN_AP_CONFIG
{
    char ssid[32];
    int rssi;
    char mac[18];
    int channel;
} JOIN_AP_CONFIG;

/**
 * @brief Structure for the WiFi connection when in soft access point mode.
 * 
 */
typedef struct SOFT_ACCESS_POINT_CONF
{
    char ssid[32];
    char pwd[32];
    int channel;
} SOFT_ACCESS_POINT_CONF;

/**
 * @brief Structure for the TCP connection.
 * 
 */
typedef struct TCP_CHANNEL
{
    int channel;
    int size;
} TCP_CHANNEL;

/**
 * @brief Class representing the WiFi module.
 * It uses the Arduino Serial class to communicate with the module.
 * Protocol is based on the AT command set provided by ESP8266 - WROOM-02 - AT Firmware for WIFI and MQTT firmware.
 * see: https://github.com/kbeaugrand/ESP8266-AT-WIFI-MQTT for more information.
 */
class UARTWifi
{
public:
    // ====================== COMMON ======================

    /**
     * @brief Initializes the WiFi module.
     *
     */
    void begin(void);

    /**
     * @brief Resets the WiFi module.
     *
     */
    void reset(void);

    // ====================== WIFI ======================

    /**
     * @brief Lists the available WiFi networks.
     *
     * @return An array of ACCESS_POINT structures.
     */
    void scan(Array<ACCESS_POINT, MAX_SCAN_RESULT_COUNT> *scan_results);

    /**
     * @brief Configures the Wifi module connection mode (Station, Access Point, Station and Access Point).
     *
     * @param mode The connection mode.
     * @return true if the configuration was successful.
     * @return false  if the configuration was not successful.
     */
    bool configureWiFiMode(WIFI_MODE mode);

    /**
     * @brief Connects to the WiFi network as a station.
     *
     * @param ssid The SSID of the WiFi network.
     * @param pwd The password of the WiFi network.
     * @return true if the connection was successful.
     * @return false if the connection was not successful.
     */
    bool connectAP(const char *ssid, const char *pwd);

    /**
     * @brief Connects to the WiFi network as an Access Point.
     *
     * @param ssid The SSID of the WiFi network.
     * @param pwd The password of the WiFi network.
     * @return true if the connection was successful.
     * @return false if the connection was not successful.
     */
    bool startAP(const char *ssid, const char *pwd, byte chl = 1, WIFI_AUTH_MODE ecn = WIFI_AUTH_WAP_WAP2_PSK, bool hidden = false);

    /**
     * @brief Request for the Wifi module mode.
     *
     * @return true if the request was successful.
     * @return false if the request was not successful.
     */
    WIFI_MODE getWiFiMode(void);

    /**
     * @brief Gets the current access point configured in the Wifi module as a station.
     *
     * @return The SSID of the current access point.
     */
    bool getAccessPoint(JOIN_AP_CONFIG *config);

    /**
     * @brief Disconnects from the WiFi network.
     *
     * @return true if the disconnection was successful.
     * @return false if the disconnection was not successful.
     */
    bool quitAP(void);

    /**
     * @brief Gets the current access point configured in the Wifi module as an Access Point.
     *
     * @return String
     */
    bool getSoftAP(SOFT_ACCESS_POINT_CONF *config);

    // ====================== TCP/IP ======================

    /**
     * @brief Creates a TCP server.
     *
     * @param port The port to listen to.
     * @param mode The server mode.
     * @return
     */
    bool createServer(int port);

    /**
     * @brief Closes the TCP server.
     *
     * @return true if the server was closed.
     * @return false if the server was not closed.
     */
    bool closeServer(int port);

    /**
     * @brief Gets the current TCP server status.
     *
     * @return true if the server is running.
     * @return false if the server is not running.
     */
    bool getServerStatus(SERVER_CONFIG *config);

    /**
     * @brief Gets the current TCP client status.
     *
     * @return true if the client is connected.
     * @return false if the client is not connected.
     */
    bool getClientStatus(Array<TCP_CHANNEL, MAX_TCP_CLIENT_COUNT> *tcp_clients);

    /**
     * @brief Reads the data from the Wifi module.
     * If there is data available, it is read and returned to the buffer.
     * The corresponding channel is returned in the channel parameter.
     * The corresponding size is returned in the size parameter.
     *
     * @param buf The buffer to store the data.
     * @return int The number of bytes read.
     */
    bool readData(char *buf, int chl, int size);

    /**
     * @brief Sends the data to the connection channel.
     *
     * @param id The id of the connection channel.
     * @param str The data to send.
     * @return true if the data was sent.
     * @return false if the data was not sent.
     */
    bool send(byte id, char *str);
};

#endif