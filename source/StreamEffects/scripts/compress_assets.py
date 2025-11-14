import os
import gzip
import shutil
from pathlib import Path

Import("env")

def compress_assets():
    """
    Function: compress_assets
    Purpose: Compress all web assets in the assets directory using gzip compression.
             Only compresses files if source is newer than existing compressed version.
    Inputs: None
    Outputs: None (side effect: creates .gz files)
    """
    assets_dir = Path("assets")
    
    # Early return if assets directory doesn't exist
    if not assets_dir.exists():
        print("Assets directory not found, skipping compression")
        return
    
    # Define file extensions that should be compressed for web serving
    compress_extensions = ['.html', '.css', '.js', '.svg', '.ico']
    
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

# Execute compression before build
compress_assets()