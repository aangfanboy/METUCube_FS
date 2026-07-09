#!/usr/bin/env python3
"""
Turn a DS-generated photo dump (.ds, from apps/DS/fsw/tables/ds_file_tbl.c's
PHOTO_DATA_FILE) into a clean, directly-viewable .pgm file.

Why this is needed: DS archives the raw bytes of every SB message it stores,
CCSDS/cFE headers included (same reasoning as tools/ds_hk_parser -- see that
tool's docstring for the full file-format background: CFE_FS_Header_t (64B,
big-endian) + DS_FileHeader_t (76B, native) + repeated CCSDS packets).
PAYLOADMC_captureFrame() (apps/payloadMC_app/fsw/src/payloadMC_app_cmds.c)
splits one captured GVSP frame (already a complete PGM image: "P5\nW H\n255\n"
+ raw pixel bytes) into PAYLOADMC_PhotoChunkPkt_t packets and publishes them
in order on PAYLOADMC_PHOTO_CHUNK_MID; DS appends each one's raw SB-message
bytes to the .ds file. This script strips all that framing back off and
concatenates just the ChunkData bytes, in order, to reconstruct the original
PGM image byte-for-byte.

Usage:
    python extract_photo_from_ds.py <path-to-photo00000000.ds> [-o output.pgm]
"""

import argparse
import struct
import sys
from pathlib import Path

# ---------------------------------------------------------------------------
# File-format constants (see tools/ds_hk_parser/parse_hk_to_csv.py for the
# full derivation/citations -- identical cFE FS header + DS header layout)
# ---------------------------------------------------------------------------
CFE_FS_HDR_DESC_MAX_LEN = 32
OS_MAX_PATH_LEN = 64
PAYLOADMC_PHOTO_CHUNK_MID = 0xEC5  # apps/payloadMC_app/fsw/inc/payloadMC_app_msgids.h

CFE_FS_HEADER_FMT = f">IIIIIIII{CFE_FS_HDR_DESC_MAX_LEN}s"
CFE_FS_HEADER_SIZE = struct.calcsize(CFE_FS_HEADER_FMT)  # 64

DS_FILE_HEADER_FMT = f"<IIHH{OS_MAX_PATH_LEN}s"
DS_FILE_HEADER_SIZE = struct.calcsize(DS_FILE_HEADER_FMT)  # 76

CCSDS_PRI_HDR_FMT = ">HHH"  # StreamId, Sequence, Length
CCSDS_PRI_HDR_SIZE = struct.calcsize(CCSDS_PRI_HDR_FMT)  # 6

TLM_SEC_TIME_SIZE = 6  # 4-byte seconds + 2-byte subseconds, big-endian (not needed here)
TLM_SPARE_SIZE = 4     # CFE_MSG_TelemetryHeader_t.Spare[4]

CHUNK_LEN_FMT = "<H"  # PAYLOADMC_PhotoChunkPkt_t.ChunkLen -- native (little-endian) uint16
CHUNK_LEN_SIZE = struct.calcsize(CHUNK_LEN_FMT)


def read_exact(f, size, what):
    data = f.read(size)
    if len(data) != size:
        raise EOFError(f"Unexpected EOF while reading {what} (got {len(data)}/{size} bytes)")
    return data


def skip_fs_and_ds_headers(f):
    fs_raw = read_exact(f, CFE_FS_HEADER_SIZE, "CFE_FS_Header_t")
    content_type = struct.unpack(">I", fs_raw[:4])[0]
    if content_type != 0x63464531:  # 'cFE1'
        raise ValueError(f"Not a cFE file (ContentType=0x{content_type:08X})")
    read_exact(f, DS_FILE_HEADER_SIZE, "DS_FileHeader_t")


def extract_chunks(f):
    """Yield raw ChunkData bytes (already trimmed to ChunkLen) for each packet, in file order."""
    while True:
        pri_hdr_raw = f.read(CCSDS_PRI_HDR_SIZE)
        if len(pri_hdr_raw) == 0:
            return
        if len(pri_hdr_raw) != CCSDS_PRI_HDR_SIZE:
            print("  (stopping: truncated CCSDS primary header at end of file)", file=sys.stderr)
            return

        stream_id, _sequence, length_field = struct.unpack(CCSDS_PRI_HDR_FMT, pri_hdr_raw)
        total_packet_len = length_field + 7  # CCSDS: Length = total octets - 7
        remaining = total_packet_len - CCSDS_PRI_HDR_SIZE

        rest = f.read(remaining)
        if len(rest) != remaining:
            print("  (stopping: truncated trailing packet at end of file)", file=sys.stderr)
            return

        if stream_id != PAYLOADMC_PHOTO_CHUNK_MID:
            print(f"  (skipping packet with unexpected StreamId=0x{stream_id:04X})", file=sys.stderr)
            continue

        payload = rest[TLM_SEC_TIME_SIZE + TLM_SPARE_SIZE:]
        chunk_len = struct.unpack(CHUNK_LEN_FMT, payload[:CHUNK_LEN_SIZE])[0]
        chunk_data = payload[CHUNK_LEN_SIZE:CHUNK_LEN_SIZE + chunk_len]

        if len(chunk_data) != chunk_len:
            print(f"  (warning: chunk claims {chunk_len} bytes but only {len(chunk_data)} present, using what's there)",
                  file=sys.stderr)

        yield chunk_data


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("ds_file", type=Path, help="Path to the DS-generated photo .ds file")
    parser.add_argument("-o", "--output", type=Path, default=None,
                         help="Output .pgm path (default: <ds_file>.pgm)")
    args = parser.parse_args()

    output_path = args.output or args.ds_file.with_suffix(".pgm")

    chunk_count = 0
    total_bytes = 0
    with open(args.ds_file, "rb") as f, open(output_path, "wb") as out_f:
        skip_fs_and_ds_headers(f)

        for chunk in extract_chunks(f):
            out_f.write(chunk)
            chunk_count += 1
            total_bytes += len(chunk)

    print(f"Reassembled {chunk_count} chunks, {total_bytes} bytes -> {output_path}")

    # Sanity check: a valid PGM starts with "P5\n"
    with open(output_path, "rb") as check_f:
        magic = check_f.read(3)
    if magic != b"P5\n":
        print(f"WARNING: output does not start with the PGM 'P5\\n' magic (got {magic!r}) -- "
              "chunks may be missing, out of order, or this wasn't a photo dump.", file=sys.stderr)
    else:
        print("Looks like a valid P5 PGM -- open it with GIMP/IrfanView, or convert: "
              f"magick {output_path} {output_path.with_suffix('.png')}")


if __name__ == "__main__":
    main()
