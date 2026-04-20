#!/bin/bash

# Check PlatformIO Python Dependencies
# This script checks for installed modules in different Python environments

echo "=== PlatformIO Python Dependencies Checker ==="
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to check if a module exists in a Python environment
check_module() {
    local python_exec="$1"
    local module_name="$2"
    
    if [ ! -f "$python_exec" ]; then
        return 2  # Python executable not found
    fi
    
    "$python_exec" -c "import $module_name" 2>/dev/null
    return $?
}

# Function to get all installed packages in an environment
get_installed_packages() {
    local python_exec="$1"
    
    if [ ! -f "$python_exec" ]; then
        return 1
    fi
    
    "$python_exec" -m pip list --format=freeze 2>/dev/null | grep -v "^-e " | sort
}

# Function to get package name without version
get_package_name() {
    echo "$1" | cut -d'=' -f1 | cut -d'>' -f1 | cut -d'<' -f1 | cut -d'!' -f1
}

# Required core modules for ESP32 development (always check these)
get_core_modules() {
    echo "littlefs:littlefs-python"
    echo "fatfs:fatfs-ng==0.1.15" 
    echo "esptool:esptool"
    echo "kconfiglib:kconfiglib"
}

# Function to compare environments dynamically
compare_environments() {
    local reference_python="$1"
    local target_python="$2"
    local reference_name="$3"
    local target_name="$4"
    
    if [ ! -f "$reference_python" ] || [ ! -f "$target_python" ]; then
        return 1
    fi
    
    echo -e "${BLUE}=== Comparing $target_name to $reference_name ===${NC}"
    
    # Get installed packages from both environments
    local temp_ref=$(mktemp)
    local temp_target=$(mktemp)
    
    get_installed_packages "$reference_python" > "$temp_ref"
    get_installed_packages "$target_python" > "$temp_target"
    
    echo -e "${YELLOW}Packages in $reference_name but missing in $target_name:${NC}"
    
    local missing_count=0
    while IFS= read -r ref_package; do
        local ref_name=$(get_package_name "$ref_package")
        
        # Skip some packages that shouldn't be copied
        case "$ref_name" in
            "pip"|"setuptools"|"wheel"|"platformio") continue ;;
        esac
        
        if ! grep -q "^${ref_name}=" "$temp_target" 2>/dev/null && ! grep -q "^${ref_name}>" "$temp_target" 2>/dev/null && ! grep -q "^${ref_name}<" "$temp_target" 2>/dev/null; then
            echo -e "  ${RED}✗${NC} $ref_package"
            echo "$target_python -m pip install \"$ref_package\"" >> "$temp_target.commands"
            ((missing_count++))
        fi
    done < "$temp_ref"
    
    if [ $missing_count -eq 0 ]; then
        echo -e "  ${GREEN}✓ All packages are present${NC}"
    else
        echo -e "\n${BLUE}Commands to sync $target_name with $reference_name:${NC}"
        if [ -f "$temp_target.commands" ]; then
            cat "$temp_target.commands" | sed 's/^/  /'
        fi
    fi
    
    # Cleanup
    rm -f "$temp_ref" "$temp_target" "$temp_target.commands"
    
    return $missing_count
}

# Function to check specific functions in fatfs
check_fatfs_functions() {
    local python_exec="$1"
    
    if [ ! -f "$python_exec" ]; then
        return 2
    fi
    
    "$python_exec" -c "from fatfs import Partition, RamDisk, create_extended_partition" 2>/dev/null
    return $?
}

# Function to install missing modules
install_module() {
    local python_exec="$1"
    local pip_exec="${python_exec%/python}/pip"
    local module_name="$2"
    local display_name="$3"
    
    echo -e "${YELLOW}Installing $display_name...${NC}"
    if "$python_exec" -m pip install "$module_name" > /dev/null 2>&1; then
        echo -e "${GREEN}✓ Successfully installed $display_name${NC}"
        return 0
    else
        echo -e "${RED}✗ Failed to install $display_name${NC}"
        return 1
    fi
}

