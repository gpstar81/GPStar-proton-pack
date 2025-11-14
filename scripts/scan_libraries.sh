#!/usr/bin/env bash
# Purpose:
#   Phase-based library scanner for PlatformIO projects
#   Phase 1: Discover project platformio.ini files
#   Phase 2: Extract library dependencies from each file
#   Phase 3: Query PlatformIO registry for latest versions
# Notes:
#   - Run from the repository's scripts/ directory
#   - Requires `pio` (PlatformIO CLI) and `jq` in PATH

set -euo pipefail

# Locate script dir and source directory
s_script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
s_source_dir="$s_script_dir/../source"

# Required tools validation
if ! command -v pio >/dev/null 2>&1; then
  printf "Error: 'pio' (PlatformIO CLI) is required but not found in PATH.\n" >&2
  exit 2
fi
if ! command -v jq >/dev/null 2>&1; then
  printf "Error: 'jq' is required but not found in PATH.\n" >&2
  exit 2
fi

# Temporary files for data storage
s_tmp_projects="$(mktemp)"
s_tmp_libraries="$(mktemp)"
trap 'rm -f "$s_tmp_projects" "$s_tmp_libraries"' EXIT

printf "=== PHASE 1: Project Discovery ===\n" >&2

# Phase 1: Discover project directories and their platformio.ini files
printf "Scanning source directory: %s\n" "$s_source_dir" >&2
i_project_count=0

for s_project_path in "$s_source_dir"/*/; do
  if [ -d "$s_project_path" ]; then
    s_project_name="$(basename "$s_project_path")"
    s_platformio_file="$s_project_path/platformio.ini"
    
    if [ -f "$s_platformio_file" ]; then
      # Display clean relative path from repo root
      s_display_path="/source/$s_project_name/platformio.ini"
      printf "Found project: %s -> %s\n" "$s_project_name" "$s_display_path" >&2
      printf "%s\n" "$s_platformio_file" >> "$s_tmp_projects"
      i_project_count=$((i_project_count + 1))
    else
      printf "Skipping %s (no platformio.ini found)\n" "$s_project_name" >&2
    fi
  fi
done

printf "Phase 1 Complete: Found %d projects with platformio.ini files\n\n" "$i_project_count" >&2

printf "=== PHASE 2: Library Extraction ===\n" >&2

# Phase 2: Extract library dependencies from each platformio.ini file
i_library_count=0
while IFS= read -r s_platformio_file; do
  s_project_name="$(basename "$(dirname "$s_platformio_file")")"
  printf "Processing libraries in project: %s\n" "$s_project_name" >&2
  
  # Extract lib_deps entries using sed
  while IFS= read -r s_lib_line; do
    # Skip empty lines
    [ -z "$s_lib_line" ] && continue
    
    # Remove inline comments (semicolon or hash)
    s_lib_clean="${s_lib_line%%;*}"
    s_lib_clean="${s_lib_clean%%#*}"
    
    # Trim whitespace
    s_lib_clean="$(printf '%s' "$s_lib_clean" | sed 's/^[[:space:]]*//; s/[[:space:]]*$//')"
    [ -z "$s_lib_clean" ] && continue
    
    # Extract library name (before @) and version spec (after @)
    s_lib_name="${s_lib_clean%%@*}"
    s_lib_spec=""
    if [[ "$s_lib_clean" == *"@"* ]]; then
      s_lib_spec="${s_lib_clean#*@}"
    fi
    
    # Trim library name
    s_lib_name="$(printf '%s' "$s_lib_name" | sed 's/^[[:space:]]*//; s/[[:space:]]*$//')"
    [ -z "$s_lib_name" ] && continue
    
    #printf "  Found library: %s@%s\n" "$s_lib_name" "$s_lib_spec" >&2
    printf "%s|%s|%s\n" "$s_lib_name" "$s_lib_spec" "$s_project_name" >> "$s_tmp_libraries"
    i_library_count=$((i_library_count + 1))
    
  done < <(sed -n '/^lib_deps/,/^[^[:space:]]/{/^lib_deps/d; /^[[:space:]]/p;}' "$s_platformio_file")
  
done < "$s_tmp_projects"

printf "Phase 2 Complete: Extracted %d library entries\n\n" "$i_library_count" >&2

printf "=== PHASE 3: Version Checking ===\n" >&2

# Create cache file for library version lookups
s_tmp_version_cache="$(mktemp)"
trap 'rm -f "$s_tmp_projects" "$s_tmp_libraries" "$s_tmp_version_cache"' EXIT

