#!/usr/bin/env bash
# Setup SSH for passwordless access between MPI hosts in Docker containers

set -e

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║  Setting up SSH for MPI Multi-Host Communication              ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# Define all hosts
HOSTS="mpi-head mpi-node-1 mpi-node-2 mpi-node-3"

# Wait for containers to be ready
echo "⏳ Waiting for containers to be ready..."
sleep 3

# Generate SSH keys in all containers
echo ""
echo "🔑 Generating SSH keys in all containers..."
for host in $HOSTS; do
    echo "  - $host"
    docker compose exec -T $host bash -c "
        mkdir -p /root/.ssh && 
        chmod 700 /root/.ssh && 
        [ ! -f /root/.ssh/id_rsa ] && ssh-keygen -t rsa -b 2048 -f /root/.ssh/id_rsa -N '' || true
    " 2>/dev/null
done

# Collect all public keys
echo ""
echo "📋 Collecting public keys..."
ALL_KEYS=$(for host in $HOSTS; do
    docker compose exec -T $host bash -c "cat /root/.ssh/id_rsa.pub" 2>/dev/null
done)
echo "  ✓ Collected all keys"

# Distribute keys to all containers
echo ""
echo "🔐 Configuring passwordless SSH access..."
for host in $HOSTS; do
    echo "  - Configuring $host"
    echo "$ALL_KEYS" | docker compose exec -T $host bash -c "
        cat > /root/.ssh/authorized_keys &&
        chmod 600 /root/.ssh/authorized_keys
    " 2>/dev/null
done

# Add known hosts entries
echo ""
echo "🌐 Adding known hosts..."
for host in $HOSTS; do
    echo "  - Configuring $host"
    docker compose exec -T $host bash -c "
        ssh-keyscan -H $HOSTS 2>/dev/null >> /root/.ssh/known_hosts &&
        chmod 600 /root/.ssh/known_hosts
    " 2>/dev/null || true
done

# Test SSH connections
echo ""
echo "🧪 Testing SSH connections..."
echo "  Testing from mpi-head to all nodes..."

success_count=0
for host in mpi-node-1 mpi-node-2 mpi-node-3; do
    if docker compose exec -T mpi-head bash -c "ssh -o StrictHostKeyChecking=no -o ConnectTimeout=3 root@$host 'echo OK' 2>/dev/null" > /dev/null 2>&1; then
        echo "    ✓ mpi-head → $host: SUCCESS"
        success_count=$((success_count + 1))
    else
        echo "    ✗ mpi-head → $host: FAILED (retrying...)"
        sleep 1
        if docker compose exec -T mpi-head bash -c "ssh -o StrictHostKeyChecking=no -o ConnectTimeout=5 root@$host 'echo OK' 2>/dev/null" > /dev/null 2>&1; then
            echo "    ✓ mpi-head → $host: SUCCESS (retry)"
            success_count=$((success_count + 1))
        else
            echo "    ✗ mpi-head → $host: FAILED"
        fi
    fi
done

echo ""
if [ $success_count -eq 3 ]; then
    echo "✅ SSH setup complete! All connections working."
else
    echo "⚠️  SSH setup complete but some connections failed ($success_count/3 successful)."
    echo "   Try waiting a few more seconds and test manually:"
    echo "   docker compose exec mpi-head ssh mpi-node-1 'hostname'"
fi

echo ""
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║  MPI Cluster Ready!                                            ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""
echo "Next steps:"
echo "  1. Build the program:"
echo "     docker compose exec mpi-head make"
echo ""
echo "  2. Run the MPI simulation:"
echo "     docker compose exec mpi-head mpirun -np 12 --hostfile hosts-docker.txt ./build/mpi_shock_simulation"
echo ""

