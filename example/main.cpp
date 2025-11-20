#include <yapl/debug.hpp>
#include <yapl/player.hpp>

int main(int argc, char **argv) {
    if (argc < 2) {
        LOG_ERROR("Usage: \n\t\t {} <video_url>", argv[0]);
    }
    yapl::player p;
    p.load(argv[1]);
    p.play();

    return 0;
}
