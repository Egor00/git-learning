#!/usr/bin/bash
for arg in "$@"
do
	case "$arg" in
		-h|--help)
		echo "Usage:./l1.sh [OPTION]... [SUFFIX(ES)]..." #USAGE
		echo "Creates synonims of every file with given suffix." #About
		echo "Creates synonims of every file with given suffix and number of links greater" 
		echo "then 1 by putting suffixes in files' names upfront and removing dot."
		echo
		echo "Mandatory arguments to long options are mandatory for short options too."
		echo "   -h, --help		display this help and exit."
		exit 0
		;;
		*)
		suffs+=( "${arg#.}" )
        	;;
	esac
done
file_links=()
for f in *
do
	n=( $(ls -i "$f") )
	n_2="${n[0]}"
	file_links+=("$n_2")
done
duplicates=($(printf '%s\n' "${file_links[@]}" | sort | uniq -d))
for f in *
do
	ch_s=0
	for s in "${suffs[@]}"
	do
		if [ "$s" = "${f##*.}" ]
		then
			ch_s=1
		fi
	done
	if [[ ("$ch_s" = "0") || (! -f "$f") ]]
	then
		continue
	fi
	n=( $(ls -i "$f") )
	for l in "${duplicates[@]}"
	do
		if [[ "${n[0]}" = "$l" && "$f" == *"."* ]]
		then
			base="${f%.*}"
			suff="${f##*.}"
			suff="${suff::${#suff}}"
			new_name="${suff}${base}"
			cp "$f" "$new_name"
		fi
	done
done
