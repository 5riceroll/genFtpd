#!/bin/bash
# you need install memory leak check tool : valgrind
valgrind --leak-check=full --show-reachable=no --log-file=t.log  ./genFtpd -c genFtpd.conf
