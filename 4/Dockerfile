FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install OpenMPI, SSH server, and build tools
RUN apt-get update && \
    apt-get install -y \
    openmpi-bin \
    openmpi-common \
    libopenmpi-dev \
    openssh-server \
    openssh-client \
    g++ \
    make \
    vim \
    net-tools \
    iputils-ping \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Configure SSH for MPI
RUN mkdir -p /var/run/sshd && \
    echo 'root:mpipass' | chpasswd && \
    sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config && \
    sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config && \
    sed -i 's/#PubkeyAuthentication yes/PubkeyAuthentication yes/' /etc/ssh/sshd_config && \
    sed -i 's/#StrictHostKeyChecking ask/StrictHostKeyChecking no/' /etc/ssh/ssh_config && \
    echo "StrictHostKeyChecking no" >> /etc/ssh/ssh_config && \
    echo "UserKnownHostsFile /dev/null" >> /etc/ssh/ssh_config

# Generate SSH host keys
RUN ssh-keygen -A

# Create SSH directory
RUN mkdir -p /root/.ssh && chmod 700 /root/.ssh

# Set MPI environment variables
ENV OMPI_ALLOW_RUN_AS_ROOT=1
ENV OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1
ENV OMPI_MCA_btl_vader_single_copy_mechanism=none

# Create workspace directory
WORKDIR /workspace

# Expose SSH port
EXPOSE 22

# Default command (can be overridden in docker-compose)
CMD ["/usr/sbin/sshd", "-D"]

