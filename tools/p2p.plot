set terminal postscript eps color
set xlabel "Message size (kb)"
set ylabel "Execution time (sec)"

set nokey
set output "p2p.eps"
plot\
	"p2p.out" using ($1/1024):2:5 index 0 with errorlines 0
