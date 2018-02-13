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

//
// Convolution function that applies a bilinear filter to the image data sent in.
//
// \param data Image data to be modified
// \param width width of image
// \param height height of image
// \param out output data
//
void convolution(float* data, int width, int height, float* out, int radius, int type){
	//Create temp 2D data to hold input
	float tempData[height][3 * width];

	//Kernel for image filter
	float kernel[2 * radius + 1][2 * radius + 1];
	/*float kernel[21][21] =
	{
		{0.001174, 0.001291,		0.001405,		0.001514,		0.001616,		0.001707,		0.001786,		0.001849,		0.001896,		0.001925,		0.001934,		0.001925,		0.001896,		0.001849,		0.001786,		0.001707,		0.001616,		0.001514,		0.001405,		0.001291,		0.001174},
			{0.001291,		0.001419,		0.001545,		0.001665,		0.001777,		0.001877,		0.001964,		0.002033,		0.002085,		0.002116,		0.002127,		0.002116,		0.002085,		0.002033,		0.001964,		0.001877,		0.001777,		0.001665,		0.001545,		0.001419,		0.001291},
		{0.001405,		0.001545,		0.001682,		0.001813,		0.001934,		0.002044,		0.002138,		0.002214,		0.00227,		0.002304,		0.002315,		0.002304,		0.00227,		0.002214,		0.002138,		0.002044,		0.001934,		0.001813,		0.001682,		0.001545,		0.001405},
		{0.001514,		0.001665,		0.001813,		0.001954,		0.002085,		0.002203,		0.002304,		0.002386,		0.002446,		0.002483,		0.002496,		0.002483,		0.002446,		0.002386,		0.002304,		0.002203,		0.002085,		0.001954,		0.001813,		0.001665,		0.001514},
		{0.001616,		0.001777,		0.001934,		0.002085,		0.002225,		0.00235,		0.002459,		0.002546,		0.00261,		0.00265,		0.002663,		0.00265,		0.00261,		0.002546,		0.002459,		0.00235,		0.002225,		0.002085,		0.001934,		0.001777,		0.001616},
		{0.001707,		0.001877,		0.002044,		0.002203,		0.00235,		0.002483,		0.002597,		0.00269,		0.002758,		0.0028,		0.002814,		0.0028,		0.002758,		0.00269,		0.002597,		0.002483,		0.00235,		0.002203,		0.002044,		0.001877,		0.001707},
		{0.001786,		0.001964,		0.002138,		0.002304,		0.002459,		0.002597,		0.002717,		0.002814,		0.002885,		0.002928,		0.002943,		0.002928,		0.002885,		0.002814,		0.002717,		0.002597,		0.002459,		0.002304,		0.002138,		0.001964,		0.001786},
		{0.001849,		0.002033,		0.002214,		0.002386,		0.002546,		0.00269,		0.002814,		0.002914,		0.002987,		0.003033,		0.003048,		0.003033,		0.002987,		0.002914,		0.002814,		0.00269,		0.002546,		0.002386,		0.002214,		0.002033,		0.001849},
		{0.001896,		0.002085,		0.00227,		0.002446,		0.00261,		0.002758,		0.002885,		0.002987,		0.003063,		0.003109,		0.003125,		0.003109,		0.003063,		0.002987,		0.002885,		0.002758,		0.00261,		0.002446,		0.00227,		0.002085,		0.001896},
		{0.001925,		0.002116,		0.002304,		0.002483,		0.00265,		0.0028,		0.002928,		0.003033,		0.003109,		0.003156,		0.003172,		0.003156,		0.003109,		0.003033,		0.002928,		0.0028,		0.00265,		0.002483,		0.002304,		0.002116,		0.001925},
		{0.001934,		0.002127,		0.002315,		0.002496,		0.002663,		0.002814,		0.002943,		0.003048,		0.003125,		0.003172,		0.003188,		0.003172,		0.003125,		0.003048,		0.002943,		0.002814,		0.002663,		0.002496,		0.002315,		0.002127,		0.001934},
		{0.001925,		0.002116,		0.002304,		0.002483,		0.00265,		0.0028,		0.002928,		0.003033,		0.003109,		0.003156,		0.003172,		0.003156,		0.003109,		0.003033,		0.002928,		0.0028,		0.00265,		0.002483,		0.002304,		0.002116,		0.001925},
		{0.001896,		0.002085,		0.00227,		0.002446,		0.00261,		0.002758,		0.002885,		0.002987,		0.003063,		0.003109,		0.003125,		0.003109,		0.003063,		0.002987,		0.002885,		0.002758,		0.00261,		0.002446,		0.00227,		0.002085,		0.001896},
		{0.001849,		0.002033,		0.002214,		0.002386,		0.002546,		0.00269,		0.002814,		0.002914,		0.002987,		0.003033,		0.003048,		0.003033,		0.002987,		0.002914,		0.002814,		0.00269,		0.002546,		0.002386,		0.002214,		0.002033,		0.001849},
		{0.001786,		0.001964,		0.002138,		0.002304,		0.002459,		0.002597,		0.002717,		0.002814,		0.002885,		0.002928,		0.002943,		0.002928,		0.002885,		0.002814,		0.002717,		0.002597,		0.002459,		0.002304,		0.002138,		0.001964,		0.001786},
		{0.001707,		0.001877,		0.002044,		0.002203,		0.00235,		0.002483,		0.002597,		0.00269,		0.002758,		0.0028,		0.002814,		0.0028,		0.002758,		0.00269,		0.002597,		0.002483,		0.00235,		0.002203,		0.002044,		0.001877,		0.001707},
		{0.001616,		0.001777,		0.001934,		0.002085,		0.002225,		0.00235,		0.002459,		0.002546,		0.00261,		0.00265,		0.002663,		0.00265,		0.00261,		0.002546,		0.002459,		0.00235,		0.002225,		0.002085,		0.001934,		0.001777,		0.001616},
		{0.001514,		0.001665,		0.001813,		0.001954,		0.002085,		0.002203,		0.002304,		0.002386,		0.002446,		0.002483,		0.002496,		0.002483,		0.002446,		0.002386,		0.002304,		0.002203,		0.002085,		0.001954,		0.001813,		0.001665,		0.001514},
		{0.001405,		0.001545,		0.001682,		0.001813,		0.001934,		0.002044,		0.002138,		0.002214,		0.00227,		0.002304,		0.002315,		0.002304,		0.00227,		0.002214,		0.002138,		0.002044,		0.001934,		0.001813,		0.001682,		0.001545,		0.001405},
		{0.001291,		0.001419,		0.001545,		0.001665,		0.001777,		0.001877,		0.001964,		0.002033,		0.002085,		0.002116,		0.002127,		0.002116,		0.002085,		0.002033,		0.001964,		0.001877,		0.001777,		0.001665,		0.001545,		0.001419,		0.001291},
		{0.001174,		0.001291,		0.001405,		0.001514,		0.001616,		0.001707,		0.001786,		0.001849,		0.001896,		0.001925,		0.001934,		0.001925,		0.001896,		0.001849,		0.001786,		0.001707,		0.001616,		0.001514,		0.001405,		0.001291,		0.001174}
	};*/

	//vars used in calculation
	int i, j, a, offset, k = 0, b = 0;
	float w, sum = 0.0, distance, calculatedEuler = 1.0 / (2.0 * M_PI, powf(radius / 3.0, 2));
	offset = radius;

	//populate kernel
	for(i = 0; i < (2 * radius + 1); i++){
		for(j = 0; j < (2 * radius + 1); j++){
			if(type == 0){
				kernel[i][j] = 1.0 / ((2.0 * radius + 1.0) * (2.0 * radius + 1.0));
			} else if (type == 1){
				distance = (i * i) + (j * j) /(2 * (radius / 3.0) * (radius / 3.0));
				kernel[i][j] = calculatedEuler * exp(-1.0 * distance);
				sum = sum + kernel[i][j];
			} else {
				//sharpen
			}
		}
	}
	if(type == 1){
		for(i = 0; i < (2 * radius + 1); i++){
			for(j = 0; j < (2 * radius + 1); j++){
				kernel[i][j] = (1.0 / sum) * kernel[i][j];
			}
		}
	}

	//convert 1D input array to a 2D array
	for(i = 0; i < height; i++){
		for(j = 0; j < 3 * width; j++){
			tempData[i][j] = data[k];
			k++;
		}
	}

	//actual convolution
	//red
	for(k = 0; k < height; k++){
		for(a = 0; a < width; a = a + 3){
			for(i = (k - 3 * offset); i < (k + 3 * offset + 3); i = i + 3){
				for(j = (a - 3 * offset); j <  (a + 3 * offset + 3); j = j + 3){

					//calculate weight for bilinear filter
					// w = tempData[k][a] - tempData[i][j] / 2.0;
					// w = w * w;
					// if(w < 0.0) w = 0.0;
					// else if (w > 1.0) w = 1.0;
					// w = exp(-1.0 * w);

					//handling boundaries; uses boundary padding through reflection for its boundary condition
					if(i < 0 && j < 0) sum = sum + w * tempData[i + offset][j + offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i < 0 && j < (width - 1)) sum = sum + w * tempData[i + offset][j] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i < (height - 1) && j < 0) sum = sum + w * tempData[i][j + offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i < 0 && j > (width - 1)) sum = sum + w * tempData[i + offset][j - offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i > (height - 1) && j < 0) sum = sum + w * tempData[i - offset][j + offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i > (height - 1) && j > (width - 1)) sum = sum + w * tempData[i - offset][j - offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i > (height - 1)) sum = sum + w * tempData[i - offset][j] * kernel[i - (k - offset)][j - (a - offset)];
					else if (j > (width - 1)) sum = sum + w * tempData[i][j - offset] * kernel[i - (k - offset)][j - (a - offset)];
					else sum = sum + w * tempData[i][j] * kernel[i - (k - offset)][j - (a - offset)];
				}
			}
			//store summation for each pixel in the pixel location
			out[b] = sum;
			b = b + 3;

			//reset sum
			sum = 0.0;
		}
	}

	//green
	b = 1;
	for(k = 0; k < height; k++){
		for(a = 1; a < width; a = a + 3){
			for(i = (k - 3 * offset + 1); i < (k + 3 * offset + 4); i = i + 3){
				for(j = (a - 3 * offset + 1); j <  (a + 3 * offset + 4); j = j + 3){

					//calculate weight for bilinear filter
					// w = tempData[k][a] - tempData[i][j] / 2.0;
					// w = w * w;
					// if(w < 0.0) w = 0.0;
					// else if (w > 1.0) w = 1.0;
					// w = exp(-1.0 * w);

					//handling boundaries; uses boundary padding through reflection for its boundary condition
					if(i < 0 && j < 0) sum = sum + w * tempData[i + offset][j + offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i < 0 && j < (width - 1)) sum = sum + w * tempData[i + offset][j] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i < (height - 1) && j < 0) sum = sum + w * tempData[i][j + offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i < 0 && j > (width - 1)) sum = sum + w * tempData[i + offset][j - offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i > (height - 1) && j < 0) sum = sum + w * tempData[i - offset][j + offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i > (height - 1) && j > (width - 1)) sum = sum + w * tempData[i - offset][j - offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i > (height - 1)) sum = sum + w * tempData[i - offset][j] * kernel[i - (k - offset)][j - (a - offset)];
					else if (j > (width - 1)) sum = sum + w * tempData[i][j - offset] * kernel[i - (k - offset)][j - (a - offset)];
					else sum = sum + w * tempData[i][j] * kernel[i - (k - offset)][j - (a - offset)];
				}
			}
			//store summation for each pixel in the pixel location
			out[b] = sum;
			b = b + 3;

			//reset sum
			sum = 0.0;
		}
	}

	b = 2;
	//blue
	for(k = 0; k < height; k++){
		for(a = 2; a < width; a = a + 3){
			for(i = (k - 3 * offset + 2); i < (k + 3 * offset + 5); i = i + 3){
				for(j = (a - 3 * offset + 2); j <  (a + 3 * offset + 5); j = j + 3){

					//calculate weight for bilinear filter
					// w = tempData[k][a] - tempData[i][j] / 2.0;
					// w = w * w;
					// if(w < 0.0) w = 0.0;
					// else if (w > 1.0) w = 1.0;
					// w = exp(-1.0 * w);

					//handling boundaries; uses boundary padding through reflection for its boundary condition
					if(i < 0 && j < 0) sum = sum + w * tempData[i + offset][j + offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i < 0 && j < (width - 1)) sum = sum + w * tempData[i + offset][j] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i < (height - 1) && j < 0) sum = sum + w * tempData[i][j + offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i < 0 && j > (width - 1)) sum = sum + w * tempData[i + offset][j - offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i > (height - 1) && j < 0) sum = sum + w * tempData[i - offset][j + offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i > (height - 1) && j > (width - 1)) sum = sum + w * tempData[i - offset][j - offset] * kernel[i - (k - offset)][j - (a - offset)];
					else if (i > (height - 1)) sum = sum + w * tempData[i - offset][j] * kernel[i - (k - offset)][j - (a - offset)];
					else if (j > (width - 1)) sum = sum + w * tempData[i][j - offset] * kernel[i - (k - offset)][j - (a - offset)];
					else sum = sum + w * tempData[i][j] * kernel[i - (k - offset)][j - (a - offset)];
				}
			}
			//store summation for each pixel in the pixel location
			out[b] = sum;
			b = b + 3;

			//reset sum
			sum = 0.0;
		}
	}
}

