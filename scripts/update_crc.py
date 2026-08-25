#!/usr/bin/env python3
import os
import re
import binascii

def crc32_file(filename):
    with open(filename, 'rb') as f:
        return binascii.crc32(f.read()) & 0xFFFFFFFF

def main():
    so_file = 'build/libnetworkhook.so'
    if not os.path.exists(so_file):
        print("❌ .so file not found")
        exit(1)

    new_crc = crc32_file(so_file)
    print(f"📊 New CRC: 0x{new_crc:08X}")

    with open('native_hook.cpp', 'r') as f:
        content = f.read()

    content = re.sub(
        r'return _crc\(\(const uint8_t\*\)base, sz\) == 0x[0-9A-F]{8};',
        f'return _crc((const uint8_t*)base, sz) == 0x{new_crc:08X};',
        content
    )

    with open('native_hook.cpp', 'w') as f:
        f.write(content)

    print("✅ CRC updated in source code")

if __name__ == "__main__":
    main()
