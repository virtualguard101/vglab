#!/bin/bash

set -euxo pipefail

# Ubuntu 24.04
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2404/x86_64/cuda-ubuntu2404.pin
sudo mv cuda-ubuntu2404.pin /etc/apt/preferences.d/cuda-repository-pin-600
wget https://developer.download.nvidia.com/compute/cuda/13.2.1/local_installers/cuda-repo-ubuntu2404-13-2-local_13.2.1-595.58.03-1_amd64.deb
sudo dpkg -i cuda-repo-ubuntu2404-13-2-local_13.2.1-595.58.03-1_amd64.deb
sudo cp /var/cuda-repo-ubuntu2404-13-2-local/cuda-*-keyring.gpg /usr/share/keyrings/
sudo apt-get update
sudo apt-get -y install cuda-toolkit-13-2

# persist CUDA environment variables to ~/.bashrc
if ! rg -q '^export PATH=/usr/local/cuda/bin:\$PATH$' "$HOME/.bashrc"; then
  echo 'export PATH=/usr/local/cuda/bin:$PATH' >> "$HOME/.bashrc"
fi

if ! rg -q '^export LD_LIBRARY_PATH=/usr/local/cuda/lib64:\$LD_LIBRARY_PATH$' "$HOME/.bashrc"; then
  echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> "$HOME/.bashrc"
fi

source "$HOME/.bashrc"
