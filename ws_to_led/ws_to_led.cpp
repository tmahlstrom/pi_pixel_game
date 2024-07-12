#include "led-matrix.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>
#include <csignal>
#include <vector>
#include <sstream>
#include <thread>
#include <functional>

#include "graphics.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace rgb_matrix;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::server<websocketpp::config::asio> server;

// Constants for the LED matrix dimensions
const int ROWS = 64;
const int COLS = 256;

bool initial_message_recieved = false;

volatile bool interrupt_received = false;

static void InterruptHandler(int signo) {
    interrupt_received = true;
}

void displayRGBArrays(const std::vector<std::vector<int>>& arrays, Canvas *canvas) {
    if (arrays.size() != ROWS * COLS) {
        std::cerr << "Invalid number arrays dimensions." << std::endl;
        std::cerr.flush();
        return;
    }

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            int index = row * COLS + col;
            if (index >= arrays.size()) {
                std::cerr << "Index out of bounds: " << index << std::endl;
                continue;
            }
            int r = arrays[index][2];
            int g = arrays[index][1];
            int b = arrays[index][0];

            // Log the coordinates and color values for debugging
            // std::cout << "Setting pixel at (" << col << ", " << row << ") to RGB(" << r << ", " << g << ", " << b << ")" << std::endl;

            // Calculate flipped row index
            int flippedRow = ROWS - 1 - row;

            // Set the pixel on the canvas

            canvas->SetPixel(col, flippedRow, r, g, b); // Flipped along Y-axis

        }
    }
}

std::vector<std::vector<int>> parseData(const std::string& data) {
    std::vector<std::vector<int>> rgbArrays;

    // Interpret the data as bytes
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data.data());
    size_t size = data.size();

    // Each pixel data consists of 3 bytes (R, G, B)
    for (size_t i = 0; i < size; i += 3) {
        if (i + 2 < size) {
            std::vector<int> pixelData;
            pixelData.push_back(bytes[i]);     // Red
            pixelData.push_back(bytes[i + 1]); // Green
            pixelData.push_back(bytes[i + 2]); // Blue
            rgbArrays.push_back(pixelData);
        } else {
            // Handle incomplete data (if any)
            break;
        }
    }

    return rgbArrays;
}

void on_message(server* s, websocketpp::connection_hdl hdl, server::message_ptr msg, Canvas *canvas) {
    initial_message_recieved = true;
    std::string data = msg->get_payload();
    std::vector<std::vector<int>> rgbArrays = parseData(data);
    displayRGBArrays(rgbArrays, canvas);
    // s->send(hdl, data, msg->get_opcode());
}

