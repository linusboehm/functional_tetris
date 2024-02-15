#include <algorithm>
#include <iostream>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include "./constants.h"

namespace rv = std::ranges::views;
using namespace tetris;

row_bs AddShape(const row_bs row, const shape_row_bs to_add, const uint8_t l_offset = 0) {
    return row | (row_bs(to_add.to_ulong()) << l_offset);
}

bool FitsInRow(const row_bs row, const shape_row_bs to_add, const uint8_t l_offset = 0) {
    return ((row_bs(to_add.to_ulong()) << l_offset) & row).none();
}

int main() {
    initscr();
    echo();

    auto get_final_position = rv::transform([&](auto state) {
        auto sub_rows = state.board_            //
                        | rv::reverse           // go from top->bottom
                        | rv::slide(SHAPE_SZ);  // check whole shape
        [[maybe_unused]] auto collision_window = std::ranges::find_if(
            sub_rows,
            [&](auto rng) {  // find_if predicate
                std::ranges::rotate(state.shape_,
                                    std::ranges::find_if_not(state.shape_, [](const auto in) {
                                        return in == shape_row_bs{0b0000};
                                    }));
                const auto crash =
                    !std::ranges::all_of(rv::zip(rng, state.shape_ | rv::reverse), [&](auto in) {
                        return FitsInRow(std::get<0>(in), std::get<1>(in), state.offset_);
                    });
                if (!crash) {
                    if (state.row_of_shape_ > 0) --state.row_of_shape_;
                    PrintBoard(state);
                    auto post_state = UserMove(state);
                    if (std::ranges::all_of(rv::zip(rng, post_state.shape_ | rv::reverse),
                                            [&](auto in) {
                                                return FitsInRow(std::get<0>(in), std::get<1>(in),
                                                                 post_state.offset_);
                                            })) {
                        state = post_state;
                        PrintBoard(state);
                    }
                }
                return crash;
            });
        return state;
    });

    auto add_to_board = rv::transform([&](auto state) {
        std::ranges::for_each(
            rv::zip(state.board_ | rv::drop(state.row_of_shape_) | rv::take(SHAPE_SZ),
                    state.shape_),
            [&](auto e) {
                std::get<0>(e) = AddShape(std::get<0>(e), std::get<1>(e), state.offset_);
            });
        return state;
    });

    auto remove_full_rows = rv::transform([&](auto state) {
        std::ranges::for_each(rv::iota(state.row_of_shape_, state.row_of_shape_ + SHAPE_SZ + 1)  //
                                  | rv::reverse                                                  //
                                  | rv::filter([&](auto idx) { return state.board_[idx].all(); }),
                              [&](auto idx) {
                                  // that's a rotate!!!
                                  state.board_[idx] &= row_bs(0b100000000001);
                                  const auto first = std::next(std::begin(state.board_), idx);
                                  const auto middle = std::next(std::begin(state.board_), idx + 1);
                                  state.deleted_ += 1;
                                  std::rotate(first, middle, std::end(state.board_));
                              });
        return state;
    });

    State state;
    state.board_.fill(0b100000000001);

    srand(time(NULL));
    std::ranges::find_if(rv::iota(0)  //
                             | rv::transform([&](auto round) {
                                   state.shape_ = rand_shapes();
                                   state.round_ = round;
                                   state.row_of_shape_ = MAX_ROWS + 1;
                                   state.offset_ = 5;
                                   return state;
                               })                  //
                             | get_final_position  //
                             | add_to_board        //
                             | remove_full_rows,
                         [&](const auto& in) {
                             state = in;
                             // PrintBoard(in);
                             return (in.board_[MAX_ROWS] & row_bs(0b011111111110)).any();
                         });
    endwin();
}
