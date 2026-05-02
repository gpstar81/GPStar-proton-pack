import os
import gzip
import shutil
from pathlib import Path

# Import PlatformIO environment only if running within build context
try:
    Import("env")
    print("Running within PlatformIO build context")
except NameError:
    print("Running standalone for testing")

def combine_css_files():
    """
    Function: combine_css_files
    Purpose: Combine shared CSS with device-specific styles into a single CSS file.
             Order: Shared libraries first, then device-specific customizations last.
    Inputs: SharedLib/WebAssets/*.css (shared), assets/style.css (device-specific)
    Outputs: combined.css (temporary combined file), style.css.gz (final compressed file)
    """
    print("=== Starting CSS combination ===")
    assets_dir = Path("assets")
    shared_dir = Path("../SharedLib/WebAssets")

    print(f"Current working directory: {Path.cwd()}")
    print(f"Assets directory: {assets_dir.absolute()} (exists: {assets_dir.exists()})")
    print(f"Shared directory: {shared_dir.absolute()} (exists: {shared_dir.exists()})")

    # Early return if directories don't exist
    if not assets_dir.exists() or not shared_dir.exists():
        print("Required directories not found, skipping CSS combination")
        return

    # Original device-specific styles (keep as source, don't overwrite)
    device_css = assets_dir / "style.css"
    combined_css = assets_dir / "combined.css"
    final_gz = assets_dir / "style.css.gz"

    # Define the combination order
    css_sources = [
        shared_dir / "base.css",
        shared_dir / "controls.css",
        shared_dir / "animations.css",
        device_css  # Device-specific customizations last
    ]

    # Check which source files exist
    existing_sources = [f for f in css_sources if f.exists()]
    print(f"Found {len(existing_sources)} existing CSS files")
    if not existing_sources:
        print("No CSS source files found, skipping combination")
        return

    # Check if combination is needed - rebuild if:
    # 1. Final .gz doesn't exist, OR
    # 2. Any source file is newer than final .gz
    needs_update = (
        not final_gz.exists() or
        any(f.stat().st_mtime > final_gz.stat().st_mtime for f in existing_sources)
    )

    if needs_update:
        print(f"Combining CSS files into {combined_css.absolute()}")

        try:
            with open(combined_css, 'w', encoding='utf-8') as output:
                output.write("/* Combined CSS - Generated automatically */\n")
                output.write("/* DO NOT EDIT - Modify source files instead */\n\n")

                for css_file in existing_sources:
                    print(f"  Adding {css_file.name}...")
                    output.write(f"/* === {css_file.name} === */\n")
                    try:
                        with open(css_file, 'r', encoding='utf-8') as input_file:
                            content = input_file.read().strip()
                            if content:
                                output.write(content)
                                output.write("\n\n")
                            else:
                                print(f"    File was empty")
                    except Exception as e:
                        print(f"    Warning: Could not read {css_file}: {e}")

            print(f"Successfully created combined CSS file: {combined_css.absolute()}")
            print(f"File size: {combined_css.stat().st_size} bytes")

        except Exception as e:
            print(f"ERROR: Failed to create combined CSS file: {e}")

    else:
        print("CSS files are up to date, skipping combination")

def combine_js_files():
    """
    Function: combine_js_files
    Purpose: Combine device-specific JavaScript with shared libraries into a single JS file.
             Order: Device-specific first, then shared libraries appended.
    Inputs: assets/common.js (device-specific), SharedLib/WebAssets/*.js (shared)
    Outputs: combined.js (temporary combined file), common.js.gz (final compressed file)
    """
    print("=== Starting JavaScript combination ===")
    assets_dir = Path("assets")
    shared_dir = Path("../SharedLib/WebAssets")

    print(f"Current working directory: {Path.cwd()}")
    print(f"Assets directory: {assets_dir.absolute()} (exists: {assets_dir.exists()})")
    print(f"Shared directory: {shared_dir.absolute()} (exists: {shared_dir.exists()})")

    # Early return if directories don't exist
    if not assets_dir.exists() or not shared_dir.exists():
        print("Required directories not found, skipping JavaScript combination")
        return

    # Original device-specific scripts (keep as source, don't overwrite)
    device_js = assets_dir / "common.js"
    combined_js = assets_dir / "combined.js"
    final_gz = assets_dir / "common.js.gz"

    # Define the combination order - device-specific first, shared libraries appended
    js_sources = [
        device_js,  # Device-specific functions first
        shared_dir / "api.js",
        shared_dir / "dom.js",
        shared_dir / "help.js",
        shared_dir / "utils.js"
    ]

    # Check which source files exist
    existing_sources = [f for f in js_sources if f.exists()]
    print(f"Found {len(existing_sources)} existing JavaScript files")
    if not existing_sources:
        print("No JavaScript source files found, skipping combination")
        return

    # Check if combination is needed - rebuild if:
    # 1. Final .gz doesn't exist, OR
    # 2. Any source file is newer than final .gz
    needs_update = (
        not final_gz.exists() or
        any(f.stat().st_mtime > final_gz.stat().st_mtime for f in existing_sources)
    )

    if needs_update:
        print(f"Combining JavaScript files into {combined_js.absolute()}")

        try:
            with open(combined_js, 'w', encoding='utf-8') as output:
                output.write("/* Combined JavaScript - Generated automatically */\n")
                output.write("/* DO NOT EDIT - Modify source files instead */\n\n")

                for js_file in existing_sources:
                    print(f"  Adding {js_file.name}...")
                    output.write(f"/* === {js_file.name} === */\n")
                    try:
                        with open(js_file, 'r', encoding='utf-8') as input_file:
                            content = input_file.read().strip()
                            if content:
                                output.write(content)
                                output.write("\n\n")
                            else:
                                print(f"    File was empty")
                    except Exception as e:
                        print(f"    Warning: Could not read {js_file}: {e}")

            print(f"Successfully created combined JavaScript file: {combined_js.absolute()}")
            print(f"File size: {combined_js.stat().st_size} bytes")

        except Exception as e:
            print(f"ERROR: Failed to create combined JavaScript file: {e}")

    else:
        print("JavaScript files are up to date, skipping combination")

