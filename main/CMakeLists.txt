set(COMPONENT_SRCS main.c st7565.c gpio_helper.c spiffs_helper.c display_helper.c catface_helper.c speedometer_helper.c can_helper.c cat_battle_msgs.c font8x16.c)
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()
