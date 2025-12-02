FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    openmpi-bin \
    openmpi-common \
    libopenmpi-dev \
    openssh-server \
    openssh-client \
    vim \
    && rm -rf /var/lib/apt/lists/*

# Configure SSH
RUN mkdir /var/run/sshd && \
    echo 'root:root' | chpasswd && \
    sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config && \
    sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config && \
    sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

# Generate SSH host keys
RUN ssh-keygen -A

# Create working directory
WORKDIR /app

# Copy source files
COPY src/ /app/src/
COPY Makefile /app/

# Build programs from all folders
# Folder 1: async and sync
# Folder 2: async and sync
# Folder 3: async and sync
RUN cd /app && \
    rm -f src/1/*.o src/2/*.o src/3/*.o && \
    make clean && \
    make SRCDIR=src/1 TARGET=async && \
    mv async async_1 && \
    make clean && \
    make SRCDIR=src/1 TARGET=sync && \
    mv sync sync_1 && \
    make clean && \
    make SRCDIR=src/2 TARGET=async && \
    mv async async_2 && \
    make clean && \
    make SRCDIR=src/2 TARGET=sync && \
    mv sync sync_2 && \
    make clean && \
    make SRCDIR=src/3 TARGET=async && \
    mv async async_3 && \
    make clean && \
    make SRCDIR=src/3 TARGET=sync && \
    mv sync sync_3

# Expose SSH port
EXPOSE 22

# Start SSH service
CMD ["/usr/sbin/sshd", "-D"]

