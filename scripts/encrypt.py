#!/usr/bin/env python3
import os
import subprocess
import re

def main():
    with open('endpoint.txt', 'r') as f:
        endpoint = f.read().strip()

    if not endpoint:
        print("❌ Endpoint is empty")
        exit(1)

    print(f"🔐 Encrypting: {endpoint}")

    key = os.urandom(32)
    iv = os.urandom(16)

    cmd = f'echo -n "{endpoint}" | openssl enc -aes-256-cbc -K {key.hex()} -iv {iv.hex()}'
    encrypted = subprocess.check_output(cmd, shell=True)

    ep_array = ','.join([f'0x{b:02X}' for b in encrypted])
    key_obf = ','.join([f'0xAA^{b:02X}' for b in key])
    iv_obf = ','.join([f'0xAA^{b:02X}' for b in iv])

    with open('native_hook.cpp', 'r') as f:
        content = f.read()

    content = re.sub(
        r'static const uint8_t _ep\[\] = \{.*?\};',
        f'static const uint8_t _ep[] = {{{ep_array}}};',
        content,
        flags=re.DOTALL
    )
    content = re.sub(
        r'static const uint8_t _ak\[\] = \{.*?\};',
        f'static const uint8_t _ak[] = {{{key_obf}}};',
        content,
        flags=re.DOTALL
    )
    content = re.sub(
        r'static const uint8_t _iv\[\] = \{.*?\};',
        f'static const uint8_t _iv[] = {{{iv_obf}}};',
        content,
        flags=re.DOTALL
    )

    with open('native_hook.cpp', 'w') as f:
        f.write(content)

    print("✅ Endpoint encrypted and injected")
    print(f"📊 Key: {key.hex()}")
    print(f"📊 IV: {iv.hex()}")

if __name__ == "__main__":
    main()
