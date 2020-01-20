all:
	@cmake -DCMAKE_BUILD_TYPE=Release -Bbuild -H. -DCMAKE_EXPORT_COMPILE_COMMANDS=YES && cd build && make
