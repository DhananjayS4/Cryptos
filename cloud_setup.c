#include <oqs/oqs.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_kyber_512);

    if (!kem) {
        printf("Kyber init failed\n");
        return 1;
    }

    uint8_t *pk = malloc(kem->length_public_key);
    uint8_t *sk = malloc(kem->length_secret_key);
    if (!pk || !sk) {
        printf("Memory allocation failed\n");
        if (pk) free(pk);
        if (sk) free(sk);
        OQS_KEM_free(kem);
        return 1;
    }

    if (OQS_KEM_keypair(kem, pk, sk) != OQS_SUCCESS) {
        printf("Keypair generation failed\n");
        free(pk);
        free(sk);
        OQS_KEM_free(kem);
        return 1;
    }

    FILE *f1 = fopen("pk_cloud.bin", "wb");
    if (f1) {
        fwrite(pk, 1, kem->length_public_key, f1);
        fclose(f1);
    } else {
        printf("Failed to write public key\n");
    }

    FILE *f2 = fopen("sk_cloud.bin", "wb");
    if (f2) {
        fwrite(sk, 1, kem->length_secret_key, f2);
        fclose(f2);
    } else {
        printf("Failed to write secret key\n");
    }

    printf("Keys generated\n");

    free(pk);
    free(sk);
    OQS_KEM_free(kem);
    return 0;
}
