#include "TextureLoader.h"


std::vector<SDL_Texture*> TextureLoader::listTexturesLoaded;






SDL_Texture* TextureLoader::loadTexture(std::string filename, SDL_Renderer* renderer) {
   
    std::string filepath = "S:/Game/Project2/Images/" + filename;

   

    //Try to create a surface using the filepath.
    SDL_Surface* surfaceTemp = SDL_LoadBMP(filepath.c_str());
    if (surfaceTemp != nullptr) {

        //The surface was created successfully so attempt to create a texture with it.
        SDL_Texture* textureOutput = SDL_CreateTextureFromSurface(renderer, surfaceTemp);
        //Free the surface because it's no longer needed. 
        SDL_FreeSurface(surfaceTemp);

        if (textureOutput != nullptr) {
            //Enable transparency for the texture.
            SDL_SetTextureBlendMode(textureOutput, SDL_BLENDMODE_BLEND);

            //Add the texture to the list of loaded textures to keep track of it for clean-up purposes.
            listTexturesLoaded.push_back(textureOutput);
            return textureOutput;
        }
    }

    return nullptr;
}



void TextureLoader::deallocateTextures() {
    //Destroy all the textures on the list.
    while (listTexturesLoaded.size() > 0) {
        if (listTexturesLoaded[0] != nullptr)
            SDL_DestroyTexture(listTexturesLoaded[0]);
            

        listTexturesLoaded.erase(listTexturesLoaded.begin());
    }
}