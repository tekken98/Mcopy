mcopy:main.cpp
	g++ main.cpp -D FILE_OFFSET_BITS=64 -o mcopy
	rm -rf Nat*
	valgrind ./mcopy /home/bao/Downloads/Nature.Love* ./
