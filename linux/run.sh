qemu-system-x86_64 \
        -m 16G \
        -smp 10 \
        -kernel $1/arch/x86/boot/bzImage \
        -append "console=ttyS0 root=/dev/sda earlyprintk=serial net.ifnames=0 nokaslr" \
        -drive file=$2/bullseye.img,format=raw \
        -net user,host=10.0.2.10,hostfwd=tcp:127.0.0.1:10021-:22 \
        -net nic,model=e1000 \
        -enable-kvm \
        -nographic \
        -pidfile vm.pid \
        2>&1 | tee vm.log

        # -object memory-backend-ram,size=8G,id=m0 \
        # -object memory-backend-ram,size=8G,id=m1 \
        # -numa node,nodeid=0,memdev=m0,cpus=0-9 \
        # -numa node,nodeid=1,memdev=m1 \