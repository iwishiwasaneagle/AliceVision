// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#ifndef OPENMVG_SFM_DATA_IO_HPP
#define OPENMVG_SFM_DATA_IO_HPP

#include "openMVG/sfm/sfm_data.hpp"

namespace openMVG {
namespace sfm {

enum ESfM_Data
{
  VIEWS           = 1,
  EXTRINSICS      = 2,
  INTRINSICS      = 4,
  STRUCTURE       = 8,
  OBSERVATIONS    = 16,
  CONTROL_POINTS  = 32,
  ALL = VIEWS | EXTRINSICS | INTRINSICS | STRUCTURE | OBSERVATIONS | CONTROL_POINTS
};

///Check that each pose have a valid intrinsic and pose id in the existing View ids
bool ValidIds(const SfM_Data & sfm_data, ESfM_Data flags_part);

/// Load SfM_Data SfM scene from a file
bool Load(SfM_Data & sfm_data, const std::string & filename, ESfM_Data flags_part);

/// Save SfM_Data SfM scene to a file
bool Save(const SfM_Data & sfm_data, const std::string & filename, ESfM_Data flags_part);

} // namespace sfm
} // namespace openMVG

#endif // OPENMVG_SFM_DATA_IO_HPP
