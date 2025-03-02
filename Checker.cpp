#include "Checker.h"
#include  <cstdlib>
#include <iostream>

using namespace std;




SDL_Texture* Checker::textureRedKing = nullptr, * Checker::textureRedRegular = nullptr,
* Checker::textureBlueKing = nullptr, * Checker::textureBlueRegular = nullptr,* Checker::TokenR_instantKing=nullptr, * Checker::TokenB_instantKing = nullptr, * Checker::Portal = nullptr;





Checker::Checker(int setPosX, int setPosY, Team setTeam) :
	posX(setPosX), posY(setPosY), team(setTeam) {
}



void Checker::loadTextures(SDL_Renderer* renderer) {

    textureRedKing = TextureLoader::loadTexture("red_king.bmp", renderer);
    textureRedRegular = TextureLoader::loadTexture("red_regular.bmp", renderer);
  
    textureBlueKing = TextureLoader::loadTexture("blue_king.bmp", renderer);
    textureBlueRegular = TextureLoader::loadTexture("blue_regular.bmp", renderer);

	TokenR_instantKing = TextureLoader::loadTexture("king_token_red.bmp", renderer);
	TokenB_instantKing = TextureLoader::loadTexture("king_token_blue.bmp", renderer);

	Portal = TextureLoader::loadTexture("portal.bmp", renderer);
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



int Checker::tryToMoveToPosition(int x, int y,vector<Checker>& listCheckers, int& indexCheckerErase, bool canOnlyMove2Squares) {
    //Attempt to move to the input position if possible.  Flag a checker to be removed and/or turn this one into a king if required.
    
    int xDirection;
    if ((x - posX) > 0) {
        xDirection = 1;
    } else {
        xDirection = -1;
    }

    int yDirection;
    if ((y - posY) > 0) {
        yDirection = 1;
    } else {
        yDirection = -1;
    }

    int distance = checkHowFarCanMoveInDirection(xDirection, yDirection, listCheckers); //จะตรวจสอบว่าหมากสามารถเคลื่อนที่ในทิศทางที่คำนวณไว้ได้ไกลแค่ไหน (1 หรือ 2 ก้าว).
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
                    if (checkerSelected.posX == xRemove && checkerSelected.posY == yRemove && checkerSelected.getTeam() != Checker::Team::Token_king_red && checkerSelected.getTeam() != Checker::Team::Token_king_blue && checkerSelected.getTeam() != Checker::Team::Portal) {
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
                if (posY == 9)
                    isAKing = true;
                break;
            
            case Team::blue:
                if (posY == 0 )
                    isAKing = true;
                break;
            }
			position_token(posX, posY, listCheckers);
			return distance; //1 เดินปกติ,2 กินหมาก
        }
    }
	position_token(posX, posY, listCheckers);
    return 0;
}




