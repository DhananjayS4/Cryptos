#include <oqs/oqs.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 5000

int recv_all(int sock, uint8_t *buffer, int length) {
    int total = 0, bytes;
    while (total < length) {
        bytes = recv(sock, buffer + total, length - total, 0);
        if (bytes <= 0) return -1;
        total += bytes;
    }
    return total;
}

int main() {

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_kyber_512);

    FILE *f = fopen("sk_cloud.bin", "rb");
    uint8_t *sk = malloc(kem->length_secret_key);
    fread(sk, 1, kem->length_secret_key, f);
    fclose(f);

    uint8_t *ct = malloc(kem->length_ciphertext);
    uint8_t shared_secret[32];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Server listening...\n");

    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

    recv_all(new_socket, ct, kem->length_ciphertext);

    OQS_KEM_decaps(kem, shared_secret, ct, sk);

    int size;
    recv_all(new_socket, (uint8_t*)&size, sizeof(int));

    FILE *out = fopen("received.jpg", "wb");

    char buffer[4096];
    int total = 0, bytes;

    while (total < size) {
        int chunk = (size - total > 4096) ? 4096 : (size - total);
        bytes = recv(new_socket, buffer, chunk, 0);
        fwrite(buffer, 1, bytes, out);
        total += bytes;
    }

    fclose(out);
    printf("Image received\n");

    close(new_socket);
    close(server_fd);
}
