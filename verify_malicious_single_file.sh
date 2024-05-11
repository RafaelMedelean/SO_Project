#!/bin/bash

# Check if the correct number of arguments is passed
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <file_path>"
    exit 1
fi

file=$1

# Verify if the file exists
if [ ! -f "$file" ]; then
    echo "File not found: $file"
    exit 1
fi

# Gather file metrics
lines=$(wc -l < "$file")
words=$(wc -w < "$file")
chars=$(wc -m < "$file")

# Criteria for a suspicious file
if [ "$lines" -lt 3 ] || [ "$words" -gt 1000 ] || [ "$chars" -gt 2000 ]; then
    echo "$file"
    exit 0
fi

# Check for non-ASCII characters and malicious keywords
if LC_ALL=C grep -q '[^\x00-\x7F]' "$file" || grep -qi -e 'corrupted' -e 'dangerous' -e 'risk' -e 'attack' -e 'malware' -e 'malicious' "$file"; then
    echo "$file"
    exit 0
fi

# If no suspicious patterns are found, print "SAFE"
echo "SAFE"
