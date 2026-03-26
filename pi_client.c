#include <oqs/oqs.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

int send_all(int sock, uint8_t *buffer, int length) {
    int total = 0, bytes;
    while (total < length) {
        bytes = send(sock, buffer + total, length - total, 0);
        if (bytes <= 0) return -1;
        total += bytes;
    }
    return total;
}

int main() {

    int sock;
    struct sockaddr_in serv_addr;

    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_kyber_512);

    FILE *f = fopen("pk_cloud.bin", "rb");
    uint8_t *pk = malloc(kem->length_public_key);
    fread(pk, 1, kem->length_public_key, f);
    fclose(f);

    uint8_t *ct = malloc(kem->length_ciphertext);
    uint8_t shared_secret[32];

    OQS_KEM_encaps(kem, ct, shared_secret, pk);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    send_all(sock, ct, kem->length_ciphertext);

    FILE *img = fopen("capture.jpg", "rb");
    fseek(img, 0, SEEK_END);
    int size = ftell(img);
    rewind(img);

    send_all(sock, (uint8_t*)&size, sizeof(int));

    char buffer[4096];
    int bytes;

    while ((bytes = fread(buffer, 1, 4096, img)) > 0) {
        send_all(sock, (uint8_t*)buffer, bytes);
    }

    fclose(img);
    close(sock);

    printf("Image sent\n");
}

