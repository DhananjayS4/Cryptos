# Kyber Secure Image Transfer (PQC)

A completely secure image transfer protocol and graphical user interface that seamlessly bridges a quantum-resistant key-exchange backend with a beautiful, modern Python frontend.

## Features

- **Quantum-Resistant Key Exchange**: Utilizes the Kyber-512 Post-Quantum Cryptography algorithm (`liboqs`) to securely derive a shared secret over an insecure channel.
- **Symmetric Payload Encryption**: Ensures payload privacy by symmetrically encrypting traversing files using robust OpenSSL AES-256-CBC logic, initialized intrinsically by your quantum-resistant shared secret.
- **Modern User Interface**: A dynamic, premium dark-mode CustomTkinter UI for easily dispatching files over the PQC channel.
- **Memory Stable Backend**: All C clients are completely memory stable, defensively programmed against socket bounds, and properly cleanly freeing memory.

## Prerequisites

- **C Library Base**: `liboqs` (Open Quantum Safe library), OpenSSL (`libssl-dev` or `libcrypto`)
- **Python Dependencies**: `python3`, `customtkinter`
- **Compiler Requirements**: `gcc`, `make`

## Building and Running

1. **Build the C Clients**:
   ```bash
   make clean
   make all
   ```

2. **Generate Post-Quantum Secure Keys**:
   ```bash
   ./cloud_setup
   ```
   *(This initializes public and secret Kyber keys locally.)*

3. **Start the Receiving Server**:
   Provide the `./cloud_server` executable its own terminal. It will cleanly bind and listen for encrypted PQC transmissions.
   ```bash
   ./cloud_server
   ```

4. **Start the Graphical Client**:
   In another terminal, boot the CustomTkinter UI interface. Select an image through the GUI prompt and press Send!
   ```bash
   python ui.py
   ```
   *The Python GUI will invoke `./pi_client`, which reads the public key, establishes AES-CBC with Kyber, encrypts your image payload, and securely transmits it to `./cloud_server`, which outputs the decrypted result as `received.jpg`.*

## License
MIT
