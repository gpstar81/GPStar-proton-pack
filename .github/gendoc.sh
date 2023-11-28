#!/bin/bash

# Requires the Markdown-to-PDF utility: https://www.npmjs.com/package/md-to-pdf

cd ..
md2pdf OPERATION.md
md2pdf OPERATION_EEPROM.md

echo "Done."
echo ""
