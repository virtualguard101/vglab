FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

# pre-install softwares
RUN sed -i \
    -e 's|http://archive.ubuntu.com/|http://mirrors.ustc.edu.cn/|g' \
    -e 's|http://security.ubuntu.com/|http://mirrors.ustc.edu.cn/|g' \
    /etc/apt/sources.list.d/ubuntu.sources
RUN dpkg --add-architecture i386
RUN apt-get -y update && apt-get upgrade -y
RUN apt-get install -y lib32z1 apt-transport-https \
    python3 python3-pip python3-venv python3-poetry python3-dev python3-setuptools \
    libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev libc6-dbg libc6-dbg:i386 libgcc-s1:i386 \
    vim nano netcat-openbsd openssh-server git unzip curl tmux konsole wget sudo \
    bison flex build-essential gcc-multilib \
    qemu-system-x86 qemu-user qemu-user-binfmt \
    gcc gdb gdbserver gdb-multiarch clang lldb make cmake

# enable ssh login
RUN rm -f /etc/service/sshd/down
RUN sed -ri 's/^#?PermitRootLogin\s+.*/PermitRootLogin yes/' /etc/ssh/sshd_config &&\
    sed -ri 's/#UseDNS\ no/UseDNS\ no/g' /etc/ssh/sshd_config && \
    sed -ri "s/StrictModes yes/StrictModes no/g" /etc/ssh/sshd_config && \
    sed -ri "s/UsePAM yes/UsePAM no/g" /etc/ssh/sshd_config

# enable login with password
RUN echo 'PasswordAuthentication yes' >> /etc/ssh/sshd_config

# defaultly we have a user `ubuntu` in the image
RUN echo "ubuntu:123456" | chpasswd && \
    echo "root:123456" | chpasswd

# add sudo
RUN usermod -aG sudo ubuntu

# enable ssh key login
#RUN mkdir /home/ubuntu/.ssh && \
#    echo "Your ssh key" > /home/ubuntu/.ssh/authorized_keys

# keep container running
RUN echo "#!/bin/sh\nservice ssh restart\nsleep infinity" > /root/start.sh
RUN chmod +x /root/start.sh

# create venv for pip
RUN python3 -m venv /pip_venv && \
    chown -R ubuntu:ubuntu /pip_venv && \
    echo "\n\n# pip venv\nsource /pip_venv/bin/activate" >> /home/ubuntu/.bashrc

# pwn-related tools
RUN /pip_venv/bin/pip config set global.index-url http://pypi.tuna.tsinghua.edu.cn/simple && \
    /pip_venv/bin/pip config set global.trusted-host pypi.tuna.tsinghua.edu.cn && \
    /pip_venv/bin/pip install -U pip && \
    /pip_venv/bin/pip install --no-cache-dir \
    pwntools \
    ropgadget \
    z3-solver \
    smmap2 \
    apscheduler \
    ropper \
    unicorn \
    keystone-engine \
    capstone \
    angr \
    pebble \
    r2pipe \
    LibcSearcher \
    poetry

RUN git clone https://github.com/pwndbg/pwndbg && \
    cd pwndbg && chmod +x setup.sh && ./setup.sh

CMD ["/root/start.sh"]

EXPOSE 22
