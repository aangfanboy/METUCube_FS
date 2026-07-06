#!/usr/bin/env python3
"""
Decode a DS-generated combined HK (.hk) file from the METUCube cFS build and
write every stored sample to a CSV file.

Usage:
    python parse_hk_to_csv.py <path-to-hkNNNNNNNN.hk> [-o output.csv] [--automations automations.json]

File layout (all offsets/types verified against the actual source, cited
inline below -- if the HK copy table or any subsystem's *_msg.h changes,
the struct definitions in this script must be updated to match):

    CFE_FS_Header_t   64 bytes, BIG-ENDIAN   (cfe/modules/fs/config/default_cfe_fs_filedef.h)
    DS_FileHeader_t   76 bytes, native/LE    (apps/DS/fsw/src/ds_file.h) -- written via plain
                                              OS_write(), NOT byte-swapped like the FS header
    repeated CCSDS telemetry packets:
        CCSDS primary header   6 bytes, BIG-ENDIAN  (cfe/modules/msg/fsw/inc/ccsds_hdr.h)
        CFE secondary header   6 bytes time (BIG-ENDIAN) + 4 bytes spare
                                (cfe/modules/msg/option_inc/default_cfe_msg_hdr_pri.h,
                                 default_cfe_msg_sechdr.h)
        payload: Power + Comm + Adcs + Payload + Mppt HK structs, concatenated
                 back-to-back at native (little-endian) byte order with each
                 struct's own natural C alignment/padding -- exactly mirroring
                 how apps/HK/fsw/src/hk_utils.c:HK_ProcessIncomingHkData() does
                 a raw memcpy() into the combined buffer using sizeof()-based
                 offsets (see apps/HK/fsw/tables/hk_cpy_tbl.c for the order).

KNOWN DATA ISSUE: the CANUSBDebugger automations.json test rig sends the
Payload HK response as 20x uint16 (message_id 0xA6 / 166), but the real
implementation (apps/payloadMC_app + CANIOMC_PayloadTlmPayload_t) expects
20x uint8. Until that's reconciled, decoded "Payload_Reading_*" columns in
the CSV are the raw first 20 bytes of whatever the simulator sent, i.e. NOT
meaningful telemetry -- they are shown as-is, not silently fixed up here.
Only the Power/EPS board sends genuinely real hardware data right now; all
other subsystems in a decoded file come from CANUSBDebugger automations.

FIXED (previously silent bug): canIOMC_app_cmds.c matched EPS's HK response
against a hardcoded MessageID (0x000B) that the real EPS hardware never
actually sends -- it responds on 0x66, the same ID used for the OBC's
request. That branch never fired, so Power/EPS fields stayed at their
zero-initialized value forever regardless of real CAN traffic. Also added
the real EPS payload's trailing 2 bytes of packed boolean flags (10 flags),
which the code previously discarded entirely. This script's PowerHk struct
now includes BoolFlags[2] to match -- but the exact bit order/meaning of
the 10 flags inside those 2 bytes is NOT confirmed, so they are exposed
as two raw bytes (Power_BoolFlags_0/1), not individually decoded booleans.
"""

import argparse
import csv
import ctypes as C
import json
import struct
import sys
from pathlib import Path

# ---------------------------------------------------------------------------
# File-format constants
# ---------------------------------------------------------------------------
CFE_FS_HDR_DESC_MAX_LEN = 32   # cfe/modules/fs/config/default_cfe_fs_interface_cfg.h
OS_MAX_PATH_LEN = 64           # osal/default_config.cmake (OSAL_CONFIG_MAX_PATH_LEN)
HK_COMBINED_PKT1_MID = 0x089C  # apps/HK/fsw/inc/hk_msgids.h

CFE_FS_HEADER_FMT = f">IIIIIIII{CFE_FS_HDR_DESC_MAX_LEN}s"
CFE_FS_HEADER_SIZE = struct.calcsize(CFE_FS_HEADER_FMT)  # 64

DS_FILE_HEADER_FMT = f"<IIHH{OS_MAX_PATH_LEN}s"
DS_FILE_HEADER_SIZE = struct.calcsize(DS_FILE_HEADER_FMT)  # 76

CCSDS_PRI_HDR_FMT = ">HHH"  # StreamId, Sequence, Length
CCSDS_PRI_HDR_SIZE = struct.calcsize(CCSDS_PRI_HDR_FMT)  # 6

