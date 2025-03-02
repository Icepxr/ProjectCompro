#include "Checker.h"
#include  <cstdlib>
#include <iostream>
#include <algorithm>

using namespace std;




SDL_Texture* Checker::textureRedKing = nullptr, * Checker::textureRedRegular = nullptr,
* Checker::textureBlueKing = nullptr, * Checker::textureBlueRegular = nullptr,* Checker::TokenR_instantKing=nullptr, * Checker::TokenB_instantKing = nullptr, * Checker::Portal = nullptr;





Checker::Checker(int setPosX, int setPosY, Team setTeam) :
	posX(setPosX), posY(setPosY), team(setTeam),isAKing(false) {

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
    draw(renderer, squareSizePixels, posX, posY,false);

}



void Checker::drawPossibleMoves(SDL_Renderer* renderer, int squareSizePixels, std::vector<Checker>& listCheckers, bool canOnlyMove2Squares) {
    //Check how far this checker can move in the four diagional directions and draw a transparent preview in each spot if required.
    //If it canOnlyMove2Squares with this move then only draw the directions that allow it to do so.
    for (int xDir : {-1, 1}) {
        for (int yDir : {-1, 1}) {
            int maxDistance = checkHowFarCanMoveInDirection(xDir, yDir, listCheckers);

            if (maxDistance > 0) {
                if (isAKing) {
                    // For kings, draw all valid positions along the diagonal
                    for (int dist = 1; dist <= maxDistance; dist++) {
                        int newX = posX + (xDir * dist);
                        int newY = posY + (yDir * dist);

                        // If in capture-only mode, only show positions that result in captures
                        if (!canOnlyMove2Squares ||
                            willCaptureInPath(posX, posY, newX, newY, xDir, yDir, listCheckers)) {
                            draw(renderer, squareSizePixels, newX, newY, true);
                        }
                    }
                }
                else {
                    // For regular pieces, just show the maximum valid move
                    if (!canOnlyMove2Squares || maxDistance == 2) {
                        draw(renderer, squareSizePixels, posX + (xDir * maxDistance),
                            posY + (yDir * maxDistance), true);
                    }
                }
            }
        }
    }
}


bool Checker::willCaptureInPath(int startX, int startY, int endX, int endY,
    int xDir, int yDir, std::vector<Checker>& listCheckers) {
    int x = startX;
    int y = startY;
    bool foundOpponent = false;

    while (x != endX || y != endY) {
        x += xDir;
        y += yDir;

        Checker* checkerSelected = findCheckerAtPosition(x, y, listCheckers);
        if (checkerSelected) {
            if (checkerSelected->team != team) {
                // Found an opponent piece
                foundOpponent = true;
            }
            else {
                // Found a friendly piece - can't move through it
                return false;
            }
        }
        else if (foundOpponent) {
            // Found an empty square after an opponent - this is a capture
            return true;
        }
    }

    return false;
}




int Checker::checkHowFarCanMoveInAnyDirection(std::vector<Checker>& listCheckers) {
    return (std::max(checkHowFarCanMoveInDirection(1, 1, listCheckers),
            std::max(checkHowFarCanMoveInDirection(-1, 1, listCheckers),
            std::max(checkHowFarCanMoveInDirection(1, -1, listCheckers),
                     checkHowFarCanMoveInDirection(-1, -1, listCheckers)))));
}



