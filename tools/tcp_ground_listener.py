#!/usr/bin/env python3
"""
TCP Ground Listener
- Listens on a TCP port (default: 127.0.0.1:5000)
- Accepts multiple client connections concurrently
- Logs connections and bytes received
- Optionally writes raw data to rotating files or stdout
- Designed to receive COMMMC APP telemetry/file transfer packets

Usage examples:
  ./tcp_ground_listener.py              # listen on 127.0.0.1:5000
  ./tcp_ground_listener.py -H 0.0.0.0 -p 5000
  ./tcp_ground_listener.py -o recv.bin  # write all bytes to a single file
  ./tcp_ground_listener.py --split      # write each connection to a separate file under outdir

Note: This is a dumb byte sink by default. Parsing CFE/mission packets could be added later.
"""

import argparse
import logging
import os
import signal
import socket
import sys
import threading
import time
from contextlib import closing
from datetime import datetime
from pathlib import Path
from typing import Optional

LOG = logging.getLogger("tcp-ground-listener")

_stop_event = threading.Event()


def setup_logging(verbosity: int) -> None:
    level = logging.DEBUG
    if verbosity == 1:
        level = logging.INFO
    elif verbosity >= 2:
        level = logging.DEBUG
    logging.basicConfig(
        level=level,
        format="%(asctime)s %(levelname)s %(threadName)s %(message)s",
        datefmt="%H:%M:%S",
    )


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="TCP Ground Listener")
    p.add_argument("--host", "-H", default="127.0.0.1", help="Interface to bind (default: 127.0.0.1)")
    p.add_argument("--port", "-p", type=int, default=5000, help="TCP port to bind (default: 5000)")
    p.add_argument("--outfile", "-o", default=None, help="Write all received bytes to this file (append mode)")
    p.add_argument("--outdir", default="./tcp_recv", help="Directory for --split outputs (default: ./tcp_recv)")
    p.add_argument("--split", action="store_true", help="Write each connection to a separate file in --outdir")
    p.add_argument("--hex", action="store_true", help="Hexdump small chunks to stdout (<= 1024 bytes per chunk)")
    p.add_argument("--max-bytes", type=int, default=0, help="Stop after receiving N bytes (0 = unlimited)")
    p.add_argument("-v", action="count", default=0, help="Increase verbosity (repeatable)")
    return p.parse_args()


def hexdump(b: bytes, width: int = 16) -> str:
    def is_printable(x: int) -> bool:
        return 32 <= x <= 126

    lines = []
    for i in range(0, len(b), width):
        chunk = b[i : i + width]
        hex_part = " ".join(f"{x:02x}" for x in chunk)
        ascii_part = "".join(chr(x) if is_printable(x) else "." for x in chunk)
        lines.append(f"{i:08x}  {hex_part:<{width*3}}  |{ascii_part}|")
    return "\n".join(lines)


def handle_client(conn: socket.socket, addr, args: argparse.Namespace, out_lock: threading.Lock) -> None:
    thread_name = threading.current_thread().name
    LOG.info("Client connected from %s:%s", addr[0], addr[1])
    total = 0

    # Determine output target
    single_file: Optional[object] = None
    split_fp: Optional[object] = None

    try:
        if args.outfile:
            # shared file across connections, open in append, protected by lock
            single_file = open(args.outfile, "ab", buffering=0)
        elif args.split:
            Path(args.outdir).mkdir(parents=True, exist_ok=True)
            ts = datetime.utcnow().strftime("%Y%m%dT%H%M%S")
            split_name = f"conn_{addr[0]}_{addr[1]}_{ts}.bin"
            split_fp = open(os.path.join(args.outdir, split_name), "wb", buffering=0)

        conn.settimeout(1.0)
        while not _stop_event.is_set():
            try:
                data = conn.recv(4096)
            except socket.timeout:
                continue
            except Exception as e:
                LOG.warning("recv error: %s", e)
                break

            if not data:
                break

            total += len(data)

            if args.hex and len(data) <= 1024:
                LOG.info("chunk %d bytes:\n%s", len(data), hexdump(data))
            else:
                LOG.debug("chunk %d bytes", len(data))

            # write outputs
            if single_file:
                with out_lock:
                    single_file.write(data)
                    single_file.flush()
            elif split_fp:
                split_fp.write(data)
                split_fp.flush()

            if args.max_bytes and total >= args.max_bytes:
                LOG.info("max-bytes reached: %d >= %d, closing", total, args.max_bytes)
                break

        LOG.info("Client %s:%s disconnected, bytes=%d", addr[0], addr[1], total)
    finally:
        try:
            conn.shutdown(socket.SHUT_RDWR)
        except Exception:
            pass
        conn.close()
        if split_fp:
            split_fp.close()
        if single_file:
            single_file.close()


def serve(args: argparse.Namespace) -> None:
    out_lock = threading.Lock()

    with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((args.host, args.port))
        s.listen(5)
        LOG.info("Listening on %s:%d", args.host, args.port)

        def sig_handler(signum, frame):
            LOG.info("Signal %s received, shutting down...", signum)
            _stop_event.set()

        signal.signal(signal.SIGINT, sig_handler)
        signal.signal(signal.SIGTERM, sig_handler)

        threads = []
        try:
            while not _stop_event.is_set():
                try:
                    s.settimeout(1.0)
                    conn, addr = s.accept()
                except socket.timeout:
                    continue
                except OSError as e:
                    if _stop_event.is_set():
                        break
                    LOG.error("accept failed: %s", e)
                    continue

                t = threading.Thread(target=handle_client, args=(conn, addr, args, out_lock), daemon=True)
                t.start()
                threads.append(t)
        finally:
            _stop_event.set()
            LOG.info("Server stopping, waiting for %d client threads...", len(threads))
            for t in threads:
                t.join(timeout=2.0)
            LOG.info("Server stopped")


def main() -> int:
    args = parse_args()
    setup_logging(args.v)

    try:
        serve(args)
        return 0
    except OSError as e:
        LOG.error("Socket error: %s", e)
        return 2
    except KeyboardInterrupt:
        return 130


if __name__ == "__main__":
    sys.exit(main())