# Find PlatformIO installations
echo -e "${BLUE}1. Detecting PlatformIO installations...${NC}"

# SOURCE: Check for VS Code/Extension installed PlatformIO (usually the working reference)
VSCODE_PIO_PYTHON=""
if [ -f "$HOME/.platformio/penv/bin/python" ]; then
    VSCODE_PIO_PYTHON="$HOME/.platformio/penv/bin/python"
    echo -e "  ${GREEN}[SOURCE]${NC} VS Code PlatformIO: $VSCODE_PIO_PYTHON"
elif [ -f "$HOME/.platformio/penv/Scripts/python.exe" ]; then
    # Windows path
    VSCODE_PIO_PYTHON="$HOME/.platformio/penv/Scripts/python.exe"
    echo -e "  ${GREEN}[SOURCE]${NC} VS Code PlatformIO: $VSCODE_PIO_PYTHON"
else
    echo -e "  ${RED}[SOURCE]${NC} VS Code PlatformIO: Not found at ~/.platformio/penv/"
fi

# TARGET: Check for other PlatformIO installations to sync to
HOMEBREW_PIO_PYTHON=""
if command -v brew >/dev/null 2>&1; then
    BREW_PIO_PATH=$(brew --prefix platformio 2>/dev/null)
    if [ -n "$BREW_PIO_PATH" ] && [ -f "$BREW_PIO_PATH/libexec/bin/python" ]; then
        HOMEBREW_PIO_PYTHON="$BREW_PIO_PATH/libexec/bin/python"
        echo -e "  ${YELLOW}[TARGET]${NC} Homebrew PlatformIO: $HOMEBREW_PIO_PYTHON"
    else
        echo -e "  ${RED}[TARGET]${NC} Homebrew PlatformIO: Not found"
    fi
fi

# Check for pipx-installed PlatformIO
PIPX_PIO_PYTHON=""
if [ -f "$HOME/.local/pipx/venvs/platformio/bin/python" ]; then
    PIPX_PIO_PYTHON="$HOME/.local/pipx/venvs/platformio/bin/python"
    echo -e "  ${YELLOW}[TARGET]${NC} Pipx PlatformIO: $PIPX_PIO_PYTHON"
else
    echo -e "  ${RED}[TARGET]${NC} Pipx PlatformIO: Not found at ~/.local/pipx/venvs/platformio/"
fi

# Check for system-wide Python (informational only)
SYSTEM_PYTHON=""
if command -v python3 >/dev/null 2>&1; then
    SYSTEM_PYTHON=$(which python3)
    echo -e "  ${BLUE}[INFO]${NC} System Python3: $SYSTEM_PYTHON"
elif command -v python >/dev/null 2>&1; then
    SYSTEM_PYTHON=$(which python)
    echo -e "  ${BLUE}[INFO]${NC} System Python: $SYSTEM_PYTHON"
else
    echo -e "  ${RED}[INFO]${NC} System Python: Not found"
fi

echo

# Check dependencies in each environment
echo -e "${BLUE}2. Checking dependencies in each environment...${NC}"

check_environment() {
    local python_exec="$1"
    local env_name="$2"
    local show_all_packages="$3"
    
    echo -e "\n${YELLOW}Environment: $env_name${NC}"
    echo "  Python: $python_exec"
    
    if [ ! -f "$python_exec" ]; then
        echo -e "  ${RED}✗ Python executable not found${NC}"
        return
    fi
    
    # Get Python version
    local python_version=$("$python_exec" --version 2>&1)
    echo "  Version: $python_version"
    
    # Check core ESP32 dependencies first
    echo "  Core ESP32 Dependencies:"
    get_core_modules | while IFS=: read -r module package_name; do
        if check_module "$python_exec" "$module"; then
            local version=$(get_module_version "$python_exec" "$module")
            
            # Special check for fatfs to ensure it has ESP32 functions
            if [ "$module" = "fatfs" ]; then
                if check_fatfs_functions "$python_exec"; then
                    echo -e "    $module: ${GREEN}✓ installed${NC} (version: $version, ESP32 support: ✓)"
                else
                    echo -e "    $module: ${YELLOW}⚠ installed but missing ESP32 functions${NC} (version: $version)"
                fi
            else
                echo -e "    $module: ${GREEN}✓ installed${NC} (version: $version)"
            fi
        else
            echo -e "    $module: ${RED}✗ missing${NC} (install: $package_name)"
        fi
    done
    
    # Show package count
    local package_count=$(get_installed_packages "$python_exec" | wc -l | tr -d ' ')
    echo -e "  ${BLUE}Total packages installed: $package_count${NC}"
    
    # Show all packages if requested
    if [ "$show_all_packages" = "show-all" ]; then
        echo "  All installed packages:"
        get_installed_packages "$python_exec" | while IFS= read -r package; do
            echo "    $package"
        done
    fi
}

