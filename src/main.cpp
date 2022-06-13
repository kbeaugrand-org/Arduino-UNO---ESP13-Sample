#include <Arduino.h>

#include "uartWiFi.h"
#include "logging.h"

#define STA
#define TCP_BUFFER_SIZE 128

#ifdef SOFT_AP
#include "connect_soft_accesspoint.h"
#define WIFI_SSID "ESP-62FFD20C"
#define WIFI_PWD "WZMxHnSG33zyd3uMhH"
#endif

#ifdef STA
#include "connect_station.h"
#define WIFI_SSID "Bbox-62FFD20C"
#define WIFI_PWD "WZMxHnSG33zyd3uMhH"
#endif

#include "web_server.h"

UARTWifi uartWiFi;

Array<ACCESS_POINT, MAX_SCAN_RESULT_COUNT> scan_results;
char tcp_buffer[TCP_BUFFER_SIZE];

void setup()
{
  uartWiFi.begin();

#ifdef SOFT_AP
  connect_soft_accesspoint(WIFI_SSID, WIFI_PWD, 6);
#endif

#ifdef STA
  connect_to_station(WIFI_SSID, WIFI_PWD);
#endif

  start_web_server();

  // unsigned long start = millis();
  // while (millis() - start < 5000)
  //   ;

  // close_web_server();
  // uartWiFi.quitAP();
}

void loop()
{
  Array<TCP_CHANNEL, 4> tcp_channels;

  if (!uartWiFi.getClientStatus(&tcp_channels))
    return;

  for (unsigned int i = 0; i < tcp_channels.size(); i++)
  {
    if (tcp_channels[i].size <= 0)
    {
      continue;
    }

    uartWiFi.readData(tcp_buffer, tcp_channels[i].channel, TCP_BUFFER_SIZE);
    
    Serial.println(tcp_buffer);

    uartWiFi.send(tcp_channels[i].channel, tcp_buffer);
  }

  tcp_channels.clear();
}