void Checker::position_token(int posX, int posY, vector<Checker>& listCheckers) {
    int instant_king_Redpoint = 0;
    int instant_king_Bluepoint = 0;
	int portal_pointX = 0;
	int portal_pointY = 0;
    
    for (int i = 0; i < listCheckers.size(); i++) {
        if (listCheckers[i].getTeam() == Checker::Team::Token_king_red) {
            if (listCheckers[i].getPosY() == 4) {
                instant_king_Redpoint = listCheckers[i].getPosX();
            }
            
        }
        if (listCheckers[i].getTeam() == Checker::Team::Token_king_blue) {
            if (listCheckers[i].getPosY() == 5) {
                instant_king_Bluepoint = listCheckers[i].getPosX();
            }
        }
        if (listCheckers[i].getTeam() == Checker::Team::Portal) {
			portal_pointX = listCheckers[i].getPosX();
			portal_pointY = listCheckers[i].getPosY();
        }

    }
	// got x,y of token king and portal
    for (int i = 0; i < listCheckers.size(); i++) {

		//token king red
        if (listCheckers[i].getPosY() == 4) {
            if (listCheckers[i].getPosX() == instant_king_Redpoint) {

                if (listCheckers[i].getTeam() == Checker::Team::red) {
                    isAKing = true;
					for (int i = 0; i < listCheckers.size(); i++) {
						if (listCheckers[i].getPosX() == instant_king_Redpoint && listCheckers[i].getPosY() == 4 && listCheckers[i].getTeam() == Checker::Team::Token_king_red) {
							listCheckers.erase(listCheckers.begin() + i);
						}
					}
                }
            }    
        } 

		//token king blue
        if (listCheckers[i].getPosY() == 5) {
            if (listCheckers[i].getPosX() == instant_king_Bluepoint) {

                if (listCheckers[i].getTeam() == Checker::Team::blue) {
                    isAKing = true;
                    for (int i = 0; i < listCheckers.size(); i++) {
                        if (listCheckers[i].getPosX() == instant_king_Bluepoint && listCheckers[i].getPosY() == 5 && listCheckers[i].getTeam() == Checker::Team::Token_king_blue) {
                            listCheckers.erase(listCheckers.begin() + i);
                        }
                    }
                }
                
            }
        }

        //portal
        if (listCheckers[i].getPosY() >= 3 && listCheckers[i].getPosY() <= 6) {
            if (listCheckers[i].getPosX() == portal_pointX && listCheckers[i].getPosY() == portal_pointY && (listCheckers[i].getTeam() == Checker::Team::red || listCheckers[i].getTeam() == Checker::Team::blue)) {

                for (int i = 0; i < listCheckers.size(); i++) {
                    if (listCheckers[i].getPosX() == portal_pointX && listCheckers[i].getPosY() == portal_pointY && listCheckers[i].getTeam() == Checker::Team::Portal) {
                        listCheckers.erase(listCheckers.begin() + i);
                    }
                }
                int portal_pointX = rand() % 10;

                int portal_pointY = rand() % 10;

                while (((portal_pointX + portal_pointY) % 2 != 0) || (portal_pointY < 3 || portal_pointY>6) || ((portal_pointX == instant_king_Bluepoint && portal_pointY == 5)) || (portal_pointX == instant_king_Redpoint && portal_pointY == 4)&&listCheckers[i].getPosX()==portal_pointX && listCheckers[i].getPosY()==portal_pointY) {
                    portal_pointX = rand() % 10;
                    portal_pointY = rand() % 10;
                }
				cout << "point of portal X = " << portal_pointX << endl; // 0 2 4 6 8
				cout << "point of portal Y = " << portal_pointY << endl; //1 3 5 7 9

                if (listCheckers[i].getTeam() == Checker::Team::red) {
                    listCheckers.push_back(Checker(portal_pointX, portal_pointY, Checker::Team::red));
                }
                else if (listCheckers[i].getTeam() == Checker::Team::blue){
                        listCheckers.push_back(Checker(portal_pointX, portal_pointY, Checker::Team::blue));
                }
                listCheckers.erase(listCheckers.begin() + i);


            }
        }
    }
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
   
	case Team::Token_king_red:
		textureDrawSelected = TokenR_instantKing;
		break;
	case Team::Token_king_blue:
		textureDrawSelected = TokenB_instantKing;
		break;
	case Team::Portal:  
		textureDrawSelected = Portal;
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
        

        SDL_RenderCopy(renderer, textureDrawSelected, NULL, &rect);
    }
}



int Checker::checkHowFarCanMoveInDirection(int xDirection, int yDirection, std::vector<Checker>& listCheckers) {

    if (abs(xDirection) == 1 && abs(yDirection) == 1) {
		if (isAKing) {
			int x = posX + xDirection;
			int y = posY + yDirection;
			//Ensure that the position is within the bounds of the game board (a square minus the four coners).
			if (x > -1 && x < 10 && y > -1 && y < 10) {
				Checker* checkerSelected = findCheckerAtPosition(x, y, listCheckers);
				if (checkerSelected == nullptr)
					return 1;
				else if (checkerSelected->team != team) {
					x = posX + xDirection * 2;
					y = posY + yDirection * 2;
					if (x > -1 && x < 10 && y > -1 && y < 10) {
						checkerSelected = findCheckerAtPosition(x, y, listCheckers);
						if (checkerSelected == nullptr)
							return 2;
					}
				}
			}

        }
        //Ensure that this checker can move in the input direction either because it's a king or based on the direction that it's team allows.
        if ((team == Team::red && yDirection > 0) ||(team == Team::blue && yDirection < 0) ) {
            int x = posX + xDirection;
            int y = posY + yDirection;

            //Ensure that the position is within the bounds of the game board (a square minus the four coners).
            if (x > -1 && x < 10 && y > -1 && y < 10) {

                //Try to find another checker in the specified position and determine how far this checker can move based on that information.
                Checker* checkerSelected = findCheckerAtPosition(x, y, listCheckers);
                if (checkerSelected == nullptr)
                    return 1;

                else if (checkerSelected->team != team) {
                    x = posX + xDirection * 2;
                    y = posY + yDirection * 2;

                    if (x > -1 && x < 10 && y > -1 && y < 10 ) {
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
        if (checkerSelected.posX == x && checkerSelected.posY == y&&checkerSelected.getTeam() != Checker::Team::Token_king_red&& checkerSelected.getTeam() != Checker::Team::Token_king_blue && checkerSelected.getTeam() != Checker::Team::Portal)
            return &checkerSelected;

    return nullptr;
}

