// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#ifndef OPENMVG_SFM_MATCHES_PROVIDER_HPP
#define OPENMVG_SFM_MATCHES_PROVIDER_HPP

#include <openMVG/types.hpp>
#include <openMVG/system/Logger.hpp>
#include <openMVG/sfm/sfm_data.hpp>
#include <openMVG/matching/indMatch.hpp>
#include <openMVG/matching/indMatch_utils.hpp>

namespace openMVG {
namespace sfm {

/**
 * @brief Load match files.
 *
 * @param[out] out_pairwiseMatches
 * @param[in] sfmData
 * @param[in] folder
 * @param[in] matchesMode
 */
inline bool loadPairwiseMatches(
    matching::PairwiseMatches& out_pairwiseMatches,
    const SfM_Data& sfmData,
    const std::string& folder,
    const std::vector<features::EImageDescriberType>& descTypes,
    const std::string& matchesMode)
{
  OPENMVG_LOG_DEBUG("- Loading matches...");
  if (!matching::Load(out_pairwiseMatches, sfmData.GetViewsKeys(), folder, descTypes, matchesMode))
  {
    OPENMVG_LOG_WARNING("Unable to read the matches file(s) from: " << folder << " (mode: " << matchesMode << ")");
    return false;
  }
  return true;
}


} // namespace sfm
} // namespace openMVG

#endif // OPENMVG_SFM_MATCHES_PROVIDER_HPP
