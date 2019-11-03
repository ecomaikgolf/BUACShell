CC=g++
OPTS=-Wall -O3 -Wpedantic -std=c++11
NAME=bua
LIB=-lcurl

main:
	$(CC) $(OPTS) -o $(NAME) bua.cpp $(LIB) 
	sudo chown root bua && sudo chmod 4755 bua

install:
	$(CC) $(OPTS) -o $(NAME) bua.cpp $(LIB) 
	sudo cp bua /usr/bin/
	sudo chown root /usr/bin/bua && sudo chmod 4755 /usr/bin/bua
	sudo -u $(SUDO_USER) ./setup.sh
	$(info )
	$(info [*] Developed by: )
	$(info 	- Ernesto Martínez García)
	$(info 	- Ignacio Encinas Rubio)
	$(info )
