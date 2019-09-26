#! /bin/bash

SERV_NAME=rcf-serv-blue
SOCKET_NAME=rcfservb.socket

ps -ef|awk '/'$SERV_NAME'/'|grep -v awk|awk '/'$SERV_NAME'/{ print $2;}'|xargs -r kill -9

echo "deletel socket: /tmp/rcfservb.socket"
rm /tmp/$SOCKET_NAME
