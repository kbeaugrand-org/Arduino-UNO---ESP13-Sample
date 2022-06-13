#ifndef __CONNECT_SOFT_ACCESSPOINT_H__
#define __CONNECT_SOFT_ACCESSPOINT_H__

/**
 * @brief Connects the device to the WiFi network.
 * 
 * @param ssid 
 * @param pwd 
 * @param channel
 */
void connect_soft_accesspoint(const char* ssid, const char *pwd, int channel);

#endif