//
// Tone Maps the HDR data by applying gamma correction
// \param data data to gamma correct
// \param gamma value to correct by
// \param size size of data
//
float* toneMap(float* data, float gamma, float gain, float bias, int size){
	//To store L in
	float* lumData = new float[size];

	//Store L-corrected in
	float* lum2 = new float[size];

	//output data
	float* newData = new float[3*size];

	//vars used for calcuation
	float scale;
	int i = 0;
	float r, g, b;

	//the loop itself
	for(i = 0; i < size; i++){
		//loading in values
		r = data[3 * i];
		g = data[(3 * i) + 1];
		b = data[(3 * i) + 2];

		//calculating L
		lumData[i] = (1.0 / 61.0) * (20.0 * r + 40.0 * g + b);

		//calcualating L corrected
		lum2[i] = powf((gain * lumData[i] + bias), gamma);

		//calculating scale to correct by
		scale = lum2[i] / lumData[i];

		//correcting original values and clamping
		r = r * scale;
		if(r > 255) r = 255;
		else if (r < 0.0) r = 0.0;
		g = g * scale;
		if(g > 255) g = 255;
		else if (g < 0.0) g = 0.0;
		b = b * scale;
		if(b > 255) b = 255;
		else if (b < 0.0) b = 0.0;

		//saving corrected values
		newData[3 * i] = round(r);
		newData[(3 * i) + 1] = round(g);
		newData[(3 * i) + 2] = round(b);
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

	//vars used for new code--loading in image, perform corrections, display and output.
	float gamma = 1.0, bias = 1.0, gain = 1.0;
	ppm* image = new ppm();
	int width, height, radius = 1, type = -1;
	float* data;
	float* newData;

  //Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	//setup for loading image; create new object and check commandline args
	if(argc < 3){
		cout << "usage: prog02 input output" << endl;
		exit(EXIT_FAILURE);
	}

	//Try to figure out if it's a ppm or a hdr image

	//read in image data if ppm
	image->readData(argv[1]);

 //create window for the image, then check to make sure it loaded properly
 SDL_Window *windowImage = SDL_CreateWindow("Loaded Image", 100, 100, image->returnWidth(), image->returnHeight(), SDL_WINDOW_SHOWN);
 if (windowImage == NULL){
	 logSDLError(std::cout, "CreateWindowImage");
	 SDL_Quit();
	 return 1;
 }

 //ditto, but renderer instead of window
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
						//if left arrow pressed, decrease gamma by 0.1 and tone map image again
						gamma = gamma - 0.1;
						newData = toneMap((float*)image->returnData(), gamma, gain, bias, width * height);
						break;
					case SDLK_RIGHT:
						//same as for left arrow, but for right arrow and increase gamma instead of decrease
						gamma = gamma + 0.1;
						newData = toneMap((float*)image->returnData(), gamma, gain, bias, width * height);
						break;
					case SDLK_DOWN:
						//if left arrow pressed, decrease gamma by 0.1 and tone map image again
						gain = gain - 0.1;
						newData = toneMap((float*)image->returnData(), gamma, gain, bias, width * height);
						break;
					case SDLK_UP:
						//same as for left arrow, but for right arrow and increase gamma instead of decrease
						gain = gain + 0.1;
						newData = toneMap((float*)image->returnData(), gamma, gain, bias, width * height);
						break;
					case SDLK_a:
						//if left arrow pressed, decrease gamma by 0.1 and tone map image again
						bias = bias - 0.1;
						newData = toneMap((float*)image->returnData(), gamma, gain, bias, width * height);
						break;
					case SDLK_d:
						//same as for left arrow, but for right arrow and increase gamma instead of decrease
						bias = bias + 0.1;
						newData = toneMap((float*)image->returnData(), gamma, gain, bias, width * height);
						break;
					case SDLK_v:
						//if left arrow pressed, decrease gamma by 0.1 and tone map image again
						radius = radius - 1;
						if(radius < 1) radius = 1;
						convolution((float*)image->returnData(), image->returnWidth(), image->returnHeight(), newData, radius, type);
						break;
					case SDLK_b:
						//same as for left arrow, but for right arrow and increase gamma instead of decrease
						radius = radius + 1;
						convolution((float*)image->returnData(), image->returnWidth(), image->returnHeight(), newData, radius, type);
						break;
					case SDLK_n:
						//if left arrow pressed, decrease gamma by 0.1 and tone map image again
						type = type - 1;
						if(radius < 0) type = 0;
						convolution((float*)image->returnData(), image->returnWidth(), image->returnHeight(), newData, radius, type);
						break;
					case SDLK_m:
						//same as for left arrow, but for right arrow and increase gamma instead of decrease
						type = type + 1;
						if(type > 2) type = 2;
						convolution((float*)image->returnData(), image->returnWidth(), image->returnHeight(), newData, radius, type);
						break;
          default:
            break;
        }
      }
    }
		//if not a ppm, store data in one and update the texture with it
		SDL_UpdateTexture(imageTexture, NULL, (unsigned char*)newData, 3*image->returnWidth());

			//render loaded texture here
		renderTexture(imageTexture, rendererImage, 0, 0);

			//Update the screen
		SDL_RenderPresent(rendererImage);

    //Display the frame rate to stdout, as well as current gamma value
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

	//write data to a SDR ppm
	image->setData((unsigned char*)newData);
	image->writeData(argv[2]);

	//clear memory
	delete image;

  return 0;
}
