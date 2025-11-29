#!/bin/bash

# Script to help set up passwordless SSH for distributed MPI
# This script generates SSH keys and helps distribute them to other nodes

echo "=== Distributed MPI SSH Setup ==="
echo ""

# Check if SSH key already exists
if [ -f ~/.ssh/id_rsa ]; then
    echo "SSH key already exists at ~/.ssh/id_rsa"
    read -p "Do you want to generate a new key? (y/n): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Using existing SSH key."
    else
        ssh-keygen -t rsa -b 4096 -f ~/.ssh/id_rsa -N ""
    fi
else
    echo "Generating new SSH key..."
    ssh-keygen -t rsa -b 4096 -f ~/.ssh/id_rsa -N ""
fi

# Read hosts from hosts.txt
if [ ! -f hosts.txt ]; then
    echo "Error: hosts.txt not found!"
    exit 1
fi

echo ""
echo "Reading hosts from hosts.txt..."
echo ""

# Extract non-comment, non-empty lines from hosts.txt
hosts=$(grep -v '^#' hosts.txt | grep -v '^$' | grep -v '^localhost$')

if [ -z "$hosts" ]; then
    echo "No remote hosts found in hosts.txt (only localhost)."
    echo "For single-node testing, SSH setup is not required."
    exit 0
fi

echo "Found the following hosts:"
echo "$hosts"
echo ""

# Ask for username
read -p "Enter your username for SSH access (default: $(whoami)): " username
username=${username:-$(whoami)}

echo ""
echo "You will need to copy your SSH key to each host."
echo "For each host, run:"
echo "  ssh-copy-id $username@<HOST_IP>"
echo ""
echo "Or use this script to automate (you'll be prompted for passwords):"
echo ""

for host in $hosts; do
    echo "Copying SSH key to $username@$host..."
    ssh-copy-id "$username@$host" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "✓ Successfully copied key to $host"
    else
        echo "✗ Failed to copy key to $host (you may need to do this manually)"
    fi
done

echo ""
echo "=== Setup Complete ==="
echo "Test SSH connection to each host:"
for host in $hosts; do
    echo "  ssh $username@$host 'echo Connection successful'"
done

