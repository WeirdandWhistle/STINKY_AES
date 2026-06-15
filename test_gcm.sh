#!/bin/bash

python3 -c '
KEY  = bytes.fromhex("00112233445566778899aabbccddeeff")
IV   = bytes.fromhex("abcdef12345678901f2f3f4f")
PT   = bytes.fromhex("abcd1028743610923784610275861307580834765028374506")
AD   = bytes.fromhex("abcd1028743610923784610275861307580834765028374506")

from cryptography.hazmat.primitives.ciphers.aead import AESGCM
aesgcm = AESGCM(KEY)
ct_with_tag = aesgcm.encrypt(IV, PT, AD)

print("Ciphertext:", ct_with_tag[:-16].hex())
print("Tag:       ", ct_with_tag[-16:].hex())
'
make
./main