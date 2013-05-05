# Makefile include to define targets for various parts of the Atmel Software
# Framework
# -----------------------------------------------------------------------------
# To use this, just include this file in your makefile and specify the required
# asf parts as dependency. On first run (or after deleting the .asf.location
# file) you will be asked for the path to the asf root directory!
# -----------------------------------------------------------------------------


# Check if there is already an asf location provided. If not, ask for the
# location and store it for later use.
LOCATION_FILE:=".asf.location"
LOCATION:=$(shell \
	if [ -f ${LOCATION_FILE} ]; then \
		cat ${LOCATION_FILE}; \
	else \
		echo -n "Please enter path to asf: " 1>&2; \
		read -r location; \
		echo $$location | tee ${LOCATION_FILE}; \
	fi)
# Check if the provided location is valid and fail if not.
# Note that in order to be valid, a location has to be a path to a directory.
# This does not check whether that directory is a valid asf directory!
ifeq '$(shell [ -d ${LOCATION} ] && echo Y || echo N)' 'N'
$(shell [ -f ${LOCATION_FILE} ] && rm ${LOCATION_FILE})
$(error Given location of Atmel Software Framework is not a directory)
endif

.PHONY: test
test:
	echo "I am a test ${LOCATION}"


