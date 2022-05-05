#include <stdint.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <math.h>
#include "headers/MLX90640_API.h"

// ===== 24-bit ANSI Colours Version =====
// Requires a compatible terminal (e.g. Konsole).
//
// (Colour gradient code from `interp.cpp`)


// ### ANSI Formatting ###
// Ref: https://en.wikipedia.org/wiki/ANSI_escape_code#SGR
// This example code originally used "\x1b[..." instead of "\e[...", but they should be equivalent.

// ## Regular Formatting Codes ##
// "\e[m" is the same as "\e[0m"
#define ANSI_NORMAL                         "\e[m"
#define ANSI_RESET                          "\e[m"
#define ANSI_BOLD                           "\e[1m"
#define ANSI_FAINT                          "\e[2m"
#define ANSI_ITALIC                         "\e[3m"
#define ANSI_UNDERLINE                      "\e[4m"
#define ANSI_BLINK_SLOW                     "\e[5m"
#define ANSI_BLINK_FAST                     "\e[6m"
#define ANSI_INVERT                         "\e[7m"
#define ANSI_HIDE                           "\e[8m"
#define ANSI_STRIKE                         "\e[9m"
#define ANSI_UNDERLINEX2                    "\e[21m"
#define ANSI_NOT_BOLD_FAINT                 "\e[22m"
#define ANSI_NOT_ITALIC                     "\e[23m"
#define ANSI_NOT_UNDERLINE                  "\e[24m"
#define ANSI_NOT_BLINK                      "\e[25m"
#define ANSI_NOT_INVERT                     "\e[27m"
#define ANSI_NOT_HIDE                       "\e[28m"
#define ANSI_NOT_STRIKE                     "\e[29m"
#define ANSI_FRAME                          "\e[51m"
#define ANSI_ENCIRCLE                       "\e[52m"
#define ANSI_NOT_FRAME_ENCIRCLE             "\e[54m"
#define ANSI_OVERLINE                       "\e[53m"
#define ANSI_NOT_OVERLINE                   "\e[55m"

// ## Colours - Foreground (`ANSI_C_*`) ##
// 90-97 (bright colours) are technically non-standard (but widely-supported?)
#define ANSI_C_BLACK                        "\e[30m"
#define ANSI_C_RED                          "\e[31m"
#define ANSI_C_GREEN                        "\e[32m"
#define ANSI_C_YELLOW                       "\e[33m"
#define ANSI_C_BLUE                         "\e[34m"
#define ANSI_C_MAGENTA                      "\e[35m"
#define ANSI_C_CYAN                         "\e[36m"
#define ANSI_C_WHITE                        "\e[37m"
#define ANSI_C_BBLACK                       "\e[90m"
#define ANSI_C_BRED                         "\e[91m"
#define ANSI_C_BGREEN                       "\e[92m"
#define ANSI_C_BYELLOW                      "\e[93m"
#define ANSI_C_BBLUE                        "\e[94m"
#define ANSI_C_BMAGENTA                     "\e[95m"
#define ANSI_C_BCYAN                        "\e[96m"
#define ANSI_C_BWHITE                       "\e[97m"

// ## Colours - Background (`ANSI_C_BG_*`) ##
// 100-107 (bright colours) are technically non-standard (but widely-supported?)
#define ANSI_C_BG_BLACK                     "\e[40m"
#define ANSI_C_BG_RED                       "\e[41m"
#define ANSI_C_BG_GREEN                     "\e[42m"
#define ANSI_C_BG_YELLOW                    "\e[43m"
#define ANSI_C_BG_BLUE                      "\e[44m"
#define ANSI_C_BG_MAGENTA                   "\e[45m"
#define ANSI_C_BG_CYAN                      "\e[46m"
#define ANSI_C_BG_WHITE                     "\e[47m"
#define ANSI_C_BG_BBLACK                    "\e[100m"
#define ANSI_C_BG_BRED                      "\e[101m"
#define ANSI_C_BG_BGREEN                    "\e[102m"
#define ANSI_C_BG_BYELLOW                   "\e[103m"
#define ANSI_C_BG_BBLUE                     "\e[104m"
#define ANSI_C_BG_BMAGENTA                  "\e[105m"
#define ANSI_C_BG_BCYAN                     "\e[106m"
#define ANSI_C_BG_BWHITE                    "\e[107m"


