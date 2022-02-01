/* 
 * File:   global_includes.h
 * Author: thomas
 *
 * Created on October 29, 2017, 4:46 PM
 */

#ifndef GLOBAL_INCLUDES_H
#define	GLOBAL_INCLUDES_H

#include <p18cxxx.h>
//#include <p33Exxxx.h>


#include "hardware_profile.h"

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Scheduler includes. */
//#include <FreeRTOSConfig.h>
//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
//#include "croutine.h"

void init_pins(void);
void resetState(void);

#define MAX_S16 32767

#endif	/* GLOBAL_INCLUDES_H */