TLM_SEC_TIME_FMT = ">IH"  # 4-byte seconds + 2-byte subseconds, big-endian
TLM_SEC_TIME_SIZE = struct.calcsize(TLM_SEC_TIME_FMT)  # 6
TLM_SPARE_SIZE = 4  # CFE_MSG_TelemetryHeader_t.Spare[4]


# ---------------------------------------------------------------------------
# Payload sub-structs -- field-for-field mirrors of the C definitions.
# Each is parsed independently (own ctypes.sizeof()), then placed at a
# running byte offset -- NOT nested in one big ctypes struct, so that no
# extra inter-struct alignment padding gets invented that the real C code
# (raw memcpy at sizeof()-based offsets) never produces.
# ---------------------------------------------------------------------------
class PowerHk(C.LittleEndianStructure):
    """apps/powerMC_app/fsw/inc/powerMC_app_msg.h : POWERMC_HkTlm_Power_t"""
    _fields_ = [
        ("CmdCounter", C.c_uint8),
        ("ErrCounter", C.c_uint8),
        ("EpsStale", C.c_uint8),
        ("Reserved", C.c_uint8 * 1),
        ("ChannelCurrents", C.c_uint8 * 10),
        ("BuckVoltages", C.c_uint16 * 5),
        ("BoolFlags", C.c_uint8 * 2),  # 10 packed EPS boolean status flags (bit-packed, raw bytes)
    ]


class CommHk(C.LittleEndianStructure):
    """apps/commMC_app/fsw/inc/commMC_app_msg.h : COMMMC_HkTlm_Comm_t"""
    _fields_ = [
        ("CmdCounter", C.c_uint8),
        ("ErrCounter", C.c_uint8),
        ("CommStale", C.c_uint8),
        ("Reserved", C.c_uint8 * 1),
        ("CurrentConnectionRate", C.c_uint32),
        ("Readings", C.c_uint16 * 5),
    ]


class AdcsHk(C.LittleEndianStructure):
    """apps/adcsMC_app/fsw/inc/adcsMC_app_msg.h : ADCSMC_HkTlm_Adcs_t"""
    _fields_ = [
        ("CmdCounter", C.c_uint8),
        ("ErrCounter", C.c_uint8),
        ("AdcsStale", C.c_uint8),
        ("Reserved", C.c_uint8 * 1),
        ("QuaternionEst", C.c_float * 4),
        ("AngularVelEst", C.c_float * 3),
        ("BiasEst", C.c_float * 3),
        ("PosEst", C.c_float * 3),
        ("VelEst", C.c_float * 3),
        ("PqEst", C.c_float * 3),
        ("PbEst", C.c_float * 3),
        ("SunUnitVector1", C.c_float * 3),
        ("SunUnitVector2", C.c_float * 3),
        ("MagUnitVector1", C.c_float * 3),
        ("MagUnitVector2", C.c_float * 3),
        ("SunSensorTemp", C.c_uint16 * 2),
    ]


class PayloadHk(C.LittleEndianStructure):
    """apps/payloadMC_app/fsw/inc/payloadMC_app_msg.h : PAYLOADMC_HkTlm_Payload_t"""
    _fields_ = [
        ("CmdCounter", C.c_uint8),
        ("ErrCounter", C.c_uint8),
        ("PayloadStale", C.c_uint8),
        ("Reserved", C.c_uint8 * 1),
        ("NumberOfTakenPhotos", C.c_uint32),
        ("ActiveCameraN", C.c_uint32),
        ("TimeSeconds", C.c_uint32),      # CFE_TIME_SysTime_t.Seconds
        ("TimeSubseconds", C.c_uint32),   # CFE_TIME_SysTime_t.Subseconds
        ("CpuTemperature", C.c_int32),
        ("Readings", C.c_uint8 * 20),
    ]


class MpptHk(C.LittleEndianStructure):
    """apps/mpptMC_app/fsw/inc/mpptMC_app_msg.h : MPPTMC_HkTlm_Mppt_t"""
    _fields_ = [
        ("CmdCounter", C.c_uint8),
        ("ErrCounter", C.c_uint8),
        ("MpptStale", C.c_uint8),
        ("Reserved", C.c_uint8 * 1),
        ("Readings", C.c_uint16 * 18),
    ]


