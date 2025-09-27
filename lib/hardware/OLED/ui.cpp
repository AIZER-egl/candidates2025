#include <stdio.h>
#include <vector>
#include <string>

extern "C" {
    #include "ssd1306.h"
}

class menuItem {
public:
struct UIElements
    {
        std::string label;
        int id;
        bool active;
    };
    std::vector<UIElements> ui_elements;

    menuItem(std::string n, int i, bool a) {
        UIElements element = {n, i, a};
        ui_elements.push_back(element);
    }

private:
    void toggleActive() {
        ui_elements.back().active = !ui_elements.back().active;
    }
};