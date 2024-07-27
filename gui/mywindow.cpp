#include "mywindow.h"
#include "gui.h"
#include <imgui.h>
#include <vector>
#include <ctime>
#include <random>

const int width = 160;
const int height = 144;
const int pixel_size = 4;
std::vector<ImU32> framebuffer(width * height, IM_COL32(0, 0, 0, 255)); // Initialize with black color

// Fill the framebuffer with your data
void FillFramebuffer(std::vector<ImU32>& buffer) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Example: Fill with a simple pattern
            // buffer[y * width + x] = ((x + y) % 2) ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 0, 0, 255);
            buffer[y * width + x] = IM_COL32(std::rand() % 255, std::rand() % 255, std::rand() % 255, 255);
        }
    }
}

// Draw the framebuffer
void DrawFramebuffer(ImDrawList* draw_list, const std::vector<ImU32>& buffer, ImVec2 position) {
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            ImVec2 p = ImVec2(x * pixel_size + position.x, y * pixel_size + position.y);
            draw_list->AddRectFilled(p, ImVec2(p.x + pixel_size, p.y + pixel_size), buffer[y * width + x]);
        }
    }
}

void InitMyWindow() {
    std::srand(std::time(nullptr));
}

unsigned char GuiWrite(void *udata, unsigned short addr, unsigned char data) {    
    /* TODO: TEST , data is 2bits */
    if (data == 0) {
        /* black */
        framebuffer[addr] = IM_COL32(0, 0, 0, 255);
    } else if (data == 1) {
        framebuffer[addr] = IM_COL32(255, 0, 0, 255);
    } else if (data == 2) {
        framebuffer[addr] = IM_COL32(0, 255, 0, 255);
    } else if (data == 3) {
        framebuffer[addr] = IM_COL32(0, 0, 255, 255);
    } else {
        /* error */
        framebuffer[addr] = IM_COL32(0, 255, 255, 255);
    }
    // framebuffer[addr] = IM_COL32(std::rand() % 255, std::rand() % 255, std::rand() % 255, std::rand() % 255);
    return 0;
}

void ShowMyWindow() {        
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f)); // Center window

    ImGui::SetNextWindowSize(ImVec2(width * pixel_size, height * pixel_size + 20));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));    
    ImGui::Begin("GBC");
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    // FillFramebuffer(framebuffer);
    DrawFramebuffer(draw_list, framebuffer, ImGui::GetCursorScreenPos());
    ImGui::End();
    ImGui::PopStyleVar();
}