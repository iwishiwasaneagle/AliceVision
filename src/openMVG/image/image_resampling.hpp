// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#ifndef OPENMVG_IMAGE_IMAGE_RESAMPLING_HPP_
#define OPENMVG_IMAGE_IMAGE_RESAMPLING_HPP_


namespace openMVG {
namespace image {

  /**
   ** Half sample an image (ie reduce it's size by a factor 2) using bilinear interpolation
   ** @param src input image
   ** @param out output image
   **/
  template < typename Image >
  void ImageHalfSample( const Image & src , Image & out )
  {
    const int new_width  = src.Width() / 2 ;
    const int new_height = src.Height() / 2 ;

    out.resize( new_width , new_height ) ;

    const Sampler2d<SamplerLinear> sampler;

    for( int i = 0 ; i < new_height ; ++i )
    {
      for( int j = 0 ; j < new_width ; ++j )
      {
        // Use .5f offset to ensure mid pixel and correct bilinear sampling
        out( i , j ) =  sampler( src, 2.f * (i+.5f), 2.f * (j+.5f) );
      }
    }
  }

  /**
   ** @brief Ressample an image using given sampling positions
   ** @param src Input image
   ** @param sampling_pos A list of coordinates where the image needs to be ressampled (samples are (Y,X) )
   ** @param output_width Width of the output image.
   ** @param output_height Height of the output image
   ** @param sampling_func Ressampling functor used to sample the Input image
   ** @param[out] Output image
   ** @note sampling_pos.size() must be equal to output_width * output_height
   **/
  template <typename Image , typename RessamplingFunctor>
  void GenericRessample( const Image & src ,
                         const std::vector< std::pair< float , float > > & sampling_pos ,
                         const int output_width ,
                         const int output_height ,
                         const RessamplingFunctor & sampling_func ,
                         Image & out )
  {
    assert( sampling_pos.size() == output_width * output_height );

    out.resize( output_width , output_height );

    std::vector< std::pair< float , float > >::const_iterator it_pos = sampling_pos.begin();

    for( int i = 0 ; i < output_height ; ++i )
    {
      for( int j = 0 ; j < output_width ; ++j , ++it_pos )
      {
        const float input_x = it_pos->second ;
        const float input_y = it_pos->first ;

        out( i , j ) = sampling_func( src , input_y , input_x ) ;
      }
    }
  }

} // namespace image
} // namespace openMVG

#endif
