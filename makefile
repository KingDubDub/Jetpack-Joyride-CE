# ----------------------------
# Makefile Options
# ----------------------------

NAME        ?= JETPACK
COMPRESSED  ?= YES
ICON        ?= icon.png
DESCRIPTION ?= "Jetpack Joyride for the TI84 PCE"
ARCHIVED    ?= YES


CFLAGS ?= -Wall -Wextra -Oz
CXXFLAGS ?= -Wall -Wextra -Oz

# ----------------------------

ifndef CEDEV
$(error CEDEV environment path variable is not set)
endif

include $(CEDEV)/meta/makefile.mk