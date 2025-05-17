# ----------------------------
# Makefile Options
# ----------------------------

NAME = JET
ICON = icon.png
DESCRIPTION     = "Jetpack Joyride for the TI-84PCE"
ARCHIVED        = YES
COMPRESSED      = YES
COMPRESSED_MODE = zx7
LTO             = NO
OUTPUT_MAP      = NO

# increase heap size for more slack when malloc'ing
BSSHEAP_LOW = D031F6

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
