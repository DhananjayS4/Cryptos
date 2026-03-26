#include <oqs/oqs.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/evp.h>

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
    if (!kem) {
        printf("Kyber init failed\n");
        return 1;
    }

    FILE *f = fopen("sk_cloud.bin", "rb");
    if (!f) {
        printf("Secret key not found. Run setup first.\n");
        OQS_KEM_free(kem);
        return 1;
    }
    uint8_t *sk = malloc(kem->length_secret_key);
    fread(sk, 1, kem->length_secret_key, f);
    fclose(f);

    uint8_t *ct = malloc(kem->length_ciphertext);
    uint8_t shared_secret[32];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printf("Socket creation failed\n");
        free(sk); free(ct); OQS_KEM_free(kem);
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printf("Bind failed\n");
        close(server_fd); free(sk); free(ct); OQS_KEM_free(kem);
        return 1;
    }
    if (listen(server_fd, 3) < 0) {
        printf("Listen failed\n");
        close(server_fd); free(sk); free(ct); OQS_KEM_free(kem);
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        printf("Accept failed\n");
        close(server_fd); free(sk); free(ct); OQS_KEM_free(kem);
        return 1;
    }

    // Receive Kyber ciphertext
    if (recv_all(new_socket, ct, kem->length_ciphertext) < 0) {
        printf("Failed to recv ciphertext\n");
        close(new_socket); close(server_fd); free(sk); free(ct); OQS_KEM_free(kem);
        return 1;
    }

    if (OQS_KEM_decaps(kem, shared_secret, ct, sk) != OQS_SUCCESS) {
        printf("Kyber Decapsulation failed\n");
        close(new_socket); close(server_fd); free(sk); free(ct); OQS_KEM_free(kem);
        return 1;
    }

    // Receive IV
    uint8_t iv[16];
    if (recv_all(new_socket, iv, sizeof(iv)) < 0) {
        printf("Failed to recv IV\n");
        goto end;
    }

    // Receive Cipher Size
    uint32_t net_size;
    if (recv_all(new_socket, (uint8_t*)&net_size, sizeof(uint32_t)) < 0) {
        printf("Failed to recv size\n");
        goto end;
    }
    uint32_t cipher_size = ntohl(net_size);

    uint8_t *cipher_data = malloc(cipher_size);
    if (!cipher_data) goto end;

    if (recv_all(new_socket, cipher_data, cipher_size) < 0) {
        printf("Failed to recv cipher data\n");
        free(cipher_data);
        goto end;
    }

    // Decrypt AES
    uint8_t *plain_data = malloc(cipher_size);
    int len1, len2;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, shared_secret, iv);
    EVP_DecryptUpdate(ctx, plain_data, &len1, cipher_data, cipher_size);
    int ret = EVP_DecryptFinal_ex(ctx, plain_data + len1, &len2);
    EVP_CIPHER_CTX_free(ctx);

    if (ret > 0) {
        FILE *out = fopen("received.jpg", "wb");
        if (out) {
            fwrite(plain_data, 1, len1 + len2, out);
            fclose(out);
            printf("Image securely decrypted and saved to received.jpg\n");
        }
    } else {
        printf("Decryption failed! Data may be corrupted or key mismatch.\n");
    }

    free(cipher_data);
    free(plain_data);

end:
    close(new_socket);
    close(server_fd);
    free(sk);
    free(ct);
    OQS_KEM_free(kem);
    return 0;
}
