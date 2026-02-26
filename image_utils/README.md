./pack \
    --read-only --floppy0-image dos311.img \
    --hard0-256c \
    | sudo dd of=/dev/sdd

./pack \
    --read-only --floppy0-image floppy0.img \
    --read-only --floppy1-image win30/DISK01.IMG \
    --read-only --floppy1-image win30/DISK02.IMG \
    --read-only --floppy1-image win30/DISK03.IMG \
    --read-only --floppy1-image win30/DISK04.IMG \
    --read-only --floppy1-image win30/DISK05.IMG \
    --hard0-image hard0.img \
    | sudo dd of=/dev/sdd

sudo dd if=/dev/sdd | ./unpack

sudo dd if=/dev/sdd | ./unpack --header-only

sudo mount -o offset=512,rw,user,uid=1000,gid=1000,umask=007 /dev/sdd /media/floppy0/

sudo umount /media/floppy0

sudo mount -o offset=1507328,rw,user,uid=1000,gid=1000,umask=007 /dev/sdd /media/hard0/

sudo umount /media/hard0

sudo mount -o loop,offset=32256,rw,user,uid=1000,gid=1000,umask=007 hard0.img /media/hard0/

