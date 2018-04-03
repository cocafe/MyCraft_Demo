#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "debug.h"

uint32_t g_debug_level = PRINT_INFO_BIT | PRINT_ERROR_BIT | PRINT_DEBUG_BIT;
uint32_t g_debug_sync = PRINT_INFO_BIT | PRINT_ERROR_BIT;