# Function to get module version (kept for core module checking)
get_module_version() {
    local python_exec="$1"
    local module_name="$2"
    
    "$python_exec" -c "
try:
    import $module_name
    print(getattr($module_name, '__version__', 'unknown'))
except:
    print('not installed')
" 2>/dev/null
}

# Check the SOURCE environment first (VS Code PlatformIO)
if [ -n "$VSCODE_PIO_PYTHON" ]; then
    echo -e "${GREEN}=== SOURCE: VS Code PlatformIO (Reference Environment) ===${NC}"
    check_environment "$VSCODE_PIO_PYTHON" "VS Code PlatformIO [SOURCE]" "$show_all_flag"
else
    echo -e "${RED}=== SOURCE: VS Code PlatformIO - NOT FOUND ===${NC}"
    echo -e "${YELLOW}This is usually the working PlatformIO installation with all dependencies.${NC}"
    echo -e "${YELLOW}Consider installing PlatformIO via VS Code extension first.${NC}"
fi

# Check TARGET environments that we want to sync
if [ -n "$HOMEBREW_PIO_PYTHON" ]; then
    echo -e "\n${YELLOW}=== TARGET: Homebrew PlatformIO (Sync Destination) ===${NC}"
    check_environment "$HOMEBREW_PIO_PYTHON" "Homebrew PlatformIO [TARGET]" "$show_all_flag"
fi

if [ -n "$PIPX_PIO_PYTHON" ]; then
    echo -e "\n${YELLOW}=== TARGET: Pipx PlatformIO (Sync Destination) ===${NC}"
    check_environment "$PIPX_PIO_PYTHON" "Pipx PlatformIO [TARGET]" "$show_all_flag"
fi

# Show system Python only for informational purposes
if [ -n "$SYSTEM_PYTHON" ] && [ "$show_all_flag" = "show-all" ]; then
    echo -e "\n${BLUE}=== INFO: System Python (Informational) ===${NC}"
    check_environment "$SYSTEM_PYTHON" "System Python [INFO]" "$show_all_flag"
fi

echo
echo -e "${BLUE}3. Package synchronization analysis...${NC}"

# If VS Code PlatformIO exists, use it as SOURCE to sync TARGET environments
if [ -n "$VSCODE_PIO_PYTHON" ]; then
    echo -e "${GREEN}Using VS Code PlatformIO as SOURCE (reference for syncing)${NC}"
    
    if [ -n "$HOMEBREW_PIO_PYTHON" ]; then
        compare_environments "$VSCODE_PIO_PYTHON" "$HOMEBREW_PIO_PYTHON" "VS Code PlatformIO [SOURCE]" "Homebrew PlatformIO [TARGET]"
    fi
    
    if [ -n "$PIPX_PIO_PYTHON" ]; then
        echo
        compare_environments "$VSCODE_PIO_PYTHON" "$PIPX_PIO_PYTHON" "VS Code PlatformIO [SOURCE]" "Pipx PlatformIO [TARGET]"
    fi
    
    if [ -z "$HOMEBREW_PIO_PYTHON" ] && [ -z "$PIPX_PIO_PYTHON" ]; then
        echo -e "${YELLOW}No TARGET environments found to sync.${NC}"
    fi
    
    # Only show system Python comparison if explicitly requested
    if [ -n "$SYSTEM_PYTHON" ] && [ "$show_all_flag" = "show-all" ]; then
        echo
        compare_environments "$VSCODE_PIO_PYTHON" "$SYSTEM_PYTHON" "VS Code PlatformIO [SOURCE]" "System Python [INFO]"
    fi
