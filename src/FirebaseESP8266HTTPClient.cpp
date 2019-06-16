/*
 * HTTP Client wrapper v1.0.0
 * 
 * The MIT License (MIT)
 * Copyright (c) 2019 K. Suwatchai (Mobizt)
 * 
 * 
 * Permission is hereby granted, free of charge, to any person returning a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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

bool FirebaseHTTPClient::http_begin(const std::string host, uint16_t port)
{
  _host = host;
  _port = port;
  return true;
}

bool FirebaseHTTPClient::http_connected()
{
  return client.available() > 0 || client.connected();
}

bool FirebaseHTTPClient::http_sendHeader(const char *header)
{
  if (!http_connected())
    return false;
  client.print(header);
  return true;
}

int FirebaseHTTPClient::http_sendRequest(const char *header, const char *payload)
{
  size_t size = strlen(payload);
  if (!http_connect())
    return HTTPC_ERROR_CONNECTION_REFUSED;
  if (!http_sendHeader(header))
    return HTTPC_ERROR_SEND_HEADER_FAILED;
  if (size > 0)
    client.print(payload);
  return 0;
}

bool FirebaseHTTPClient::http_connect(void)
{
  if (http_connected())
  {
    while (client.available() > 0)
      client.read();
    return true;
  }

  if (!client.connect(_host.c_str(), _port))
    return false;
  return http_connected();
}
#endif