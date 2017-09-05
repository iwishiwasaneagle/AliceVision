// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#pragma once

#include "openMVG/sfm/sfm_data.hpp"

#include <openMVG/geometry/pose3.hpp>
#include <openMVG/cameras/Camera_Pinhole.hpp>

#include <string>

namespace openMVG {
namespace sfm {

bool read_openMVG_Camera(const std::string & camName, cameras::Pinhole_Intrinsic & cam, geometry::Pose3 & pose);

bool read_Strecha_Camera(const std::string & camName, cameras::Pinhole_Intrinsic & cam, geometry::Pose3 & pose);

/**
@brief Reads a set of Pinhole Cameras and its poses from a ground truth dataset.
@param[in] sRootPath, the directory containing an image folder "images" and a GT folder "gt_dense_cameras".
@param[out] sfm_data, the SfM_Data structure to put views/poses/intrinsics in.
@param[in] useUID, set to false to disable UID".
@return Returns true if data has been read without errors
**/
bool readGt(const std::string & sRootPath, SfM_Data & sfm_data, bool useUID = true);

} // namespace sfm
} // namespace openMVG