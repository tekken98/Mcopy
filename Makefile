mcp:main.cpp
	g++ main.cpp -D FILE_OFFSET_BITS=64 -o mcp
	rm -rf Nat*
	#valgrind -s ./mcopy /home/bao/Downloads/Nature.Love* ./
