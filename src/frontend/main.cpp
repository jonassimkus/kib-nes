#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>  
#include <cstddef>
#include <cstdint>

#include "SDL3/SDL_dialog.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_iostream.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL_timer.h"
#include "core/headers/cartridge.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "imgui_internal.h"

#include "core/rom_loader.cpp"
#include "core/headers/cpu.h"

#include <unordered_map>

cpu *m_cpu;
cartridge m_cart;

static const SDL_DialogFileFilter filters[] = {
    { "nes",  "nes" },
    { "All files",   "*" }
};

static void SDLCALL callback(void* userdata, const char* const* filelist, int filter){
    if (!filelist) {
        SDL_Log("An error occured: %s", SDL_GetError());
        return;
    } else if (!*filelist) {
        SDL_Log("The user did not select any file.");
        return;
    }

    m_cart = load_rom(*filelist);
    m_cpu->load_cartridge(&m_cart);
}

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("kibnes", 800, 600,
        SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_BORDERLESS);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        return 1;
    }
    SDL_SetRenderVSync(renderer, 1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);


    // Initilize cpu
    m_cpu = new cpu();

    bool running = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);  
            if (event.type == SDL_EVENT_QUIT){
                running = false;
            }
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)){
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN){
            }

            if (event.type == SDL_EVENT_KEY_UP){
            }

            if (event.type == SDL_EVENT_DROP_FILE){
                const char* dropped_fileDir = event.drop.data;

                m_cart = load_rom(dropped_fileDir);
                m_cpu->load_cartridge(&m_cart);
            }
        }
        

        if (m_cpu->get_cpu_state() == CPU_RUNNING){
            m_cpu->cycle();
        }
       
        SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);

        SDL_SetRenderDrawColor(renderer, 25, 25, 45, 255);
        SDL_RenderClear(renderer);
    

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open")) {
                    SDL_ShowOpenFileDialog(*callback, NULL, window, filters, 2, NULL, false);
                }

                if (ImGui::MenuItem("Close Rom")){
                }

                if (ImGui::MenuItem("Exit")){
                    running = false;
                }

                ImGui::EndMenu();
            }

            if (ImGui::Button("Cycle")){
                m_cpu->cycle();
            }

            ImGui::SameLine(ImGui::GetWindowWidth()-24);
            if (ImGui::Button("X")){
                running = false;
            }

            ImGui::EndMainMenuBar();
        }


        ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos({main_viewport->WorkPos.x, main_viewport->WorkPos.y + main_viewport->WorkSize.y - ImGui::GetFrameHeight()-12});
        ImGui::SetNextWindowSize({main_viewport->WorkSize.x, (main_viewport->WorkSize.y - (36*8) - ImGui::GetFrameHeight()-6)});

        ImGui::Render();


        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

        //SDL_DelayNS(SDL_MS_TO_NS(1.67));
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}