#include "Checker.h"


SDL_Texture* Checker::textureRedKing = nullptr, * Checker::textureRedRegular = nullptr,
* Checker::textureBlueKing = nullptr, * Checker::textureBlueRegular = nullptr;





Checker::Checker(int setPosX, int setPosY, Team setTeam) :
	posX(setPosX), posY(setPosY), team(setTeam) {
}



void Checker::loadTextures(SDL_Renderer* renderer) {
    textureRedKing = TextureLoader::loadTexture("Checker Red King.bmp", renderer);
    textureRedRegular = TextureLoader::loadTexture("Checker Red Regular.bmp", renderer);

  

    textureBlueKing = TextureLoader::loadTexture("Checker Blue King.bmp", renderer);
    textureBlueRegular = TextureLoader::loadTexture("Checker Blue Regular.bmp", renderer);

   
}



void Checker::draw(SDL_Renderer* renderer, int squareSizePixels) {
    draw(renderer, squareSizePixels, posX, posY);
}



void Checker::drawPossibleMoves(SDL_Renderer* renderer, int squareSizePixels, std::vector<Checker>& listCheckers, bool canOnlyMove2Squares) {
    //Check how far this checker can move in the four diagional directions and draw a transparent preview in each spot if required.
    //If it canOnlyMove2Squares with this move then only draw the directions that allow it to do so.
    int distance = checkHowFarCanMoveInDirection(1, 1, listCheckers);
    if (distance > 0 && (canOnlyMove2Squares == false || distance == 2))
        draw(renderer, squareSizePixels, posX + distance, posY + distance, true);

    distance = checkHowFarCanMoveInDirection(1, -1, listCheckers);
    if (distance > 0 && (canOnlyMove2Squares == false || distance == 2))
        draw(renderer, squareSizePixels, posX + distance, posY - distance, true);

    distance = checkHowFarCanMoveInDirection(-1, 1, listCheckers);
    if (distance > 0 && (canOnlyMove2Squares == false || distance == 2))
        draw(renderer, squareSizePixels, posX - distance, posY + distance, true);

    distance = checkHowFarCanMoveInDirection(-1, -1, listCheckers);
    if (distance > 0 && (canOnlyMove2Squares == false || distance == 2))
        draw(renderer, squareSizePixels, posX - distance, posY - distance, true);
}



int Checker::checkHowFarCanMoveInAnyDirection(std::vector<Checker>& listCheckers) {
    return (std::max(checkHowFarCanMoveInDirection(1, 1, listCheckers),
            std::max(checkHowFarCanMoveInDirection(-1, 1, listCheckers),
            std::max(checkHowFarCanMoveInDirection(1, -1, listCheckers),
                     checkHowFarCanMoveInDirection(-1, -1, listCheckers)))));
}



int Checker::tryToMoveToPosition(int x, int y, std::vector<Checker>& listCheckers, int& indexCheckerErase, bool canOnlyMove2Squares) {
    //Attempt to move to the input position if possible.  Flag a checker to be removed and/or turn this one into a king if required.

    int xDirection = ((x - posX) > 0 ? 1 : -1);
    int yDirection = ((y - posY) > 0 ? 1 : -1);

    int distance = checkHowFarCanMoveInDirection(xDirection, yDirection, listCheckers);
    if (distance > 0 && (canOnlyMove2Squares == false || distance == 2)) {
        int xMovable = posX + xDirection * distance;
        int yMovable = posY + yDirection * distance;

        //Ensure that (xMovable, yMovable) is the input position (x, y).  Note that it might just be in it's direction.
        if (xMovable == x && yMovable == y) {
            //If it's moving a distance of two then set indexCheckerErase to the checker that's being jumped over.
            if (distance == 2) {
                int xRemove = posX + xDirection;
                int yRemove = posY + yDirection;

                bool checkerFound = false;
                for (int count = 0; count < listCheckers.size() && checkerFound == false; count++) {
                    Checker& checkerSelected = listCheckers[count];
                    if (checkerSelected.posX == xRemove && checkerSelected.posY == yRemove) {
                        indexCheckerErase = count;
                        checkerFound = true;
                    }
                }
            }

            //Update the position of this checker and make it a king if needed.
            posX = xMovable;
            posY = yMovable;

            switch (team) {
            case Team::red:
                if (posY == 11)
                    isAKing = true;
                break;
            
            case Team::blue:
                if (posY == 0)
                    isAKing = true;
                break;
            }

            return distance;
        }
    }

    return 0;
}



