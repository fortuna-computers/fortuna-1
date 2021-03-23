#ifndef LIBFORTUNA1_TSUPPORT_HH
#define LIBFORTUNA1_TSUPPORT_HH

#include <memory>
#include <fortuna1.hh>

struct TestArgs {
   TestArgs(int argc, char** argv);
   
   std::unique_ptr<Fortuna1> create_fortuna();
   Z80_Info run_code(std::string const& code, size_t num_steps);
   
   bool real_hardware_mode = false;
   std::string serial_port = "";
   bool log_bytes = false;
   bool log_messages = false;

private:
    static void show_help(const char* program);
    Fortuna1* fortuna1_ = nullptr;
};

std::ostream& operator<<(std::ostream& os, std::vector<uint8_t> const& bytes);

#define ASSERT_GT(msg, value, greater_than) { \
    std::cout << msg << "... "; \
    auto __v = value;                       \
    auto __g = greater_than;                 \
    if (__v > __g) { std::cout << "\e[0;32m✔\e[0m\n"; } else { std::cout << "\e[0;31mX\e[0m   Expected received value of " << __v << " to be > " << __g << "\n"; exit(1); } \
}

#define ASSERT_Q(expected, received) { \
    auto __e = expected;                       \
    auto __r = received;                       \
    if (std::tie(__e) == std::tie(__r)) { std::cout << "\e[0;32m✔\e[0m"; fflush(stdout); } else { std::cout << "\e[0;31mX\e[0m   Expected: " << __e << "  Received: " << __r << "\n"; exit(1); } \
}

#define ASSERT_EQ(msg, expected, received) { \
    std::cout << msg << "... ";              \
    auto __e = expected;                       \
    auto __r = received;                       \
    if (std::tie(__e) == std::tie(__r)) { std::cout << "\e[0;32m✔\e[0m\n"; } else { std::cout << "\e[0;31mX\e[0m   Expected: " << __e << "  Received: " << __r << "\n"; exit(1); } \
}

void title(std::string const& text);

#endif //LIBFORTUNA1_TSUPPORT_HH