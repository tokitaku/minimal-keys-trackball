#include "trackball_speed_step.h"

#include <stddef.h>

static const struct trackball_speed_ratio speed_levels[] = {
    {.multiplier = 1, .divisor = 4}, {.multiplier = 1, .divisor = 2},
    {.multiplier = 3, .divisor = 4}, {.multiplier = 1, .divisor = 1},
    {.multiplier = 5, .divisor = 4}, {.multiplier = 3, .divisor = 2},
    {.multiplier = 7, .divisor = 4}, {.multiplier = 2, .divisor = 1},
};

static int compare_ratio(struct trackball_speed_ratio left, struct trackball_speed_ratio right) {
    const uint64_t left_value = (uint64_t)left.multiplier * right.divisor;
    const uint64_t right_value = (uint64_t)right.multiplier * left.divisor;

    return (left_value > right_value) - (left_value < right_value);
}

bool trackball_speed_step(struct trackball_speed_ratio current, int direction,
                          struct trackball_speed_ratio *next) {
    if (!next || current.multiplier == 0 || current.divisor == 0 ||
        (direction != -1 && direction != 1)) {
        return false;
    }

    if (direction < 0) {
        *next = speed_levels[0];

        for (size_t index = 0; index < sizeof(speed_levels) / sizeof(speed_levels[0]); index++) {
            if (compare_ratio(speed_levels[index], current) >= 0) {
                break;
            }

            *next = speed_levels[index];
        }
    } else {
        *next = speed_levels[(sizeof(speed_levels) / sizeof(speed_levels[0])) - 1];

        for (size_t index = 0; index < sizeof(speed_levels) / sizeof(speed_levels[0]); index++) {
            if (compare_ratio(speed_levels[index], current) > 0) {
                *next = speed_levels[index];
                break;
            }
        }
    }

    return true;
}
