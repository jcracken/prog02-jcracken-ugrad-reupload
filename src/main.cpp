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

void convolution(float* data, int width, int height, float* out){
	float tempData[height][width];
	float kernel[5][5] = {
		{1.0, 4.0, 7.0, 4.0, 1.0},
		{4.0, 16.0, 26.0, 16.0, 4.0},
		{7.0, 26.0, 41.0, 26.0, 7.0},
		{4.0, 16.0, 26.0, 16.0, 4.0},
		{1.0, 4.0, 7.0, 4.0, 1.0}
	};
	int i, j, a, k = 0, b = 0;
	float w, sum = 0.0;
	for(i = 0; i < 5; i++){
		for(j = 0; j < 5; j++){
			kernel[i][j] = (1.0 / 273.0) * kernel[i][j];
			kernel[i][j] = 1.0 / 25.0;
		}
	}
	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			tempData[i][j] = log(data[k] + FLT_MIN);
			k++;
		}
	}
	for(k = 0; k < height; k++){
		for(a = 0; a < width; a++){
			for(i = (k - 2); i < (k + 2); i++){
				for(j = (a - 2); j < (a + 2); j++){
					w = tempData[k][a] - tempData[i][j];
					w = w * w;
					if(w < 0.0) w = 0.0;
					else if (w > 1.0) w = 1.0;
					w = exp(-1 * w);
					if(i < 0 && j < 0) sum = sum + w * tempData[i + 5][j + 5] * kernel[i - (k - 2)][j - (a - 2)];
					else if (i < 0 && j < (width - 1)) sum = sum + w * tempData[i + 5][j] * kernel[i - (k - 2)][j - (a - 2)];
					else if (i < (height - 1) && j < 0) sum = sum + w * tempData[i][j + 5] * kernel[i - (k - 2)][j - (a - 2)];
					else if (i < 0 && j > (width - 1)) sum = sum + w * tempData[i + 5][j - 5] * kernel[i - (k - 2)][j - (a - 2)];
					else if (i > (height - 1) && j < 0) sum = sum + w * tempData[i - 5][j + 5] * kernel[i - (k - 2)][j - (a - 2)];
					else if (i > (height - 1) && j > (width - 1)) sum = sum + w * tempData[i - 5][j - 5] * kernel[i - (k - 2)][j - (a - 2)];
					else if (i > (height - 1)) sum = sum + w * tempData[i - 5][j] * kernel[i - (k - 2)][j - (a - 2)];
					else if (j > (width - 1)) sum = sum + w * tempData[i][j - 5] * kernel[i - (k - 2)][j - (a - 2)];
					else sum = sum + w * tempData[i][j] * kernel[i - (k - 2)][j - (a - 2)];
				}
			}
			out[b] = sum;
			b++;
			sum = 0.0;
		}
	}
}

void writeRGBE(char* filename, int width, int height, float* data){
	FILE* f = fopen(filename,"wb");
	RGBE_WriteHeader(f, width, height, NULL);
	RGBE_WritePixels(f, data, width * height);
	fclose(f);
}

float* readRGBE(char* filename, int* width, int* height){
	FILE* f = fopen(filename,"rb");
	RGBE_ReadHeader(f, width, height, NULL);
	float* data = new float[3 * *width * *height];
	RGBE_ReadPixels_RLE(f, data, *width, *height);
	fclose(f);
	return data;
}

unsigned char* scaleToPPM(float* data, int size){
	unsigned char* newData = new unsigned char[size];
	for(int i = 0; i < size; i++){
		newData[i] = (unsigned char)round(data[i] * 255);
	}
	return newData;
}

float* toneMap(float* data, float gamma, int size){
	float* lumData = new float[size];
	float* lum2 = new float[size];
	float* newData = new float[3*size];
	float scale;
	int i = 0;
	float r, g, b;
	for(i = 0; i < size; i++){
		r = data[3 * i];
		g = data[(3 * i) + 1];
		b = data[(3 * i) + 2];
		lumData[i] = (1.0 / 61.0) * (20.0 * r + 40.0 * g + b);
		lum2[i] = exp(gamma * log(lumData[i] + FLT_MIN)) - FLT_MIN;
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
		newData[3 * i] = r;
		newData[(3 * i) + 1] = g;
		newData[(3 * i) + 2] = b;
	}
	return newData;
}

