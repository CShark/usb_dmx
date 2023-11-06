/* Includes ----------------------------------------------------------*/
#include "profiling.h"

/* Private Definitions -----------------------------------------------*/
#define DEBUG_PRINTF printf
#define __PROF_STOPED 0xFF

/* External variables ------------------------------------------------*/
/* Private variables -------------------------------------------------*/
static uint32_t time_start;                     // profiler start time
static const char *prof_name;                   // profiler name
static uint32_t time_event[MAX_EVENT_COUNT];    // events time
static const char *event_name[MAX_EVENT_COUNT]; // events name
static uint8_t event_count = __PROF_STOPED;     // events counter
static char event_args[MAX_EVENT_COUNT][4];     // events arguments
static char event_ignore = 0;

/* Private function prototypes ---------------------------------------*/
/* -------------------------------------------------------------------*/

/**
 * @brief Start profiler, save profiler name and start time
 *
 * @param profile_name Profiler name
 */
void PROFILING_START(const char *profile_name) {
    prof_name = profile_name;
    event_count = 0;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // DWT->LAR = 0xC5ACCE55;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // enable counter
    // DWT->CYCCNT  = time_start = 0;
    time_start = DWT->CYCCNT;
}

/**
 * @brief Ignore the current profiling session e.g. based on metadata later available
 *
 * @param ignore
 */
void PROFILING_IGNORE(char ignore) {
    event_ignore = ignore;
}

/**
 * @brief  Event. Save events name and time
 *
 * @param event Event name
 */
void PROFILING_EVENT(const char *event) {
    if (event_count == __PROF_STOPED)
        return;

    if (event_count < MAX_EVENT_COUNT) {
        time_event[event_count] = DWT->CYCCNT;
        event_name[event_count] = event;
        event_count++;
    }
}

/**
 * @brief  Event. Save events name and time
 *
 * @param event Event name
 */
void PROFILING_EVENTARGS(const char *event, char arg1, char arg2, char arg3, char arg4) {
    if (event_count == __PROF_STOPED)
        return;

    if (event_count < MAX_EVENT_COUNT) {
        time_event[event_count] = DWT->CYCCNT;
        event_name[event_count] = event;
        event_args[event_count][0] = arg1;
        event_args[event_count][1] = arg2;
        event_args[event_count][2] = arg3;
        event_args[event_count][3] = arg4;
        event_count++;
    }
}

/**
 * @brief Stop profiler. Print event table to ITM Stimulus Port 0
 */
void PROFILING_STOP(void) {
    int32_t tick_per_1us;
    int32_t time_prev;
    int32_t timestamp;
    int32_t delta_t;

    tick_per_1us = SystemCoreClock / 1000000;

    if (event_count == __PROF_STOPED) {
        DEBUG_PRINTF("\r\nWarning: PROFILING_STOP WITHOUT START.\r\n");
        return;
    }

    if (!event_ignore) {
        DEBUG_PRINTF("Profiling \"%s\" sequence: \r\n"
                     "--Event-----------------------|--timestamp--|----delta_t---\r\n",
                     prof_name);
        time_prev = 0;

        for (int i = 0; i < event_count; i++) {
            timestamp = (time_event[i] - time_start) / tick_per_1us;
            delta_t = timestamp - time_prev;
            time_prev = timestamp;
            if (event_args[i][0] == 0 && event_args[i][1] == 0 && event_args[i][2] == 0 && event_args[i][3] == 0) {
                DEBUG_PRINTF("%-30s:%9ld μs | +%9ld μs\r\n", event_name[i], timestamp, delta_t);
            } else {
                DEBUG_PRINTF("%-30s:%9ld μs | +%9ld μs | %02X %02X %02X %02X\r\n", event_name[i], timestamp, delta_t, event_args[i][0], event_args[i][1], event_args[i][2], event_args[i][3]);
            }
        }

        DEBUG_PRINTF("\r\n");
    }
    
    event_count = __PROF_STOPED;
}
