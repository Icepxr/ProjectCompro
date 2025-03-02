#include "Game.h"
#include <iostream>
#include <cstdlib>
using namespace std;    

Game::Game(SDL_Window* window, SDL_Renderer* renderer, int boardSizePixels) : 
	squareSizePixels(boardSizePixels / (10 + 6)), gameModeCurrent(GameMode::playing) { //boardSizePixels = 1024
    //Run the game.
    if (window != nullptr && renderer != nullptr) {
        //Load the textures for the checkers.
        Checker::loadTextures(renderer);

        textureCheckerBoard = TextureLoader::loadTexture("Board checker.bmp", renderer);

        textureTeamRedWon = TextureLoader::loadTexture("Team Red Won Text.bmp", renderer);
        textureTeamBlueWon = TextureLoader::loadTexture("Team Blue Won Text.bmp", renderer);


        resetBoard();


        //Start the game loop and run until it's time to stop.
        bool running = true;
        while (running) {
            processEvents(running);
            draw(renderer);
			
        }


        //Deallocate the textures.
        TextureLoader::deallocateTextures();
    }
}



void Game::processEvents(bool& running) {
    bool mouseDownThisFrame = false;

    //Process events.
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;

        case SDL_MOUSEBUTTONDOWN:
            mouseDownThisFrame = (mouseDownStatus == 0);
            if (event.button.button == SDL_BUTTON_LEFT)
                mouseDownStatus = SDL_BUTTON_LEFT;
            else if (event.button.button == SDL_BUTTON_RIGHT)
                mouseDownStatus = SDL_BUTTON_RIGHT;
            break;
        case SDL_MOUSEBUTTONUP:
            mouseDownStatus = 0;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.scancode) {
                //Quit the game.
            case SDL_SCANCODE_ESCAPE:
                running = false;
                break;

            case SDL_SCANCODE_R:

                resetBoard();
                break;
            }
        }
    }


	//เปลี่ยนตำแหน่งที่เมาส์ไปเป็นตำแหน่งบนหน้าจอโดยใช้ตำแหน่งของตารางมีขอบที่ 192 และ 192
    if (mouseDownThisFrame) {
        int mouseX = 0, mouseY = 0;
        SDL_GetMouseState(&mouseX, &mouseY);
        //Convert from the window's coordinate system to the game's coordinate system.
        int offsetX = 192; 
        int offsetY = 192;
        int squareX = ((mouseX - offsetX) / squareSizePixels) ;
        int squareY = ((mouseY - offsetY) / squareSizePixels) ;
		cout << "Square X = " << squareX << " Square Y = " << squareY << endl;


        if (gameModeCurrent == GameMode::playing)
            checkCheckersWithMouseInput(squareX, squareY);
    }
}



void Game::checkCheckersWithMouseInput(int x, int y) {
    if (x > -1 && x < 10 && y > -1 && y < 10) {
        // If no checker is selected, try to find and select one at the input position
        if (indexCheckerInPlay == -1) {
            for (int count = 0; count < listCheckers.size(); count++) {
                Checker* checkerSelected = &listCheckers[count];
                if (checkerSelected->getPosX() == x && checkerSelected->getPosY() == y &&
                    checkerSelected->getTeam() == teamSelectedForGameplay) {
                    indexCheckerInPlay = count;
                    break;  // Select the first valid piece
                }
            }
        }
        else {
            // Otherwise, a checker is selected, so attempt to move it.
            int indexCheckerErase = -1;
            int distanceMoved = listCheckers[indexCheckerInPlay].tryToMoveToPosition(x, y, listCheckers, indexCheckerErase, checkerInPlayCanOnlyMove2Squares);

            // If a checker needs to be erased (captured), do so
            if (indexCheckerErase > -1 && indexCheckerErase < listCheckers.size()) {
                listCheckers.erase(listCheckers.begin() + indexCheckerErase);
                if (indexCheckerInPlay > indexCheckerErase) {
                    indexCheckerInPlay--;
                }
            }

            // Process the move
            switch (distanceMoved) {
            case 0:
                indexCheckerInPlay = -1;  // Deselect the checker after it captures or cannot move
                if (checkerInPlayCanOnlyMove2Squares) {
                    checkerInPlayCanOnlyMove2Squares = false;
                    incrementTeamSelectedForGameplay();
                }
                break;

            case 1:
                indexCheckerInPlay = -1;  // Deselect the checker after moving
                incrementTeamSelectedForGameplay();
                break;

            case 2:
                // After a capture, check if another capture is possible
                if (indexCheckerInPlay > -1 && indexCheckerInPlay < listCheckers.size() &&
                    listCheckers[indexCheckerInPlay].canCaptureInAnyDirection(listCheckers)) {
                    // If the checker can capture again, keep it selected
                    checkerInPlayCanOnlyMove2Squares = true;
                }
                else {
                    indexCheckerInPlay = -1;  // Deselect after move
                    checkerInPlayCanOnlyMove2Squares = false;
                    incrementTeamSelectedForGameplay();
                }
                break;

            default:
                // For kings moving multiple squares
                if (indexCheckerInPlay > -1 && indexCheckerInPlay < listCheckers.size() &&
                    listCheckers[indexCheckerInPlay].canCaptureInAnyDirection(listCheckers)) {
                    // If the checker can capture again after moving, keep it selected
                    checkerInPlayCanOnlyMove2Squares = true;
                }
                else {
                    indexCheckerInPlay = -1;  // Deselect after move
                    checkerInPlayCanOnlyMove2Squares = false;
                    incrementTeamSelectedForGameplay();
                }
                break;
            }
            checkWin();
        }
    }
}



