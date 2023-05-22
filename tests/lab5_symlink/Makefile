targets  := rootfs symlink_check.x

.PHONY: rootfs

include ../include.mk

rootfs:
	rm -rf rootfs
	mkdir rootfs rootfs/lib rootfs/my rootfs/my/my
	echo 'Hello, world!' > rootfs/real
	ln -s /my/b rootfs/lib/a
	ln -s /real rootfs/my/my/c
	ln -s /my/my/c rootfs/my/b
