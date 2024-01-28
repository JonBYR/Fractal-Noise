#include <SFML/Graphics.hpp>
#include "Math.h"
typedef struct { //creates vector struct to be used for dot product
    float x, y;
} vector2;
//computes dot product of distance vector and gradient vector
vector2 randomGradient(int ix, int iy) {
    // No precomputed gradients mean this works for any number of grid coordinates
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;
    unsigned a = ix, b = iy;
    a *= 3284157443;

    b ^= a << s | a >> w - s;
    b *= 1911520717;

    a ^= b << s | b >> w - s;
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

    // Create the vector from the angle
    vector2 v;
    v.x = sin(random);
    v.y = cos(random);

    return v;
} //hashing function to create pseudorandom unit vector in each grid cell 

float dotGridGradient(int ix, int iy, float x, float y) 
{
    vector2 grad = randomGradient(ix, iy); //gets gradient from integer coordinates
    //distance vectors
    float dx = x - (float)ix;
    float dy = y - (float)iy; //distance between point x,y in grid cell and corners of grid cell
    return (dx * grad.x + dy * grad.y); //dot product between distance vector and unit vector
}
float interpolate(float a0, float a1, float w) {
    return (a1 - a0) * (3 - w * 2.0) * w * w + a0; //cubic interpolation formula
}
float perlin(float x, float y) //x and y are a specific point in each grid cell
{
    int x0 = (int)x;
    int y0 = (int)y; //determine the corners of the grid cell
    int x1 = x0 + 1;
    int y1 = y0 + 1; //corners of the other side of the grid (length between both is 1
    float sx = x - (float)x0;
    float sy = y - (float)y0; //compute interpolation weights
    float n0 = dotGridGradient(x0, y0, x, y);
    float n1 = dotGridGradient(x1, y0, x, y);
    float ix0 = interpolate(n0, n1, sx); //compute dot product for top two corners and interpolate between these two values
    float n2 = dotGridGradient(x0, y1, x, y);
    float n3 = dotGridGradient(x1, y1, x, y);
    float ix1 = interpolate(n2, n3, sx); //compute dot product for bottom two corners and interpolate between these two values
    //both interpolations are computed horizontally
    float value = interpolate(ix0, ix1, sy); //interpolate between two horizontal values to get interpolation on y axis
    return value; //value is the perlin noise for this point
}
int main()
{
    const int windowWidth = 1920;
    const int windowHeight = 1080;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Testing Perlin");
    sf::Uint8* pixels = new sf::Uint8[windowWidth * windowHeight * 4]; //generates pixels with a RGB colour channel
    const int GRID_SIZE = 400; //frequency of noise == grid size
    for (int i = 0; i < windowWidth; i++) {
        for (int j = 0; j < windowHeight; j++) { //loops through each grid cell
            int index = (j * windowWidth + i) * 4; //gets current pixel
            float val = 0;
            float freq = 1;
            float amp = 1; //amplitude is equal to the the verticality of the grid map (i.e higher values mean higher inclines and lower values mean flatter terain)
            int octaves = 12; //number of interations
            for (int o = 0; o < octaves; o++) {
                val += perlin(i * freq / GRID_SIZE, j * freq / GRID_SIZE) * amp; //gets value of perlin noise after multiple iterations
                freq *= 2; //lacurity (amount frequency grows after each octave)
                amp /= 2; //persistance (amount amplitude shrinks after each octave)
            }
            val *= 1.2; //contrast
            if (val > 1.0f) val = 1.0f;
            else if (val < -1.0f) val = -1.0f; //perlin noise is between -1 and 1 so any smaller/larger values are 'clipped'
            int colour = (int)(((val + 1.0f) * 0.5f) * 255); //converts -1 to 1 into 0-255 for colour values
            pixels[index] = colour;
            pixels[index + 1] = colour;
            pixels[index + 2] = colour; //pixel is set to a greyscale value to show noise
            pixels[index + 3] = 255; //alpha channel
        }
    }
    sf::Texture texture;
    sf::Sprite sprite;
    texture.create(windowWidth, windowHeight);
    texture.update(pixels); //texture contains all pixels that have been manipulated with perlin noise
    sprite.setTexture(texture);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}