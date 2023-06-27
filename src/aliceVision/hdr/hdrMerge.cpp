// This file is part of the AliceVision project.
// Copyright (c) 2019 AliceVision contributors.
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "hdrMerge.hpp"
#include <cassert>
#include <cmath>
#include <limits>
#include <iostream>
#include <fstream>

#include <aliceVision/alicevision_omp.hpp>
#include <aliceVision/system/Logger.hpp>


namespace aliceVision {
namespace hdr {

/**
 * f(x)=min + (max-min) * \frac{1}{1 + e^{10 * (x - mid) / width}}
 * https://www.desmos.com/calculator/xamvguu8zw
 *              ____
 * sigmoid:         \________
 *                sigMid
 */
inline float sigmoid(float zeroVal, float endVal, float sigwidth, float sigMid, float xval)
{
    return zeroVal + (endVal - zeroVal) * (1.0f / (1.0f + expf(10.0f * ((xval - sigMid) / sigwidth))));
}

/**
 * https://www.desmos.com/calculator/cvu8s3rlvy
 *
 *                       ____
 * sigmoid inv:  _______/
 *                    sigMid
 */
inline float sigmoidInv(float zeroVal, float endVal, float sigwidth, float sigMid, float xval)
{
    return zeroVal + (endVal - zeroVal) * (1.0f / (1.0f + expf(10.0f * ((sigMid - xval) / sigwidth))));
}

void hdrMerge::process(const std::vector< image::Image<image::RGBfColor> > &images,
                        const std::vector<double> &times,
                        const rgbCurve &weight,
                        const rgbCurve &response,
                        image::Image<image::RGBfColor> &radiance,
                        image::Image<image::RGBfColor> &lowLight,
                        image::Image<image::RGBfColor> &highLight,
                        image::Image<image::RGBfColor> &noMidLight,
                        MergingParams &mergingParams)
{
  //checks
  assert(!response.isEmpty());
  assert(!images.empty());
  assert(images.size() == times.size());

  // get images width, height
  const std::size_t width = images.front().Width();
  const std::size_t height = images.front().Height();

  // resize and reset radiance image to 0.0
  radiance.resize(width, height, true, image::RGBfColor(0.f, 0.f, 0.f));

  ALICEVISION_LOG_TRACE("[hdrMerge] Images to fuse:");
  for(int i = 0; i < images.size(); ++i)
  {
    ALICEVISION_LOG_TRACE(images[i].Width() << "x" << images[i].Height() << ", time: " << times[i]);
  }

  rgbCurve weightShortestExposure = weight;
  weightShortestExposure.freezeSecondPartValues();
  rgbCurve weightLongestExposure = weight;
  weightLongestExposure.freezeFirstPartValues();

  //const std::string mergeInfoFilename = "C:/Temp/mergeInfo.csv";
  //std::ofstream file(mergeInfoFilename);

  const double minValue = mergingParams.minSignificantValue;
  const double maxValue = mergingParams.maxSignificantValue;

  highLight.resize(width, height, true, image::RGBfColor(0.f, 0.f, 0.f));
  lowLight.resize(width, height, true, image::RGBfColor(0.f, 0.f, 0.f));
  noMidLight.resize(width, height, true, image::RGBfColor(0.f, 0.f, 0.f));

  #pragma omp parallel for
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      //for each pixels
      image::RGBfColor& radianceColor = radiance(y, x);
      image::RGBfColor& highLightColor = highLight(y, x);
      image::RGBfColor& lowLightColor = lowLight(y, x);
      image::RGBfColor& noMidLightColor = noMidLight(y, x);

      //if ((x%1000 == 650) && (y%1000 == 850))
      //{
      //    if(!file)
      //    {
      //        ALICEVISION_LOG_WARNING("Unable to create file " << mergeInfoFilename << " for storing merging info.");
      //    }
      //    else
      //    {
      //        file << x << "," << y << std::endl;
      //        file << "time,R,resp(R),coeff,resp(R)/time,G,resp(G),coeff,resp(G)/time,B,resp(B),coeff,resp(B)/time," << std::endl;
      //        for(std::size_t i = 0; i < images.size(); i++)
      //        {
      //            const double time = times[i];
      //            file << time << ",";
      //            for(std::size_t channel = 0; channel < 3; ++channel)
      //            {
      //                const double value = images[i](y, x)(channel);
      //                const double r = response(value, channel);
      //                double w = std::max(0.001f, (i == 0) ? weightShortestExposure(value, channel)
      //                                                     : (i == (images.size() - 1) ? weightLongestExposure(value, channel)
      //                                                                                 : weight(value, channel)));

      //                file << value << ",";
      //                file << r << ",";
      //                file << w << ",";
      //                file << r/time << ",";
      //            }
      //            file << std::endl;
      //        }

      //    }
      //}

      const double meanValueHighExp = (images[images.size() - 1](y, x)(0) + images[images.size() - 1](y, x)(1) +
                                       images[images.size() - 1](y, x)(2)) /
                                      3.0;

      if(meanValueHighExp < mergingParams.noiseThreshold) // Noise case
      {
          for(std::size_t channel = 0; channel < 3; ++channel)
          {
              radianceColor(channel) = mergingParams.targetCameraExposure * response(meanValueHighExp, channel) / times[images.size() - 1];
              highLightColor(channel) = 0.0;
              lowLightColor(channel) = 1.0;
              noMidLightColor(channel) = 0.0;
          }
      }
      else
      {
          std::vector<std::vector<double>> vv_coeff;
          std::vector<std::vector<double>> vv_value;
          std::vector<std::vector<double>> vv_normalizedValue;

          // Compute merging range
          std::vector<int> v_firstIndex;
          std::vector<int> v_lastIndex;
          for(std::size_t channel = 0; channel < 3; ++channel)
          {
              int firstIndex = mergingParams.refImageIndex;
              while(firstIndex > 0 &&
                    (response(images[firstIndex](y, x)(channel), channel) > 0.05 || firstIndex == images.size() - 1))
              {
                  firstIndex--;
              }
              v_firstIndex.push_back(firstIndex);

              int lastIndex = v_firstIndex[channel] + 1;
              while(lastIndex < images.size() - 1 && response(images[lastIndex](y, x)(channel), channel) < 0.995)
              {
                  lastIndex++;
              }
              v_lastIndex.push_back(lastIndex);
          }

          // Compute merging coeffs and values to be merged
          for(std::size_t channel = 0; channel < 3; ++channel)
          {
              std::vector<double> v_coeff;
              std::vector<double> v_normalizedValue;
              std::vector<double> v_value;

              {
                  const double value = response(images[0](y, x)(channel), channel);
                  const double normalizedValue = value / times[0];
                  double coeff = std::max(0.001f, weightShortestExposure(value, channel));

                  v_value.push_back(value);
                  v_normalizedValue.push_back(normalizedValue);
                  v_coeff.push_back(coeff);
              }

              for(std::size_t e = 1; e < images.size() - 1; e++)
              {
                  const double value = response(images[e](y, x)(channel), channel);
                  const double normalizedValue = value / times[e];
                  double coeff = std::max(0.001f, weight(value, channel));

                  v_value.push_back(value);
                  v_normalizedValue.push_back(normalizedValue);
                  v_coeff.push_back(coeff);
              }

              {
                  const double value = response(images[images.size() - 1](y, x)(channel), channel);
                  const double normalizedValue = value / times[images.size() - 1];
                  double coeff = std::max(0.001f, weightLongestExposure(value, channel));

                  v_value.push_back(value);
                  v_normalizedValue.push_back(normalizedValue);
                  v_coeff.push_back(coeff);
              }

              vv_coeff.push_back(v_coeff);
              vv_normalizedValue.push_back(v_normalizedValue);
              vv_value.push_back(v_value);
          }

          // Compute light masks if required (monitoring and debug purposes)
          if(mergingParams.computeLightMasks)
          {
              for(std::size_t channel = 0; channel < 3; ++channel)
              {
                  int idxMaxValue = 0;
                  int idxMinValue = 0;
                  double maxValue = 0.0;
                  double minValue = 10000.0;
                  bool jump = true;
                  for(std::size_t e = 0; e < images.size(); ++e)
                  {
                      if(vv_value[channel][e] > maxValue)
                      {
                          maxValue = vv_value[channel][e];
                          idxMaxValue = e;
                      }
                      if(vv_value[channel][e] < minValue)
                      {
                          minValue = vv_value[channel][e];
                          idxMinValue = e;
                      }
                      jump = jump && ((vv_value[channel][e] < 0.1 && e < images.size() - 1) ||
                                      (vv_value[channel][e] > 0.9 && e > 0));
                  }
                  highLightColor(channel) = minValue > 0.9 ? 1.0 : 0.0;
                  lowLightColor(channel) = maxValue < 0.1 ? 1.0 : 0.0;
                  noMidLightColor(channel) = jump ? 1.0 : 0.0;
              }
          }

          // Compute the final result and adjust the exposure to the reference one.
          for(std::size_t channel = 0; channel < 3; ++channel)
          {
              double v = 0.0;
              double sumCoeff = 0.0;
              for(std::size_t i = v_firstIndex[channel]; i <= v_lastIndex[channel]; ++i)
              {
                  v += vv_coeff[channel][i] * vv_normalizedValue[channel][i];
                  sumCoeff += vv_coeff[channel][i];
              }
              radianceColor(channel) = mergingParams.targetCameraExposure *
                                       (sumCoeff != 0.0 ? v / sumCoeff : vv_normalizedValue[channel][mergingParams.refImageIndex]);
          }
      }
    }
  }
}

