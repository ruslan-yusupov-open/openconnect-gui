// Изолированный стенд: production callback, подмена только окружения Qt/Logger.
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

struct VpnInfo { };
constexpr int PRG_TRACE = 99;

class Logger {
public:
    static Logger& instance()
    {
        static Logger logger;
        return logger;
    }

    void addMessage(const char* message)
    {
        messages.emplace_back(message);
    }

    std::vector<std::string> messages;
};

#include "progress_callback.inc"

int main(int argc, char** argv)
{
    if (argc != 2)
        return 2;

    const std::string scenario = argv[1];
    std::vector<std::string> expected;
    if (scenario == "empty") {
        progress_vfn(nullptr, 0, "");
        expected = { "" };
    } else if (scenario == "newline") {
        progress_vfn(nullptr, 0, "\n");
        expected = { "" };
    } else if (scenario == "plain") {
        progress_vfn(nullptr, 0, "connected");
        expected = { "connected" };
    } else if (scenario == "trim_one") {
        progress_vfn(nullptr, 0, "connected\n\n");
        expected = { "connected\n" };
    } else if (scenario == "long") {
        const std::string message(600, 'x');
        progress_vfn(nullptr, 0, "%s", message.c_str());
        expected = { std::string(511, 'x') };
    } else if (scenario == "trace") {
        progress_vfn(nullptr, PRG_TRACE, "hidden");
        progress_vfn(nullptr, PRG_TRACE, "");
    } else if (scenario == "formatted") {
        progress_vfn(nullptr, 0, "%s %d\n", "attempt", 2);
        expected = { "attempt 2" };
    } else if (scenario == "formatted_empty") {
        progress_vfn(nullptr, 0, "%s", "");
        expected = { "" };
    } else {
        return 2;
    }

    if (Logger::instance().messages != expected) {
        std::cerr << "FAIL: " << scenario << '\n';
        return 1;
    }
    std::cout << "PASS: " << scenario << '\n';
    return 0;
}
