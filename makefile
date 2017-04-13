make:
	rm -f ass3.tar.gz
	cd Assign3/
	make clean
	cd ../
	tar -cvf ass3.tar.gz Assign3/
