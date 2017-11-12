/*
 * Copyright (C) 2017 Tomas Nilsson (joekickass)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 *    http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "config/ConnectionParams.h"
#include "aws_iot_config.h"

ConnectionParams::ConnectionParams(AwsIotSigv4& sigv4) :
  sigv4(sigv4)
{
}

ConnectionParams::~ConnectionParams()
{
  if (path != 0) {
    delete[] path;
  }
}

char* ConnectionParams::getHost()
{
  return AWS_IOT_MQTT_HOST;
}

unsigned int ConnectionParams::getPort()
{
  return AWS_IOT_MQTT_PORT;
}

char* ConnectionParams::getPath()
{
  if (path == 0) {
    sigv4.createPath(&path);
  }
  return path;
}

char* ConnectionParams::getFingerprint()
{
  return "";
}

char* ConnectionParams::getProtocol()
{
  return "mqtt";
}

bool ConnectionParams::useSsl()
{
  return true;
}

unsigned int ConnectionParams::getVersion()
{
  return 4;
}
char* ConnectionParams::getClientId()
{
  return AWS_IOT_MQTT_CLIENT_ID;
}
