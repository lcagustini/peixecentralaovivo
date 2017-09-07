all:
	g++ src/main.cpp -o main -lGLEW -lGL -lglfw -lassimp -Wno-write-strings

run: all
	./main
