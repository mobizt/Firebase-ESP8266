#ifndef FirebaseESP8266HTTPClient_CPP
#define FirebaseESP8266HTTPClient_CPP


#include "FirebaseESP8266HTTPClient.h"


FirebaseHTTPClient::FirebaseHTTPClient()
{
}

FirebaseHTTPClient::~FirebaseHTTPClient()
{
  client.stop();
}

bool FirebaseHTTPClient::http_begin(const char* host, uint16_t port, const char* uri)
{
  memset(_host, 0, sizeof _host);
  strcpy(_host, host);
  _port = port;
  memset(_uri, 0, sizeof _uri);
  strcpy(_uri, uri);
  return true;
}


bool FirebaseHTTPClient::http_connected()
{
  return client.available() > 0 || client.connected();
}


bool FirebaseHTTPClient::http_sendHeader(const char* header)
{
  if (!http_connected())return false;
  client.print(header);
  return true;
}

int FirebaseHTTPClient::http_sendRequest(const char* header, const char* payload)
{
  size_t size = strlen(payload);
  if (!http_connect()) return HTTPC_ERROR_CONNECTION_REFUSED;
  if (!http_sendHeader(header))return HTTPC_ERROR_SEND_HEADER_FAILED;
  if (size > 0) client.print(payload);
  return 0;
}


bool FirebaseHTTPClient::http_connect(void)
{
  if (http_connected()) {
    while (client.available() > 0) client.read();
    return true;
  }

  if (!client.connect(_host, _port))return false;
  return http_connected();

}
#endif
