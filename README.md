# Task description

2 threads are writting into the same vector, third thread should read vector by blocks into output file.

# Build
g++ -std=c++0x threads.cpp -o exe -lpthread

# Run
./exe
