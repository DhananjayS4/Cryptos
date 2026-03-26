CC=gcc
CFLAGS=-Wall -Wextra -O2
LDFLAGS=-loqs -lcrypto

all: cloud_setup cloud_server pi_client

cloud_setup: cloud_setup.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

cloud_server: cloud_server.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

pi_client: pi_client.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f cloud_setup cloud_server pi_client *.exe pk_cloud.bin sk_cloud.bin received.jpg capture.jpg

