#!/bin/bash
echo "Key: 00112233445566778899aabbccddeeff, plaintext: 0123456789abcdef0123456789abcdef"

ciphertext=$(echo "0123456789abcdef0123456789abcdef" | xxd -r -p | openssl enc -aes-128-ecb -K 00112233445566778899aabbccddeeff -nopad | xxd -p)
echo "ciphertext: $ciphertext"
echo "code version:"
./main