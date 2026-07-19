# Trackball Persistent Speed Controls

## Goal

On layer 4, the two buttons beneath the right-hand trackball change the
trackball cursor speed by 0.25x per press. The selected speed is retained
after restart or power loss.

## User behavior

- The speed starts at 1.00x when no saved value exists.
- The speed-down button reduces the current speed by 0.25x.
- The speed-up button increases the current speed by 0.25x.
- Valid speeds are 0.25x through 2.00x, inclusive. A press at either limit
  keeps the limit unchanged.
- Every successful adjustment is saved; the same multiplier is restored on
  the next boot.

## Design

Add a small ZMK behavior that targets the existing runtime input processor
named `mouse`.

1. On press, the behavior reads the processor's active scaling configuration.
2. It converts the scale to one of eight quarter-step levels (1/4 through 2/1)
   and moves one level down or up, clamped to the valid range.
3. It calls `zmk_input_processor_runtime_set_scaling(..., true)` so the updated
   multiplier and divisor become the persistent setting.
4. It does nothing on release, so a tap is a permanent adjustment rather than
   a hold-only temporary override.

The project will enable `CONFIG_SETTINGS=y` in the appropriate shield
configuration so the runtime processor has non-volatile storage available.
The keymap will bind the two specified layer-4 trackball buttons to the new
down and up behaviors. Existing misplaced `rip_ldpi` and `rip_hdpi` bindings
will be removed rather than leaving duplicate controls.

## Scope and boundaries

- This changes cursor movement scaling only. It does not change PMW3610
  X/Y-scale, scroll speed, sensor configuration, or Bluetooth timing.
- The behavior persists one setting for the `mouse` runtime processor; it does
  not create per-Bluetooth-profile speed settings.
- The behavior uses the runtime input processor's public API and does not
  modify the pinned external module.

## Error handling

- If the `mouse` processor cannot be found or its current configuration cannot
  be read, the behavior returns an error and leaves the current speed intact.
- Scaling values outside the supported quarter-step range are normalized to
  the nearest boundary before a one-step adjustment.
- A storage-save failure is returned and logged by the runtime processor API;
  the immediately selected runtime speed remains in effect.

## Verification

- Add focused tests or a build-time test fixture for level conversion and
  boundary behavior: 0.25x cannot decrease, 2.00x cannot increase, and each
  intermediate press moves exactly 0.25x.
- Confirm the layer-4 bindings point only to the new step-down and step-up
  behaviors at the requested physical positions.
- Run the available firmware build or test command. If the workspace has no
  local `west` installation, run `git diff --check` and rely on GitHub Actions
  for the firmware build.