// ## Custom Colours (8-Bit/24-Bit) ##
//
// **WARNING:** these examples+notes were written for PHP and are untested in C++!
//
// These CANNOT be used on their own!
// The names end with a trailing "_" to indicate their incompleteness.
//
// ## 8-Bit Colours (aka 256-Colour) ##
// `ANSI_C8_`/`ANSI_C8_BG_` + int 0-255 + "m".
// The 0-255 int is one of:
//    0-  7: standard colours, as per 30-37.
//    8- 15: "bright" colours, as per 90-97.
//   16-231: 6×6×6 cube (216 colours): 16 + (36×r) + (6×g) + b [where r,g,b are 0-5]
//  232-255: greyscale; black-to-white; 24 steps.
// See: https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit
//
// 8-Bit Example:
//   printf("%s%dmHello world%s\n", ANSI_C8_, 93, ANSI_RESET); // purple text
//
// ## 24-Bit Colours ##
// `ANSI_C24_`/`ANSI_C24_BG_` + R;G;B as 0-255 ints + "m".
//
// 24-Bit Example:
//   printf("%s135;0;255mHello world%s\n", ANSI_C24_, ANSI_RESET); // purple text: rgb(135,0,255)
//   printf("%s%d;%d;%dmHello world%s\n", ANSI_C24_, 135, 0, 255, ANSI_RESET); // purple text: rgb(135,0,255)
// 24-Bit Example (Hex Colour Code):
//   printf("%s%d;%d;%dmHello world%s\n", ANSI_C24_, 0x87, 0x0, 0xff, ANSI_RESET); // purple text: #8700ff
#define ANSI_C8_                            "\e[38;5;"
#define ANSI_C8_BG_                         "\e[48;5;"
#define ANSI_C24_                           "\e[38;2;"
#define ANSI_C24_BG_                        "\e[48;2;"

// "Default" foreground/background colour ("implementation defined")
#define ANSI_C_DEFAULT                      "\e[39m"
#define ANSI_C_BG_DEFAULT                   "\e[49m"

// ## Non-standard! ##
// Only implemented in: mintty
#define ANSI_SUPERSCRIPT                    "\e[73m"
#define ANSI_SUBSCRIPT                      "\e[74m"
#define ANSI_NOT_SUPERSCRIPT_SUBSCRIPT      "\e[75m"
// Only implemented in: KiTTY, VTE, mintty, iTerm2
// `…_C8_`/`…_C24_` require the same int arguments as `ANSI_C8_`/`ANSI_C24_`.
// Examples:
//   printf("%s%dmHello world%s\n", ANSI_UNDERLINE_C8_, 93, ANSI_UNDERLINE_COLOUR_DEFAULT); // purple _underline_
//   printf("%s%d;%d;%dmHello world%s\n", ANSI_UNDERLINE_C24_, 135, 0, 255, ANSI_UNDERLINE_COLOUR_DEFAULT); // purple _underline_
#define ANSI_UNDERLINE_C8_                  "\e[58;5;"
#define ANSI_UNDERLINE_C24_                 "\e[58;2;"
#define ANSI_UNDERLINE_COLOUR_DEFAULT       "\e[59m"


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_NONE    "\x1b[30m"
#define ANSI_COLOR_RESET   "\x1b[0m"


// ####################################
// ### SELECT OUTPUT FORMAT ###
// ####################################
// Always sign-prefixed ('+'/'-'); 6 chars long inc +/- & dot; left-space-padded.
// eg "+12.34", "-23.45", "+345.67".
// Values 100+ are longer than others, breaking layout!
//#define FMT_STRING "%+06.2f "

