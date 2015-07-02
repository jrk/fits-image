#include <Halide.h>
using namespace Halide;

#include "fits_image.h"
#include "image_io.h"

int main (int argc, char const *argv[])
{
    assert(argc == 2);

    buffer_t inbuf = load_fits(argv[1]);

    printf("Loaded FITS: %d x %d pixels\n", inbuf.extent[0], inbuf.extent[1]);
    
    Image<float> in(&inbuf);
    Func out;
    Var x, y;
    
    out(x,y) = in(x,y) * 1.001f; // some random math
    
    Image<float> res = out.realize(in.width(), in.height());

    save_fits(Buffer(res).raw_buffer(), "test.fits");

    return 0;
}
