target_include_directories(app PRIVATE include)

if((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  target_sources_ifdef(CONFIG_ZMK_BEHAVIOR_TRACKBALL_SPEED app PRIVATE
    src/trackball_speed_step.c
    src/behaviors/behavior_trackball_speed.c
  )
endif()
