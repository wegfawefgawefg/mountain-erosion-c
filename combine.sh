#!/usr/bin/env bash

OUTFILE="concat.c"
> "$OUTFILE"  # Clear the output file

# 1. Append the directory structure at the top
echo "///// Directory structure:" >> "$OUTFILE"
tree ./src >> "$OUTFILE"
echo >> "$OUTFILE"

# 2. Recursively find and concatenate .h, .c, .glsl, and .slsl files
while IFS= read -r -d '' file; do
  echo "///// filename: $file" >> "$OUTFILE"
  cat "$file" >> "$OUTFILE"
  echo >> "$OUTFILE"
done < <(find ./src -type f \( -name "*.h" -o -name "*.c" -o -name "*.glsl" -o -name "*.slsl" \) -print0 | sort -z)
