#!/bin/bash

kpartx -d -v /dev/loop1
losetup -d /dev/loop1
