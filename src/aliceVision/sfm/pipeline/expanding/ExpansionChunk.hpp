// This file is part of the AliceVision project.
// Copyright (c) 2023 AliceVision contributors.
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <aliceVision/types.hpp>
#include <aliceVision/track/TracksHandler.hpp>
#include <aliceVision/sfmData/SfMData.hpp>
#include <aliceVision/sfm/pipeline/expanding/ExpansionHistory.hpp>
#include <aliceVision/sfm/pipeline/expanding/SfmBundle.hpp>

namespace aliceVision {
namespace sfm {

class ExpansionChunk
{
public:
    ExpansionChunk(std::shared_ptr<ExpansionHistory> & historyHandler);

    /**
     * @brief Compute a chunk of views assuming the sfmData already has an initial set of 
     * reconstructed cameras and 3D points to connect to.
     * @param sfmData the sfmData which describes the current sfm state
     * @param tracksHandler the scene tracks handler
     * @param viewsChunks a list of view ids to process in this chunk
    */
    bool process(sfmData::SfMData & sfmData, 
                const track::TracksHandler & tracksHandler, 
                const std::set<IndexT> & viewsChunk);

    SfmBundle * getBundleHandler() const
    {
        return _bundleHandler.get();
    }

private:

    /**
     * @Brief assign the computed pose to the view
     * @param sfmData the sfmData to update
     * @param viewId the viewId of interest
     * @param pose the homogeneous matrix computed from the resection
    */
    void addPose(sfmData::SfMData & sfmData, IndexT viewId, const Eigen::Matrix4d & pose);

    /**
     * @brief Try to upgrade sfm with new landmarks
     * @param sfmData the object to update
     * @param tracks all tracks of the scene as a map {trackId, track}
     * @param viewIds the set of views to triangulate 
     */
    bool triangulate(sfmData::SfMData & sfmData, const track::TracksHandler & tracksHandler, const std::set<IndexT> & viewIds);

private:
    std::unique_ptr<SfmBundle> _bundleHandler;
    std::shared_ptr<ExpansionHistory> _historyHandler;

private:    
    size_t _resectionIterations = 1024;
    size_t _triangulationMinPoints = 2;
    double _minTriangulationAngleDegrees = 3.0;
};

} // namespace sfm
} // namespace aliceVision

