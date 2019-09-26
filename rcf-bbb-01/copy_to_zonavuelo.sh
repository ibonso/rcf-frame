#!/bin/bash

DIR_REMOTO=/home/ibon/etxea/kfcgi/kcgi-0.10.7/rcf-bbb-01
SERVIDOR=www.zonavuelo.com

scp -rp *.c*      ibon@$SERVIDOR:$DIR_REMOTO
scp -rp *.h       ibon@$SERVIDOR:$DIR_REMOTO
scp -rp *.hpp     ibon@$SERVIDOR:$DIR_REMOTO
scp -rp makefile  ibon@$SERVIDOR:$DIR_REMOTO




