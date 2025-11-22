#!/bin/bash
# Convenience script to build and run the MPI simulation in Docker

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║       Lab 4: Docker Multi-Host MPI Simulation Runner          ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Parse arguments
NUM_PROCESSES=12
SKIP_CSV=""
ACTION="run"

while [[ $# -gt 0 ]]; do
    case $1 in
        -np|--num-processes)
            NUM_PROCESSES="$2"
            shift 2
            ;;
        --no-output|--skip-csv)
            SKIP_CSV="--no-output"
            shift
            ;;
        --setup)
            ACTION="setup"
            shift
            ;;
        --clean)
            ACTION="clean"
            shift
            ;;
        --shell)
            ACTION="shell"
            shift
            ;;
        --status)
            ACTION="status"
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  -np, --num-processes N    Number of MPI processes (default: 12)"
            echo "  --no-output, --skip-csv   Skip CSV output generation"
            echo "  --setup                   Setup containers and SSH only"
            echo "  --clean                   Stop and remove all containers"
            echo "  --shell                   Open shell in mpi-head container"
            echo "  --status                  Show cluster status"
            echo "  --help, -h                Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                                    # Run with defaults (12 processes)"
            echo "  $0 -np 8                              # Run with 8 processes"
            echo "  $0 --no-output                        # Run without CSV output"
            echo "  $0 --setup                            # Setup cluster only"
            echo "  $0 --shell                            # Open interactive shell"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Function to check if containers are running
check_containers() {
    if ! docker compose ps | grep -q "Up"; then
        return 1
    fi
    return 0
}

# Function to setup cluster
setup_cluster() {
    echo -e "${YELLOW}📦 Starting Docker containers...${NC}"
    docker compose up -d --build
    
    echo ""
    echo -e "${YELLOW}⏳ Waiting for containers to be ready...${NC}"
    sleep 5
    
    echo ""
    echo -e "${YELLOW}🔑 Setting up SSH...${NC}"
    chmod +x setup-ssh.sh
    ./setup-ssh.sh
    
    echo ""
    echo -e "${GREEN}✅ Cluster setup complete!${NC}"
}

# Execute action
case $ACTION in
    setup)
        setup_cluster
        ;;
    
    clean)
        echo -e "${YELLOW}🧹 Cleaning up Docker containers...${NC}"
        docker compose down
        echo -e "${GREEN}✅ Cleanup complete!${NC}"
        ;;
    
    shell)
        if ! check_containers; then
            echo -e "${RED}❌ Containers are not running. Starting them...${NC}"
            setup_cluster
        fi
        echo -e "${BLUE}🐚 Opening shell in mpi-head...${NC}"
        echo ""
        docker compose exec mpi-head bash
        ;;
    
    status)
        echo -e "${BLUE}📊 Cluster Status:${NC}"
        echo ""
        docker compose ps
        echo ""
        echo -e "${BLUE}💻 Resource Usage:${NC}"
        echo ""
        docker stats --no-stream mpi-head mpi-node-1 mpi-node-2 mpi-node-3 2>/dev/null || echo "Containers not running"
        ;;
    
    run)
        # Check if containers are running
        if ! check_containers; then
            echo -e "${YELLOW}⚠️  Containers not running. Starting cluster...${NC}"
            echo ""
            setup_cluster
        else
            echo -e "${GREEN}✅ Containers are already running${NC}"
        fi
        
        echo ""
        echo -e "${YELLOW}🔨 Building the simulation...${NC}"
        if ! docker compose exec -T mpi-head bash -c "cd /workspace && make clean && make"; then
            echo -e "${RED}❌ Build failed!${NC}"
            exit 1
        fi
        
        echo ""
        echo -e "${GREEN}✅ Build successful!${NC}"
        echo ""
        echo -e "${BLUE}╔════════════════════════════════════════════════════════════════╗${NC}"
        echo -e "${BLUE}║              Running MPI Simulation                            ║${NC}"
        echo -e "${BLUE}║  Processes: $NUM_PROCESSES across 4 Docker containers                    ║${NC}"
        echo -e "${BLUE}╚════════════════════════════════════════════════════════════════╝${NC}"
        echo ""
        
        # Run the simulation
        docker compose exec mpi-head bash -c "
            mpirun -np $NUM_PROCESSES \
                   --hostfile hosts-docker.txt \
                   --display-map \
                   ./build/mpi_shock_simulation $SKIP_CSV
        "
        
        EXIT_CODE=$?
        echo ""
        if [ $EXIT_CODE -eq 0 ]; then
            echo -e "${GREEN}✅ Simulation completed successfully!${NC}"
            if [ -z "$SKIP_CSV" ]; then
                echo ""
                echo -e "${BLUE}📄 Output files generated:${NC}"
                docker compose exec -T mpi-head bash -c "ls -lh /workspace/*.csv 2>/dev/null | tail -2" || true
            fi
        else
            echo -e "${RED}❌ Simulation failed with exit code $EXIT_CODE${NC}"
            exit $EXIT_CODE
        fi
        ;;
esac