elif [ -n "$HOMEBREW_PIO_PYTHON" ]; then
    echo -e "${YELLOW}VS Code PlatformIO not found. Using Homebrew PlatformIO as fallback reference.${NC}"
    
    if [ -n "$PIPX_PIO_PYTHON" ]; then
        compare_environments "$HOMEBREW_PIO_PYTHON" "$PIPX_PIO_PYTHON" "Homebrew PlatformIO" "Pipx PlatformIO [TARGET]"
    fi
    
    if [ -n "$SYSTEM_PYTHON" ] && [ "$show_all_flag" = "show-all" ]; then
        compare_environments "$HOMEBREW_PIO_PYTHON" "$SYSTEM_PYTHON" "Homebrew PlatformIO" "System Python [INFO]"
    fi
elif [ -n "$PIPX_PIO_PYTHON" ]; then
    echo -e "${YELLOW}VS Code PlatformIO not found. Using Pipx PlatformIO as fallback reference.${NC}"
    
    if [ -n "$SYSTEM_PYTHON" ] && [ "$show_all_flag" = "show-all" ]; then
        compare_environments "$PIPX_PIO_PYTHON" "$SYSTEM_PYTHON" "Pipx PlatformIO" "System Python [INFO]"
    fi
else
    echo -e "${RED}No PlatformIO installation found to use as SOURCE${NC}"
    echo -e "${YELLOW}Install PlatformIO via VS Code extension, pipx, or Homebrew first.${NC}"
fi

echo
echo -e "${BLUE}4. Core module validation...${NC}"

# Focus on TARGET environments for fixing core modules
echo -e "${BLUE}Recommendation: Keep VS Code PlatformIO as SOURCE, sync TARGET environments${NC}"
echo -e "VS Code PlatformIO usually has the most complete and working set of dependencies."

# Legacy fix function for core modules only
fix_environment() {
    local python_exec="$1"
    local env_name="$2"
    local should_fix="$3"
    
    if [ ! -f "$python_exec" ]; then
        return
    fi
    
    local needs_fix=false
    local temp_file=$(mktemp)
    
    # Check each core module and collect commands
    get_core_modules | while IFS=: read -r module package_name; do
        if ! check_module "$python_exec" "$module"; then
            echo "$python_exec -m pip install $package_name" >> "$temp_file"
            echo "needs_fix" >> "$temp_file.flag"
        elif [ "$module" = "fatfs" ] && ! check_fatfs_functions "$python_exec"; then
            echo "$python_exec -m pip uninstall fatfs -y" >> "$temp_file"
            echo "$python_exec -m pip install fatfs-ng==0.1.15" >> "$temp_file"
            echo "needs_fix" >> "$temp_file.flag"
        fi
    done
    
    if [ -f "$temp_file.flag" ]; then
        needs_fix=true
    fi
    
    if [ "$needs_fix" = true ]; then
        echo -e "\n${YELLOW}$env_name needs core module fixes${NC}"
        
        if [ "$should_fix" = "--fix" ]; then
            echo "Fixing core modules in $env_name..."
            
            # Execute fix commands
            while IFS= read -r cmd; do
                echo -e "${YELLOW}Running: $cmd${NC}"
                if eval "$cmd" > /dev/null 2>&1; then
                    echo -e "${GREEN}✓ Success${NC}"
                else
                    echo -e "${RED}✗ Failed${NC}"
                fi
            done < "$temp_file"
        else
            echo -e "${BLUE}Commands to fix core modules in this environment:${NC}"
            while IFS= read -r cmd; do
                echo "  $cmd"
            done < "$temp_file"
        fi
    else
        echo -e "\n${GREEN}$env_name has all core modules${NC}"
    fi
    
    # Cleanup
    rm -f "$temp_file" "$temp_file.flag"
}

