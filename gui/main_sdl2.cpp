// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <unordered_map>
#include <vector>
#include <SDL.h>
#include <thread>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif
#include "mywindow.h"
#include "gui.h"
#include "rom_dialog.h"
#include "nfd.h"

using std::vector;

#define SAMPLE_RATE GBC_AUDIO_SAMPLE_RATE  // Standard sample rate for audio
#define SAMPLES_FRAME (2 * GBC_AUDIO_SAMPLE_SIZE)

void (*gui_close_callback)(void* udata) = NULL;
void *gui_callback_udata = NULL;

static uint8_t key_pressed = 0;
static SDL_Window* window;
static SDL_GLContext gl_context;
static vector<int8_t> audio_buffer(SAMPLES_FRAME);  // Buffer for audio samples
SDL_AudioDeviceID audio_device;
static int sample_counter = 0;

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

#define INIT_WINDOW_WIDTH 1100
#define INIT_WINDOW_HEIGHT 700

static std::unordered_map<int, SDL_Keycode> key_map = {
    {SDLK_a, GBC_KEY_A},
    {SDLK_b, GBC_KEY_B},
    {SDLK_RETURN, GBC_KEY_START},
    {SDLK_s, GBC_KEY_SELECT},

    {SDLK_UP, GBC_KEY_UP},
    {SDLK_DOWN, GBC_KEY_DOWN},
    {SDLK_LEFT, GBC_KEY_LEFT},
    {SDLK_RIGHT, GBC_KEY_RIGHT},
};

void HandleKeyPress(SDL_Keycode key, int action)
{
    key_pressed = 0;
    auto kiter = key_map.find(key);
    if (kiter == key_map.end()) {
        return;
    }

    if (action == SDL_KEYDOWN)
        key_pressed = kiter->second;
}

void AudioThread()
{
    SDL_AudioSpec desired_spec, obtained_spec;
    desired_spec.freq = SAMPLE_RATE;
    desired_spec.format = AUDIO_S8;
    desired_spec.channels = 1;  // Mono
    desired_spec.samples = SAMPLES_FRAME;
    desired_spec.callback = NULL;

    if ((audio_device = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &obtained_spec, 0)) < 0) {
        fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
        return;
    }

    SDL_PauseAudioDevice(audio_device, 0);  // Start playing audio
    while (true) {
        SDL_Delay(1000);
    }

    SDL_CloseAudioDevice(audio_device);
    return;
}

void GuiAudioWrite(int8_t l_sample, int8_t r_sample)
{
    if (sample_counter >= audio_buffer.size()) {
        return;
    }

    audio_buffer[sample_counter++] = l_sample;
}

void GuiAudioUpdate(void *udata)
{
    if (SDL_GetQueuedAudioSize(audio_device) > (2 * SAMPLES_FRAME)) {
        fprintf(stderr, "Clear audio queue\n");
        SDL_ClearQueuedAudio(audio_device);
        return;
    }

    if (audio_device == 0) {
        return;
    }

    if (SDL_QueueAudio(audio_device, audio_buffer.data(), sample_counter) < 0) {
        fprintf(stderr, "Failed to queue audio: %s\n", SDL_GetError());
        return;
    }

    sample_counter = 0;
}

void InitAudio()
{
    std::thread audio_thread(AudioThread);
    audio_thread.detach();
}

// Main code
int GuiInit()
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("ImGui GBC", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }

    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(0); // disable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);
    InitAudio();
    return 0;
}

void OpenFileDialog(char **path) {
    char *outPath = NULL;
    nfdresult_t result = NFD_OpenDialog( NULL, NULL, &outPath );

    if (result == NFD_OKAY) {
        *path = outPath;
    } else if ( result == NFD_CANCEL ) {
        puts("User pressed cancel.");
    }
    else {
        printf("Error: %s\n", NFD_GetError() );
    }
}

int RomDialog(char **cartidge, char **boot_rom)
{
    ImGuiIO& io = ImGui::GetIO();
    SDL_Event event;
    bool done = false;
    ImVec4 clear_color = ImVec4(0x0d / float(0xff),  0x11 / float(0xff),  0x17 / float(0xff), 1.0f);

    if (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            done = true;
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
            done = true;
    }
    if (done) {
        exit(1);
        return 1;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("ImGui GBC");
    ImGui::SetNextWindowSize(ImVec2(300, 200));
    ImGui::BeginChild("Rom", ImVec2(300, 200), true);

    ImGui::Text("Select a ROM file to load, boot ROM is optional.");
    ImGui::Separator();

    if (ImGui::Button("Load ROM")) {
        OpenFileDialog(cartidge);
    }

    if (ImGui::Button("Load Boot ROM")) {
        OpenFileDialog(boot_rom);
    }

    if (*boot_rom)
        ImGui::Text("%s", *boot_rom);

    ImGui::EndChild();
    ImGui::End();

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);

    return 0;
}

void GuiUpdate()
{
        ImGuiIO& io = ImGui::GetIO();
        SDL_Event event;
        bool done = false;
        ImVec4 clear_color = ImVec4(0x0d / float(0xff),  0x11 / float(0xff),  0x17 / float(0xff), 1.0f);

        if (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;

            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                HandleKeyPress(event.key.keysym.sym, event.type);
            }
        }
        if (done) {
            if (gui_close_callback != NULL)
                gui_close_callback(gui_callback_udata);
            return;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ShowMyWindow();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
}

uint8_t GuiPollKeypad()
{
    return key_pressed;
}

void GuiSetCloseCallback(void (*callback)(void* udata)) {
    gui_close_callback = callback;
}

void GuiSetUserData(void* udata) {
    gui_callback_udata = udata;
}

void GuiDestroy()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}