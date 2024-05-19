CC := clang
CFLAGS := -Wall -g

PROGRAMS := demo-client demo-server \
            test-client-1 test-server-1 \
            test-client-2 test-server-2 \
            test-client-3 test-server-3

.PHONY: all clean

all: $(PROGRAMS)

demo-client: demo-client.c srtp.o srtp-common.o srtp-pcb.o byteorder64.o d_print.o
	$(CC) $(CFLAGS) $^ -o $@

demo-server: demo-server.c srtp.o srtp-common.o srtp-pcb.o byteorder64.o d_print.o
	$(CC) $(CFLAGS) $^ -o $@

test-client-1: test-client-1.c srtp.o srtp-common.o srtp-pcb.o byteorder64.o d_print.o
	$(CC) $(CFLAGS) $^ -o $@

test-server-1: test-server-1.c srtp.o srtp-common.o srtp-pcb.o byteorder64.o d_print.o
	$(CC) $(CFLAGS) $^ -o $@

test-client-2: test-client-2.c srtp.o srtp-common.o srtp-pcb.o byteorder64.o d_print.o
	$(CC) $(CFLAGS) $^ -o $@

test-server-2: test-server-2.c srtp.o srtp-common.o srtp-pcb.o byteorder64.o d_print.o
	$(CC) $(CFLAGS) $^ -o $@

test-client-3: test-client-3.c srtp.o srtp-common.o srtp-pcb.o byteorder64.o d_print.o
	$(CC) $(CFLAGS) $^ -o $@

test-server-3: test-server-3.c srtp.o srtp-common.o srtp-pcb.o byteorder64.o d_print.o
	$(CC) $(CFLAGS) $^ -o $@

srtp.o: srtp.c srtp.h srtp-packet.h srtp-common.h srtp-fsm.h srtp-pcb.h byteorder64.h d_print.h
	$(CC) $(CFLAGS) -c $< -o $@

srtp-common.o: srtp-common.c srtp-common.h
	$(CC) $(CFLAGS) -c $< -o $@

srtp-pcb.o: srtp-pcb.c srtp-pcb.h
	$(CC) $(CFLAGS) -c $< -o $@

byteorder64.o: byteorder64.c byteorder64.h
	$(CC) $(CFLAGS) -c $< -o $@

d_print.o: d_print.c d_print.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(PROGRAMS) *~
