#!/bin/bash
#genFtpd Startup script for the genFtpd Server
# it is v.0.0.1 version.
# processname: genFtpd
 
genFtpdd=/usr/local/bin/genFtpd
genFtpdp=/usr/local/bin/
configFile=/etc/genFtpd/genFtpd.conf
RETVAL=0
prog="genFtpd"
 
num=`netstat -ntlp | grep genFtpd | wc -l`
# Check that networking is up.
[ -x $genFtpdd ] || exit 0
# Start genFtpd daemons functions.
start() {
if [ $num -gt 0 ];then
   echo "genFtpd already running...."
   exit 1
fi
   cd $genFtpdp && sudo ./$prog -c $configFile
   echo $"Starting $prog..."
}
# Stop genFtpd daemons functions.
stop() {
  echo $"Stopping $prog..."
  sudo pkill -f $prog
}
# show genFtpd status
status() {
  if [ $num -gt 0 ];then
     echo -e "$prog status: ok"	
  else
     echo -e "$prog status: fail"     	
  fi	
}
# See how we were called.
case "$1" in
  start)
          start
          ;;
  stop)
          stop
          ;;
  restart)
          stop #after stop, it cannot start, why?
          start	
	  ;;
  status)
          status $prog
          RETVAL=$?
          ;;
  *)
          echo $"Usage: $prog {start|stop|restart|status}"
          exit 1
esac
exit $RETVAL

