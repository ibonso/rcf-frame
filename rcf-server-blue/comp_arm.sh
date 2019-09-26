#url instrucciones
#http://jensd.be/800/linux/cross-compiling-for-arm-with-ubuntu-16-04-lts
# Compila pero el ejecutable no se ejecuta...
#

ibon@xubuntu16:~/beaglebone/rcf-frame/rcf-server-blue$
arm-linux-gnueabi-gcc  rcf-serv-blue.c  -I .  -I  ../rcf-bbb-01/ -I./copiado_de_blue -I /usr/include  -L /usr/lib/ -L /home/ibon/beaglebone/rcf-frame/rcf-server-blue/copiado_de_blue/ -lm -lrt -lpthread -lroboticscape -lm -static
