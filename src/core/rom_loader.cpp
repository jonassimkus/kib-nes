#include <SDL3/SDL.h>
#include "core/headers/cartridge.h"

cartridge load_rom(const char *filelist){
    cartridge cart;

    while (filelist) {
        SDL_Log("Full path to selected file: '%s'", filelist);

        size_t size ;

        void* content{
            SDL_LoadFile(filelist, &size)
        };

        if (content == NULL){
            SDL_Log("An error occured: %s", SDL_GetError());
            filelist++;
            break;
        }

        const uint8_t* rom = static_cast<const uint8_t*>(content);

        for (int i = 0; i < size; ++i){
            cart.append_cache(rom[i]);
        }

        cart.onload();

        SDL_free(content);
        filelist++;
        break;
    }

    return cart;
}