int Checker::getPosX() {
    return posX;
}


int Checker::getPosY() {
    return posY;
}



Checker::Team Checker::getTeam() {
    return team;
}



void Checker::draw(SDL_Renderer* renderer, int squareSizePixels, int x, int y, bool drawTransparent) {
    //Select the correct texture to be drawn.
    SDL_Texture* textureDrawSelected = nullptr;

    switch (team) {
    case Team::red:
        textureDrawSelected = (isAKing ? textureRedKing : textureRedRegular);
        break;
    
    case Team::blue:
        textureDrawSelected = (isAKing ? textureBlueKing : textureBlueRegular);
        break;
   
    }


    //If a texture has been selected then draw it at the correct position.

    if (textureDrawSelected != nullptr) {
        if (drawTransparent)
            SDL_SetTextureAlphaMod(textureDrawSelected, 128);
        else
            SDL_SetTextureAlphaMod(textureDrawSelected, 255);

        
        int offsetX = 192; // Example offset for the board
        int offsetY = 192; // Example offset for the board
        SDL_Rect rect = {
        offsetX + (x * squareSizePixels), // Add offset to x
        offsetY + (y * squareSizePixels), // Add offset to y
        squareSizePixels,
        squareSizePixels
        };
        /*SDL_Rect rect = {
            (x + 1) * squareSizePixels,
            (y + 1) * squareSizePixels,
            squareSizePixels,
            squareSizePixels };*/

        SDL_RenderCopy(renderer, textureDrawSelected, NULL, &rect);
    }
}



int Checker::checkHowFarCanMoveInDirection(int xDirection, int yDirection, std::vector<Checker>& listCheckers) {
    if (abs(xDirection) == 1 && abs(yDirection) == 1) {

        //Ensure that this checker can move in the input direction either because it's a king or based on the direction that it's team allows.
        if (isAKing || (team == Team::red && yDirection > 0) ||(team == Team::blue && yDirection < 0) ) {
            int x = posX + xDirection;
            int y = posY + yDirection;

            //Ensure that the position is within the bounds of the game board (a square minus the four coners).
            if (x > -1 && x < 10 && y > -1 && y < 10 &&
                (x < 3 && y < 3) == false && (x >= 9 && y < 3) == false && (x < 3 && y >= 9) == false && (x >= 9 && y >= 9) == false) {

                //Try to find another checker in the specified position and determine how far this checker can move based on that information.
                Checker* checkerSelected = findCheckerAtPosition(x, y, listCheckers);
                if (checkerSelected == nullptr)
                    return 1;

                else if (checkerSelected->team != team) {
                    x = posX + xDirection * 2;
                    y = posY + yDirection * 2;

                    if (x > -1 && x < 10 && y > -1 && y < 10 &&
                        (x < 3 && y < 3) == false && (x >= 9 && y < 3) == false && (x < 3 && y >= 9) == false && (x >= 9 && y >= 9) == false) {
                        checkerSelected = findCheckerAtPosition(x, y, listCheckers);
                        if (checkerSelected == nullptr)
                            return 2;
                    }
                }
            }
        }
    }

    return 0;
}



Checker* Checker::findCheckerAtPosition(int x, int y, std::vector<Checker>& listCheckers) {
    for (auto& checkerSelected : listCheckers)
        if (checkerSelected.posX == x && checkerSelected.posY == y)
            return &checkerSelected;

    return nullptr;
}