// A simple FITS-format IO library, to/from Halide Images, modeled after
// image_io.h. It expects either Halide::Image to be already included/defined, 
// or the standalone version from static_image.h, by the time this is included.

#ifndef FITS_IMAGE_H
#define FITS_IMAGE_H

#include <stdint.h>
#include <memory>
#include <limits>
#include <stdlib.h>
#include <cassert>
#include <fitsio.h>

#ifndef BUFFER_T_DEFINED
#define BUFFER_T_DEFINED

// Note that while __attribute__ can go before or after the declaration,
// __declspec apparently is only allowed before.
#ifndef HALIDE_ATTRIBUTE_ALIGN
    #ifdef _MSC_VER
        #define HALIDE_ATTRIBUTE_ALIGN(x) __declspec(align(x))
    #else
        #define HALIDE_ATTRIBUTE_ALIGN(x) __attribute__((aligned(x)))
    #endif
#endif

typedef struct buffer_t {
    uint64_t dev;
    uint8_t* host;
    int32_t extent[4];
    int32_t stride[4];
    int32_t min[4];
    int32_t elem_size;

    HALIDE_ATTRIBUTE_ALIGN(1) bool host_dirty;

    HALIDE_ATTRIBUTE_ALIGN(1) bool dev_dirty;

    HALIDE_ATTRIBUTE_ALIGN(1) uint8_t _padding[10 - sizeof(void *)];
} buffer_t;

#endif

// TODO: stop exiting, instead return NULL data pointers, etc.
void printerror( int status)
{
    /*****************************************************/
    /* Print out cfitsio error messages and exit program */
    /*****************************************************/
    if (status) {
       fits_report_error(stderr, status); /* print error report */
       exit( status );    /* terminate the program, returning error status */
    }
    return;
}

buffer_t load_fits(const char *filename) {
    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
    int status,  nfound, anynull;
    long naxes[2], fpixel, nbuffer, npixels, ii;

    float nullval;

    status = 0;

    if ( fits_open_file(&fptr, filename, READONLY, &status) )
         printerror( status );

    /* read the NAXIS1 and NAXIS2 keyword to get image size */
    if ( fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status) ) {
        printerror( status );
    }

    npixels  = naxes[0] * naxes[1];         /* number of pixels in the image */
    fpixel   = 1;
    nullval  = 0;                /* don't check for null values in the image */
    
    // Allocate our buffer
    buffer_t buf = {0};
    buf.extent[0] = naxes[0];
    buf.extent[1] = naxes[1];
    buf.stride[0] = 1;
    buf.stride[1] = naxes[0];
    buf.elem_size = sizeof(float);

    buf.host = new uint8_t[sizeof(float)*npixels + 40];
    while ((size_t)buf.host & 0x1f) buf.host++; // align host pointer

    /* Note that even though the FITS images contains unsigned integer */
    /* pixel values (or more accurately, signed integer pixels with    */
    /* a bias of 32768),  this routine is reading the values into a    */
    /* float array.   Cfitsio automatically performs the datatype      */
    /* conversion in cases like this.                                  */

    if ( fits_read_img(fptr, TFLOAT, fpixel, npixels,
                       &nullval, (float*)buf.host, &anynull, &status) )
    {
        printerror( status );
    }

    if ( fits_close_file(fptr, &status) ) {
        printerror( status );
    }

    return buf;
}

// Just float for now
void save_fits(const buffer_t *buf, const char *filename) {
    assert(buf->elem_size == sizeof(float));
    
    fitsfile *outfptr;
    int status = 0;
    
    fits_create_file(&outfptr, filename, &status);
    
    if (status) {    
        printerror(status);
    }

    int naxis = 0;
    int npixels = 1;
    for (int i = 0; i < 4; i++) {
        if (!buf->extent[i]) break;
        naxis++;
        npixels *= buf->extent[i];
    }
    long naxes[] = { buf->extent[0], buf->extent[1], buf->extent[2], buf->extent[3] };

    fits_create_img(outfptr, FLOAT_IMG, naxis, naxes, &status);
    
    fits_write_img(outfptr, TFLOAT, 1, npixels, buf->host, &status);
    
    if (status) {
        printerror(status);
    }
}

#endif