// Ints only; no '+' prefix (only '-'); 3 chars long; left-space-padded.
// eg " 12", "123", "-23", "345".
// 3-digit -ve values are rare; 3-digit +ve values are common.
//  So 3 chars for +ve & -ve values should suffice for this use.
//  That also makes it easier to fit on my 320x240 SPI TFT without
//  shrinking the font beyond legibility!
//#define FMT_STRING "%3.0f "

// Same as above, but always showing 3 digits to make the text display
//  look more like an image.
// Ints only; no '+' prefix (only '-'); 3 chars long; left-zero-padded.
// eg "012", "123", "-23", "345".
// 3-digit -ve values are rare; 3-digit +ve values are common.
//  So 3 chars for +ve & -ve values should suffice for this use.
//  That also makes it easier to fit on my 320x240 SPI TFT without
//  shrinking the font beyond legibility!
#define FMT_STRING " %03.0f "

// *shrug* Some kind of symbol/visual display?
//#define FMT_STRING "\u2588\u2588\u2588"


// ### BLANK OUTPUT FOR DOUBLED ROWS ###
// Must be same length as FMT_STRING generates.
//
// 5 spaces - for 3-digit formats
#define FMT_BLANK "     "


#define DOUBLE_ROWS 1


#define TEMP_RANGE_LIMIT_MIN 0.0
#define TEMP_RANGE_LIMIT_MAX 50.0

#define MLX_I2C_ADDR 0x33

// Valid frame rates are 1, 2, 4, 8, 16, 32 and 64
// The i2c baudrate is set to 1mhz to support these
#define FPS 64

//void get_ansi_24bit_false_colour(double v, float vMin, float vMax, int *ir, int *ig, int *ib, int palette) {
// TODO not sure if defining an array of all palettes, and choosing one via func arg, is possible/practical unless all palettes were the same size...
void print_ansi_24bit_false_colour(double v) {
    // Heatmap code borrowed from: http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients
    
    // ## Colour Palette Definition ##
    // (Each colour defined as {R,G,B} as 0.0~1.0 floats)
    
    // 7 colours: black, blue, green, yellow, red, magenta, white
    // (default from interp.cpp example code)
    const int NUM_COLORS = 7;
    static float color[NUM_COLORS][3] = { {0,0,0}, {0,0,1}, {0,1,0}, {1,1,0}, {1,0,0}, {1,0,1}, {1,1,1} };

    // 7 colours: black, blue, cyan, green, yellow, red, white
    // (7-colour example from andrewnoske.com)
    //const int NUM_COLORS = 7;
    //static float color[NUM_COLORS][3] = { {0,0,0}, {0,0,1}, {0,1,1}, {0,1,0}, {1,1,0}, {1,0,0}, {1,1,1} };

    // 5 colours: blue, cyan, green, yellow, red
    // (5-colour example from andrewnoske.com)
    //const int NUM_COLORS = 5;
    //static float color[NUM_COLORS][3] = { {0,0,1}, {0,1,1}, {0,1,0}, {1,1,0}, {1,0,0} };

    // 5 colours: blue, green, yellow, red, magenta
    // (default from interp.cpp example code, with black+white removed)
    //const int NUM_COLORS = 5;
    //static float color[NUM_COLORS][3] = { {0,0,1}, {0,1,0}, {1,1,0}, {1,0,0}, {1,0,1} };
    
    
    int idx1, idx2;          // Our desired color will be between these two indexes in "color".
    float fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.
    
    // Normalise v (degC float, likely between -20.0 and +300.0) to a 0.0~1.0 float.
    float vMin = 0.0;        // Temperature represented by coldest colour in palette.
    float vMax = 100.0;      // Temperature represented by warmest colour in palette.
    float vNorm = (v - vMin) / (vMax - vMin);
    //float vRange = vMax-vMin;
    //v -= vMin;
    //v /= vRange;
    
    if (vNorm <= 0) { idx1 = idx2 = 0; }                   // accounts for input <=0
    else if (vNorm >= 1) { idx1 = idx2 = NUM_COLORS-1; }   // accounts for input >=1
    else
    {
        vNorm *= (NUM_COLORS-1);
        idx1 = floor(vNorm);                 // Our desired color will be after this index.
        idx2 = idx1+1;                   // ... and before this index (inclusive).
        fractBetween = vNorm - float(idx1);  // Distance between the two indexes (0-1).
    }

    int ir, ig, ib;  // Calculated 0~255 R,G,B values

    ir = (int)((((color[idx2][0] - color[idx1][0]) * fractBetween) + color[idx1][0]) * 255.0);
    ig = (int)((((color[idx2][1] - color[idx1][1]) * fractBetween) + color[idx1][1]) * 255.0);
    ib = (int)((((color[idx2][2] - color[idx1][2]) * fractBetween) + color[idx1][2]) * 255.0);

    printf("%d;%d;%d", ir, ig, ib);
}

