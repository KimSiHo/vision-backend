#!/bin/bash

HOOK_PATH=".git/hooks/pre-commit"

echo "🔧 Installing pre-commit hook..."
cp tools/pre-commit "$HOOK_PATH"
chmod +x "$HOOK_PATH"

echo "✅ Pre-commit hook installed successfully."
