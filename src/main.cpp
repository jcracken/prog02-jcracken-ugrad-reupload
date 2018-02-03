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
#include "rgbe.h"

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

float* readRGBE(string filename, int* width, int* height){
	FILE* f = fopen(filename,"rb");
	RGBE_ReadHeader(f, width, height, NULL);
	float* data = new float[3 * *width * *height];
	RGBE_ReadPixels_RLE(f, data, *width, *height);
	fclose(f);
	return data;
}

void writeRGBE(string filename, int width, int height, float* data){
	FILE* f = fopen(filename,"wb");
	RGBE_WriteHeader(f, width, height, NULL);
	RGBE_WritePixels(f, data, width * height);
	fclose(f);
}

void toneMap(float* data, float gamma, int size){
	float* lumData = new float[size];
	float* lum2 = new float[size];
	float scale;
	int i = 0;
	unsigned char r, g, b.
	for(i = 0; i < size; i++){
		r = data[3 * i];
		g = data[(3 * i) + 1];
		b = data[(3 * i) + 2];
		lumData[i] = (1.0 / 61.0) * (20.0 * r + 40.0 * g + b);
		lum2[i] = exp(gamma * log(lumData[i]));
		scale = lum2[i] / lumData[i];
		r = r * scale;
		if(r > 1.0) r = 1.0;
		else if (r < 0.0) r = 0.0;
		g = g * scale;
		if(g > 1.0) g = 1.0;
		else if (g < 0.0) g = 0.0;
		b = b * scale;
		if(b > 1.0) b = 1.0;
		else if (b < 0.0) b = 0.0;
		data[3 * i] = r;
		data[(3 * i) + 1] = g;
		data[(3 * i) + 2] = b;
	}
}

void toneMapFiltered(float* data, float gamma, int size){
	float* lumData = new float[size];
	float* lum2 = new float[size];
	float scale, B, S;
	int i = 0;
	unsigned char r, g, b.
	for(i = 0; i < size; i++){
		r = data[3 * i];
		g = data[(3 * i) + 1];
		b = data[(3 * i) + 2];
		lumData[i] = (1.0 / 61.0) * (20.0 * r + 40.0 * g + b);
		B = log(lumData[i]); //convolution
		S = log(lumData[i]) - B;
		lum2[i] = exp(gamma * B + S);
		scale = lum2[i] / lumData[i];
		r = r * scale;
		if(r > 1.0) r = 1.0;
		else if (r < 0.0) r = 0.0;
		g = g * scale;
		if(g > 1.0) g = 1.0;
		else if (g < 0.0) g = 0.0;
		b = b * scale;
		if(b > 1.0) b = 1.0;
		else if (b < 0.0) b = 0.0;
		data[3 * i] = r;
		data[(3 * i) + 1] = g;
		data[(3 * i) + 2] = b;
	}
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
	SDL_Window *windowImage = SDL_CreateWindow("Loaded Image", 100, 100, image->returnWidth(), image->returnHeight(), SDL_WINDOW_SHOWN);
	if (windowImage == NULL){
		logSDLError(std::cout, "CreateWindowImage");
		SDL_Quit();
		return 1;
	}
	SDL_Renderer *rendererImage = SDL_CreateRenderer(windowImage, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (rendererImage == NULL){
		logSDLError(std::cout, "CreateRendererImage");
    SDL_DestroyWindow(windowImage);
		SDL_Quit();
		return 1;
	}

	//The texture we'll be using
	SDL_Texture *imageTexture;

  //Initialize the texture.  SDL_PIXELFORMAT_RGB24 specifies 3 bytes per
  //pixel, one per color channel
	imageTexture = SDL_CreateTexture(rendererImage,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STATIC,image->returnWidth(),image->returnHeight());
  //Copy the raw data array into the texture.
	SDL_UpdateTexture(imageTexture, NULL, image->returnData(), 3*image->returnWidth());
  if (imageTexture == NULL){
    logSDLError(std::cout, "CreateImageTextureFromSurface");
  }

  //Make sure it loaded ok
	if (imageTexture == NULL){
    SDL_DestroyTexture(imageTexture);
    SDL_DestroyRenderer(rendererImage);
    SDL_DestroyWindow(windowImage);
		SDL_Quit();
		return 1;
	}
	//render loaded texture here
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
      }
    }

    //Display the frame rate to stdout
    const Uint64 end = SDL_GetPerformanceCounter();
    const static Uint64 freq = SDL_GetPerformanceFrequency();
    const double seconds = ( end - start ) / static_cast< double >( freq );
    //You may want to comment this line out for debugging purposes
    std::cout << "Frame time: " << seconds * 1000.0 << "ms" << std::endl;
  }


  //After the loop finishes (when the window is closed, or escape is
  //pressed, clean up the data that we allocated.
	SDL_DestroyTexture(imageTexture);
	SDL_DestroyRenderer(rendererImage);
	SDL_DestroyWindow(windowImage);
	SDL_Quit();


  return 0;
}
