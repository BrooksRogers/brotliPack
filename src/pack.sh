#!/bin/bash


# Example: ./pack.sh my_binary my_packed_flag
# Let me know if you want this script to also embed a timestamp, signature checks, or something else.


if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <input_binary> <output_packed_binary>"
    exit 1
fi

INPUT="$1"
OUTPUT="$2"

if [[ ! -x ./packer ]] || [[ ! -x ./self_extractor_inmem ]]; then
    echo "Error: Make sure 'packer' and 'self_extractor_inmem' are built first."
    echo "Run: make"
    exit 1
fi

TMP_COMPRESSED="tmp_flag_compressed"
MARKER="marker.txt"

if [[ ! -f "$MARKER" ]]; then
    echo "Error: marker.txt not found. Please ensure it exists."
    exit 1
fi

./packer "$INPUT" "$TMP_COMPRESSED" || exit 1

cat self_extractor_inmem "$MARKER" "$TMP_COMPRESSED" > "$OUTPUT"
chmod +x "$OUTPUT"

rm -f "$TMP_COMPRESSED"

echo "[+] Packed binary written to: $OUTPUT"