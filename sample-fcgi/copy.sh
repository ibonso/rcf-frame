#/bin/basn

INST_DIR=/usr/local/sbin
FILE_TO_COPY=rcf-frame-fcgi


echo "Stop rcf-frame service first"
systemctl stop rcf-frame
echo "$FILE_TO_COPY will be copies to $INST_DIR"
cp $FILE_TO_COPY  $INST_DIR
ls -la $INST_DIR/$FILE_TO_COPY
echo "Start again rcf-frame service first"
systemctl start rcf-frame