int Checker::tryToMoveToPosition(int x, int y, std::vector<Checker>& listCheckers, int& indexCheckerErase, bool canOnlyMove2Squares) {
    if (x == posX && y == posY) {
		position_token(posX, posY, listCheckers);
        return 0; // Prevent self-move
    } 

    int xDirection = (x > posX) ? 1 : -1;
    int yDirection = (y > posY) ? 1 : -1;
    int xDistance = abs(x - posX);
    int yDistance = abs(y - posY);

    // Ensure movement is diagonal
    if (xDistance != yDistance) {
        position_token(posX, posY, listCheckers);
        return 0;
    } 

    int maxAllowedDistance = checkHowFarCanMoveInDirection(xDirection, yDirection, listCheckers);

    // Check if the move is within allowed distance
    if (maxAllowedDistance <= 0 || xDistance > maxAllowedDistance) { 
        position_token(posX, posY, listCheckers);
        return 0;
    }

    // If in capture-only mode, ensure we're making a capture
    if (canOnlyMove2Squares) {
        bool willCapture = willCaptureInPath(posX, posY, x, y, xDirection, yDirection, listCheckers);
        if (!willCapture) { 
            position_token(posX, posY, listCheckers);

            return 0; 
        }
    
    }

    int xMovable = posX;
    int yMovable = posY;
    bool jumpedOverPiece = false;

    // Check the path for obstacles and captures
    while (xMovable != x || yMovable != y) {
        xMovable += xDirection;
        yMovable += yDirection;

        Checker* checkerSelected = findCheckerAtPosition(xMovable, yMovable, listCheckers);
        if (checkerSelected) {
            if (checkerSelected->team != team) {
                if (jumpedOverPiece) {
                    position_token(posX, posY, listCheckers);
                    return 0; // Can't jump over multiple pieces in a single move
				}
                // Found opponent's piece - check next position
                int nextX = xMovable + xDirection;
                int nextY = yMovable + yDirection;

                // Make sure we're not going beyond the target position
                if ((xDirection > 0 && nextX > x) || (xDirection < 0 && nextX < x) ||
                    (yDirection > 0 && nextY > y) || (yDirection < 0 && nextY < y)) {
                    position_token(posX, posY, listCheckers);
                    return 0; // Would go past the target
                }

                // Ensure the landing square is valid
                if (findCheckerAtPosition(nextX, nextY, listCheckers) == nullptr) {
                    jumpedOverPiece = true;
                    indexCheckerErase = std::distance(listCheckers.begin(), std::find_if(listCheckers.begin(), listCheckers.end(), [xMovable, yMovable](Checker& c) {
                        return c.posX == xMovable && c.posY == yMovable;
                    }));
                    position_token(posX, posY, listCheckers);
                }
                else {
                    position_token(posX, posY, listCheckers);

                    return 0; // Can't jump if landing square is occupied
                }
            }
            else {
                position_token(posX, posY, listCheckers);

                return 0; // Blocked by friendly piece
            }
        }
    }

    // Move the checker
    posX = x;
    posY = y;

    // If the checker reaches the promotion row, promote it to a king
    if ((team == Team::red && posY == 9) || (team == Team::blue && posY == 0)) {
        isAKing = true;
    }
    position_token(posX, posY, listCheckers);
    // Return 2 if we jumped over a piece, otherwise return the distance
    return jumpedOverPiece ? 2 : xDistance;
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

                
				portal_pointX = rand() % 10;
				portal_pointY = rand() % 10;
                while (((portal_pointX + portal_pointY) % 2 != 0) || (portal_pointY < 3 || portal_pointY>6) || ((portal_pointX == instant_king_Bluepoint && portal_pointY == 5)) || (portal_pointX == instant_king_Redpoint && portal_pointY == 4)&&listCheckers[i].getPosX()==portal_pointX && listCheckers[i].getPosY()==portal_pointY) {
                    portal_pointX = rand() % 10;
                    portal_pointY = rand() % 10;
                }
				cout << "point of portal X = " << portal_pointX << endl; // 0 2 4 6 8
				cout << "point of portal Y = " << portal_pointY << endl; //1 3 5 7 9

                if (listCheckers[i].getTeam() == Checker::Team::red) {
                    listCheckers.push_back(Checker(portal_pointX, portal_pointY, Checker::Team::red));
                }
                if (listCheckers[i].getTeam() == Checker::Team::blue){
                        listCheckers.push_back(Checker(portal_pointX, portal_pointY, Checker::Team::blue));
                }
                listCheckers.erase(listCheckers.begin() + i);

                for (int j = 0; j < listCheckers.size(); j++) {
                    if (listCheckers[j].getTeam() == Checker::Team::Portal) {
                        listCheckers.erase(listCheckers.begin() + j);
                    }

                }


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

    if (textureDrawSelected) {
       if (drawTransparent) {
           SDL_SetTextureAlphaMod(textureDrawSelected, 128);
       } else {
           SDL_SetTextureAlphaMod(textureDrawSelected, 255);
       }

        
        int offsetX = 192; // Example offset for the board
        int offsetY = 192; // Example offset for the board
        SDL_Rect rect = {
        offsetX + (x * squareSizePixels), // Add offset to x
        offsetY + (y * squareSizePixels), // Add offset to y
        squareSizePixels,
        squareSizePixels
        };
        

        SDL_RenderCopy(renderer, textureDrawSelected, NULL, &rect);
        if (drawTransparent) {
            SDL_SetTextureAlphaMod(textureDrawSelected, 255);
        }
    }
}



