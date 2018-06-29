set terminal postscript eps color
set xlabel "Message size (kb)"
set ylabel "Execution time (sec)"

set nokey
set output "collective.eps"
plot\
	"collective.out" using ($1/1024):2:5 with errorlines 0
