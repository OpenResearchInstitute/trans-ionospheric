#!/bin/bash
strings src/_build/nrf52832_xxaa_version.c.o | grep BUILD_TIME
