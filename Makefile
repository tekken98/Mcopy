mcp:main.cpp
	cp -u ~/include/str.h ./
	cp -u ~/include/opt.h ./
	cp -u ~/bin/man2html ./
	g++ main.cpp -D FILE_OFFSET_BITS=64 -o mcp
	rm -rf Nat*
	#valgrind -s ./mcopy /home/bao/Downloads/Nature.Love* ./