void Game::incrementTeamSelectedForGameplay() {
    //Select the next team and ensure that it has at least one move left.  If not skip to the next one up to four times.
    for (int count = 0; count < 2; count++) {
        switch (teamSelectedForGameplay) {
        case Checker::Team::red:
            teamSelectedForGameplay = Checker::Team::blue;
            break;
        
        case Checker::Team::blue:
            teamSelectedForGameplay = Checker::Team::red;
            break;
        

        }
        
        if (teamStillHasAtLeastOneMoveLeft(teamSelectedForGameplay))
            return;
    }
}



void Game::draw(SDL_Renderer* renderer) {
    //Clear the screen.
    SDL_RenderClear(renderer);

    if (textureCheckerBoard != nullptr)
        SDL_RenderCopy(renderer, textureCheckerBoard, NULL, NULL);

    //Draw the checkers.
    for (auto& checkerSelected : listCheckers)
        checkerSelected.draw(renderer, squareSizePixels);

    //If a checker is selected then draw it's possible moves.
    if (indexCheckerInPlay > -1 && indexCheckerInPlay < listCheckers.size())
        listCheckers[indexCheckerInPlay].drawPossibleMoves(renderer, squareSizePixels, listCheckers, checkerInPlayCanOnlyMove2Squares);


    //If the game has ended then draw an image that has a black overlay with white text that indicates the winner.
    //Select the correct texture to be drawn.
    SDL_Texture* textureDrawSelected = nullptr;

    switch (gameModeCurrent) {
    case GameMode::teamRedWon:      textureDrawSelected = textureTeamRedWon;    break;
    case GameMode::teamBlueWon:     textureDrawSelected = textureTeamBlueWon;   break;
    }

    //Draw the texture overlay if needed.
    if (textureDrawSelected != nullptr)

        SDL_RenderCopy(renderer, textureDrawSelected, NULL, NULL);


    //Send the image to the window.
    SDL_RenderPresent(renderer);
}




void Game::resetBoard() {
    //Reset the game variables.
	cout << "resetBoard" << endl;
	cout << "---------------------------------------" << endl;

	int instant_king_Redpoint = rand() % 10;
	if (instant_king_Redpoint % 2 != 0&&instant_king_Redpoint<9) {
		instant_king_Redpoint++;
	}
	else if (instant_king_Redpoint == 9) {
		instant_king_Redpoint--;
	}
    int instant_king_Bluepoint = rand() % 10;
	if (instant_king_Bluepoint % 2 == 0) {
		instant_king_Bluepoint++;
	}
	int portal_pointX = rand() % 10;

	int portal_pointY = rand() % 10;

	while (((portal_pointX + portal_pointY)%2 !=0)  || (portal_pointY<3||portal_pointY>6) ||((portal_pointX==instant_king_Bluepoint&&portal_pointY==5)) || (portal_pointX == instant_king_Redpoint && portal_pointY == 4)) {
		portal_pointX = rand() % 10;
		portal_pointY = rand() % 10;
	}

    std::cout << "point of instant red point king X = " << instant_king_Redpoint << std::endl; // 0 2 4 6 8
    std::cout << "point of instant blue point king X = " << instant_king_Bluepoint<< std::endl; //1 3 5 7 9
    
	std::cout << "point of portal X = " << portal_pointX << std::endl; // 0 2 4 6 8
    std::cout << "point of portal Y = " << portal_pointY << std::endl; //1 3 5 7 9
    cout << "---------------------------------------" << endl;

    gameModeCurrent = GameMode::playing;
    listCheckers.clear();
    teamSelectedForGameplay = Checker::Team::red;

    //Loop through the entire board and place checkers in the black squares on the first and last three rows.
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            
            if ((x + y) % 2 == 0) {
                if (y < 2) {
                    listCheckers.push_back(Checker(x, y, Checker::Team::red));
                }
                if (y >= 8) {
                    listCheckers.push_back(Checker(x, y, Checker::Team::blue));
                }
                if (y == 4) {
                    if (x == instant_king_Redpoint) {
                        if (instant_king_Redpoint % 2 == 0) {
                            listCheckers.push_back(Checker(x, y, Checker::Team::Token_king_red));

                        }
                    }
                }
                if (y == 5) {
                    if (x == instant_king_Bluepoint) {
                        if (instant_king_Bluepoint % 2 != 0) {
                            listCheckers.push_back(Checker(x, y, Checker::Team::Token_king_blue));
                        }
                    }

                }
                if (y > 1 && y < 8 || (portal_pointX!=instant_king_Bluepoint && portal_pointX!=instant_king_Redpoint)) {
                    if (x == portal_pointX && y == portal_pointY) {
                        listCheckers.push_back(Checker(x, y, Checker::Team::Portal));
                    }
                }
            }
			
        }
    }	

}



void Game::checkWin() {
    //Check all the teams to see if they have any moves left and store the combined result.
    int result =
        teamStillHasAtLeastOneMoveLeft(Checker::Team::red) << 1 |
        teamStillHasAtLeastOneMoveLeft(Checker::Team::blue) << 0;

    //Check the result to see if only one of the teams can move and if so then set the game mode accordingly.
    switch (result) {
    case (1 << 1):
        gameModeCurrent = GameMode::teamRedWon;
        break;
    
    case (1 << 0):
        gameModeCurrent = GameMode::teamBlueWon;
        break;
    
    }
}



bool Game::teamStillHasAtLeastOneMoveLeft(Checker::Team team) {
    //Check the input team to see if it has at least one checker that can move
    for (auto& checkerSelected : listCheckers)
        if (checkerSelected.getTeam() == team &&
            checkerSelected.checkHowFarCanMoveInAnyDirection(listCheckers) > 0)
            return true;

    return false;
}