int main(){
    int state = 0;
    int outLines = 0; // How many lines does the cursor need to move up later?

    printf("Starting...\n");
    outLines++;
    static uint16_t eeMLX90640[832];
    float emissivity = 1;
    uint16_t frame[834];
    static float image[768];
    float eTa;
    static uint16_t data[768*sizeof(float)];

    std::fstream fs;

    MLX90640_SetDeviceMode(MLX_I2C_ADDR, 1); // ???
    MLX90640_SetSubPageRepeat(MLX_I2C_ADDR, 1); // ???
    switch(FPS){
        case 1:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b001);
            break;
        case 2:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b010);
            break;
        case 4:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b011);
            break;
        case 8:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b100);
            break;
        case 16:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b101);
            break;
        case 32:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b110);
            break;
        case 64:
            MLX90640_SetRefreshRate(MLX_I2C_ADDR, 0b111);
            break;
        default:
            printf("Unsupported framerate: %d", FPS);
            return 1;
    }
    MLX90640_SetChessMode(MLX_I2C_ADDR);
    printf("Configured...\n");
    outLines++;

    paramsMLX90640 mlx90640;
    MLX90640_DumpEE(MLX_I2C_ADDR, eeMLX90640);
    MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
    int refresh = MLX90640_GetRefreshRate(MLX_I2C_ADDR);
    printf("EE Dumped...\n");
    outLines++;

    int frames = 30;
    int subpage;
    int skipSubpages = 6; // how many subpages/fields to skip, to bypass data full of NaN? 1 full frame (2 fields) seems to be enough
    static float mlx90640To[768];

    MLX90640_StartMeasurement(MLX_I2C_ADDR, 0);

    // Erase above progress/status lines
    printf("\e[%dA", outLines); // move cursor up n lines
    for (int i=0; i<outLines; i++) {
        printf("% 60s\n", " "); // overwrite each line with spaces
    }
    printf("\e[%dA", outLines);
    outLines = 0;

    while (1){
        while (!MLX90640_CheckInterrupt(MLX_I2C_ADDR)){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        outLines = 0; // How many lines does the cursor need to move up later?

        MLX90640_GetData(MLX_I2C_ADDR, frame);
        // MLX90640_InterpolateOutliers(frame, eeMLX90640);
        subpage = MLX90640_GetSubPageNumber(frame);

        // Start the next meausrement
        MLX90640_StartMeasurement(MLX_I2C_ADDR, !subpage);

        eTa = MLX90640_GetTa(frame, &mlx90640);
        MLX90640_CalculateTo(frame, &mlx90640, emissivity, eTa, mlx90640To);
        MLX90640_BadPixelsCorrection((&mlx90640)->brokenPixels, mlx90640To, 1, &mlx90640);
        MLX90640_BadPixelsCorrection((&mlx90640)->outlierPixels, mlx90640To, 1, &mlx90640);

        //printf("Subpage: %d\n", subpage);
        //outLines++;

        if (skipSubpages > 0) {
            // Need to skip this field as it's likely full of NaN values
            printf("Skipping subpage... (%d)\n", skipSubpages);
            outLines++;
            skipSubpages--;
            printf("\e[%dA", outLines);
            //printf("\x1b[2A"); // move up 2 lines
            continue;
        }

        // Determine min & max temperatures for colour palette range
        float vMin = TEMP_RANGE_LIMIT_MIN,
              vMax = TEMP_RANGE_LIMIT_MAX;
        for (int i = 0; i < 768; i++) {
            // Which is faster..?
            //float v = mlx90640To[i];
            //if (v < vMin)
            //    vMin = v;
            //else if (v > vMax)
            //    vMax = v;
            if (mlx90640To[i] < vMin)
                vMin = mlx90640To[i];
            else if (mlx90640To[i] > vMax)
                vMax = mlx90640To[i];
        }
        //printf("Temp. range: %3.0f°C ~ %3.0f°C\n", vMin, vMax);
        //printf("%sTemp. range:%s %s%3.0f%s°C ~ %s%3.0f%s°C\n", ANSI_C_BWHITE, ANSI_RESET, ANSI_C_BCYAN, vMin, ANSI_RESET, ANSI_C_BMAGENTA, vMax, ANSI_RESET);
        printf("%sTemp. range:%s %s%3.0f%s°C ~ %s%3.0f%s°C\n", ANSI_BOLD ANSI_C_BWHITE, ANSI_RESET, ANSI_BOLD ANSI_C_BCYAN, vMin, ANSI_RESET, ANSI_BOLD ANSI_C_BMAGENTA, vMax, ANSI_RESET);
        outLines++;

        // Coloured background; bold, bright white text
        //printf(ANSI_BOLD ANSI_C_BWHITE);
        printf("%s%s", ANSI_BOLD, ANSI_C_BWHITE);

        for(int x = 0; x < 32; x++){
            //int rowCount = ( DOUBLE_ROWS ? 2 : 1 );
            //for (int z = 0; z < rowCount; z++) {
            for (int z = 0; z < ( DOUBLE_ROWS ? 2 : 1 ); z++) {
                // Example script displayed y=0 through y=23, but this gives a mirrored image.
                for(int y = 0; y < 24; y++){
                //for (int y = 23; y >= 0; y--) {
                    //std::cout << image[32 * y + x] << ",";
                    //float val = mlx90640To[32 * (23-y) + x];
                    float val = mlx90640To[32 * y + x];
                    int ansiR=0, ansiG=0, ansiB=0;

                    // (No need to write ANSI_RESET after each pixel...)
                
                    // Coloured *text* (foreground)
                    //printf(ANSI_C24_); // "\e[38;2;"
                    //print_ansi_24bit_false_colour(val); // "128;192;255" [R;G;B]
                    //printf("m");
                    //printf(FMT_STRING, val);
                    ////printf("m" FMT_STRING, val);
                
                    // ...alternatively...
                    // Coloured *background*
                    //print_ansi_24bit_false_colour(val, ansiR, ansiG, ansiB);
                    //printf(ANSI_BOLD ANSI_C_BWHITE ANSI_C24_BG_ "%d;%d;%dm" FMT_STRING, ansiR, ansiG, ansiB, val);
                    printf(ANSI_BOLD ANSI_C_BWHITE ANSI_C24_BG_); // "\e[48;2;"
                    print_ansi_24bit_false_colour(val); // "128;192;255" [R;G;B]
                    printf("m");
                    if (z == 0) {
                        printf(FMT_STRING, val);
                    } else {
                        printf(FMT_BLANK);
                    }
                    //printf("m" FMT_STRING, val);
                }
                //printf(ANSI_C_DEFAULT);
                printf(ANSI_C_BG_DEFAULT);
                std::cout << std::endl;
                outLines++;
            }
        }
        printf(ANSI_RESET);

        //printf("\x1b[33A"); // Move cursor UP by 33
        printf("\e[%dA", outLines); // Move cursor UP by x lines
    }
    return 0;
}
