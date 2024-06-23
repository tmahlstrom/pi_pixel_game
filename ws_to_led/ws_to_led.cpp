#include "led-matrix.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>
#include <csignal>
#include <vector>
#include <sstream>
#include <thread>
#include <functional>

using namespace rgb_matrix;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::server<websocketpp::config::asio> server;

// Constants for the LED matrix dimensions
const int ROWS = 64;
const int COLS = 256;

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
    std::string data = msg->get_payload();
    std::vector<std::vector<int>> rgbArrays = parseData(data);
    displayRGBArrays(rgbArrays, canvas);
    // s->send(hdl, data, msg->get_opcode());
}

int main(int argc, char *argv[]) {
    // Initialize the RGB matrix with options
    RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;
    if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
        return 1;
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
