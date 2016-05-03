// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
 * Copyright (C) 2016  Intel Corporation. All rights reserved.
 *
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <AP_HAL/AP_HAL.h>
#include "AP_RangeFinder_MaxsonarSerialLV.h"
#include <AP_SerialManager/AP_SerialManager.h>
#include <ctype.h>

#define MAXSONAR_SERIAL_LV_BAUD_RATE 9600

extern const AP_HAL::HAL& hal;

/* 
   The constructor also initialises the rangefinder. Note that this
   constructor is not called until detect() returns true, so we
   already know that we should setup the rangefinder
*/
AP_RangeFinder_MaxsonarSerialLV::AP_RangeFinder_MaxsonarSerialLV(RangeFinder &_ranger, uint8_t instance,
                                                                 RangeFinder::RangeFinder_State &_state,
                                                                 AP_SerialManager &serial_manager) :
    AP_RangeFinder_Backend(_ranger, instance, _state)
{
    uart = serial_manager.find_serial(AP_SerialManager::SerialProtocol_Lidar, 0);
    if (uart != nullptr) {
        uart->begin(serial_manager.find_baudrate(AP_SerialManager::SerialProtocol_Lidar, 0));
    }
}

/* 
   detect if a MaxSonar rangefinder is connected. We'll detect by
   trying to take a reading on Serial. If we get a result the sensor is
   there.
*/
bool AP_RangeFinder_MaxsonarSerialLV::detect(RangeFinder &_ranger, uint8_t instance, AP_SerialManager &serial_manager)
{
    return serial_manager.find_serial(AP_SerialManager::SerialProtocol_Lidar, 0) != nullptr;
}

// read - return last value measured by sensor
bool AP_RangeFinder_MaxsonarSerialLV::get_reading(uint16_t &reading_cm)
{
    if (uart == nullptr) {
        return false;
    }

    int32_t sum = 0;
    int16_t nbytes = uart->available();
    uint16_t count = 0;
    uint16_t centimeters = 0;

    /* MaxSonarSeriaLV might need a manual reconection */
    if (nbytes == 0) {
        uart->end();
        uart->begin(MAXSONAR_SERIAL_LV_BAUD_RATE);
        nbytes = uart->available();
    }

    while (nbytes-- > 0) {
        char c = uart->read();
        if (c == '\r') {
            linebuf[linebuf_len] = 0;
            sum += (int)atoi(linebuf);
            count++;
            linebuf_len = 0;
        } else if (isdigit(c)) {
            linebuf[linebuf_len++] = c;
            if (linebuf_len == sizeof(linebuf)) {
                // too long, discard the line
                linebuf_len = 0;
            }
        }
    }


    if (count == 0) {
        return false;
    }

    // This sonar gives the metrics in inches, so we have to transform this to centimeters
    centimeters = 2.54f * sum / count;

    // Remove the value of one element from the average
    reading_cm_average = reading_cm_average * (1 - average_weight);

    // Add on the average a new element
    reading_cm_average = reading_cm_average ? reading_cm_average + centimeters * average_weight : centimeters;

    reading_cm = reading_cm_average;

    return true;
}

/* 
   update the state of the sensor
*/
void AP_RangeFinder_MaxsonarSerialLV::update(void)
{
    if (get_reading(state.distance_cm)) {
        // update range_valid state based on distance measured
        last_reading_ms = AP_HAL::millis();
        update_status();
    } else if (AP_HAL::millis() - last_reading_ms > 200) {
        set_status(RangeFinder::RangeFinder_NoData);
    }
}
