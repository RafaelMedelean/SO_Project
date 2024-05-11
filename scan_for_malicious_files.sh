#!/bin/bash

# Verificarea argumentelor
if [ "$#" -lt 2 ]; then
  echo "Usage: $0 izolated_space_dir dir1 [dir2 ... dirN]"
  exit 1
fi

# Directorul în care vor fi mutați fișierii periculoși
izolated_space_dir=$1

# Eliminăm primul argument pentru că este izolated_space_dir
shift

# Funcție pentru a verifica și procesa fiecare fișier
process_file() {
  local file=$1
  local izolated_space_dir=$2
  permissions=$(stat -c %A "$file")
  echo "File: $file, Permissions: $permissions"
  
  if [ "$permissions" == "-r-xr-xr-x" ]; then # ""----------
    echo "Verifying $file for malicious content..."
    
    # Verificarea numărului de linii, cuvinte și caractere
    local line_count=$(wc -l < "$file")
    local word_count=$(wc -w < "$file")
    local char_count=$(wc -m < "$file")
    echo "Lines: $line_count, Words: $word_count, Characters: $char_count"
    
    # Căutăm în conținutul fișierului pattern-uri specifice de maleficență
    if grep -q -e 'corrupted' -e 'dangerous' -e 'risk' -e 'attack' -e 'malware' -e 'malicious' -e '[^\x00-\x7F]' "$file"; then
      echo "Malicious file detected: $file. Moving to $izolated_space_dir."
      mv "$file" "$izolated_space_dir"
    else
      echo "File $file does not contain any malicious indicators."
    fi
  fi
}

# Exportăm funcția pentru a fi disponibilă în subshell-uri
export -f process_file

# Verificăm și procesăm fișierele din fiecare director specificat
for dir in "$@"; do
  if [ -d "$dir" ]; then
    echo "Scanning directory $dir for files with no permissions..."
    find "$dir" -type f -exec bash -c 'process_file "$0" "$@"' {} "$izolated_space_dir" \;
  else
    echo "Warning: Directory $dir does not exist."
  fi
done

echo "Scan complete."
