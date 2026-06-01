#!/usr/bin/env python3
"""
    merge_hex.py - Merge Core0 and Core1 HEX files into a single combined HEX.

    Core0 (V3F) is the primary core. Its HEX file is placed first in the
    combined image. Core1 (V5F) data follows at its designated offset.

    Usage:
        python3 merge_hex.py

    Prerequisites:
        - core0/build/ch.hex
        - core1/build/ch.hex

    Output:
        combined.hex
"""

import os
import sys


# ---------------------------------------------------------------------------
# Intel HEX helpers
# ---------------------------------------------------------------------------

def _hex_checksum(hex_str):
    """Compute Intel HEX checksum for a colon-prefixed record body.

    *hex_str* must start with ``:`` but *must not* include the trailing
    checksum field.  Returns the one's‑complement checksum byte.
    """
    # Sum all bytes encoded in hex_str[1:] (skip the leading ':').
    raw = bytes.fromhex(hex_str[1:])
    s = sum(raw) & 0xFF
    return (~s + 1) & 0xFF


def make_data_record(offset, data):
    """Build an Intel HEX data record (type 00) string."""
    line = ":{:02X}{:04X}00{}".format(len(data), offset,
                                       "".join("{:02X}".format(b) for b in data))
    ck = _hex_checksum(line)
    return "{}{:02X}\n".format(line, ck)


def make_extended_addr_record(upper16):
    """Build an Intel HEX Extended Linear Address record (type 04).

    Type 04 is the standard way to encode bits [31:16] of a 32-bit address.
    """
    line = ":02000004{:04X}".format(upper16 & 0xFFFF)
    ck = _hex_checksum(line)
    return "{}{:02X}\n".format(line, ck)


def make_eof_record():
    """Build the End-Of-File record (type 01)."""
    return ":00000001FF\n"


def parse_hex_line(line):
    """Parse a single Intel HEX line.

    Returns ``(address, data_bytes, record_type)``.
    """
    line = line.strip()
    if not line.startswith(":"):
        return None, None, None

    byte_count = int(line[1:3], 16)
    address = int(line[3:7], 16)
    record_type = int(line[7:9], 16)
    data_str = line[9:9 + byte_count * 2]
    data = bytes.fromhex(data_str) if data_str else b""

    return address, data, record_type


# ---------------------------------------------------------------------------
# Input parsing
# ---------------------------------------------------------------------------

def hex_to_records(filepath):
    """Read an Intel HEX file, return ``[(absolute_addr, data), ...]``.

    Supports record types:
    - 00 (Data)
    - 01 (End Of File)
    - 02 (Extended Segment Address: base = segment << 4)
    - 04 (Extended Linear Address: base = upper << 16)
    """
    records = []
    base_address = 0

    with open(filepath, "r") as f:
        for line in f:
            addr, data, rtype = parse_hex_line(line)
            if rtype == 2:          # Extended Segment Address Record
                # Two data bytes = segment number; physical base = segment * 16.
                base_address = int.from_bytes(data, "big") << 4
            elif rtype == 4:         # Extended Linear Address Record
                # Two data bytes = upper 16 bits; physical base = upper << 16.
                base_address = int.from_bytes(data, "big") << 16
            elif rtype == 0:         # Data Record
                records.append((base_address + addr, data))
            elif rtype == 1:         # End Of File
                break

    return records


def record_bounds(records):
    """Return ``(min_addr, max_addr_excl)`` for a list of ``(addr, data)``."""
    if not records:
        return 0, 0
    addrs = [r[0] for r in records]
    sizes = [len(r[1]) for r in records]
    return min(addrs), max(a + s for a, s in zip(addrs, sizes))


def regions_overlap(start1, end1, start2, end2):
    """Return True if the two byte ranges [start1, end1) ∩ [start2, end2)."""
    return start1 < end2 and start2 < end1


# ---------------------------------------------------------------------------
# Validation
# ---------------------------------------------------------------------------

