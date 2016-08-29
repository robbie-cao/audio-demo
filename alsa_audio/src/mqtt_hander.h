/*
 * mqtt_hander.h
 *
 *  Created on: Aug 26, 2016
 *      Author: wyf
 */

#ifndef MQTT_HANDER_H_
#define MQTT_HANDER_H_

#include <json-c/json.h>

int MQTT_Message_Send(json_object *msg);
void start_matt(void);

#endif /* MQTT_HANDER_H_ */