int Checker::checkHowFarCanMoveInDirection(int xDirection, int yDirection, std::vector<Checker>& listCheckers) {

    if (abs(xDirection) != 1 || abs(yDirection) != 1) return 0;

    // Regular checkers can only move forward based on their team
    if (!isAKing && ((team == Team::red && yDirection < 0) || (team == Team::blue && yDirection > 0)))
        return 0;

    int x = posX + xDirection;
    int y = posY + yDirection;

    // If out of bounds, return 0
    if (x < 0 || x >= 10 || y < 0 || y >= 10) return 0;

    // Check first position
    Checker* firstChecker = findCheckerAtPosition(x, y, listCheckers);

    if (firstChecker) {
        // First position is occupied
        if (firstChecker->team == team) {
            // Blocked by friendly piece
            return 0;
        }
        else {
            // Found opponent's piece - check if we can capture
            int jumpX = x + xDirection;
            int jumpY = y + yDirection;

            // Check if the landing square is valid
            if (jumpX >= 0 && jumpX < 10 && jumpY >= 0 && jumpY < 10 &&
                !findCheckerAtPosition(jumpX, jumpY, listCheckers)) {
                // Can capture - kings stop after capturing just like regular pieces
                return 2;
            }
            return 0;
        }
    }

    // For kings, check how far we can move without capturing
    if (isAKing) {
        int maxDistance = 1; // We can at least move 1 square

        // Continue checking empty squares along the diagonal
        while (true) {
            x += xDirection;
            y += yDirection;

            if (x < 0 || x >= 10 || y < 0 || y >= 10) break; // Check bounds

            Checker* nextChecker = findCheckerAtPosition(x, y, listCheckers);
            if (nextChecker) {
                // Found a piece - if opponent's piece, check for capture
                if (nextChecker->team != team) {
                    int jumpX = x + xDirection;
                    int jumpY = y + yDirection;

                    // Check if we can capture
                    if (jumpX >= 0 && jumpX < 10 && jumpY >= 0 && jumpY < 10 &&
                        !findCheckerAtPosition(jumpX, jumpY, listCheckers)) {
                        // Kings can capture but stop at the capturing position (jumpX, jumpY)
                        // Return the distance to the capture landing position
                        return maxDistance + 2; // +2 represents jumping over the opponent piece
                    }
                }
                // Blocked by any piece (opponent with no capture or friendly)
                break;
            }

            // Empty square - increase maximum distance
            maxDistance++;
        }

        return maxDistance;
    }
    else {
        // Regular pieces can only move 1 square without capturing
        return 1;
    }
}

bool Checker::canCaptureInAnyDirection(std::vector<Checker>& listCheckers) { //newly added
    // Check all four diagonal directions for possible captures
    for (int xDir : {-1, 1}) {
        for (int yDir : {-1, 1}) {
            // For regular pieces, only check forward directions
            if (!isAKing &&
                ((team == Team::red && yDir < 0) ||
                    (team == Team::blue && yDir > 0))) {
                continue;
            }

            // Check if there's an opponent's piece adjacent
            int adjacentX = posX + xDir;
            int adjacentY = posY + yDir;

            if (adjacentX >= 0 && adjacentX < 10 && adjacentY >= 0 && adjacentY < 10) {
                Checker* adjacentChecker = findCheckerAtPosition(adjacentX, adjacentY, listCheckers);

                if (adjacentChecker && adjacentChecker->team != team) {
                    // Check if the square beyond is empty
                    int landingX = adjacentX + xDir;
                    int landingY = adjacentY + yDir;

                    if (landingX >= 0 && landingX < 10 && landingY >= 0 && landingY < 10 &&
                        findCheckerAtPosition(landingX, landingY, listCheckers) == nullptr) {
                        return true; // Can capture
                    }
                }
            }
        }
    }

    return false; // No captures available
}



Checker* Checker::findCheckerAtPosition(int x, int y, std::vector<Checker>& listCheckers) {
    for (auto& checkerSelected : listCheckers)
        if (checkerSelected.posX == x && checkerSelected.posY == y&&checkerSelected.getTeam() != Checker::Team::Token_king_red&& checkerSelected.getTeam() != Checker::Team::Token_king_blue && checkerSelected.getTeam() != Checker::Team::Portal)
            return &checkerSelected;

    return nullptr;
}