# Order matches apps/HK/fsw/tables/hk_cpy_tbl.c exactly (Power, Comm, Adcs,
# Payload, Mppt). This drives both the byte offsets and the CSV columns.
SUBSYSTEMS = [
    ("Power", PowerHk),
    ("Comm", CommHk),
    ("Adcs", AdcsHk),
    ("Payload", PayloadHk),
    ("Mppt", MpptHk),
]


def flatten(name, hk_struct):
    """Turn one decoded subsystem struct into an ordered list of (column, value) pairs."""
    row = []
    for field_name, field_type in hk_struct._fields_:
        value = getattr(hk_struct, field_name)
        if hasattr(value, "__len__"):
            for i, v in enumerate(value):
                row.append((f"{name}_{field_name}_{i}", v))
        else:
            row.append((f"{name}_{field_name}", value))
    return row


def read_exact(f, size, what):
    data = f.read(size)
    if len(data) != size:
        raise EOFError(f"Unexpected EOF while reading {what} (got {len(data)}/{size} bytes)")
    return data


def parse_fs_header(f):
    raw = read_exact(f, CFE_FS_HEADER_SIZE, "CFE_FS_Header_t")
    (content_type, sub_type, length, spacecraft_id, processor_id,
     application_id, time_seconds, time_subseconds, description) = struct.unpack(CFE_FS_HEADER_FMT, raw)
    if content_type != 0x63464531:  # 'cFE1'
        raise ValueError(f"Not a cFE file (ContentType=0x{content_type:08X}, expected 0x63464531)")
    return {
        "SubType": sub_type,
        "SpacecraftID": spacecraft_id,
        "ProcessorID": processor_id,
        "ApplicationID": application_id,
        "TimeSeconds": time_seconds,
        "TimeSubseconds": time_subseconds,
        "Description": description.split(b"\x00", 1)[0].decode(errors="replace"),
    }


def parse_ds_header(f):
    raw = read_exact(f, DS_FILE_HEADER_SIZE, "DS_FileHeader_t")
    close_seconds, close_subsecs, file_table_index, file_name_type, file_name = struct.unpack(
        DS_FILE_HEADER_FMT, raw
    )
    return {
        "CloseSeconds": close_seconds,
        "CloseSubsecs": close_subsecs,
        "FileTableIndex": file_table_index,
        "FileNameType": file_name_type,
        "FileName": file_name.split(b"\x00", 1)[0].decode(errors="replace"),
    }


def iter_packets(f):
    """Yield (packet_time_seconds_float, payload_bytes) for each stored telemetry packet.

    Stops gracefully (with a warning) on a truncated trailing packet, which
    happens if the .hk file was copied while DS was still writing it.
    """
    while True:
        pri_hdr_raw = f.read(CCSDS_PRI_HDR_SIZE)
        if len(pri_hdr_raw) == 0:
            return  # clean EOF between packets
        if len(pri_hdr_raw) != CCSDS_PRI_HDR_SIZE:
            print("  (stopping: truncated CCSDS primary header at end of file -- "
                  "file likely still being written)", file=sys.stderr)
            return

        stream_id, sequence, length_field = struct.unpack(CCSDS_PRI_HDR_FMT, pri_hdr_raw)
        apid = stream_id & 0x07FF
        total_packet_len = length_field + 7  # CCSDS: Length = total octets - 7
        remaining = total_packet_len - CCSDS_PRI_HDR_SIZE

        rest = f.read(remaining)
        if len(rest) != remaining:
            print("  (stopping: truncated trailing packet at end of file -- "
                  "file likely still being written)", file=sys.stderr)
            return
        sec_time_raw = rest[:TLM_SEC_TIME_SIZE]
        payload = rest[TLM_SEC_TIME_SIZE + TLM_SPARE_SIZE:]

        seconds, subseconds = struct.unpack(TLM_SEC_TIME_FMT, sec_time_raw)
        packet_time = seconds + subseconds / 65536.0  # secondary header subseconds are 16-bit here

        if stream_id != HK_COMBINED_PKT1_MID:
            print(f"  (skipping packet with unexpected StreamId=0x{stream_id:04X}, APID=0x{apid:03X})",
                  file=sys.stderr)
            continue

        yield packet_time, payload


