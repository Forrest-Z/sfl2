/* 
 * Copyright (C) 2004
 * Swiss Federal Institute of Technology, Lausanne. All rights reserved.
 * 
 * Developed at the Autonomous Systems Lab.
 * Visit our homepage at http://asl.epfl.ch/
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */


#include "SimpleImage.hpp"



/**
 *  \file
 *
 *  \brief Implementation
 */



#include "wrap_gl.hpp"
#include <iostream>		// hax: should use exceptions
using std::cerr;



SimpleImage::
SimpleImage(png_uint_32 _width,
	    png_uint_32 _height):
  width(_width),
  height(_height)
{
  pixel = new png_byte[height * width * BPP];
  if( ! pixel){
    cerr << "ERROR in SimpleImage::SimpleImage(): couldn't allocate pixels\n";
    exit(EXIT_FAILURE);
  }

  row_pointers = new png_bytep[height];
  if( ! pixel){
    cerr << "ERROR in SimpleImage::SimpleImage(): "
	 << "couldn't allocate row pointers\n";
    exit(EXIT_FAILURE);
  }

  for(unsigned int k = 0; k < height; k++)
    row_pointers[height - k - 1] = (png_bytep) pixel + k * width * BPP;  
  // Mesa uses lower-left-corner
  // PNG  uses upper-left-corner
  // if both used the same, the above would be:
  //    row_pointers[k] = (png_bytep) pixel + k * width * BPP;  
}



SimpleImage::
~SimpleImage()
{
  if(pixel)
    delete[] pixel;
  if(row_pointers)
    delete[] row_pointers;
}



bool SimpleImage::
write_png(const string &filename)
{
  FILE *fp;
  png_structp png_ptr;
  png_infop info_ptr;
  
  fp = fopen(filename.c_str(), "wb");
  if( ! fp){
    cerr << "SimpleImage::write_png(\"" << filename
	 << "\"): couldn't open file\n";
    return false;
  }
  
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if( ! png_ptr){
    cerr << "SimpleImage::write_png(\"" << filename
	 << "\"): couldn't create write struct\n";
    fclose(fp);
    return false;
  }
  
  info_ptr = png_create_info_struct(png_ptr);
  if( ! info_ptr){
    cerr << "SimpleImage::write_png(\"" << filename
	 << "\"): couldn't create info struct\n";
    fclose(fp);
    png_destroy_write_struct(&png_ptr,  (png_infopp) NULL);
    return false;
  }
  
  if(setjmp(png_jmpbuf(png_ptr)))
    {
      cerr << "SimpleImage::write_png(\"" << filename
	   << "\"): couldn't setjmp\n";
      fclose(fp);
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return false;
    }
  
  png_init_io(png_ptr, fp);
  
  png_set_IHDR(png_ptr, info_ptr, width, height,
	       sizeof(png_byte) * 8,
	       PNG_COLOR_TYPE_RGB,
	       PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE,
	       PNG_FILTER_TYPE_BASE);
  
  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, info_ptr);
  
  png_destroy_write_struct(&png_ptr, &info_ptr);
  
  fclose(fp);
  
  return true;
}



void SimpleImage::
set_pixel(png_uint_32 x,
	  png_uint_32 y,
	  png_byte r,
	  png_byte g,
	  png_byte b)
{
  int idx = BPP * ( y * width + x);
  pixel[idx++] = r;
  pixel[idx++] = g;
  pixel[idx  ] = b;
}



void SimpleImage::
read_framebuf(unsigned int xoff,
	      unsigned int yoff)
{
  glReadBuffer(GL_FRONT);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(xoff, yoff, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixel);
}