def copy_shared_help():
    """
    Function: copy_shared_help
    Purpose: Copy and compress help.json from SharedLib/WebAssets if newer than local compressed version.
    Inputs: uncompressed help.json in SharedLib/WebAssets/
    Outputs: creates help.json.gz in assets/
    """
    assets_dir = Path("assets")
    shared_dir = Path("../SharedLib/WebAssets")
    
    if not assets_dir.exists() or not shared_dir.exists():
        return
    
    source_file = shared_dir / "help.json"
    compressed_file = assets_dir / "help.json.gz"
    
    if not source_file.exists():
        return
    
    # Compress if source is newer than compressed or compressed doesn't exist
    if not compressed_file.exists() or source_file.stat().st_mtime > compressed_file.stat().st_mtime:
        print(f"Compressing shared help.json -> {compressed_file}")
        with open(source_file, 'rb') as f_input:
            with gzip.open(compressed_file, 'wb') as f_output:
                shutil.copyfileobj(f_input, f_output)

def compress_assets():
    """
    Function: compress_assets
    Purpose: Compress all web assets in the assets directory using gzip compression.
             Only compresses files if source is newer than existing compressed version.
    Inputs: uncompressed files in assets/
    Outputs: creates .gz files)
    """
    assets_dir = Path("assets")

    # Early return if assets directory doesn't exist
    if not assets_dir.exists():
        print("Assets directory not found, skipping compression")
        return

    # Special handling for combined CSS file - compress as style.css.gz
    combined_css = assets_dir / "combined.css"
    style_css_gz = assets_dir / "style.css.gz"

    if combined_css.exists():
        # Always compress to ensure fresh builds
        print(f"Compressing: {combined_css} -> {style_css_gz}")
        with open(combined_css, 'rb') as f_input:
            with gzip.open(style_css_gz, 'wb') as f_output:
                shutil.copyfileobj(f_input, f_output)

        # Clean up intermediate file after successful compression
        combined_css.unlink()

    # Special handling for combined JavaScript file - compress as common.js.gz
    combined_js = assets_dir / "combined.js"
    common_js_gz = assets_dir / "common.js.gz"

    if combined_js.exists():
        # Always compress to ensure fresh builds
        print(f"Compressing: {combined_js} -> {common_js_gz}")
        with open(combined_js, 'rb') as f_input:
            with gzip.open(common_js_gz, 'wb') as f_output:
                shutil.copyfileobj(f_input, f_output)

        # Clean up intermediate file after successful compression
        combined_js.unlink()

    # Define file extensions that should be compressed for web serving
    # Note: .css and .js excluded since we handle combined files specially above
    compress_extensions = ['.html', '.svg', '.ico']

    # Process each file in the assets directory
    for file_path in assets_dir.iterdir():
        # Skip directories and files with unsupported extensions
        if not file_path.is_file():
            continue
        if file_path.suffix.lower() not in compress_extensions:
            continue

        # Generate the compressed file path
        compressed_path = file_path.with_suffix(file_path.suffix + '.gz')

        # Check if compression is needed (source newer than compressed or compressed doesn't exist)
        b_needs_compression = (
            not compressed_path.exists() or
            file_path.stat().st_mtime > compressed_path.stat().st_mtime
        )

        if b_needs_compression:
            print(f"Compressing: {file_path} -> {compressed_path}")

            # Perform the compression operation
            with open(file_path, 'rb') as f_input:
                with gzip.open(compressed_path, 'wb') as f_output:
                    shutil.copyfileobj(f_input, f_output)

# Execute combination first, then shared files, then compression
combine_css_files()
combine_js_files()
copy_shared_help()
compress_assets()