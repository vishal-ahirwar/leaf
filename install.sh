#!/usr/bin/env bash
set -euo pipefail

LEAF_DIR="${HOME}/leaf"
BASE_URL="https://github.com/vishal-ahirwar/leaf/releases/latest/download/"

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "Error: '$1' is required but not installed." >&2
    exit 1
  fi
}

download() {
  local url="$1"
  local out="$2"
  if command -v curl >/dev/null 2>&1; then
    curl -fL "$url" -o "$out"
  elif command -v wget >/dev/null 2>&1; then
    wget -O "$out" "$url"
  else
    echo "Error: curl or wget is required to download files." >&2
    exit 1
  fi
}

add_path_once() {
  local profile_file="$1"
  local export_line='export PATH="$HOME/leaf:$PATH"'
  touch "$profile_file"
  if ! grep -Fq "$export_line" "$profile_file"; then
    printf "\n%s\n" "$export_line" >>"$profile_file"
    echo "Updated PATH in $profile_file"
  else
    echo "PATH already configured in $profile_file"
  fi
}

require_cmd unzip

os_name="$(uname -s)"
declare -a candidate_assets=()
case "$os_name" in
  Linux*)
    candidate_assets=("leaf-linux.zip")
    ;;
  Darwin*)
    candidate_assets=("leaf-macos.zip" "leaf-darwin.zip")
    ;;
  *)
    echo "Unsupported OS: $os_name. This script supports Linux and macOS." >&2
    exit 1
    ;;
esac

echo "Creating directory $LEAF_DIR..."
mkdir -p "$LEAF_DIR"

zip_path="$LEAF_DIR/leaf.zip"
downloaded=0
for asset in "${candidate_assets[@]}"; do
  url="${BASE_URL}/${asset}"
  echo "Trying $url ..."
  if download "$url" "$zip_path"; then
    downloaded=1
    break
  fi
done

if [[ "$downloaded" -ne 1 ]]; then
  echo "Failed to download a compatible Leaf archive for $os_name from release $LEAF_VERSION." >&2
  exit 1
fi

unzip -o "$zip_path" -d "$LEAF_DIR"
rm -f "$zip_path"

echo "Configuring PATH..."
if [[ -n "${BASH_VERSION:-}" ]]; then
  add_path_once "${HOME}/.bashrc"
  echo "Run: source ~/.bashrc"
elif [[ -n "${ZSH_VERSION:-}" ]]; then
  add_path_once "${HOME}/.zshrc"
  echo "Run: source ~/.zshrc"
else
  add_path_once "${HOME}/.profile"
  echo "Run: source ~/.profile"
fi

echo "Installation complete. Verify with: leaf --version"
