#!/usr/bin/env python
from rpi-rgb-led-matrix/bindings/python/samples/samplebase import SampleBase
from rgbmatrix import graphics
import time


class GraphicsTest(SampleBase):
    def __init__(self, *args, **kwargs):
        super(GraphicsTest, self).__init__(*args, **kwargs)

    def run(self):
        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../../fonts/5x8.bdf")

        red = graphics.Color(255, 0, 0)
        graphics.DrawLine(canvas, 5, 5, 22, 13, red)

        green = graphics.Color(0, 255, 0)
        graphics.DrawCircle(canvas, 15, 15, 10, green)

        white = graphics.Color(255, 255, 255)
        graphics.DrawText(canvas, font, 111, 35, white, "Loading")

        time.sleep(100)   # show display for 10 seconds before exit


# Main function
if __name__ == "__main__":
    graphics_test = GraphicsTest()
    if (not graphics_test.process()):
        graphics_test.print_help()
