/*
 * HTTP Client wrapper v1.0.3
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
  _client.stop();
}

bool FirebaseHTTPClient::begin(const std::string host, uint16_t port)
{
  _host = host;
  _port = port;
#ifndef USING_AXTLS
  _client.setBufferSizes(512, 512);
  _client.setInsecure();
#endif
  _client.setNoDelay(true);
  return true;
}

bool FirebaseHTTPClient::connected()
{
  return _client.connected();
}

bool FirebaseHTTPClient::sendHeader(const char *header)
{

  if (!connected())
    return false;
  _client.print(header);
  return true;
}

int FirebaseHTTPClient::sendRequest(const char *header, const char *payload)
{

  size_t size = strlen(payload);
  if (!connect())
    return HTTPC_ERROR_CONNECTION_REFUSED;
  if (!sendHeader(header))
    return HTTPC_ERROR_SEND_HEADER_FAILED;
  if (size > 0)
    _client.print(payload);

  return 0;
}

bool FirebaseHTTPClient::connect(void)
{
  if (connected())
  {
    while (_client.available() > 0)
      _client.read();
    return true;
  }

  if (!_client.connect(_host.c_str(), _port))
    return false;

  return connected();
}
#endif