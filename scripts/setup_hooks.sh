#!/bin/bash
#
# Instructions: Run this script once after cloning the repository

# Make hooks executable
chmod +x .githooks/*

# Configure Git to use our hooks directory
git config core.hooksPath .githooks

echo "Git hooks configured successfully!"
