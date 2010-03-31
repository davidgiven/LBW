#!/bin/sh

LBWHOME="$PWD"
bb="$LBWHOME/busybox"
export LBWHOME

cd "$HOME"
root="Application Data/Cowlark Technologies/LBW"
"$bb" mkdir -p "$root"
cd "$root"

"$bb" mkdir -p bin etc dev proc tmp doc
if "$bb" test -h home -o ! -r home; then
	"$bb" rm -f home
	"$bb" ln -s "$HOME" home
fi

"$bb" chmod a+rx "$bb"
if ! "$bb" test -x bin/busybox; then
	echo "Initialising BusyBox shell for the first time. This will take"
	echo "about 15 seconds."
	
	# Populate /bin with links to BusyBox.
	"$bb" ln -s "$bb" bin/busybox
	"$bb" --help | "$bb" grep 'Currently defined functions:' -A999 | "$bb" grep '^\s.*,' | "$bb" tr , '\n' | (cd bin && "$bb" xargs -n1 "$bb" ln -s busybox)
	
	# Create some useful programs.
	echo "#!/bin/sh" > bin/fakeroot
	echo 'LBW_FAKEROOT=1 exec "$@"' >> bin/fakeroot
	"$bb" chmod a+rx bin/fakeroot
	
	# Populate /doc with useful stuff.
	"$bb" ln -s "$LBWHOME/BusyBox.txt" doc
	"$bb" ln -s "$LBWHOME/LICENSE" doc
	"$bb" ln -s "$LBWHOME/README" doc
	"$bb" ln -s "$LBWHOME/interix.termcap" etc
	
	# Create profile, if not already done.
	if ! "$bb" test -f etc/profile; then
		echo 'uname -a' > etc/profile
		echo 'alias ls="ls -CF --color"' >> etc/profile
	fi
	echo "...done."
fi

PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin
export PATH

TERM=interix
export TERM

cd /
exec "$bb" chroot "$HOME/$root" /bin/sh --login
"$bb" ash
