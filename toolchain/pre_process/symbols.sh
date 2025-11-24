#!/bin/bash
SCRIPTDIR=$(dirname "$BASH_SOURCE")
source "$SCRIPTDIR/../MaxOS.sh"

set -euo pipefail
LC_ALL=C

# Get the args
INPUT="${1:-}"
OUT="${2:-}"
if [ -z "$INPUT" ] || [ -z "$OUT" ]; then
  fail "Usage: $0 <elf_input> <out_cpp>"
fi


# Empty file as there needs to be something to compile to grab the symbols from
TMPDIR="$(mktemp -d /tmp/gen_sym.XXXXXX)"
trap 'rm -rf "$TMPDIR"' EXIT
RAW="$TMPDIR/nm.raw"
if [ "$INPUT" = "pre" ]; then
  : > "$RAW"
else

  # Check if the file exists
  if [ ! -f "$INPUT" ]; then
    fail "Error: input file '$INPUT' not found"
  fi

  # Locate the required commands
  CXXFILT="$(command -v c++filt || true)"
  NM="$(command -v nm || true)"
  if [ -z "$NM" ]; then
    echo "Error: nm not found (required to read ELF input)"
    exit 3
  fi

  # Get all the function addresses
  $NM --defined-only --numeric-sort "$INPUT" | awk '$2=="T" || $2=="t" { printf "%s\t%s\n",$1,$3 }' > "$RAW"
fi

get_demangled_names() {
   if [ ! -s "$RAW" ]; then
      return 0
    fi
    cut -f2- "$RAW" | $CXXFILT 2>/dev/null
}

# Get the date in the format Xth Month YYYY
DATE_STR="$(date +'%-d')"
case "$DATE_STR" in
  1|21|31) DATE_STR="${DATE_STR}st" ;;
  2|22) DATE_STR="${DATE_STR}nd" ;;
  3|23) DATE_STR="${DATE_STR}rd" ;;
  *) DATE_STR="${DATE_STR}th" ;;
esac
DATE_STR="$DATE_STR $(date +' %B %Y')"

# Write the header
cat > "$OUT" <<'EOF'

/**
 * @file symbols.h
 * @brief Defines kernel symbol resolution functions
 *
 * @date ${DATE_STR}
 * @author Max Tyson
 */
#ifndef MAXOS_COMMON_SYMBOLS_H
#define MAXOS_COMMON_SYMBOLS_H

#include <stdint.h>
#include <stddef.h>
#include <memory/physical.h>

namespace MaxOS::common{

  /**
  * @struct KernelSymbol
  * @brief Holds the address and name of a kernel symbol
  *
  * @typedef kernel_sym_t
  * @brief alias for KernelSymbol
  */
  typedef struct KernelSymbol {
    uintptr_t address;              ///< The address of the symbol (loaded address)
    const char* name;               ///< The stripped name of the symbol (including namespaces)
  } kernel_sym_t;

  /// Holds a list of address/name pairs for kernel symbols
  const kernel_sym_t kernel_symbols[] = {
EOF
if [ ! -s "$RAW" ]; then
  COUNT=0
else
  # Write each symbol out, formatted
  COUNT=$(paste <(cut -f1 "$RAW") <(get_demangled_names) | \
    awk -F'\t' '

      # Ensure no duplicates
      !seen[$1]++ {
        address = $1;
        name = $2;

        # Remove any unprintable chars
        gsub(/[^[:print:]\t]/, "", name);

        # Clean extra whitespace
        sub(/^[[:space:]]+/, "", name);
        sub(/[[:space:]]+$/, "", name);

        # Remove the trailing stuff
        gsub(/ (const|volatile)+$/, "", name);
        gsub(/MaxOS::/, "", name)

        # Fill empty name
        if (name == "")
          name = "sym_0x" address;

        # Escape the string
        gsub(/\\/, "\\\\", name);
        gsub(/"/, "\\\"", name);

        # Format hex
        gsub(/^0x/, "", address);
        address = toupper(address);
        sub(/^0+/, "", address);
        if (address == "")
          address = "0";

        printf "  { (uintptr_t)0x%s, \"%s\" },\n", address, name;
        count++;
      }
     ' | tee -a "$OUT" | wc -l
  )
fi

# Write the footer
cat >> "$OUT" <<EOF
  };

  /// The number of symbols detected to be in the kernel
  constexpr size_t KERNEL_SYMBOLS_COUNT = $COUNT;

  const char* resolve_symbol(uintptr_t rip, uintptr_t link_base = memory::HIGHER_HALF_KERNEL_OFFSET, uintptr_t load_base = memory::HIGHER_HALF_KERNEL_OFFSET);
}

#endif //MAXOS_COMMON_SYMBOLS_H
EOF

# Did change
msg "Wrote $OUT (symbols: $COUNT)"