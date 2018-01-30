///
/// \file main.cpp
/// \brief SDL Demo Code
/// \author Joshua A. Levine <josh@email.arizona.edu>
/// \date 01/15/18
///
/// This code provides an introductory demonstration of SDL.  When run, a
/// small window is displayed that draws an image using an SDL_Texture
///


/*
 ***********************************************************************

 Copyright (C) 2018, Joshua A. Levine
 University of Arizona

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 ***********************************************************************
 */


//include SDL2 libraries
#include <SDL.h>
#include "ppm.h"

//C++ includes
#include <iostream>
#include <cfloat>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;


///
/// Log an SDL error with some error message to the output stream of our
/// choice
///
/// \param os The output stream to write the message to
/// \param msg The error message to write, SDL_GetError() appended to it
///
void logSDLError(std::ostream &os, const std::string &msg){
	os << msg << " error: " << SDL_GetError() << std::endl;
}


///
/// Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
/// the texture's width and height
///
/// \param tex The source texture we want to draw
/// \param ren The renderer we want to draw to
/// \param x The x coordinate to draw to
/// \param y The y coordinate to draw to
///
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y){
	//Setup the destination rectangle to be at the position we want
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;

	//Query the texture to get its width and height to use
	SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	SDL_RenderCopy(ren, tex, NULL, &dst);
}






///
/// Main function.  Initializes an SDL window, renderer, and texture,
/// and then goes into a loop to listen to events and draw the texture.
///
/// \param argc Number of command line arguments
/// \param argv Array of command line arguments
/// \return integer indicating success (0) or failure (nonzero)
///

int main(int argc, char** argv) {
  //Integers specifying the width (number of columns) and height (number
  //of rows) of the image
  int num_cols = 480;
  int num_rows = 270;

  //Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}
	//setup for loading image; create new object and check commandline args
	ppm* image = new ppm();
	if(argc < 2){
		cout << "usage: prog01 filename" << endl;
		exit(EXIT_FAILURE);
	}
	//read in image data
	image->readData(argv[1]);
	SDL_Window *window = SDL_CreateWindow("Basic SDL Test", 100, 100, num_cols, num_rows, SDL_WINDOW_SHOWN);
	if (window == NULL){
		logSDLError(std::cout, "CreateWindow");
		SDL_Quit();
		return 1;
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL){
		logSDLError(std::cout, "CreateRenderer");
    SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}
	SDL_Window *windowImage = SDL_CreateWindow("Loaded Image", 100, 100, image->returnWidth(), image->returnHeight(), SDL_WINDOW_SHOWN);
	if (window == NULL){
		logSDLError(std::cout, "CreateWindowImage");
		SDL_Quit();
		return 1;
	}
	SDL_Renderer *rendererImage = SDL_CreateRenderer(windowImage, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL){
		logSDLError(std::cout, "CreateRendererImage");
    SDL_DestroyWindow(windowImage);
		SDL_Quit();
		return 1;
	}

	//The textures we'll be using
	SDL_Texture *background;
	SDL_Texture *imageTexture;

  //A raw data array of characters.  Each column is drawn using the same
  //color that is a grayscale ramp from the leftmost to rightmost pixel.
  unsigned char* data = new unsigned char[num_cols*num_rows*3];
  //r is row, c is column, and ch is channel
  for (int r=0; r<num_rows; r++) {
    for (int c=0; c<num_cols; c++) {
      for (int ch=0; ch<3; ch++) {
        data[3*(r*num_cols + c) + ch] = 255*float(c)/num_cols;
      }
    }
  }

  //Initialize the textures.  SDL_PIXELFORMAT_RGB24 specifies 3 bytes per
  //pixel, one per color channel
  background = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STATIC,num_cols,num_rows);
	imageTexture = SDL_CreateTexture(rendererImage,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STATIC,image->returnWidth(),image->returnHeight());
  //Copy the raw data array into the textures.
  SDL_UpdateTexture(background, NULL, data, 3*num_cols);
  if (background == NULL){
    logSDLError(std::cout, "CreateTextureFromSurface");
  }
	SDL_UpdateTexture(imageTexture, NULL, image->returnData(), 3*image->returnWidth());
  if (imageTexture == NULL){
    logSDLError(std::cout, "CreateImageTextureFromSurface");
  }

  //Make sure they both loaded ok
	if (background == NULL){
    SDL_DestroyTexture(background);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}
	if (imageTexture == NULL){
    SDL_DestroyTexture(imageTexture);
    SDL_DestroyRenderer(rendererImage);
    SDL_DestroyWindow(windowImage);
		SDL_Quit();
		return 1;
	}
	//render loaded texture here, as we won't be updating every frame
	renderTexture(imageTexture, rendererImage, 0, 0);
	//Update the screen
	SDL_RenderPresent(rendererImage);

  //Variables used in the rendering loop
  SDL_Event event;
	bool quit = false;
  bool leftMouseButtonDown = false;
  int start_mouseX;
  int start_mouseY;
  float orig_x_angle;
  float orig_y_angle;

	while (!quit){
    //Grab the time for frame rate computation
    const Uint64 start = SDL_GetPerformanceCounter();

    //Clear the screen
    SDL_RenderClear(renderer);
		SDL_RenderClear(rendererImage);

		//Event Polling
    //This while loop responds to mouse and keyboard commands.
    while (SDL_PollEvent(&event)){
			if (event.type == SDL_QUIT){
				quit = true;
			}
      //Use number input to select which clip should be drawn
      if (event.type == SDL_KEYDOWN){
        switch (event.key.keysym.sym){
          case SDLK_ESCAPE:
            quit = true;
            break;
          default:
            break;
        }
      } else if (event.type == SDL_MOUSEBUTTONUP ) {
        if (event.button.button == SDL_BUTTON_LEFT)
          leftMouseButtonDown = false;

      } else if (event.type == SDL_MOUSEBUTTONDOWN ) {
        if (event.button.button == SDL_BUTTON_LEFT) {
          leftMouseButtonDown = true;
        }
      } else if (event.type == SDL_MOUSEMOTION ) {
        if (leftMouseButtonDown)
        {
          int mouseX = event.motion.x;
          int mouseY = event.motion.y;

          data[3*(mouseY*num_cols + mouseX) + 0] = 255;
          data[3*(mouseY*num_cols + mouseX) + 1] = 0;
          data[3*(mouseY*num_cols + mouseX) + 2] = 0;
        }
      }
    }

    //Update the texture, assuming data has changed.
    SDL_UpdateTexture(background, NULL, data, 3*num_cols);
    //display the texture on the screen
    renderTexture(background, renderer, 0, 0);
    //Update the screen
    SDL_RenderPresent(renderer);

    //Display the frame rate to stdout
    const Uint64 end = SDL_GetPerformanceCounter();
    const static Uint64 freq = SDL_GetPerformanceFrequency();
    const double seconds = ( end - start ) / static_cast< double >( freq );
    //You may want to comment this line out for debugging purposes
    std::cout << "Frame time: " << seconds * 1000.0 << "ms" << std::endl;
  }


  //After the loop finishes (when the window is closed, or escape is
  //pressed, clean up the data that we allocated.
  delete[] data;
  SDL_DestroyTexture(background);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
	SDL_DestroyTexture(imageTexture);
	SDL_DestroyRenderer(rendererImage);
	SDL_DestroyWindow(windowImage);
	SDL_Quit();


  return 0;
}
