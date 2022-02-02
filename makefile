# ----------------------------
# Makefile Options
# ----------------------------

NAME        ?= JETPACK
COMPRESSED  ?= YES
ICON        ?= icon.png
DESCRIPTION ?= "Jetpack Joyride for the TI-84PCE"
ARCHIVED    ?= YES

#Break the BSS to let me overuse it to death:
BSSHEAP_LOW = D031F6

CFLAGS ?= -Wall -Wextra -Oz
CXXFLAGS ?= -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)