void hdrMerge::postProcessHighlight(const std::vector< image::Image<image::RGBfColor> > &images,
    const std::vector<double> &times,
    const rgbCurve &weight,
    const rgbCurve &response,
    image::Image<image::RGBfColor> &radiance,
    float targetCameraExposure,
    float highlightCorrectionFactor,
    float highlightTargetLux)
{
    //checks
    assert(!response.isEmpty());
    assert(!images.empty());
    assert(images.size() == times.size());

    if (highlightCorrectionFactor == 0.0f)
        return;

    const image::Image<image::RGBfColor>& inputImage = images.front();
    // Target Camera Exposure = 1 for EV-0 (iso=100, shutter=1, fnumber=1) => 2.5 lux
    float highlightTarget = highlightTargetLux * targetCameraExposure * 2.5;

    // get images width, height
    const std::size_t width = inputImage.Width();

    const std::size_t height = inputImage.Height();
    image::Image<float> isPixelClamped(width, height);

#pragma omp parallel for
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            //for each pixels
            image::RGBfColor &radianceColor = radiance(y, x);

            float& isClamped = isPixelClamped(y, x);
            isClamped = 0.0f;

            for (std::size_t channel = 0; channel < 3; ++channel)
            {
                const float value = inputImage(y, x)(channel);

                // https://www.desmos.com/calculator/vpvzmidy1a
                //                       ____
                // sigmoid inv:  _______/
                //                  0    1
                const float isChannelClamped = sigmoidInv(0.0f, 1.0f, /*sigWidth=*/0.08f,  /*sigMid=*/0.95f, value);
                isClamped += isChannelClamped;
            }
            isPixelClamped(y, x) /= 3.0;
        }
    }

    image::Image<float> isPixelClamped_g(width, height);
    image::ImageGaussianFilter(isPixelClamped, 1.0f, isPixelClamped_g, 3, 3);

#pragma omp parallel for
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            image::RGBfColor& radianceColor = radiance(y, x);

            double clampingCompensation = highlightCorrectionFactor * isPixelClamped_g(y, x);
            double clampingCompensationInv = (1.0 - clampingCompensation);
            assert(clampingCompensation <= 1.0);

            for (std::size_t channel = 0; channel < 3; ++channel)
            {
                if(highlightTarget > radianceColor(channel))
                {
                    radianceColor(channel) = float(clampingCompensation * highlightTarget + clampingCompensationInv * radianceColor(channel));
                }
            }
        }
    }
}

} // namespace hdr
} // namespace aliceVision
