#!/bin/sh

export PATH="/usr/local/sbin:$PATH"

if [ "$HALD_ACTION" = "add" ]; then
  usb-rndis-driver $HAL_PROP_USB_DEVICE_BUS_NUMBER $HAL_PROP_USB_DEVICE_LINUX_DEVICE_NUMBER
fi

