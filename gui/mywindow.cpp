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
const int tile_viewer_border_width = 1;
static int tile_viewer_enabled = 0;

const int tile_viewr_col = 16;
const int tile_viewer_row = 384 / tile_viewr_col;

static double last_frame = 0;
static long long int last_cycles = 0;
std::vector<ImU32> framebuffer(width * height, IM_COL32(0, 0, 0, 255)); // Initialize with black color


// Draw the framebuffer
void DrawFramebuffer(ImDrawList* draw_list, const std::vector<ImU32>& buffer, ImVec2 position) {

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            ImVec2 p = ImVec2(x * pixel_size + position.x, y * pixel_size + position.y);
            draw_list->AddRectFilled(p, ImVec2(p.x + pixel_size, p.y + pixel_size), buffer[y * width + x]);
        }
    }
}

void DrawTileViewerFramebuffer(ImDrawList* draw_list, int bank, ImVec2 position) {
    // Draw the tile viewer framebuffer
    gbc_t *gbc = (gbc_t*)gui_callback_udata;
    for (int row = 0; row < tile_viewer_row; row++) {
        for (int col = 0; col < tile_viewr_col; col++) {
            int idx = row * tile_viewr_col + col;
            gbc_tile_t *tile;
            if (idx > 255) {
                uint8_t old = IO_PORT_READ(&gbc->mem, IO_PORT_LCDC);
                IO_PORT_WRITE(&gbc->mem, IO_PORT_LCDC, old & ~LCDC_BG_WINDOW_TILE_DATA);
                tile = gbc_graphic_get_tile(&gbc->graphic, TILE_TYPE_WIN, idx, bank);
                IO_PORT_WRITE(&gbc->mem, IO_PORT_LCDC, old);
            } else {
                tile = gbc_graphic_get_tile(&gbc->graphic, TILE_TYPE_OBJ, idx, bank);
            }
            /* debug rom */
            // tile = (gbc_tile_t*)((gbc->mbc.rom_banks)+(0x4000 * rom_counter + 0x1800 + idx * 16));

            int row_base = row * (8 + tile_viewer_border_width) * pixel_size + position.y + tile_viewer_border_width / 2 * pixel_size;
            int col_base = col * (8 + tile_viewer_border_width) * pixel_size + position.x + tile_viewer_border_width / 2 * pixel_size;
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    uint8_t color_id = TILE_PIXEL_COLORID(tile, x, y);

                    ImU32 color;
                    if (color_id == 0) {
                        color = IM_COL32(255, 255, 255, 255);
                    } else if (color_id == 1) {
                        color = IM_COL32(169, 169, 169, 255);
                    } else if (color_id == 2) {
                        color = IM_COL32(84, 84, 84, 255);
                    } else if (color_id == 3) {
                        color = IM_COL32(0, 0, 0, 255);
                    } else {
                        /* it is impossible to reach here */
                        color = IM_COL32(255, 0, 0, 255);
                    }

                    draw_list->AddRectFilled(
                        ImVec2(col_base + x * pixel_size, row_base + y * pixel_size),
                        ImVec2(col_base + (x + 1) * pixel_size, row_base + (y + 1) * pixel_size),
                        color
                    );
                }
            }
        }
    }
}


void InitMyWindow() {
    std::srand(std::time(nullptr));
}

void GuiWrite(void *udata, unsigned short addr, unsigned short data) {
    framebuffer[addr] = IM_COL32(GBC_COLOR_TO_RGB_R(data), GBC_COLOR_TO_RGB_G(data), GBC_COLOR_TO_RGB_B(data), 255);
}

bool IsPaused() {
    gbc_t *gbc = (gbc_t*)gui_callback_udata;
    return gbc->paused;
}

void VisualizeTiles() {
    ImGui::SetNextWindowSize(ImVec2(tile_viewr_col * (8 + tile_viewer_border_width) * pixel_size + 20,
    tile_viewer_row * (8 + tile_viewer_border_width) * pixel_size));
    ImGui::Begin("TileViewer Bank 0");
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    DrawTileViewerFramebuffer(draw_list, 0, ImGui::GetCursorScreenPos());
    ImGui::PopStyleVar();
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(tile_viewr_col * (8 + tile_viewer_border_width) * pixel_size + 20,
    tile_viewer_row * (8 + tile_viewer_border_width) * pixel_size));
    ImGui::Begin("TileViewer Bank 1");
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    draw_list = ImGui::GetWindowDrawList();
    DrawTileViewerFramebuffer(draw_list, 1, ImGui::GetCursorScreenPos());
    ImGui::PopStyleVar();
    ImGui::End();
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

        ImGui::SameLine();
        if (ImGui::Button(tile_viewer_enabled ? "Hide Tiles" : "View Tiles")) {
            tile_viewer_enabled = !tile_viewer_enabled;
        }

        if (tile_viewer_enabled) {
            VisualizeTiles();
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
    ImGui::SetNextWindowSize(ImVec2(width * pixel_size, height * pixel_size + 30));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("GBC");
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
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