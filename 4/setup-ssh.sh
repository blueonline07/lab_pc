#!/bin/bash

# Script to set up passwordless SSH between MPI nodes

echo "Setting up passwordless SSH for distributed MPI..."

# Generate SSH key on node1 if it doesn't exist
docker exec mpi_node1 bash -c "if [ ! -f /root/.ssh/id_rsa ]; then ssh-keygen -t rsa -f /root/.ssh/id_rsa -N ''; fi"

# Get the public key
PUBLIC_KEY=$(docker exec mpi_node1 cat /root/.ssh/id_rsa.pub)

# Copy public key to all nodes (including itself)
for node in mpi_node1 mpi_node2 mpi_node3; do
    echo "Configuring SSH on $node..."
    docker exec $node bash -c "mkdir -p /root/.ssh && chmod 700 /root/.ssh"
    docker exec $node bash -c "echo '$PUBLIC_KEY' >> /root/.ssh/authorized_keys"
    docker exec $node bash -c "chmod 600 /root/.ssh/authorized_keys"
    docker exec $node bash -c "echo '$PUBLIC_KEY' >> /root/.ssh/authorized_keys2"
    docker exec $node bash -c "chmod 600 /root/.ssh/authorized_keys2"
    # Add known_hosts to avoid prompts
    docker exec $node bash -c "ssh-keyscan -H 172.20.0.10 172.20.0.11 172.20.0.12 >> /root/.ssh/known_hosts 2>/dev/null || true"
done

echo "SSH setup complete!"
echo "Testing SSH connection..."
docker exec mpi_node1 ssh -o StrictHostKeyChecking=no root@172.20.0.11 "echo 'SSH from node1 to node2: OK'"
docker exec mpi_node1 ssh -o StrictHostKeyChecking=no root@172.20.0.12 "echo 'SSH from node1 to node3: OK'"

