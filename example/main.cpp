#include <SFML/Graphics.hpp>
#include <yapl/debug.hpp>
#include <yapl/player.hpp>

int main(int argc, char **argv) {
    if (argc < 2) {
        LOG_ERROR("Usage: \n\t\t {} <video_url>", argv[0]);
    }
    // clearScreen();
    // ProgressBar bar{1, 1};
    // VuMeter vuMeter{"Memory Usage", 2, 1, 0, 50};

    yapl::player p;
    // player.register_buffer_update_handler([&](size_t totalSize, size_t used){
    //     auto progress = static_cast<size_t>(static_cast<float>(used) /
    //     static_cast<float>(totalSize) * 100); std::cout << "Progress " <<
    //     progress << "%" << std::endl;
    // });
    p.load(argv[1]);
    p.play();

    return 0;
}
