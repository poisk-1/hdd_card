#!/bin/bash

losetup /dev/loop1 $1
kpartx -av /dev/loop1
