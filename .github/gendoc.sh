#!/bin/bash

# Requires the Markdown-to-PDF utility: https://www.npmjs.com/package/md-to-pdf

cd ..
md2pdf --config-file .github/config.js OPERATION.md
md2pdf --config-file .github/config.js OPERATION_EEPROM.md

echo "Done."
echo ""
