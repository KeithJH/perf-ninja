#include <array>   // std::array
#include <cstddef> // size_t
#include <cstdint> // fixed width types
#include <utility> // std::pair
#include <vector>  // std::vector

inline constexpr size_t sequence_size_v = 200; // The length of the generated sequences.
inline constexpr size_t sequence_count_v = 16; // The number of sequences to generate for both sequence collections.

using sequence_t = std::array<uint8_t, sequence_size_v>;
using result_t = std::array<int16_t, sequence_count_v>;

struct Baseline {
  static result_t compute_alignment(std::vector<sequence_t> const &, std::vector<sequence_t> const &);
};

struct Solution {
  using sequence_col_t = std::array<uint8_t, sequence_count_v>;
  using simd_sequence_t = std::array<sequence_col_t, sequence_size_v>;

  static result_t compute_alignment(std::vector<sequence_t> const &, std::vector<sequence_t> const &);

private:
  static simd_sequence_t transpose(const std::vector<sequence_t> &sequences);
};

struct VideoSolution {
  using simd_score_t = std::array<int16_t, sequence_count_v>;
  using simd_sequence_t = std::array<simd_score_t, sequence_size_v>;

  static result_t compute_alignment(std::vector<sequence_t> const &, std::vector<sequence_t> const &);

private:
  static simd_sequence_t transpose(const std::vector<sequence_t> &sequences);
};

std::pair<std::vector<sequence_t>, std::vector<sequence_t>> init();
