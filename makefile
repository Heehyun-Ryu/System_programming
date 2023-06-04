
.PHONY: clean

RM = rm -f

yachtdice: yachtdice_client yachtdice_server


yachtdice_client: yachtdice_client.o
	gcc -o yachtdice_client yachtdice_client.o -lcurses

yachtdice_server: yachtdice_server.o
	gcc -o yachtdice_server yachtdice_server.o

yachtdice_client.o: yachtdice_client.c
	gcc -c yachtdice_client.c

yachtdice_server.o: yachtdice_server.c
	gcc -c yachtdice_server.c

clean:
	$(RM) *.o
