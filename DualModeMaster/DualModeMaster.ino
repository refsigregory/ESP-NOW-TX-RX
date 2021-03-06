/*

  Master/Server:
    Sends a 100Byte ESPNOW data block, and awaits a response
    SLAVE ADDRESS[0x3C,0x3C,0x3C,0x3C,0x3C,0x31]
    SELF=0x3C,0x3C,0x3C,0x3C,0x3C,0x30
 */
extern "C" {
  #include "user_interface.h"
  #include <espnow.h>
}
uint8_t deviceMACAddress[] = {0x3C,0x3C,0x3C,0x3C,0x3C,0x30};
void initVariant()
{
   wifi_set_opmode(STATIONAP_MODE);
   wifi_set_macaddr(STATION_IF, &deviceMACAddress[0]);
}

#include <ESP8266WiFi.h>
#define WIFI_CHANNEL 1

//MAC ADDRESS OF THE DEVICE YOU ARE SENDING TO
byte remoteDevice[6] = {0x3C,0x3C,0x3C,0x3C,0x3C,0x31};
const byte dataLength=200;
byte cnt=0;
byte txrxData[dataLength];
long timerData[3];
long minRTT=100000, maxRTT=0, acumRTT=0, rTTCnt=0;
uint32_t sentTime=0, receivedTime=0, timeTaken=0;

void setup()
{
  Serial.begin(115200);
  Serial.print("\r\n\r\nDevice MAC: ");
  //WiFi.mode(WIFI_STA);
  //WiFi.begin();
  Serial.println(WiFi.macAddress());
  Serial.println("\r\nESP_Now Dual Mode Transmitter + Receiver [MASTER].\r\n");
  esp_now_init();
  delay(10);
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(remoteDevice, ESP_NOW_ROLE_CONTROLLER, WIFI_CHANNEL, NULL, 0);

  esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len)
  {
    //timerData[1] = micros();
    //timerData[2] = timerData[1]-timerData[0];
    receivedTime = (*(volatile uint32_t *)(0x3ff20c00));
    timeTaken = receivedTime-sentTime;
    //Serial.printf("\tReceived [%d]\tTook\t[%d]micros\r\n", data[0], timerData[2]);
    Serial.printf("\tReceived [%d]\tTook\t[%d]micros\r\n", data[0], timeTaken);
    memcpy(txrxData, data, len );
  });
}  

void loop()
{
  //timerData[0] = micros();
  sentTime =  (*(volatile uint32_t *)(0x3ff20c00));
  esp_now_send(remoteDevice, txrxData, dataLength);
  Serial.printf("\tSent [%d]\r\n", txrxData[0]);
  delay(500);

  //analytics
  rTTCnt++;
  //if(minRTT>timerData[2])
  if(minRTT>timeTaken)
  {
    minRTT = timeTaken;
  }
  //if(maxRTT<timerData[2])
  if(maxRTT<timeTaken)
  {
    maxRTT = timeTaken;
  }
  acumRTT+=timeTaken;
  printf("\t\tMinRTT\t%d\tMaxRTT\t%d\tAVG RTT\t%d\r\n", minRTT, maxRTT, acumRTT/rTTCnt);
  //analytics

  txrxData[0]++;
}
