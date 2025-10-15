#!/bin/bash
# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>

# Install benchmarking tools

echo "Installing benchmarking tools..."
echo ""

# Install hyperfine if not already installed
if ! command -v hyperfine &> /dev/null; then
    echo "Installing hyperfine..."
    cargo install hyperfine || {
        echo "Note: cargo not found. Install hyperfine manually:"
        echo "  Debian/Ubuntu: sudo apt install hyperfine"
        echo "  Fedora: sudo dnf install hyperfine"
        exit 1
    }
else
    echo "✓ hyperfine already installed"
fi

# Check for GNU time
if ! command -v /usr/bin/time &> /dev/null; then
    echo "Warning: GNU time not found at /usr/bin/time"
    echo "Install with:"
    echo "  Debian/Ubuntu: sudo apt install time"
    echo "  Fedora: sudo dnf install time"
else
    echo "✓ GNU time already installed"
fi

# Install npm statusline packages locally
echo ""
echo "Installing npm statusline packages locally..."

if command -v npm &> /dev/null; then
    cd "$(dirname "$0")"
    npm install @owloops/claude-powerline@latest ccstatusline@latest claude-statusline-powerline@latest
    echo "✓ npm packages installed to benchmark/node_modules"
else
    echo "Warning: npm not found. npm-based statuslines will not be installed."
fi

echo ""
echo "Installation complete!"
exit 0
