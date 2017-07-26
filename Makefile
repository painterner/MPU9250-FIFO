all:
	g++ *.cpp -o main -lwiringPi
clean:
	rm -i main
