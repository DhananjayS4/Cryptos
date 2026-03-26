all:
	gcc cloud_setup.c -o cloud_setup -loqs -lcrypto
	gcc cloud_server.c -o cloud_server -loqs -lcrypto
	gcc pi_client.c -o pi_client -loqs -lcrypto
