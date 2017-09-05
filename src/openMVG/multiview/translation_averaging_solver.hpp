// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#ifndef __TRANS_SOLVER_H__
#define __TRANS_SOLVER_H__

namespace openMVG
{

/**
 * @brief Compute camera center positions from relative camera translations (translation directions).
 *
 * Implementation of [1] : "5. Solving the Translations Problem" equation (3)
 */
bool solve_translations_problem_l2_chordal(
  const int* edges,
  const double* poses,
  const double* weights,
  int num_edges,
  double loss_width,
  double* X,
  double function_tolerance,
  double parameter_tolerance,
  int max_iterations
);


/**
 * @brief Registration of relative translations to global translations. It implements LInf minimization of  [2]
 *  as a SoftL1 minimization. It can use 2/3 views relative translation vectors (bearing, or triplets of translations).
 *
 * [1] "Robust Global Translations with 1DSfM."
 * Authors: Kyle Wilson and Noah Snavely.
 * Date: September 2014.
 * Conference: ECCV.
 *
 * [2] "Global Fusion of Relative Motions for Robust, Accurate and Scalable Structure from Motion."
 * Authors: Pierre Moulon, Pascal Monasse and Renaud Marlet.
 * Date: December 2013.
 * Conference: ICCV.
 *
 * Based on a BSD implementation from Kyle Wilson.
 * See http://github.com/wilsonkl/SfM_Init
 *
 * @param[in] vec_initial_estimates relative motion information
 * @param[in] b_translation_triplets tell if relative motion comes 3 or 2 views
 *   false: 2-view estimates -> 1 relativeInfo per 2 view estimates,
 *   true:  3-view estimates -> triplet of translations: 3 relativeInfo per triplet.
 * @param[in] nb_poses the number of camera nodes in the relative motion graph
 * @param[out] translations found global camera translations
 * @param[in] d_l1_loss_threshold optionnal threshold for SoftL1 loss (-1: no loss function)
 * @return True if the registration can be solved
 */
bool
solve_translations_problem_softl1
(
  const std::vector<openMVG::relativeInfo > & vec_initial_estimates,
  const bool b_translation_triplets,
  const int nb_poses,
  std::vector<Eigen::Vector3d> & translations,
  const double d_l1_loss_threshold = 0.01
);

} // namespace openMVG

#endif /* __TRANS_SOLVER_H__ */
