# TCP Ground Listener

A lightweight TCP server to receive COMMMC APP telemetry/file-transfer packets over TCP.

- Script: `tools/tcp_ground_listener.py`
- Default bind: 127.0.0.1:5000 (matches `COMMMC_APP_GROUND_ENDPOINT` default `tcp://127.0.0.1:5000`)
- Handles multiple clients concurrently
- Logs connections and bytes received
- Optional hex dump for small chunks
- Can save raw bytes to a file or per-connection files

## Run

Basic run (default host/port):

```bash
./tools/tcp_ground_listener.py
```

Bind on all interfaces and port 5000:

```bash
./tools/tcp_ground_listener.py -H 0.0.0.0 -p 5000
```

Hex dump small chunks (<= 1 KiB) to stdout and increase verbosity:

```bash
./tools/tcp_ground_listener.py -v --hex
```

Write all received bytes to a single file (append mode):

```bash
./tools/tcp_ground_listener.py -o received.bin
```

Write each connection to a separate file under a directory:

```bash
./tools/tcp_ground_listener.py --split --outdir ./tcp_recv
```

Stop after N bytes (useful for tests):

```bash
./tools/tcp_ground_listener.py --max-bytes 4096
```

## Match the COMMMC endpoint

If you changed the app to use a different endpoint, ensure the listener binds to the same host/port. For example, if `COMMMC_APP_GROUND_ENDPOINT` is set to `tcp://192.168.1.10:5001`, start the listener like:

```bash
./tools/tcp_ground_listener.py -H 0.0.0.0 -p 5001
```

Or on that specific interface:

```bash
./tools/tcp_ground_listener.py -H 192.168.1.10 -p 5001
```

## Notes

- The listener is a byte sink; it does not decode mission packets. If you want to parse specific headers (e.g., CCSDS/CFE), we can add a decoder.
- The script uses only the Python standard library; no extra dependencies needed.
- To make the script executable: `chmod +x tools/tcp_ground_listener.py`.
