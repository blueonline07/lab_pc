#!/bin/bash
# Setup SSH for passwordless access between MPI hosts in Docker containers

set -e

echo "Setting up SSH for MPI multi-host communication..."

# Wait for containers to be ready
echo "Waiting for containers to be ready..."
sleep 2

# Generate SSH key in container if it doesn't exist
echo "Generating SSH keys in containers..."
docker compose exec -T mpi-host-1 bash -c "mkdir -p /root/.ssh && chmod 700 /root/.ssh && [ ! -f /root/.ssh/id_rsa ] && ssh-keygen -t rsa -b 4096 -f /root/.ssh/id_rsa -N '' || true"
docker compose exec -T mpi-host-2 bash -c "mkdir -p /root/.ssh && chmod 700 /root/.ssh && [ ! -f /root/.ssh/id_rsa ] && ssh-keygen -t rsa -b 4096 -f /root/.ssh/id_rsa -N '' || true"

# Get public keys from both containers
PUBLIC_KEY_1=$(docker compose exec -T mpi-host-1 bash -c "cat /root/.ssh/id_rsa.pub")
PUBLIC_KEY_2=$(docker compose exec -T mpi-host-2 bash -c "cat /root/.ssh/id_rsa.pub")

# Add both public keys to authorized_keys in both containers
echo "Configuring passwordless SSH access..."
docker compose exec -T mpi-host-1 bash -c "echo '$PUBLIC_KEY_1' > /root/.ssh/authorized_keys && echo '$PUBLIC_KEY_2' >> /root/.ssh/authorized_keys && chmod 600 /root/.ssh/authorized_keys"
docker compose exec -T mpi-host-2 bash -c "echo '$PUBLIC_KEY_1' > /root/.ssh/authorized_keys && echo '$PUBLIC_KEY_2' >> /root/.ssh/authorized_keys && chmod 600 /root/.ssh/authorized_keys"

# Add known_hosts entries
echo "Adding known hosts..."
docker compose exec -T mpi-host-1 bash -c "ssh-keyscan -H mpi-host-1 mpi-host-2 2>/dev/null >> /root/.ssh/known_hosts || true"
docker compose exec -T mpi-host-2 bash -c "ssh-keyscan -H mpi-host-1 mpi-host-2 2>/dev/null >> /root/.ssh/known_hosts || true"

# Test SSH connection
echo "Testing SSH connection..."
docker compose exec -T mpi-host-1 bash -c "ssh -o StrictHostKeyChecking=no root@mpi-host-2 'echo SSH test successful' || echo 'SSH test failed (this is OK if containers are still starting)'"

echo ""
echo "SSH setup complete!"
echo ""
echo "Note: If SSH connection fails, wait a few seconds for containers to fully start, then run:"
echo "  docker compose exec mpi-host-1 ssh root@mpi-host-2 'echo test'"

