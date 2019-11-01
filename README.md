# genFtpd
genFtpd is a general ftp server like vsftpd in linux, but it‘s smaller and simpler than vsftpd.

# Installation
cd to genFtpd dir, make. read makefile for details. 

configure genFtpd.conf
 --------------------------------
 #server ip address
 serverip=(192.168.200.128)
 #server port
 serverport=(9876)
 #max connections
 servermaxip=(10)
 #max connections per IP
 permaxip=(2)
 #download limit speed(kb)
 down=(1000)
 #upload limit speed(kb)
 up=(1000)
 ---------------------------------

launch:  genFtpd -c genFtpd.conf 
usage:   genFtpd -h  
 ----------------------------------------------
 genFtpd Version: 1.0.0
 Usage: genFtpd [-h] [-c filename]
 Options:
    -h         : this help
    -c filename: set server configuration file
 ----------------------------------------------

install/uninstall genFtpd service:  make install/uninstall
genFtpd service operation: 			service genFtpd start/stop/status/restart