def verify_hex_records(records, label):
    """Verify that data records are sorted and have no internal overlap."""
    prev_end = -1
    for addr, data in sorted(records, key=lambda r: r[0]):
        end = addr + len(data)
        if addr < prev_end:
            print("  WARNING: {} has overlapping records at 0x{:08X}".format(
                  label, addr))
        prev_end = end
    print("  {} records OK (addresses checked)".format(label))


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    core0_hex = os.path.join("core0", "build", "ch.hex")
    core1_hex = os.path.join("core1", "build", "ch.hex")

    for f in (core0_hex, core1_hex):
        if not os.path.isfile(f):
            print("Error: Cannot find {}".format(f))
            print("       Make sure both cores have been built first "
                  "('make cores').")
            sys.exit(1)

    print("Parsing {} ...".format(core0_hex))
    core0_records = hex_to_records(core0_hex)
    print("  -> {} data records".format(len(core0_records)))
    verify_hex_records(core0_records, "Core0")

    print("Parsing {} ...".format(core1_hex))
    core1_records = hex_to_records(core1_hex)
    print("  -> {} data records".format(len(core1_records)))
    verify_hex_records(core1_records, "Core1")

    # Region bounds
    start0, end0 = record_bounds(core0_records)
    start1, end1 = record_bounds(core1_records)

    print("\nCore0 address range: 0x{:08X} - 0x{:08X}".format(start0, end0))
    print("Core1 address range: 0x{:08X} - 0x{:08X}".format(start1, end1))

    if regions_overlap(start0, end0, start1, end1):
        print("\nWARNING: Core0 and Core1 address regions overlap!")
        print("         Check linker scripts to ensure proper separation.")
    else:
        print("\nOK: No address overlap detected.")

    # Merge & sort
    combined = core0_records[:] + core1_records[:]
    combined.sort(key=lambda r: r[0])

    # Write output
    output = "combined.hex"
    print("\nWriting {} ...".format(output))

    total_bytes_written = 0
    records_written = 0

    with open(output, "w") as f:
        current_upper = None   # track current extended address bank

        for addr, data in combined:
            upper = (addr >> 16) & 0xFFFF
            offset = addr & 0xFFFF

            # Emit Extended Linear Address record when the bank changes
            if upper != current_upper:
                f.write(make_extended_addr_record(upper))
                print("  [0x{:04X}0000] new address bank".format(upper))
                current_upper = upper

            # Write the data, splitting into 16-byte chunks
            remaining = data
            while remaining:
                chunk = remaining[:16]
                rec = make_data_record(offset, chunk)
                # ---- round-trip assertion ----
                # Re-parse and verify that the address matches what we intended.
                parsed_addr, parsed_data, parsed_type = parse_hex_line(rec)
                recovered_addr = (current_upper << 16) | parsed_addr
                assert parsed_type == 0, \
                    "Expected data record, got type {}".format(parsed_type)
                assert parsed_data == chunk, \
                    "Data mismatch in record at 0x{:08X}".format(recovered_addr)
                # The offset in the record should match the lower 16 bits
                # of the intended address (current_upper<<16 + parsed_addr).
                assert parsed_addr == offset, \
                    "Address mismatch: expected offset 0x{:04X}, " \
                    "got 0x{:04X}".format(offset, parsed_addr)
                # ---- end assertion ----

                f.write(rec)
                records_written += 1
                total_bytes_written += len(chunk)

                offset += len(chunk)
                remaining = remaining[16:]

        # End-Of-File record
        f.write(make_eof_record())

    print("  {} records, {} total bytes".format(records_written,
                                                 total_bytes_written))

    # Final self-check: read back the combined HEX and verify round-trip
    print("\nSelf-check: re-reading combined.hex ...")
    verify_records = hex_to_records(output)
    verify_records.sort(key=lambda r: r[0])
    # Compare with original combined records
    for i, ((a1, d1), (a2, d2)) in enumerate(zip(combined, verify_records)):
        assert a1 == a2, \
            "Round-trip address mismatch at index {}: " \
            "expected 0x{:08X}, got 0x{:08X}".format(i, a1, a2)
        assert d1 == d2, \
            "Round-trip data mismatch at index {} addr 0x{:08X}: " \
            "expected {} bytes, got {}".format(i, a1, len(d1), len(d2))
    assert len(combined) == len(verify_records), \
        "Record count mismatch: original {} vs re-parsed {}".format(
            len(combined), len(verify_records))
    print("  OK - {} records verified, all addresses and data match.\n"
          .format(len(verify_records)))

    print("Done! Combined image: {}".format(output))
    print("Flash this file to program both cores.")


if __name__ == "__main__":
    main()
