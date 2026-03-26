#include <oqs/oqs.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

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
    if (!kem) {
        printf("Kyber init failed\n");
        return 1;
    }

    FILE *f = fopen("pk_cloud.bin", "rb");
    if (!f) {
        printf("Public key not found. Run setup first.\n");
        OQS_KEM_free(kem);
        return 1;
    }
    uint8_t *pk = malloc(kem->length_public_key);
    fread(pk, 1, kem->length_public_key, f);
    fclose(f);

    uint8_t *ct = malloc(kem->length_ciphertext);
    uint8_t shared_secret[32]; // Kyber-512 shared secret is 32 bytes exactly, perfect for AES-256

    if (OQS_KEM_encaps(kem, ct, shared_secret, pk) != OQS_SUCCESS) {
        printf("Encapsulation failed\n");
        free(pk); free(ct); OQS_KEM_free(kem);
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket creation failed\n");
        free(pk); free(ct); OQS_KEM_free(kem);
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address\n");
        close(sock); free(pk); free(ct); OQS_KEM_free(kem);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        close(sock); free(pk); free(ct); OQS_KEM_free(kem);
        return 1;
    }

    // Send Kyber Ciphertext
    if (send_all(sock, ct, kem->length_ciphertext) < 0) {
        printf("Failed to send ciphertext\n");
        close(sock); free(pk); free(ct); OQS_KEM_free(kem);
        return 1;
    }

    FILE *img = fopen("capture.jpg", "rb");
    if (!img) {
        printf("Image not found\n");
        close(sock); free(pk); free(ct); OQS_KEM_free(kem);
        return 1;
    }
    fseek(img, 0, SEEK_END);
    int plain_size = ftell(img);
    rewind(img);

    uint8_t *plain_data = malloc(plain_size);
    fread(plain_data, 1, plain_size, img);
    fclose(img);

    // Encrypt with AES-256-CBC
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    uint8_t iv[16];
    RAND_bytes(iv, sizeof(iv));

    int cipher_len = plain_size + EVP_MAX_BLOCK_LENGTH;
    uint8_t *cipher_data = malloc(cipher_len);
    int len1, len2;

    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, shared_secret, iv);
    EVP_EncryptUpdate(ctx, cipher_data, &len1, plain_data, plain_size);
    EVP_EncryptFinal_ex(ctx, cipher_data + len1, &len2);
    EVP_CIPHER_CTX_free(ctx);

    int total_cipher_len = len1 + len2;

    // Send IV
    if (send_all(sock, iv, sizeof(iv)) < 0) goto end;

    // Send Cipher Size
    uint32_t size_network = htonl((uint32_t)total_cipher_len);
    if (send_all(sock, (uint8_t*)&size_network, sizeof(uint32_t)) < 0) goto end;

    // Send Ciphertext
    if (send_all(sock, cipher_data, total_cipher_len) < 0) goto end;

    printf("Image encrypted and sent securely!\n");

end:
    free(plain_data);
    free(cipher_data);
    free(pk);
    free(ct);
    OQS_KEM_free(kem);
    close(sock);
    return 0;
}

