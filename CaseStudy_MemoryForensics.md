# Case Study: Investigating Illegal File Transferring (Memory Forensics)

## 1. Background & Narrative Overview
In modern corporate environments, the theft of intellectual property (IP) is a critical threat. In this case study, an insider threat—a rogue employee—is suspected of stealing highly sensitive aerospace schematics (images) and leaking them outside the corporate network to an unidentified external cloud server. 

The company's Security Operations Center (SOC) detected massive, anomalous outbound data transfers originating from the suspect's workstation over standard TCP ports. However, to evade detection, the suspect did not use traditional methods like USB flash drives (which leave permanent forensic artifacts in the Windows Registry under `USBSTOR` and `Shellbags`). Instead, the suspect built a custom, highly advanced **Secure Image Transfer** application.

## 2. The Forensic Challenge: The Network Dead End
The Incident Response (IR) team is called in. Their first step is standard **Network Forensics**. They deploy packet sniffers and use Wireshark to capture the suspect's outbound network traffic (saving it as a `.pcap` file). 

Under normal circumstances, investigators might attempt to analyze the headers, identify the protocol (like FTP or HTTP), or force a downgrade to decrypt the TLS session. However, they hit a brick wall. The captured traffic is entirely opaque, displaying perfect microscopic entropy. 

**The Suspect's Advantage:**
The rogue employee's custom tool operates on **Post-Quantum Cryptography** (specifically Kyber-512 key encapsulation) to establish a shared secret, which is then used to initialize military-grade **AES-256-CBC** symmetric encryption for the payload. Because Kyber-512 is designed to withstand attacks even from future quantum computers, mathematically breaking the network encryption or reversing the packet capture is physically impossible. The traditional Network Forensics approach completely fails.

## 3. The Solution: Live Memory Forensics
Facing unbreakable network encryption, the forensic investigators must pivot their strategy. They realize a fundamental truth of computer science: **Data in motion can be perfectly encrypted, but data in use must eventually be decrypted or processed in plain text within the computer's volatile memory (RAM).**

To perform the AES-256 encryption on the aerospace schematics, the suspect's custom C-based application *must* dynamically generate and hold the 256-bit symmetric key in the computer's RAM temporarily. 

**The Forensic Execution:**
While the suspect steps away for a coffee break leaving the large file transfer running, the IR team secures the physical workstation. Crucially, they do *not* unplug the computer (which would instantly wipe the volatile RAM and destroy the encryption keys). Instead, they perform a **Live Memory Capture** using tools like Volatility or WinPmem, extracting a complete `.raw` snapshot of the computer's 16GB of RAM to an external investigator drive.

## 4. How Our `Cryptos` Project Powers This Case Study
Your existing project (`Cryptos`) serves as the perfect "Suspect's Tool" to demonstrate this advanced scenario. Here is exactly how your solution fits into the forensic workflow:

### A. The Target Architecture
The forensic analysts are hunting your specific architecture within the massive RAM dump:
*   **The Python Frontend (`ui.py`):** The analysts can see traces of the modern Python `CustomTkinter` GUI in memory, confirming exactly which illicit image file was selected for transfer.
*   **The C Backend (`pi_client.c`):** The heavy lifting is done in C using `liboqs`. The analysts isolate the memory space (Process ID) assigned to your C executable.

### B. Carving the Payload Key
In your C code, memory is allocated (via `malloc` or standard variables) to hold the shared secret and the AES key initialized by OpenSSL. The forensic challenge for the student/investigator is to use heuristic scanning or YARA rules against the RAM dump to locate the exact contiguous 32 bytes of memory representing the AES-256 encryption key before your program has a chance to cleanly free or zero out that memory space.

### C. The Decryption Climax
Once the investigators successfully use memory forensics to "carve" your AES key out of the `pi_client`'s temporary RAM allocation, the case is cracked. They take that extracted key, apply it to the previously useless Wireshark network capture, and cleanly decrypt the AES-CBC stream. The output reveals `capture.jpg`—the stolen blueprints—providing undeniable proof of the exfiltration and successfully concluding the investigation!

## Key Takeaways
This case study demonstrates the cutting edge of digital forensics:
1. **Post-Quantum Evasion:** Demonstrates how suspects with access to modern `liboqs` cryptographic libraries can easily render traditional network traffic analysis obsolete.
2. **The Power of RAM:** Proves that Live Memory Forensics is the absolute final frontier for recovering advanced cryptographic keys. A secure network protocol is only as strong as its local memory management.
