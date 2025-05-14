# Use Ubuntu 22.04 as the base image
FROM ubuntu:22.04

# Set working directory
WORKDIR /backend_ppfv

# Copy directory 
COPY backend_ppfv /backend_ppfv
RUN chmod +x /backend_ppfv/*.sh

COPY requirements.txt /tmp/requirements.txt

# Set non-interactive mode to avoid prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update package list and install necessary dependencies
RUN apt update && apt install -y \
    software-properties-common \
    wget \
    curl \
    git \
    build-essential \
    cmake \
    ninja-build \
    python3 \
    python3-pip \
    inotify-tools \
    vim \
    pkg-config \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install --no-cache-dir -r /tmp/requirements.txt

# Install GCC and G++ (version 11.4.0 or newer)
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    apt update && \
    apt install -y gcc-11 g++-11 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100 && \
    gcc --version && g++ --version

# Install OpenSSL (version 3.2.1 or newer)
RUN apt install -y libssl-dev \
     openssl && \
     openssl version

# Install HEXL (version 1.2.5 or newer)
RUN cd /backend_ppfv && \
    git clone --branch v1.2.5 https://github.com/intel/hexl.git && \
    cd hexl && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make -j$(nproc) && make install

# Download nlohmann/json header
RUN mkdir -p /backend_ppfv/nlohmann && \
    curl -o /backend_ppfv/nlohmann/json.hpp https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp

# build the C++ source code 
RUN cmake . && make

# Default command (change if needed)
#CMD ["tail", "-f", "/dev/null"]
ENTRYPOINT ["/backend_ppfv/entrypoint.sh"]

