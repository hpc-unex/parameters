mpirun -n 2 p2p -m 0 -M 48000 -s 1024 -T 2048 -L 0 > O0_format.txt 
sed '/#/d' O0_format.txt | cut -f2 > O0.txt

mpirun -n 2 p2p -m 0 -M 48000 -s 1024 -T 2048 -L 1 > O1_format.txt 
sed '/#/d' O1_format.txt | cut -f2 > O1.txt

for x in 1 2 3 4 5 6 7 8
do
	mpirun -n $x -m 0 -M 48000 -s 1024 -T 2048 -L 2 > L0_format.txt 
	sed '/#/d' L0_format.txt | cut -f1-2 > L0.txt

done