# Check if --show-all flag is provided for detailed package listing
show_all_flag=""
if [ "$1" = "--show-all" ]; then
    show_all_flag="show-all"
    shift
fi

# Check if --fix flag is provided
if [ "$1" = "--fix" ]; then
    echo -e "${BLUE}Fixing core modules in environments automatically...${NC}"
    
    if [ -n "$HOMEBREW_PIO_PYTHON" ]; then
        fix_environment "$HOMEBREW_PIO_PYTHON" "Homebrew PlatformIO" --fix
    fi
    
    if [ -n "$PIPX_PIO_PYTHON" ]; then
        fix_environment "$PIPX_PIO_PYTHON" "Pipx PlatformIO" --fix
    fi
    
    if [ -n "$VSCODE_PIO_PYTHON" ]; then
        fix_environment "$VSCODE_PIO_PYTHON" "VS Code PlatformIO" --fix
    fi
else
    if [ -n "$HOMEBREW_PIO_PYTHON" ]; then
        fix_environment "$HOMEBREW_PIO_PYTHON" "Homebrew PlatformIO"
    fi
    
    if [ -n "$PIPX_PIO_PYTHON" ]; then
        fix_environment "$PIPX_PIO_PYTHON" "Pipx PlatformIO"
    fi
    
    if [ -n "$VSCODE_PIO_PYTHON" ]; then
        fix_environment "$VSCODE_PIO_PYTHON" "VS Code PlatformIO"
    fi
    
    echo
    echo -e "${BLUE}=== USAGE ===${NC}"
    echo -e "${GREEN}Check dependencies and compare SOURCE→TARGET:${NC}"
    echo "  $0"
    echo
    echo -e "${GREEN}Check and automatically fix CORE modules in TARGET environments:${NC}"
    echo "  $0 --fix"
    echo 
    echo -e "${GREEN}Show all installed packages in each environment:${NC}"
    echo "  $0 --show-all"
    echo
    echo -e "${GREEN}Sync packages from SOURCE (VS Code) to TARGET (Homebrew/Pipx):${NC}"
    echo "  # Copy the sync commands from the analysis section above"
    echo
    echo -e "${GREEN}Manual installation commands:${NC}"
    echo "  # Core ESP32 modules for any Python environment:"
    echo "  \$PYTHON_PATH -m pip install littlefs-python"
    echo "  \$PYTHON_PATH -m pip install fatfs-ng==0.1.15"
    echo "  \$PYTHON_PATH -m pip install esptool"
    echo "  \$PYTHON_PATH -m pip install kconfiglib"
    echo "  # For pipx installations, use 'pipx inject' instead:"
    echo "  pipx inject platformio littlefs-python"
    echo "  pipx inject platformio fatfs-ng==0.1.15"
    echo "  # Or sync specific packages with versions:"
    echo "  \$PYTHON_PATH -m pip install \"package==version\""
fi

echo
echo -e "${BLUE}=== SUMMARY ===${NC}"
echo -e "${BLUE}SOURCE → TARGET synchronization:${NC}"
get_core_modules | while IFS=: read -r module package_name; do
    echo "  • $module: Install with 'pip install $package_name'"
done
echo -e "\n${GREEN}SOURCE:${NC} VS Code PlatformIO (~/.platformio/penv/) - Working reference environment"
echo -e "${YELLOW}TARGET:${NC} Homebrew PlatformIO (brew install platformio) - Environment to sync"
echo -e "${YELLOW}TARGET:${NC} Pipx PlatformIO (pipx install platformio) - Environment to sync"
echo -e "${BLUE}INFO:${NC} System Python - Informational only"
echo -e "\n${YELLOW}Note:${NC} This script compares ALL installed packages dynamically."
echo "Use the synchronization commands above to ensure TARGET matches SOURCE."