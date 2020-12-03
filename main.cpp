extern "C"
{
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
}

#include <iostream>
#include <thread>
#include <vector>
#include <list>
#include <string>

using namespace std;

void broken_pipe_callback(int) {}

int main(int argc, char *argv[])
{
    signal(SIGPIPE, broken_pipe_callback); //忽略管道关闭信号

    auto input = argv[1];
    list<string> outputs;

    char arg = getopt(argc, argv, "o:");
    if (arg == -1)
    {
        cout << "Usage: " << argv[0] << " <input file> -o [output file] ..."
             << "Examples:" << argv[0] << " faded.264 -o 1.264 2.264 3.264"
             << endl;
        return 0;
    }

    for (auto begin = optind - 1, end = argc; begin < end; begin++)
        outputs.push_back(argv[begin]);

    mkfifo(input, S_IRUSR | S_IWUSR);
    auto input_fd = open(input, O_RDONLY);

    char buffer[1024];
    ssize_t buffer_size = 0;
    vector<int> output_fds;

    for (string output : outputs)
    {
        mkfifo(output.c_str(), S_IRUSR | S_IWUSR);
        output_fds.push_back(-1);
        auto i = output_fds.size() - 1;

        thread([i, output, &output_fds]() {
            do
            {
                if (output_fds[i] != -1)
                {
                    continue;
                }

                output_fds[i] = open(output.c_str(), O_WRONLY);
                cout << "open " << output << ' ' << output_fds[i] << endl;
            } while (this_thread::sleep_for(1s), true);
        }).detach();
    }

    while (true)
    {
        buffer_size = read(input_fd, buffer, sizeof(buffer));
        if (buffer_size == 0)
        {
            this_thread::sleep_for(1s);
            continue;
        }

        bool writen = false;

        do
        {
            for (auto &output_fd : output_fds)
            {
                if (output_fd == -1)
                    continue;

                if (-1 == write(output_fd, buffer, buffer_size))
                {
                    close(output_fd);
                    cout << "close:" << output_fd << endl;
                    output_fd = -1;
                }
                else
                {
                    writen = true;
                }
            }
        } while (!writen && (this_thread::sleep_for(1s), true));
    }

    return 0;
}