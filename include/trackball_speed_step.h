#pragma once

#include <stdbool.h>
#include <stdint.h>

struct trackball_speed_ratio {
    uint32_t multiplier;
    uint32_t divisor;
};

bool trackball_speed_step(struct trackball_speed_ratio current, int direction,
                          struct trackball_speed_ratio *next);
