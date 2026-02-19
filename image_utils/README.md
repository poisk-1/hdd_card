sudo mount -o loop,offset=32256,rw,user,uid=1000,gid=1000,umask=007 hard0.img /media/hard0/

sudo umount /media/hard0

./pack --read-only ~/Projects/poisk/Disk1.img --hard-256c | sudo dd of=/dev/sdd

sudo dd if=/dev/sdd | ./unpack

./pack --read-only floppy0.img hard0.img | sudo dd of=/dev/sdd

