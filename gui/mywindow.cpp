#include "mywindow.h"
#include "gui.h"
#include <imgui.h>
#include <vector>
#include <ctime>
#include <string>
#include <random>

const int width = 160;
const int height = 144;
const int pixel_size = 4;

static double last_frame = 0;
static long long int last_cycles = 0;
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

unsigned short GuiWrite(void *udata, unsigned short addr, unsigned short data) {
    framebuffer[addr] = IM_COL32(GBC_COLOR_TO_RGB_R(data), GBC_COLOR_TO_RGB_G(data), GBC_COLOR_TO_RGB_B(data), 255);
    // framebuffer[addr] = IM_COL32(std::rand() % 255, std::rand() % 255, std::rand() % 255, std::rand() % 255);
    return 0;
}

bool IsPaused() {
    gbc_t *gbc = (gbc_t*)gui_callback_udata;
    return gbc->paused;
}

void ClickPause() {
    gbc_t *gbc = (gbc_t*)gui_callback_udata;
    if (gbc->paused) {
        gbc->paused = 0;
    } else {
        gbc->paused = 1;
    }
}

void ClickStep() {
    gbc_t *gbc = (gbc_t*)gui_callback_udata;
    if (gbc->paused)
        gbc->debug_steps = 1;
}

void ShowHUDControlPanels() {
        ImGui::BeginChild("Control", ImVec2(300, 50), true);
        std::string pause_text = IsPaused() ? "Resume" : "Pause";
        if (ImGui::Button(pause_text.c_str())) {
            ClickPause();
        }
        if (IsPaused()) {
            ImGui::SameLine();
            if (ImGui::Button("Step")) {
                ClickStep();
            }
        }
/*         ImGui::SameLine();
        if (ImGui::Button("Button 3")) {}  */
        ImGui::EndChild();
}

void ShowHUDStatus() {
       // Create a table on the right side
        ImGui::BeginChild("Table", ImVec2(300, 300), true);
        gbc_t *gbc = (gbc_t*)gui_callback_udata;
        gbc_cpu_t *cpu = &gbc->cpu;
        cpu_register_t regs = cpu->regs;
        cpu_register_t *r = &regs;

        std::string labels[] = {
            "PC", "SP", "A", "F", "B", "C", "D", "E", "H", "L", "Z", "N", "H", "C", "IME", "IE", "IF"
        };

        int cpu_values[DEBUG_CPU_REGISTERS_SIZE];
        debug_get_all_registers(cpu, cpu_values);
        double current_time = ImGui::GetTime();
        long long cycles = cpu->cycles;

        double fps = 1.0 / (current_time - last_frame);
        int hz = (double)(cycles - last_cycles) / (current_time - last_frame);
        last_cycles = cycles;
        last_frame = current_time;

        ImGui::Text("cycles: ");
        ImGui::SameLine();
        ImGui::Text("%llu", gbc->cpu.cycles);

        ImGui::Text("speed: ");
        ImGui::SameLine();
        ImGui::Text("%dHz", hz);

        ImGui::Text("FPS: ");
        ImGui::SameLine();
        ImGui::Text("%.2f", fps);

        ImGui::Separator(); // Optional separator line

        if (ImGui::BeginTable("REG", 4))
        {
            for (int i = 0; i < DEBUG_CPU_REGISTERS_SIZE; i++) {
                ImGui::TableNextColumn();
                ImGui::Text("%s", labels[i].c_str());
                ImGui::TableNextColumn();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Red color
                ImGui::Text("%x", cpu_values[i]);
                ImGui::PopStyleColor(); // Revert to default color
                if ((i+1) % 2 == 0) {
                    ImGui::TableNextRow();
                }
            }

            ImGui::EndTable();
        }
        ImGui::EndChild();
}

void ShowHUD() {
    ImGui::Begin("HUD");

    ShowHUDControlPanels();
    ShowHUDStatus();

    ImGui::End();
}

void ShowGame() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    //ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f)); // Center window
    ImGui::SetNextWindowSize(ImVec2(width * pixel_size, height * pixel_size + 20));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("GBC");
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    // FillFramebuffer(framebuffer);
    DrawFramebuffer(draw_list, framebuffer, ImGui::GetCursorScreenPos());
    ImGui::End();
    ImGui::PopStyleVar();
}

void ShowMyWindow() {
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.5f;  // Scale the font by 1.5x
    ShowGame();
    ShowHUD();
}