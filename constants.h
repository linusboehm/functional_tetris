#pragma once

#include <bitset>
#include <cstdint>
#include <ranges>
#include <utility>

#include <ncurses.h>

namespace rv = std::ranges::views;

namespace tetris {

static constexpr uint8_t NUM_BOARD_COLUMNS = 10;
static constexpr uint8_t SHAPE_SZ = 4;
static constexpr uint8_t NUM_SHAPES = 7;
static constexpr uint8_t MAX_ROWS = 12;
// static constexpr uint32_t MAX_B_HEIGHT = 14;

using shape_row_bs = std::bitset<SHAPE_SZ>;
using shape_bs = std::array<shape_row_bs, SHAPE_SZ>;
using row_bs = std::bitset<NUM_BOARD_COLUMNS + 2>;

template <size_t N>
std::bitset<N> Reverse(std::bitset<N> in) {
    for (size_t i = 0; i < (N / 2); ++i) {
        auto rev_idx = in.size() - i - 1;
        bool tmp = in[rev_idx];
        in[rev_idx] = in[i];
        in[i] = tmp;
    }
    return in;
}

template <size_t N>
std::array<std::bitset<N>, N> rotateMatrix(std::array<std::bitset<N>, N> mat) {
    for (size_t i = 0; i < N; i++) mat[i] = Reverse(mat[i]);  // REVERSE every row
    // Performing Transpose
    for (size_t row = 0; row < N; row++) {
        for (size_t col = row; col < N; col++) {
            bool tmp = mat[col][row];
            mat[col][row] = mat[row][col];
            mat[row][col] = tmp;
        }
    }
    return mat;
}

// /////////////////////////////////
// DEFINITION OF ALL POSSIBLE SHAPES IN ShapeType::Types
// Least significant bit corresponds to left-most cell
static constexpr std::array<shape_bs, NUM_SHAPES> shapes{
    shape_bs{0b0000, 0b0110, 0b0110, 0b0000}, shape_bs{0b0000, 0b0110, 0b0011, 0b0000},
    shape_bs{0b0000, 0b0011, 0b0110, 0b0000}, shape_bs{0b0000, 0b0010, 0b0111, 0b0000},
    shape_bs{0b0000, 0b1111, 0b0000, 0b0000}, shape_bs{0b0110, 0b0010, 0b0010, 0b0000},
    shape_bs{0b0110, 0b0100, 0b0100, 0b0000},
};
// static_assert(shapes.size() == ShapeType::Types::NUMBER_OF_SHAPES);

using board = std::array<row_bs, MAX_ROWS + SHAPE_SZ>;

struct State {
    board board_;
    shape_bs shape_;
    uint32_t row_of_shape_{MAX_ROWS + 1};
    uint32_t offset_{5};
    int32_t round_{0};
    int32_t deleted_{0};
};

row_bs ShapeToRow(const shape_row_bs in, uint32_t offset) {
    return row_bs(in.to_ulong()) << offset;
}

enum class KeyPress {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    OTHER,
};

void PrintRow(const row_bs bs, const row_bs shape, const char c) {
    for (size_t i = 0; i < bs.size(); ++i) {
        const auto has_shape = shape[i];
        auto occ_char = (has_shape) ? 'O' : '#';
        addch(((bs[i] || has_shape) ? occ_char : '|'));
        addch(((bs[i] || has_shape) ? occ_char : c));
        addch(((bs[i] || has_shape) ? occ_char : c));
    }
}

KeyPress GetInput() {
    if (getch() == '\033') {  // if the first value is esc
        getch();              // skip the [
        switch (getch()) {    // the real value
            case 'A':
                return KeyPress::UP;
            case 'B':
                return KeyPress::DOWN;
            case 'C':
                return KeyPress::RIGHT;
            case 'D':
                return KeyPress::LEFT;
        }
    }
    return KeyPress::OTHER;
}

State UserMove(State state) {
    auto input = GetInput();
    if (input == KeyPress::UP)
        state.shape_ = rotateMatrix(state.shape_);
    else if (input == KeyPress::LEFT && state.offset_ > 0)
        --state.offset_;
    else if (input == KeyPress::RIGHT)
        ++state.offset_;
    return state;
}

void PrintBoard(const State& state, uint32_t curr_height = MAX_ROWS) {
    clear();
    addstr("BOARD:\n");
    for (size_t i = curr_height;; --i) {
        const uint32_t s_idx = i - state.row_of_shape_;
        const row_bs shape =
            (s_idx < SHAPE_SZ) ? (ShapeToRow(state.shape_[s_idx], state.offset_)) : 0b0000000000;
        // printw("%d %s\n", s_idx, shape.to_string().c_str());
        PrintRow(state.board_[i], shape, ' ');
        addch('\n');
        PrintRow(state.board_[i], shape, '_');
        addch('\n');
        if (i == 0) break;
    }
    addstr("====================================\n");
    printw("ROUND: %d; DELETED ROWS: %d\n", state.round_, state.deleted_);
    curs_set(0);  // hide cursor
    refresh();
}

shape_bs rand_shapes() { return shapes[rand() % NUM_SHAPES]; }

}  // namespace tetris
