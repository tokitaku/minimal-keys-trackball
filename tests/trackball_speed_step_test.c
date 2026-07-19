#include <stdio.h>
#include <stdlib.h>

#include "trackball_speed_step.h"

static void assert_step(uint32_t multiplier, uint32_t divisor, int direction,
                        uint32_t expected_multiplier, uint32_t expected_divisor) {
    struct trackball_speed_ratio next;
    const struct trackball_speed_ratio current = {
        .multiplier = multiplier,
        .divisor = divisor,
    };

    if (!trackball_speed_step(current, direction, &next) ||
        next.multiplier != expected_multiplier || next.divisor != expected_divisor) {
        fprintf(stderr, "%u/%u direction %d: expected %u/%u, got %u/%u\n", multiplier, divisor,
                direction, expected_multiplier, expected_divisor, next.multiplier, next.divisor);
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    assert_step(1, 1, -1, 3, 4);
    assert_step(1, 1, 1, 5, 4);
    assert_step(1, 4, -1, 1, 4);
    assert_step(2, 1, 1, 2, 1);
    assert_step(11, 10, -1, 1, 1);
    assert_step(11, 10, 1, 5, 4);
    assert_step(1, 10, 1, 1, 4);
    assert_step(3, 1, -1, 2, 1);

    struct trackball_speed_ratio next;
    const struct trackball_speed_ratio invalid = {.multiplier = 1, .divisor = 0};
    const struct trackball_speed_ratio valid = {.multiplier = 1, .divisor = 1};
    if (trackball_speed_step(invalid, -1, &next) || trackball_speed_step(valid, 0, &next) ||
        trackball_speed_step(valid, 1, NULL)) {
        fputs("invalid input was accepted\n", stderr);
        return EXIT_FAILURE;
    }

    puts("trackball speed step tests passed");
    return EXIT_SUCCESS;
}
