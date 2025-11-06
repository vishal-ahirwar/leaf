#!/bin/bash

LEAF_DIR="$HOME/leaf"
DOWNLOAD_URL_LEAF="https://example.com/leaf"
DOWNLOAD_URL_UPDATER="https://example.com/updater"

echo "Creating directory $LEAF_DIR..."
mkdir -p "$LEAF_DIR"

echo "Downloading leaf..."
curl -L "$DOWNLOAD_URL_LEAF" -o "$LEAF_DIR/leaf"
chmod +x "$LEAF_DIR/leaf"

echo "Downloading updater..."
curl -L "$DOWNLOAD_URL_UPDATER" -o "$LEAF_DIR/updater"
chmod +x "$LEAF_DIR/updater"

echo "Adding $LEAF_DIR to PATH..."

if [[ -n "$BASH_VERSION" ]]; then
    echo 'export PATH="$HOME/leaf:$PATH"' >> ~/.bashrc
    echo "Please restart your terminal or run 'source ~/.bashrc' for the changes to take effect."
elif [[ -n "$ZSH_VERSION" ]]; then
    echo 'export PATH="$HOME/leaf:$PATH"' >> ~/.zshrc
    echo "Please restart your terminal or run 'source ~/.zshrc' for the changes to take effect."
else
    echo 'export PATH="$HOME/leaf:$PATH"' >> ~/.profile
    echo "Please restart your terminal or run 'source ~/.profile' for the changes to take effect."
fi

echo "Installation complete."