def decode_payload(payload):
    """Slice the concatenated payload into the 5 subsystem structs at their real offsets."""
    decoded = {}
    offset = 0
    for name, struct_type in SUBSYSTEMS:
        size = C.sizeof(struct_type)
        if offset + size > len(payload):
            raise ValueError(
                f"Payload too short for {name}: need {size} bytes at offset {offset}, "
                f"only {len(payload) - offset} available"
            )
        decoded[name] = struct_type.from_buffer_copy(payload, offset)
        offset += size

    if offset != len(payload):
        print(f"  (note: {len(payload) - offset} trailing payload bytes not accounted for)", file=sys.stderr)

    return decoded


def load_automations(path):
    """CANUSBDebugger automations.json -> {sender_id: [expected values in field order]}."""
    with open(path, "r", encoding="utf-8") as f:
        entries = json.load(f)

    expected = {}
    for entry in entries:
        if not entry.get("enabled", True):
            continue
        resp = entry["response"]
        expected[resp["sender_id"]] = {
            "name": entry.get("name", "?"),
            "message_id": resp["message_id"],
            "values": [field["value"] for field in resp["fields"]],
        }
    return expected


# sender_id -> subsystem name, per apps/canIOMC_app/fsw/inc/canIOMC_app_header_defs.h
SENDER_ID_TO_SUBSYSTEM = {1: "Adcs", 2: "Comm", 4: "Mppt", 5: "Payload"}


def compare_with_automations(decoded, expected):
    for sender_id, name in SENDER_ID_TO_SUBSYSTEM.items():
        if sender_id not in expected:
            continue
        hk_struct = decoded[name]
        # Flatten array elements only, skipping counter/stale header fields, in field order
        actual_values = []
        for field_name, field_type in hk_struct._fields_:
            if field_name in ("CmdCounter", "ErrCounter", "Reserved") or field_name.endswith("Stale"):
                continue
            value = getattr(hk_struct, field_name)
            if hasattr(value, "__len__"):
                actual_values.extend(value)
            else:
                actual_values.append(value)

        exp = expected[sender_id]
        exp_values = exp["values"]
        match = len(actual_values) == len(exp_values) and all(
            abs(a - e) < 1e-3 if isinstance(e, float) else a == e
            for a, e in zip(actual_values, exp_values)
        )
        status = "MATCH" if match else "MISMATCH"
        print(f"  [{status}] {name} vs automation '{exp['name']}' "
              f"(decoded {len(actual_values)} values, expected {len(exp_values)})")
        if not match:
            print(f"           decoded : {actual_values}")
            print(f"           expected: {exp_values}")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("hk_file", type=Path, help="Path to the DS-generated .hk file")
    parser.add_argument("-o", "--output", type=Path, default=None,
                         help="CSV output path (default: <hk_file>.csv)")
    parser.add_argument("--automations", type=Path, default=None,
                         help="Optional CANUSBDebugger automations.json to cross-check decoded values against")
    args = parser.parse_args()

    output_path = args.output or args.hk_file.with_suffix(".csv")

    expected = None
    if args.automations:
        expected = load_automations(args.automations)

    columns = ["PacketTime"]
    for name, struct_type in SUBSYSTEMS:
        dummy = struct_type()
        columns.extend(col for col, _ in flatten(name, dummy))

    rows_written = 0
    with open(args.hk_file, "rb") as f, open(output_path, "w", newline="", encoding="utf-8") as csv_f:
        fs_info = parse_fs_header(f)
        ds_info = parse_ds_header(f)

        print(f"cFE file description : {fs_info['Description']!r}")
        print(f"DS on-board filename  : {ds_info['FileName']!r}")
        print(f"Spacecraft/Processor  : {fs_info['SpacecraftID']} / {fs_info['ProcessorID']}")

        writer = csv.writer(csv_f)
        writer.writerow(columns)

        for packet_time, payload in iter_packets(f):
            decoded = decode_payload(payload)

            if expected is not None:
                print(f"packet @ t={packet_time:.3f}")
                compare_with_automations(decoded, expected)

            row = [packet_time]
            for name, _ in SUBSYSTEMS:
                row.extend(v for _, v in flatten(name, decoded[name]))
            writer.writerow(row)
            rows_written += 1

    print(f"\nWrote {rows_written} samples to {output_path}")


if __name__ == "__main__":
    main()
