#define DT_DRV_COMPAT zmk_behavior_trackball_speed

#include <drivers/behavior.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/pointing/input_processor_runtime.h>

#include "trackball_speed_step.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_trackball_speed_config {
    const char *processor_name;
    int direction;
};

struct behavior_trackball_speed_data {
    const struct device *processor;
};

static int behavior_trackball_speed_init(const struct device *dev) {
    struct behavior_trackball_speed_data *data = dev->data;
    const struct behavior_trackball_speed_config *config = dev->config;

    data->processor = zmk_input_processor_runtime_find_by_name(config->processor_name);
    if (!data->processor) {
        LOG_ERR("Input processor '%s' was not found", config->processor_name);
        return -ENODEV;
    }

    return 0;
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    (void)event;

    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct behavior_trackball_speed_data *data = dev->data;
    const struct behavior_trackball_speed_config *config = dev->config;
    struct zmk_input_processor_runtime_config current;
    struct trackball_speed_ratio next;

    int ret = zmk_input_processor_runtime_get_config(data->processor, NULL, &current);
    if (ret < 0) {
        LOG_ERR("Failed to read trackball speed: %d", ret);
        return ret;
    }

    const struct trackball_speed_ratio speed = {
        .multiplier = current.scale_multiplier,
        .divisor = current.scale_divisor,
    };
    if (!trackball_speed_step(speed, config->direction, &next)) {
        LOG_ERR("Invalid trackball speed %u/%u", speed.multiplier, speed.divisor);
        return -EINVAL;
    }

    ret = zmk_input_processor_runtime_set_scaling(data->processor, next.multiplier, next.divisor,
                                                   true);
    if (ret < 0) {
        LOG_ERR("Failed to save trackball speed: %d", ret);
        return ret;
    }

    LOG_INF("Trackball speed set to %u/%u", next.multiplier, next.divisor);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    (void)binding;
    (void)event;

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_trackball_speed_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define TRACKBALL_SPEED_INST(n)                                                                  \
    static struct behavior_trackball_speed_data behavior_trackball_speed_data_##n;              \
    static const struct behavior_trackball_speed_config behavior_trackball_speed_config_##n = { \
        .processor_name = DT_INST_PROP(n, processor_name),                                      \
        .direction = DT_INST_PROP(n, direction),                                                \
    };                                                                                           \
    BEHAVIOR_DT_INST_DEFINE(                                                                    \
        n, behavior_trackball_speed_init, NULL, &behavior_trackball_speed_data_##n,            \
        &behavior_trackball_speed_config_##n, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, \
        &behavior_trackball_speed_driver_api);

DT_INST_FOREACH_STATUS_OKAY(TRACKBALL_SPEED_INST)