void displayInitialContent(Canvas *canvas, const char* bdf_font_file) {
    /*
    * Let's create a simple animation. We use the canvas to draw
    * pixels. We wait between each step to have a slower animation.
    */
    rgb_matrix::Font font;
    if (!font.LoadFont(bdf_font_file)) {
        fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
        perror("Font loading error");
        fprintf(stderr, "File exists: %s\n", access(bdf_font_file, F_OK) == 0 ? "Yes" : "No");
        fprintf(stderr, "Readable: %s\n", access(bdf_font_file, R_OK) == 0 ? "Yes" : "No");
        fprintf(stderr, "Writable: %s\n", access(bdf_font_file, W_OK) == 0 ? "Yes" : "No");
        return;
    }

    canvas->Fill(2, 2, 2);

    Color font_color(255, 255, 255);
    Color bg_color(0, 0, 0);
    char line[1024] = "LOADING";
    rgb_matrix::DrawText(canvas, font, 111, 33,
                        font_color, &bg_color, line,
                        0);

    int center_x = canvas->width() / 2 - 1;
    int center_y = canvas->height() / 2 + 2;
    int row_y = center_y;    // Row position in the middle of the canvas
    int row_half_width = 16;  // Half-width of the row (total width will be 15 pixels)

    // Define an array of colors to cycle through
    std::vector<Color> colors = {
        Color(138,63,92),
        Color(238,221,239),
        Color(63,21,37)
    };

    int color_index = 0;  // Color index to cycle through colors

    // Initial direction of movement
    int direction = 1;  // Start moving right
    int color_idx = 0;
    Color current_color = colors[color_idx];
    int y_mod = 0;

    int sleep_duration = 80000;

    while (!initial_message_recieved && !interrupt_received) {
        
        // Draw the row in the middle of the canvas with the current color
        for (int x = center_x - row_half_width; x <= center_x + row_half_width; ++x) {
            if (initial_message_recieved || interrupt_received){
                return;
            }
            // Calculate color index based on iteration count
            // Set pixel at (x, row_y) with current color
            canvas->SetPixel(x, row_y + y_mod % 2, current_color.r, current_color.g, current_color.b);
            canvas->SetPixel(x, row_y + y_mod % 2 + 1, current_color.r, current_color.g, current_color.b);
            usleep(sleep_duration);  // Adjust sleep time as needed
        }
        color_index++;
        current_color = colors[color_index % colors.size()];
        y_mod ++;

        for (int x = center_x + row_half_width; x >= center_x - row_half_width; --x) {
            if (initial_message_recieved || interrupt_received){
                return;
            }
            // Set pixel at (x, row_y) with current color
            canvas->SetPixel(x, row_y + y_mod % 2 + 2, current_color.r, current_color.g, current_color.b);
            canvas->SetPixel(x, row_y + y_mod % 2 + 3, current_color.r, current_color.g, current_color.b);
            usleep(sleep_duration);  // Adjust sleep time as needed
        }

        color_index++;
        current_color = colors[color_index % colors.size()];
        y_mod ++;
        // Update canvas display (if needed, but you mentioned not needing this)
        // canvas->UpdateScreen();

        // Pause to slow down animation
        

        // // Move the row back and forth
        // center_x += direction;

        // // Reverse direction when hitting boundaries
        // if (center_x <= canvas->width() / 2 - row_half_width) {
        //     center_x = canvas->width() / 2 - row_half_width + 1;
        //     direction = -direction;
        //     color_index++;  // Change color after reversing direction
        // } else if (center_x >= canvas->width() / 2 + row_half_width) {
        //     center_x = canvas->width() / 2 + row_half_width - 1;
        //     direction = -direction;
        //     color_index++;  // Change color after reversing direction
        // }
    }


}

int main(int argc, char *argv[]) {
    // Initialize the RGB matrix with options
    RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;
    if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
        return 1;
    }

    const char *bdf_font_file = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "f:F:")) != -1) {
        switch (opt) {
        case 'f': bdf_font_file = strdup(optarg); break;
        }
    }

    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    RGBMatrix *matrix = CreateMatrixFromFlags(&argc, &argv, &matrix_options, &runtime_opt);
    if (matrix == NULL) {
        return 1;
    }

    Canvas *canvas = matrix;

    // Create and configure the WebSocket server
    server ws_server;

    ws_server.set_access_channels(websocketpp::log::alevel::all);
    ws_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

    ws_server.init_asio();

    ws_server.set_message_handler(bind(&on_message, &ws_server, _1, _2, canvas));

    ws_server.listen(8080);
    ws_server.start_accept();

    std::thread ws_thread([&ws_server]() {
        try {
            ws_server.run();
        } catch (websocketpp::exception const & e) {
            std::cout << "WebSocket exception: " << e.what() << std::endl;
        } catch (std::exception const & e) {
            std::cout << "Std exception: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "Other exception" << std::endl;
        }
    });

    displayInitialContent(matrix, bdf_font_file);

    while (!interrupt_received) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    // Properly stop the server
    websocketpp::lib::error_code ec;
    ws_server.stop_listening();
    ws_server.stop();

    if (ws_thread.joinable()) {
        ws_thread.join();
    }

    matrix->Clear();
    delete matrix;

    return 0;
}
