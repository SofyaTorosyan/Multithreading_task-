
#include <mutex>
#include <thread>
#include <chrono>
#include <vector>
#include <fstream>
#include <utility>
#include <iostream>
#include <condition_variable>

using namespace std;

#define SIZE 2000
#define BLOCK_SIZE 10

std::mutex cv_mutex;
std::mutex vec_mutex;
std::condition_variable cv;
bool ready = false;

/* 
    2 treads write simultaneously in the same shared data - vector.
    if size of the vector divides on block_size => third waiting thread is notified to finish waiting and do its work.
*/
void write_to_vec(std::ofstream& File, std::vector<int>& vec)
{
    for(int i = 0; i < SIZE/2; i++)
    {
        /* write into vector */
        {
            std::lock_guard<std::mutex> vec_lock(vec_mutex);
            vec.push_back(i);
        }

        /* check vector size */
        if(vec.size() % BLOCK_SIZE == 0)
        {
            /* notify the waiting thread */
            std::unique_lock<std::mutex> cv_lock(cv_mutex); 
            ready = true;
            cv.notify_one();
        }
    }
}

void write_to_file(std::ofstream& File, const std::vector<int>& vec)
{
    /* Set block begin and end indexes. */
    int begin = 0;
    int end   = 0;
    while(true)
    {
        std::unique_lock<std::mutex> lock(cv_mutex);
        File << "Waiting... \n";
        /* Wait untill will be notified. */
        while (!ready) cv.wait(lock);
        ready = false;
        end = vec.size(); // reset end 
        lock.unlock();

        /* Read this block to the file. */
        File << "vec.size(): " << vec.size() << " beg: " << begin << "  end: " << end << std::endl;
        for(int j = begin; j < end; j++)
        {
            File << vec[j] << "\n";
        }
        begin = end; // reset begin

        /* Finish if vector is full(written #SIZE elements). */
        if(vec.size() == SIZE)  break;
    }
}

int main()
{
    std::ofstream File("output.txt");
    if(!File.is_open())
    {
        std::cout << "Error: Couldn't open file" << std::endl;
        return -1;
    }

    /* This data is shared data between t1 and t2 threads. */
    std::vector<int> vec; 

    /* Keep time for work.*/
    auto start = std::chrono::system_clock::now();

    /* Create 2 writing and 1 reading to file threads. */
    std::thread t3(write_to_file, std::ref(File), std::ref(vec));
    std::thread t1(write_to_vec,  std::ref(File), std::ref(vec));
    std::thread t2(write_to_vec,  std::ref(File), std::ref(vec));

    t3.join();
    t1.join();
    t2.join();

    File.close();

    std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
    std::cout << "Time for run: " << dur.count() << " seconds" << std::endl;

}