float* toneMapFiltered(float* data, float gamma, int size, int width, int height){
	float* lumData = new float[size];
	float* lum2 = new float[size];
	float* newData = new float[3*size];
	float scale, B, S;
	float* con = new float[size];
	int i = 0;
	float r, g, b;
	for(i = 0; i < size; i++){
		r = data[3 * i];
		g = data[(3 * i) + 1];
		b = data[(3 * i) + 2];
		lumData[i] = (1.0 / 61.0) * (20.0 * r + 40.0 * g + b);
	}
	convolution(lumData, width, height, con);
	//gamma = log(5.0)/(*(max_element(con, con + size)) - *(min_element(con, con + size)));
	//cout << "gamma: " << gamma << endl;
	//cout << "max: " << *(max_element(con, con + size)) << " min: " << *(min_element(con, con+ size)) << endl;
	//gamma = 1.0;
	for(i = 0; i < size; i++){
		r = data[3 * i];
		g = data[(3 * i) + 1];
		b = data[(3 * i) + 2];
		B = con[i];
		S = log(lumData[i] + FLT_MIN) - B;
		lum2[i] = exp(gamma * B + S) - FLT_MIN;
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
		newData[3 * i] = r;
		newData[(3 * i) + 1] = g;
		newData[(3 * i) + 2] = b;
	}
	return newData;
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

	bool filetype, bilinear = false;
	float gamma = 1.0;
	ppm* image = new ppm();
	int width, height;
	float* data;
	float* newData;

  //Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}
	//setup for loading image; create new object and check commandline args
	if(argc < 4){
		cout << "usage: prog02 input output filetype" << endl;
		exit(EXIT_FAILURE);
	}
	if(strcmp(argv[3],"ppm") == 0 || strcmp(argv[3],"PPM") == 0){
		filetype = true;
	} else {
		data = readRGBE(argv[1], &width, &height);
		newData = toneMap(data, gamma, width * height);
		filetype = false;
	}
	//read in image data
	if(filetype){
	 image->readData(argv[1]);
 } else {
	 image->setData(scaleToPPM(newData, 3*width*height));
	 image->setWidth(width);
	 image->setHeight(height);
 }
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
					case SDLK_LEFT:
						gamma = gamma - 0.1;
						if (!bilinear) newData = toneMap(data, gamma, width * height);
						else newData = toneMapFiltered(data, gamma, width * height, width, height);
						break;
					case SDLK_RIGHT:
						gamma = gamma + 0.1;
						if (!bilinear) newData = toneMap(data, gamma, width * height);
						else newData = toneMapFiltered(data, gamma, width * height, width, height);
						break;
					case SDLK_b:
						if(!bilinear){
							newData = toneMapFiltered(data, gamma, width * height, width, height);
							bilinear = true;
						} else {
							newData = toneMap(data, gamma, width * height);
							bilinear = false;
						}
						break;
          default:
            break;
        }
      }
    }
		if(!filetype){
			SDL_UpdateTexture(imageTexture, NULL, scaleToPPM(newData, 3*width*height), 3*width);
			image->setData(scaleToPPM(newData, 3*width*height));
			//render loaded texture here
			renderTexture(imageTexture, rendererImage, 0, 0);
			//Update the screen
			SDL_RenderPresent(rendererImage);
		}

    //Display the frame rate to stdout
    const Uint64 end = SDL_GetPerformanceCounter();
    const static Uint64 freq = SDL_GetPerformanceFrequency();
    const double seconds = ( end - start ) / static_cast< double >( freq );
    //You may want to comment this line out for debugging purposes
    cout << "Frame time: " << seconds * 1000.0 << "ms" << endl;
		cout << "Gamma: " << gamma << endl;
  }

  //After the loop finishes (when the window is closed, or escape is
  //pressed, clean up the data that we allocated.
	SDL_DestroyTexture(imageTexture);
	SDL_DestroyRenderer(rendererImage);
	SDL_DestroyWindow(windowImage);
	SDL_Quit();

	image->writeData(argv[2]);

	delete image;

  return 0;
}
