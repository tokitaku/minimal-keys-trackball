# Layer 3 Scroll Speed Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Slow the PMW3610 vertical and horizontal scrolling on layer 3 to approximately half of the current speed.

**Architecture:** Keep the existing PMW3610 driver-based scroll path and change only its shared scroll tick threshold from 24 to 48. Because `scroll-layers = <3>` is unchanged, this affects layer 3 scrolling without changing cursor movement or layer 4 behavior.

**Tech Stack:** ZMK, Zephyr Kconfig, PMW3610 driver configuration, Git

---

## File Structure

- Modify `config/boards/shields/minimal-keys/minimal-keys_R.conf`: change the single PMW3610 scroll threshold value.
- No production files, key bindings, input processors, or tests are added.
- Static shell assertions and diff checks verify the configuration because the repository has no local unit-test target for this external PMW3610 Kconfig value.

### Task 1: Slow Layer 3 Trackball Scrolling

**Files:**
- Modify: `config/boards/shields/minimal-keys/minimal-keys_R.conf:28`
- Reference without modifying: `config/boards/shields/minimal-keys/minimal-keys_R.overlay:46-54,72`
- Test: shell assertions against the two configuration files

- [ ] **Step 1: Verify the current value and demonstrate that the target assertion fails**

Run:

```bash
rg -n '^CONFIG_PMW3610_SCROLL_TICK=' config/boards/shields/minimal-keys/minimal-keys_R.conf
test "$(sed -n 's/^CONFIG_PMW3610_SCROLL_TICK=//p' config/boards/shields/minimal-keys/minimal-keys_R.conf)" = "48"
```

Expected:

```text
28:CONFIG_PMW3610_SCROLL_TICK=24
```

The `test` command exits with status 1 because the target value has not been implemented yet.

- [ ] **Step 2: Apply the minimal configuration change**

Change:

```diff
-CONFIG_PMW3610_SCROLL_TICK=24
+CONFIG_PMW3610_SCROLL_TICK=48
```

Do not change `CONFIG_PMW3610_X_SCALE`, `CONFIG_PMW3610_Y_SCALE`, key bindings, layer 4 input processors, or `scroll-layers`.

- [ ] **Step 3: Verify the target value and unchanged routing**

Run:

```bash
test "$(sed -n 's/^CONFIG_PMW3610_SCROLL_TICK=//p' config/boards/shields/minimal-keys/minimal-keys_R.conf)" = "48"
test "$(rg -c '^CONFIG_PMW3610_SCROLL_TICK=48$' config/boards/shields/minimal-keys/minimal-keys_R.conf)" -eq 1
rg -n 'layer_4_slow|zip_xy_scaler 1 4|scroll-layers = <3>' config/boards/shields/minimal-keys/minimal-keys_R.overlay
```

Expected:

```text
51:        layer_4_slow {
53:            input-processors = <&zip_xy_scaler 1 4>;
72:        scroll-layers = <3>;
```

- [ ] **Step 4: Inspect the scoped diff and check patch integrity**

Run:

```bash
git diff -- config/boards/shields/minimal-keys/minimal-keys_R.conf
git diff --check
```

Expected: the configuration diff contains only `CONFIG_PMW3610_SCROLL_TICK=24` changing to `48`, and `git diff --check` exits with status 0.

- [ ] **Step 5: Record the local build boundary**

Run:

```bash
command -v west
```

Expected in the current environment: no output and a nonzero exit status. Do not claim a local firmware build passed. Firmware compilation must be verified by GitHub Actions if the change is later pushed, and the approximately half-speed behavior must be confirmed on the physical keyboard.

- [ ] **Step 6: Commit the focused change**

Run:

```bash
git add config/boards/shields/minimal-keys/minimal-keys_R.conf docs/superpowers/plans/2026-07-25-layer3-scroll-speed.md
git commit -m "fix: slow layer 3 trackball scrolling"
```

Expected: one focused local commit containing the implementation and its execution plan. Do not push or create a pull request without explicit user approval.

### Task 2: Approval-Gated Firmware and Hardware Verification

**Files:**
- Modify: none
- Remote verification: GitHub Actions for the implementation branch, only after explicit user approval to push
- Hardware verification: the physical minimal-keys keyboard after flashing the resulting firmware

- [ ] **Step 1: Report the local verification boundary and request direction**

After Task 1, report all three states separately:

```text
Local configuration checks: passed
Firmware compilation: not run locally because west is unavailable
Physical scroll behavior: not yet verified
```

Ask whether the user wants the focused change pushed on a `codex/` feature branch so GitHub Actions can compile the firmware. Do not push, create a pull request, or describe firmware compilation as passed without that explicit approval.

- [ ] **Step 2: Run GitHub Actions only after explicit push approval**

After approval, create or switch to the intended `codex/` feature branch, push that branch, and inspect its firmware workflow to a terminal result.

Expected: the relevant firmware build completes successfully. If it fails, report the failing job and do not describe the firmware as build-verified.

- [ ] **Step 3: Provide the physical comparison procedure**

After a successful firmware build, have the user flash the generated firmware and compare:

1. On layer 3, move the trackball the same approximate distance vertically before and after the change.
2. Confirm the resulting vertical page movement is clearly slower, targeting approximately half.
3. Repeat the same comparison horizontally in an application that supports horizontal scrolling.
4. Confirm cursor movement outside layer 3 and the existing layer 4 cursor behavior are unchanged.

Do not describe the requested behavior as hardware-verified until the user confirms both vertical and horizontal results.
