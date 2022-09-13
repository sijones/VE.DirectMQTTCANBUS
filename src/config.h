/*
   VE.Direct config file.
   Cconfiguration parameters for
   VE.Direct2MQTT gateway

   GITHUB Link

   MIT License

   Copyright (c) 2020 Ralf Lehmann
   Copyright (c) 2022 Simon Jones


   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

/*
   Defines that activate features like OTA (over the air update)
   or Acitve/Passive mode
*/

//#define CAN_INT 22                        // CAN Init Pin for M5Stack
#define CAN_CS_PIN 2                      // CAN CS PIN
#define initBattChargeVoltage 56000       // Battery Charge Voltage sent to inverter
#define initBattDischargeVoltage 45000    // Battery discharge voltage, not currently used
#define initBattChargeCurrent 100000      // in mA, this is just the initial / max setting, it will self adjust
#define initBattDischargeCurrent 100000   // in mA 
#define initBattCapacity 475000           // used for charge limits when batteries becoming full.
          
// To use PYLONTECH Protocol enable below
#define USE_PYLONTECH

// use SSL to connect to MQTT Server or OTA Server
// it is strongly recommended to use SSL if you send any password over the net
// connectiong to MQTT might need a password; the same for OTA
// if you do not have SSL activated on your servers rename it to NO_USE_SSL
#define NO_USE_SSL

// Activate Over The Air Update of firmware
#define USE_OTA

//
// Use OneWire temperature sensors 
//
//#define USE_ONEWIRE

#ifdef USE_ONEWIRE
#define ONEWIRE_PIN 22
/*
   define the wait time between 2 attempts to send one wire data
   300000 = every 5 minutes
*/
int OW_WAIT_TIME = 10; // in s
time_t last_ow;
#endif



#ifdef USE_SSL
/*
   SSL certificate
   This is good for all let's encrypt certificates for MQTT or OTA servers
*/
/*
  This is lets-encrypt-x3-cross-signed.pem
*/
const char* rootCACertificate = \
                                "-----BEGIN CERTIFICATE-----\n" \
                                "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n" \
                                "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
                                "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n" \
                                "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n" \
                                "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \
                                "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n" \
                                "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n" \
                                "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n" \
                                "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n" \
                                "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n" \
                                "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n" \
                                "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n" \
                                "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n" \
                                "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n" \
                                "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n" \
                                "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n" \
                                "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n" \
                                "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n" \
                                "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n" \
                                "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n" \
                                "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n" \
                                "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n" \
                                "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n" \
                                "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n" \
                                "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n" \
                                "-----END CERTIFICATE-----\n";

#endif


/*
   WiFi parameters
*/

// WiFi SSID'S and passwords
// the strongest WiFi station will be used
const char* ssid = "";
const char* pw = "";

/*
   MQTT parameters
   ATTENTION: use a unique client id to connect to MQTT or you will be kicked out by another device
   using your id
*/
//#define MQTT_MAX_RETRIES 3   // maximum retires to reach a MQTT broker
const char* mqtt_server = "";
// no SSL ports
const uint16_t mqtt_port = 1883;
// SSL ports
//const uint16_t mqtt_port[] = {8883, 8883};
const char* mqtt_clientID = "vedirectmqtt";
const char* mqtt_username = "emon";
const char* mqtt_password = "emonmqtt";
//int mqtt_server_count = sizeof(mqtt_server) / sizeof(mqtt_server[0]);

// this is the MQTT prefix; below that we use the string from VE.Direct
// e.g. /MPPT75-15/PID  for Product ID
String MQTT_PREFIX = "SMARTBMS";
String MQTT_PARAMETER = "/Parameter"; 
#ifdef USE_ONEWIRE
String MQTT_ONEWIRE = "/Temp/OneWire";
#endif


#ifdef USE_OTA
/*

*/



#endif

/*
   Software serial parameter
   These are the pins for the VE.Direct connection
   WARNING: if your VE.Direct device uses 5V please use a 1kOhm/2kOhm divider for the receive line
   The sending line does not need any modification. The ESP uses 3.3V and that's it. A 5V device
   should be able to read that voltage as input
*/
#ifndef VEDIRECT_RX
#define VEDIRECT_RX 33  // connected to TX of the VE.Direct device; ATTENTION divider may be needed, see abowe
#endif
#ifndef VEDIRECT_TX
#define VEDIRECT_TX 32 // connected to RX of the VE:DIRECT device
#endif

/*
   Depending on the DE.Direct device there will be several Key/Value pairs;
   Define the maximum count of key/value pairs
*/
#define MAX_KEY_VALUE_COUNT 30

/*
   Number of Key-Value blocks we can buffer
   MQTT may be slower than one second, especially when we have to reconnect
   this is the number of buffers we can keep
*/
#define MAX_BLOCK_COUNT 5

/*
   Wait time in Loop
   this determines how many frames are send to MQTT
   if wait time is e.g. 10 minutes, we will send only every 10 minutes to MQTT
   Note: only the last incoming block will be send; all previous blocks will be discarded
   Wait time is in seconds
   Waittime of 1 or 0 means every received packet will be transmitted to MQTT
   Packets during OTA or OneWire will be discarded
*/
int VE_WAIT_TIME = 1; // in s
