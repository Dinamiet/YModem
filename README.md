# YModem

http://wiki.synchro.net/ref:ymodem

## Testing RX
mkfifo /tmp/write
mkfifo /tmp/read
sb -b -O -vvvv /tmp/testFile.log < /tmp/write > /tmp/read
