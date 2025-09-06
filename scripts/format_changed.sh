#!/bin/bash
# === Run clang-format only on changed C/C++ files ===

files=$(git diff --name-only --diff-filter=ACM | grep -E '\.(cpp|cc|c|hpp|h)$')

if [ -z "$files" ]; then
  echo "✅ No changed C/C++ files to format."
  exit 0
fi

echo "🎨 Running clang-format on changed files..."

for file in $files; do
  if [ -f "$file" ]; then
    clang-format -i "$file"
    echo "  ✅ formatted: $file"
  fi
done

echo "✨ Done. All modified C/C++ files formatted."
