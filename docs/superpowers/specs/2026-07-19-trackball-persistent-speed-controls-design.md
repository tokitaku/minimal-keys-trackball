# Trackball Persistent Speed Controls

## Goal

On layer 4, the two buttons beneath the right-hand trackball change the
trackball cursor speed by 0.25x per press. The selected speed is retained
after restart or power loss once its one-second save window has elapsed.

## User behavior

- The speed starts at 1.00x when no saved value exists.
- The speed-down button reduces the current speed by 0.25x.
- The speed-up button increases the current speed by 0.25x.
- Valid speeds are 0.25x through 2.00x, inclusive. A press at either limit
  keeps the limit unchanged.
- Each adjustment is queued for non-volatile storage after one second without
  another adjustment. The saved multiplier is restored on the next boot.
- A power loss during that one-second save window may restore the previously
  saved speed. This trade-off avoids writing flash on every button bounce or
  rapid series of adjustments.

## Design

Add a small ZMK behavior module that targets the existing runtime input
processor named `mouse`. The right half is the split central side, so this
behavior is compiled only for `CONFIG_ZMK_SPLIT_ROLE_CENTRAL`.

1. On press, the behavior reads the processor's active scaling configuration.
2. It compares the current rational scale against this ordered level table:
   `1/4`, `1/2`, `3/4`, `1/1`, `5/4`, `3/2`, `7/4`, `2/1`.
3. A speed-down press selects the greatest table value strictly below the
   current value; a speed-up press selects the smallest table value strictly
   above it. Exact table values move by one entry. Values outside the range
   clamp to the nearest limit. This also gives an unambiguous migration from
   values previously set through Studio, such as 1.10x.
4. It calls `zmk_input_processor_runtime_set_scaling(..., true)` so the updated
   multiplier and divisor become the persistent setting.
5. It does nothing on release, so a tap is a permanent adjustment rather than
   a hold-only temporary override.

The project will enable `CONFIG_SETTINGS=y` and set
`CONFIG_ZMK_SETTINGS_SAVE_DEBOUNCE=1000` in the right-hand central shield
configuration. The base runtime processor remains `1/1`, which is used on a
first boot without a stored value.

The keymap will bind the two specified layer-4 trackball buttons to the new
down and up behaviors at physical row 4, x=700 and x=800 (the seventh and
eighth bindings on that row, matrix entries `RC(2,6)` and `RC(2,7)`). Existing
misplaced `rip_ldpi` and `rip_hdpi` bindings at x=100 and x=200 will be
replaced with `&none` rather than leaving duplicate controls.

## Build integration

The repository root will become a local Zephyr module:

- `zephyr/module.yml` declares a module with the root `CMakeLists.txt`,
  `Kconfig`, and `dts` directory.
- `CMakeLists.txt` adds the behavior C source only for the split central build.
  The existing GitHub Actions reusable workflow detects `zephyr/module.yml`
  and passes the repository as `ZMK_EXTRA_MODULES`, so both firmware builds
  see the binding while only the right/central build links the behavior.
- The module provides a devicetree binding, a `.dtsi` file defining the
  `trackball_speed_down` and `trackball_speed_up` instances, and the C driver
  that uses the runtime input processor public API.
- `config/minimal-keys.keymap` includes the new behavior definition and binds
  its two zero-cell instances in layer 4.

## Scope and boundaries

- This changes cursor movement scaling only. It does not change PMW3610
  X/Y-scale, scroll speed, sensor configuration, or Bluetooth timing.
- The behavior persists one setting for the `mouse` runtime processor; it does
  not create per-Bluetooth-profile speed settings.
- The behavior uses the runtime input processor's public API and does not
  modify the pinned external module.
- The setting is global to the keyboard, not per Bluetooth profile.

## Error handling

- If the `mouse` processor cannot be found or its current configuration cannot
  be read, the behavior returns an error and leaves the current speed intact.
- Scaling values are compared as rational integers, not floating-point values.
  A non-table value moves to the next table value in the requested direction.
- A storage scheduling failure is returned and logged by the runtime processor
  API; the immediately selected runtime speed remains in effect. The eventual
  flash write result cannot be returned to the key press because it occurs
  asynchronously after the one-second debounce period.

## Verification

- Add focused tests or a build-time test fixture for the rational level table:
  0.25x cannot decrease, 2.00x cannot increase, each intermediate press moves
  exactly 0.25x, and non-table persisted values choose the next level in the
  requested direction.
- Confirm the layer-4 bindings point only to the new step-down and step-up
  behaviors at `RC(2,6)` and `RC(2,7)` and the earlier `rip_*` bindings are
  absent.
- Check the right build's generated `.config` contains `CONFIG_SETTINGS=y` and
  `CONFIG_ZMK_SETTINGS_SAVE_DEBOUNCE=1000`.
- On hardware, clear settings, confirm the first boot uses 1.00x, exercise all
  eight levels and both boundaries, wait at least one second, restart, and
  confirm the selected level is restored. Also record that a restart within
  the one-second debounce window restores the preceding saved value.
- Run the available firmware build or test command. If the workspace has no
  local `west` installation, run `git diff --check` and use GitHub Actions for
  the firmware build and generated-config verification.
