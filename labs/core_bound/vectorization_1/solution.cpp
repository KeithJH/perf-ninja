#include "solution.hpp"
#include <algorithm>
#include <cassert>

Solution::simd_sequence_t
Solution::transpose(const std::vector<sequence_t> &sequences) {
  assert(sequences.size() == sequence_count_v);

  simd_sequence_t simd_sequence;

  for (std::size_t seq_col = 0; seq_col < sequence_size_v; seq_col++) {
    for (std::size_t seq_idx = 0; seq_idx < sequence_count_v; seq_idx++) {
      simd_sequence[seq_col][seq_idx] = sequences[seq_idx][seq_col];
    }
  }

  return simd_sequence;
}

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t
Solution::compute_alignment(std::vector<sequence_t> const &sequences1,
                            std::vector<sequence_t> const &sequences2) {
  result_t result{};

  using score_t = std::array<int16_t, sequence_count_v>;
  using column_t = std::array<score_t, sequence_size_v + 1>;

  const simd_sequence_t simd_sequences1 = transpose(sequences1);
  const simd_sequence_t simd_sequences2 = transpose(sequences2);

  /*
   * Initialise score values.
   */
  score_t gap_open;
  gap_open.fill(-11);

  score_t gap_extension;
  gap_extension.fill(-1);

  score_t match;
  match.fill(6);

  score_t mismatch;
  mismatch.fill(-4);

  /*
   * Setup the matrix.
   * Note we can compute the entire matrix with just one column in memory,
   * since we are only interested in the last value of the last column in the
   * score matrix.
   */
  column_t score_column{};
  column_t horizontal_gap_column{};
  score_t last_vertical_gap{};

  /*
   * Initialise the first column of the matrix.
   */
  horizontal_gap_column[0] = gap_open;
  last_vertical_gap = gap_open;

  for (size_t i = 1; i < score_column.size(); ++i) {
    for (size_t seq_idx = 0; seq_idx < sequence_count_v; seq_idx++) {
      score_column[i][seq_idx] = last_vertical_gap[seq_idx];
      horizontal_gap_column[i][seq_idx] =
          last_vertical_gap[seq_idx] + gap_open[seq_idx];
      last_vertical_gap[seq_idx] += gap_extension[seq_idx];
    }
  }

  /*
   * Compute the main recursion to fill the matrix.
   */
  for (unsigned col = 1; col <= simd_sequences2.size(); ++col) {
    score_t last_diagonal_score =
        score_column[0]; // Cache last diagonal score to compute this cell.

    for (size_t seq_idx = 0; seq_idx < sequence_count_v; seq_idx++) {
      score_column[0][seq_idx] = horizontal_gap_column[0][seq_idx];
      last_vertical_gap[seq_idx] =
          horizontal_gap_column[0][seq_idx] + gap_open[seq_idx];
      horizontal_gap_column[0][seq_idx] += gap_extension[seq_idx];
    }

    for (unsigned row = 1; row <= simd_sequences1.size(); ++row) {
      // Compute next score from diagonal direction with match/mismatch.
      score_t best_cell_score = last_diagonal_score;

      for (size_t seq_idx = 0; seq_idx < sequence_count_v; seq_idx++) {
        best_cell_score[seq_idx] += (simd_sequences1[row - 1][seq_idx] ==
                                             simd_sequences2[col - 1][seq_idx]
                                         ? match[seq_idx]
                                         : mismatch[seq_idx]);

        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score[seq_idx] =
            std::max(best_cell_score[seq_idx], last_vertical_gap[seq_idx]);
        best_cell_score[seq_idx] = std::max(
            best_cell_score[seq_idx], horizontal_gap_column[row][seq_idx]);
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score[seq_idx] = score_column[row][seq_idx];
        score_column[row][seq_idx] = best_cell_score[seq_idx];
        // Compute the next values for vertical and horizontal gap.
        best_cell_score[seq_idx] += gap_open[seq_idx];
        last_vertical_gap[seq_idx] += gap_extension[seq_idx];
        horizontal_gap_column[row][seq_idx] += gap_extension[seq_idx];
        // Store optimum between gap open and gap extension.
        last_vertical_gap[seq_idx] =
            std::max(last_vertical_gap[seq_idx], best_cell_score[seq_idx]);
        horizontal_gap_column[row][seq_idx] = std::max(
            horizontal_gap_column[row][seq_idx], best_cell_score[seq_idx]);
      }
    }
  }

  // Report the best score.
  for (size_t seq_idx = 0; seq_idx < sequence_count_v; seq_idx++) {
    result[seq_idx] = score_column.back()[seq_idx];
  }

  return result;
}
