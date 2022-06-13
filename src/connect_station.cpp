#include <uartWiFi.h>
#include <logging.h>

extern UARTWifi uartWiFi;
extern Array<ACCESS_POINT, MAX_SCAN_RESULT_COUNT> scan_results;

void connect_to_station(const char *ssid, const char *pwd)
{
  uartWiFi.scan(&scan_results);

  LogInfo("Access points found : %d", scan_results.size());

  // for (ACCESS_POINT ap : scan_results)
  // {
  //   if (strcmp(ap.ssid, ssid) != 0)
  //     continue;

  LogInfo("Connecting to %s", ssid);

  uartWiFi.configureWiFiMode(STATION);

  if (uartWiFi.connectAP(ssid, pwd))
  {
    LogInfo("Connected to %s", ssid);
  }
  else
  {
    LogErr("Failed to connect to %s", ssid);
    while (1)
      ;
  }
  // }

  scan_results.clear();

  WIFI_MODE mode = uartWiFi.getWiFiMode();

  if (mode != WIFI_MODE::STATION)
  {
    LogErr("Wifi mode is not STATION");
    while (1)
      ;
  }

  JOIN_AP_CONFIG conf;
  if (!uartWiFi.getAccessPoint(&conf))
  {
    LogErr("Failed to get access point");
    while (1)
      ;
  }

  LogInfo("Current access point: %s", conf.ssid);
}