#! /bin/bash


INST_DIR=/usr/local/sbin
RCF_SERV_FILE=rcf-serv-r
RCF_SERV_SERVICE=rcf-serv-r.service


echo "Stop rcf-serf-r service first"
systemctl stop  $RCF_SERV_SERVICE
echo "$RCF_SERV_FILE will be copies to $INST_DIR"
cp $RCF_SERV_FILE  $INST_DIR
ls -la $INST_DIR/$RCF_SERV_FILE

cp rcf-proxy.a ../sample-fcgi/rcf-proxy.a
ls -la ../sample-fcgi/rcf-proxy.a

#echo "Start again rcf-serv-r first"
#systemctl start $RCF_SERV_SERVICE



