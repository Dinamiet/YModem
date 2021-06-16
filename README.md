# YModem

http://wiki.synchro.net/ref:ymodem

## Testing RX
mkfifo /tmp/write
mkfifo /tmp/read
sb -b -O -k -vvv /tmp/ymodem/build/app/YModem_receive < /tmp/write > /tmp/read


## Testing TX
mkfifo /tmp/write
mkfifo /tmp/read
rb -b -c -vvv --overwrite --ymodem < /tmp/write > /tmp/read
