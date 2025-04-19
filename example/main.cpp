#include <iostream>
#include <SFML/Graphics.hpp>
#include <sklepan/Player.hpp>
#include <indicators/progress_bar.hpp>

void clearScreen() {
    std::cout << "\033[2J\033[1;1H"; // ANSI escape code to clear the screen
}

void setCursorPosition(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H" << std::flush;
}

void clearLine(size_t line) {
    setCursorPosition(line, 0);
    std::cout << "\033[2K";
    std::cout << "\r"; 
}


void printAtPosition(size_t x, size_t y, const std::string& text) {
    setCursorPosition(x, y);
    std::cout << text;
}


class PercentageBar {
public:
    PercentageBar(size_t x, size_t y) : _x{x}, _y{y} {} 

    void setValue(size_t value) {
       setCursorPosition(_x, _y);
       _percentageString[_hundredPosition] = (value / 100) ? '1' : ' ';
       _percentageString[_tensPosition] = (value > 9) ? '0' +  value % 100 / 10 : ' ';
       _percentageString[_unitsPosition] = '0' + (value % 10);
       std::cout << _percentageString << std::flush;
       setCursorPosition(_x + 1, 0);
    }
private:
    size_t _x;
    size_t _y;
    std::string _percentageString{"[  0%]"};
    size_t _hundredPosition{1};
    size_t _tensPosition{2};
    size_t _unitsPosition{3};
};


struct ProgressBar{
    size_t totalSize;

    ProgressBar(size_t x, size_t y) : _x{x}, _y{y}, _percentageBar(_x, _y + 54) {
        _progress = std::string(50, '.');
    }

    void setValue(size_t progress) {
        if (!progress){
            return;
        }
       setCursorPosition(_x, _y);
       if (progress / 2 > 0) _progress[progress / 2 - 1] = '=';
       if (progress / 2 < 50) _progress[progress / 2] = '>';
       std::cout << _start << _progress << _end << std::flush;
       _percentageBar.setValue(progress);
        setCursorPosition(_x + 1, 0);
    }

private:
    size_t _x;
    size_t _y;
    std::string _progress;
    std::string _progressPercentage{"[  0%]"};
    size_t _hundredPosition{1};
    size_t _tensPosition{2};
    size_t _unitsPosition{3};
    const char _start = '[';
    const char _end = ']';
    PercentageBar _percentageBar;
};

class VuMeter {
public:
    VuMeter(const char* title, size_t x, size_t y, float minValue, float maxValue, bool showPercentage = true) 
        : _x{x}, _y{y}, _minValue{minValue}, _maxValue{maxValue}, _showPercentage{showPercentage}, _percentageBar{_x + 1, _y + 54} {
        _valueString = std::string(50, '.');
        _step = (_maxValue - _minValue) / 50;
        setCursorPosition(_x, _y);
        std::cout << "[" << title << "]" << std::endl;
    }

    void setValue(float value) {
        for (size_t i = 0; i < 50; i++){
            _valueString[i] = value > _step * i ? '=' : '.';
        }
        setCursorPosition(_x + 1, _y);
        std::cout << _start << _valueString << _end;
        if (_showPercentage) {
            _percentageBar.setValue(static_cast<size_t>((value - _minValue) / (_maxValue - _minValue) * 100));
        }
        std::cout << std::flush;
        setCursorPosition(_x + 2, 0);
    }
private:
    size_t _x;
    size_t _y;
    float _minValue;
    float _maxValue;
    bool _showPercentage;
    float _step;
    std::string _valueString;
    const char _start = '[';
    const char _end = ']';
    PercentageBar _percentageBar;
};


int main() {
    // clearScreen();
    // ProgressBar bar{1, 1};
    // VuMeter vuMeter{"Memory Usage", 2, 1, 0, 50};

    sklepan::Player player;
    // player.registerBufferUpdateCallback([&](size_t totalSize, size_t used){
    //     auto progress = static_cast<size_t>(static_cast<float>(used) / static_cast<float>(totalSize) * 100);
    //     std::cout << "Progress " << progress << "%" << std::endl;
    // });
    player.load("/root/player/BigBuckBunny.mp4");
    player.play();
   

    return 0;
}
