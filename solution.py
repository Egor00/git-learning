import os
import sys
import shutil

suffs = []
for arg in sys.argv[1:]:
	if (arg == "-h") or (arg == "--help"):
		print("Usage:python3 l1.py [OPTION]... [SUFFIX(ES)]...") #USAGE
		print("Creates synonims of every file with given suffix.") #About
		print("Creates synonims of every file with given suffix and number of links greater")
		print("then 1 by putting suffixes in files' names upfront and removing dot.")
		print("")
		print("Mandatory arguments to long options are mandatory for short options too.")
		print("   -h, --help		display this help and exit.")
		sys.exit(0)
	else:
		suffs.append(arg.replace('.', ''))
files = [f for f in os.listdir() if os.path.isfile(f)]
inodes = []
dup_inodes = []
inf = os.scandir()
for f in inf:
	if f.is_file(follow_symlinks=False):
		inodes.append(os.DirEntry.inode(f))
for i in inodes:
	ch = 0
	for j in inodes:
		if i == j and ch < 2:
			ch += 1
		if ch == 2:
			dup_inodes.append(i)
			break

for f in files:
		ch = False
		if ch:
			continue
		base, suff = os.path.splitext(f)
		if not "." in suff:
			continue
		suff = suff[1:]
		if not (suff in suffs):
			continue
		stat_inf = os.stat(f)
		inode = stat_inf.st_ino
		for i in dup_inodes:
			if (inode == i) and not(ch):
				new_name = suff + base
				shutil.copy2(f"{f}", f"{new_name}")
				break
		if ch:
			ch = True

