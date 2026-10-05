#!/usr/bin/env python3
"""Send "hello" to the basic_connect example over Tailscale and wait for the echo.

Run it on any machine that is on the same tailnet. Retries for up to 30 s,
because the first tunnel after boot takes a few seconds to come up.

    python tools/udp_echo.py 100.101.102.103
"""
import socket
import sys
import time


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    ip = sys.argv[1]
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 9000

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(2)
    end = time.time() + 30
    while time.time() < end:
        s.sendto(b'hello', (ip, port))
        try:
            reply, _ = s.recvfrom(256)
            print(f"RESULT: reply from {ip}: {reply.decode(errors='replace').strip()}")
            return 0
        except socket.timeout:
            pass
    print(f"RESULT: no reply from {ip}:{port} in 30 s. Is this machine on the same tailnet?")
    return 1


if __name__ == '__main__':
    sys.exit(main())