# Generate report header
printf "%-37s %-15s %-12s %-15s %s\n" "Library" "Current" "Latest" "Status" "Projects"
printf "%-37s %-15s %-12s %-15s %s\n" "-------" "-------" "------" "------" "--------"

# Process unique libraries using individual searches with caching
awk -F'|' '{print $1}' "$s_tmp_libraries" | sort -u | while IFS= read -r s_unique_lib; do
  #printf "Checking library: '%s'\n" "$s_unique_lib" >&2
  
  # Get first version spec for this library (for comparison)
  s_current_spec="$(awk -F'|' -v lib="$s_unique_lib" '$1==lib{print $2; exit}' "$s_tmp_libraries")"
  
  # Get all projects using this library
  s_projects="$(awk -F'|' -v lib="$s_unique_lib" '$1==lib{print $3}' "$s_tmp_libraries" | sort -u | tr '\n' ',' | sed 's/,$//')"
  
  # For libraries with author/name format, search by just the library name
  s_search_name="$s_unique_lib"
  if [[ "$s_unique_lib" == *"/"* ]]; then
    s_search_name="${s_unique_lib#*/}"  # Remove author prefix for search
    #printf "  Searching for library name only: '%s'\n" "$s_search_name" >&2
  fi
  
  # Check if we already have this library's version cached
  s_cached_version="$(grep "^${s_search_name}|" "$s_tmp_version_cache" 2>/dev/null | cut -d'|' -f2 || echo "")"
  
  if [ -n "$s_cached_version" ]; then
    #printf "  Using cached version: '%s'\n" "$s_cached_version" >&2
    s_latest_version="$s_cached_version"
  else
    #printf "  Querying PlatformIO registry for: '%s'\n" "$s_search_name" >&2
    
    # Query PlatformIO using individual search
    s_pio_raw_output="$(pio lib search "$s_search_name" --json-output 2>/dev/null || echo '{"items":[]}')"
    
    # Find exact match in search results, then try partial match (take only first result)
    s_latest_version="$(printf '%s' "$s_pio_raw_output" | jq -r --arg search_name "$s_search_name" '
      if (.items and (.items | length > 0)) then
        ((.items[] | select(.name == $search_name) | .versionname) // 
         (.items[] | select(.name | test($search_name; "i")) | .versionname) //
         "n/a") as $result |
        if ($result | type) == "string" then $result else "n/a" end
      else
        "n/a"
      end
    ' 2>/dev/null | head -n1 || echo "n/a")"
    
    # If no match found and this was an author/library format, try searching with the full name
    if [ "$s_latest_version" = "n/a" ] && [[ "$s_unique_lib" == *"/"* ]]; then
      s_full_search_output="$(pio lib search "$s_unique_lib" --json-output 2>/dev/null || echo '{"items":[]}')"
      s_latest_version="$(printf '%s' "$s_full_search_output" | jq -r --arg full_name "$s_unique_lib" '
        if (.items and (.items | length > 0)) then
          (.items[] | select(.name == $full_name or (.ownername + "/" + .name) == $full_name) | .versionname) // "n/a"
        else
          "n/a"
        end
      ' 2>/dev/null | head -n1 || echo "n/a")"
    fi
    
    # Cache the result for future lookups
    printf "%s|%s\n" "$s_search_name" "$s_latest_version" >> "$s_tmp_version_cache"
    #printf "  Latest version result (cached): '%s'\n" "$s_latest_version" >&2
  fi
  
  # Normalize current version for comparison (strip prefix characters like ^, ~, >=)
  s_current_normalized="$(printf '%s' "$s_current_spec" | sed 's/^[^0-9]*//; s/[^0-9.].*$//')"
  [ -z "$s_current_normalized" ] && s_current_normalized="unspecified"
  
  # Determine status by comparing versions
  s_status="unknown"
  if [ "$s_current_normalized" = "unspecified" ]; then
    s_status="no-version"
  elif [ "$s_latest_version" = "n/a" ]; then
    s_status="not-found"
  elif [ "$s_current_normalized" = "$s_latest_version" ]; then
    s_status="up-to-date"
  else
    # Use sort -V to compare versions (version-aware sorting)
    s_newer_version="$(printf '%s\n%s\n' "$s_current_normalized" "$s_latest_version" | sort -V | tail -n1)"
    if [ "$s_newer_version" = "$s_latest_version" ]; then
      s_status="outdated"
    else
      s_status="newer"
    fi
  fi
  
  # Output the formatted result row
  printf "%-37s %-15s %-12s %-15s %s\n" "$s_unique_lib" "$s_current_spec" "$s_latest_version" "$s_status" "$s_projects"
done

printf "\nPhase 3 Complete: Library version check finished\n" >&2