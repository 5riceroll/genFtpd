.PHONY:clear install uninstall
CC=gcc -g 
OBJ=genFtpd.o pub.o com.o net.o sem.o hash.o
genFtpd:${OBJ}
	${CC} -W -Wall -o $@ $^ -lcrypt -lm      
.c.o:
	${CC} -c -W -Wall $< -o $@ 
clear:
	rm -rf *.o $@ 
	rm -rf genFtpd $@
	rm -rf *.log
install:
	if [ ! -d /etc/genFtpd ];then\
		mkdir /etc/genFtpd;\
		chmod 755 /etc/genFtpd;\
	fi;\
	install -m 644 genFtpd.conf /etc/genFtpd/genFtpd.conf;\
	install -m 755 genFtpd /usr/local/bin/genFtpd;\
	install -m 755 genFtpd.sh /etc/init.d/genFtpd
uninstall:
	if [ -d /etc/genFtpd ];then\
		rm -rf /etc/genFtpd;\
	fi;\
	if [ -f /usr/local/bin/genFtpd ];then\
		rm -f /usr/local/bin/genFtpd;\
	fi;\
	if [ -f /etc/init.d/genFtpd ];then\
		rm -f /etc/init.d/genFtpd;\
	fi;
