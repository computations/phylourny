all:
	@cmake -DCMAKE_BUILD_TYPE=Release -Bbuild -H. -DCMAKE_EXPORT_COMPILE_COMMANDS=YES && cd build && make

debug:
	@cmake -DCMAKE_BUILD_TYPE=Debug -Bbuild -H. -DCMAKE_EXPORT_COMPILE_COMMANDS=YES && cd build && make

clean:
	rm -rf bin build
