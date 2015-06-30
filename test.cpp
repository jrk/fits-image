//#include <Halide.h>
#include "fits_image.h"
//#include "image_io.h"

using namespace Halide;

int main (int argc, char const *argv[])
{
    assert(argc == 2);

    buffer_t in = load_fits(argv[1]);

    printf("Loaded FITS: %d x %d pixels\n", in.extent[0], in.extent[1]);
    
    //Buffer inbuf(in);

    return 0;
}