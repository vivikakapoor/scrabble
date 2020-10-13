#include "board.h"

#include "board_square.h"
#include "exceptions.h"
#include "formatting.h"
#include <fstream>
#include <iomanip>

using namespace std;

bool Board::Position::operator==(const Board::Position& other) const {
    return this->row == other.row && this->column == other.column;
}

bool Board::Position::operator!=(const Board::Position& other) const {
    return this->row != other.row || this->column != other.column;
}

Board::Position Board::Position::translate(Direction direction) const { return this->translate(direction, 1); }

Board::Position Board::Position::translate(Direction direction, ssize_t distance) const {
    if (direction == Direction::DOWN) {
        return Board::Position(this->row + distance, this->column);
    } else {
        return Board::Position(this->row, this->column + distance);
    }
}

Board Board::read(const string& file_path) {
    ifstream file(file_path);
    if (!file) {
        throw FileException("cannot open board file!");
    }

    size_t rows;
    size_t columns;
    size_t starting_row;
    size_t starting_column;
    file >> rows >> columns >> starting_row >> starting_column;
    Board board(rows, columns, starting_row, starting_column);
    vector<BoardSquare> temp;

    // traverse through each square of board
    for (size_t i = 0; i < board.rows; i++) {
        // create space/memory for the current row
        board.squares.push_back(temp);
        for (size_t j = 0; j < board.columns; j++) {
            // read in each specific char for each square
            char type;
            file >> type;
            // instantiate a boardsquare for each column of that row, with the correct letter/word multipliers
            if (type == '.') {
                BoardSquare currSquare(1, 1);
                // push the new BoardSquare onto the vector squares of board
                board.squares[i].push_back(currSquare);
            } else if (type == '2') {
                BoardSquare currSquare(2, 1);
                board.squares[i].push_back(currSquare);
            } else if (type == '3') {
                BoardSquare currSquare(3, 1);
                board.squares[i].push_back(currSquare);
            } else if (type == 'd') {
                BoardSquare currSquare(1, 2);
                board.squares[i].push_back(currSquare);
            } else if (type == 't') {
                BoardSquare currSquare(1, 3);
                board.squares[i].push_back(currSquare);
            } else {
                throw FileException("invalid board file!");
            }
        }
    }

    return board;
}

size_t Board::get_move_index() const { return this->move_index; }

PlaceResult Board::test_place(const Move& move) const {
    // create a vector to keep track of all the positions that tiles will occupy
    vector<Board::Position> positions;
    string word;
    size_t wordlength = move.tiles.size();
    bool start_position = false;
    bool has_adjacent = false;
    size_t total_word_multiplier = 1;
    size_t total_points = 0;
    PlaceResult error("error");

    // set curr to where the first tile will be placed
    Board::Position curr(move.row, move.column);

    // section 1: takes care of letters already on the board before the tiles to be placed

    // loop to find any tiles before where the first tile will be placed
    curr = curr.translate(move.direction, -1);
    while (in_bounds_and_has_tile(curr)) {
        curr = curr.translate(move.direction, -1);
        has_adjacent = true;
    }
    curr = curr.translate(move.direction);
    // at this point, curr is the position of the first letter of the word the player is trying to make

    // loop through all the tiles that are on the board before where the first tile will be place,
    while (in_bounds_and_has_tile(curr)) {
        // fill in 'positions' with positions of these tiles
        positions.push_back(curr);

        // add the letters of these tiles to 'word'
        if (at(curr).get_tile_kind().letter == TileKind::BLANK_LETTER)
            word += at(curr).get_tile_kind().assigned;
        else
            word += at(curr).get_tile_kind().letter;

        // translate curr right/down one more to continue the loop
        curr = curr.translate(move.direction, 1);
    }

    // section 2: takes care of the tiles to be placed in this move and the tiles on the board that may be in the middle
    // of this move's tiles

    // go through all the tiles to be placed - wordlength is initially set to move.tiles.size()
    for (size_t i = 0; i < wordlength; i++) {
        // set curr to where this tile would be on the board
        if (move.direction == Direction::DOWN) {
            curr.row = move.row + i;
            curr.column = move.column;
        } else if (move.direction == Direction::ACROSS) {
            curr.column = move.column + i;
            curr.row = move.row;
        }

        // if this position is the starting position, set start_position to true for later
        if (curr == start)
            start_position = true;

        // check if curr is out of bounds, and if it is, return error placeresult
        if (!is_in_bounds(curr)) {
            cerr << "Oops, you are trying to perform an out of bounds move! Please enter a new move!" << endl;
            return error;
        }

        // if there is a tile currently in curr position
        if (at(curr).has_tile()) {
            // if user is trying to place a word starting at a position that is occupied, return error placeresult
            if (i == 0) {
                cerr << "Oops, you are trying to place a tile over an existing tile on the board! Please enter a new "
                        "move!"
                     << endl;
                return error;
            } else {
                // add the letter to word string
                if (at(curr).get_tile_kind().letter == TileKind::BLANK_LETTER)
                    word += at(curr).get_tile_kind().assigned;
                else
                    word += at(curr).get_tile_kind().letter;
                // add one to wordlength so we will go through the loop an extra time to take care of the extra tile
                wordlength++;
                has_adjacent = true;
            }
        }
        // if there's not a tile in that position, then that's where our move's tile will be placed
        else {
            // so add the letter of the tile to be placed in word
            // wordlength-move.tiles.size() is how many tiles were already on the board
            if (move.tiles[i - (wordlength - move.tiles.size())].letter == TileKind::BLANK_LETTER)
                word += move.tiles[i - (wordlength - move.tiles.size())].assigned;
            else
                word += move.tiles[i - (wordlength - move.tiles.size())].letter;
        }

        // set current position to index of where this tile would be
        positions.push_back(curr);
    }

    // section 3: check if there are any tiles after the end of the tiles the player inputted

    // translate curr down/right 1, and loop through any tiles already on the board after
    curr = curr.translate(move.direction, 1);
    while (in_bounds_and_has_tile(curr)) {
        // for every tile already on the board after the new tiles, add to positions
        positions.push_back(curr);
        // and add the letter to 'word'
        if (at(curr).get_tile_kind().letter == TileKind::BLANK_LETTER)
            word += at(curr).get_tile_kind().assigned;
        else
            word += at(curr).get_tile_kind().letter;
        has_adjacent = true;
        // translate curr down/right 1 to continue loop
        curr = curr.translate(move.direction, 1);
    }

    // at this point, word should be the finished main word that the user was trying to make
    // and positions should hold all the positions for the word

    // if start is currently empty, we are on the first move
    if (!squares[start.row][start.column].has_tile()) {
        // if none of the tiles are being placed on start position, handle the error
        if (!start_position) {
            cerr << "Oops, you need to place a tile on the start position! Please enter a new move!" << endl;
            return error;
        }
    }

    size_t currTile = 0;
    // go through the positions vector to find all the points for this word
    for (size_t i = 0; i < positions.size(); i++) {
        // if this is a tile already on the board, just add it's tilekind's points to the word
        if (at(positions[i]).has_tile()) {
            total_points += at(positions[i]).get_tile_kind().points;
        }
        // else if it's one of the new tiles to be placed,
        else {
            // if there's a word multiplier at a position, multiply it to total_word_multiplier
            total_word_multiplier *= at(positions[i]).word_multiplier;
            // add points to total_points with any letter_multiplier on that boardsquare
            total_points += move.tiles[currTile].points * at(positions[i]).letter_multiplier;
            currTile++;
        }
    }

    // update total_points with any total_word_multiplier
    total_points *= total_word_multiplier;

    // check if any words were made in the opposite direction with helper function
    PlaceResult extra_words = other_words(move);

    // if there is no adjacent cell from the main word, it's not the start position, and there are no words from the
    // other direction, return error placeresult
    if ((!has_adjacent && extra_words.words.size() == 0) && (!start_position)) {
        cerr << "Oops, you need to place atleast one of your tiles next to an already placed tile! Please enter a new "
                "move!"
             << endl;
        return error;
    }

    // set final_words to the new works from the opposite direction
    vector<string> final_words = extra_words.words;

    // if it's the start position, then that's the only case where the main word could be only one letter long!
    // so if it's either the start position or longer than just one character, add it to final_words
    if (start_position || word.length() > 1) {
        final_words.push_back(word);
    }
    // otherwise, it doesn't count as a word on its own so just reset total_points to 0 and don't add it to final_words
    else {
        total_points = 0;
    }
    // add the points from other direction words to total_points
    total_points += extra_words.points;

    // create a PlaceResult with final words vector and total points, and return
    PlaceResult result(final_words, total_points);
    return result;
}

PlaceResult Board::other_words(const Move& move) const {
    // set new_direction as the opposite of move.direction to test for adjacent cells in opposing direction
    Direction new_direction;
    if (move.direction == Direction::ACROSS)
        new_direction = Direction::DOWN;
    else if (move.direction == Direction::DOWN)
        new_direction = Direction::ACROSS;

    vector<string> words;
    size_t temp_total_points = 0;
    size_t total_points = 0;
    size_t temp_points = 0;

    // go through each tile to be placed in move
    for (size_t i = 0; i < move.tiles.size(); i++) {
        string temp_word;
        size_t total_word_multiplier = 1;

        // set an original position of the tile that we are currently on (current iteration)
        Board::Position orig(move.row, move.column);
        orig = orig.translate(move.direction, i);

        // if that position already has a tile, translate the position down/right once more
        if (in_bounds_and_has_tile(orig)) {
            orig = orig.translate(move.direction);
        }
        // set temp position to orig
        Board::Position temp = orig;

        // check if there are any adjacent tiles before this tile. This while loop finds the beginning of the possible
        // new word while temp either has a tile or is the tile to be placed for this iteration,
        while (in_bounds_and_has_tile(temp) || (temp == orig)) {
            // translate temp up/left one
            temp = temp.translate(new_direction, -1);
        }
        // translate temp up/right one to be at the first position with a tile for that word
        temp = temp.translate(new_direction);

        // moves through all the positions for the new word
        while (in_bounds_and_has_tile(temp) || (temp == orig)) {
            // if this position has a tile on the board, add it to temp_word
            if (at(temp).has_tile()) {
                if (at(temp).get_tile_kind().letter == TileKind::BLANK_LETTER)
                    temp_word += at(temp).get_tile_kind().assigned;
                else
                    temp_word += at(temp).get_tile_kind().letter;
                // temp_points equals the points for the existing tilekind in this position
                temp_points = at(temp).get_tile_kind().points;
            }
            // else if this is the position of the tile to be placed for this iteration, add the letter to temp_word
            else if (temp == orig) {
                if (move.tiles[i].letter == TileKind::BLANK_LETTER)
                    temp_word += move.tiles[i].assigned;
                else
                    temp_word += move.tiles[i].letter;
                // temp_points equals the points for this tilekind * the letter_multiplier of the boardsquare
                temp_points = move.tiles[i].points * at(temp).letter_multiplier;
                // update total_word_multiplier
                total_word_multiplier *= at(temp).word_multiplier;
            }
            // temp_total_points += temp_points * at(temp).letter_multiplier;
            temp_total_points += temp_points;
            // translate temp up/right one more
            temp = temp.translate(new_direction);
            // set temp_points to 0 for next iteration
            temp_points = 0;
        }
        // if this was just a singular tile/character, doesn't count as a word
        if (temp_word.length() != 1) {
            // but if it wasn't a singular character, push the word to words vector, add temp_total_points to
            // total_points
            words.push_back(temp_word);
            total_points += temp_total_points * total_word_multiplier;
            // set temp_total_points to 0 for next iteration
            temp_total_points = 0;
        }
    }

    // return placeresult with words vector and total_points
    PlaceResult result(words, total_points);
    return result;
}

PlaceResult Board::place(const Move& move) {
    // call test_place to get the result
    PlaceResult result = test_place(move);

    // if the result is not valid, just return this invalid result and don't place anything or change the board
    if (!result.valid) {
        return result;
    }

    Position curr(move.row, move.column);
    for (size_t i = 0; i < move.tiles.size(); i++) {
        // if there is already a tile at that position, decrement i so we don't skip over a tile in move.tiles
        if (at(curr).has_tile())
            i--;
        else {
            // set BoardSquare to the correct TileKind
            squares[curr.row][curr.column].set_tile_kind(move.tiles[i]);
        }
        // translate curr down 1 or right 1 depending on direction
        curr = curr.translate(move.direction, 1);
    }

    return result;
}

// The rest of this file is provided for you. No need to make changes.

BoardSquare& Board::at(const Board::Position& position) { return this->squares.at(position.row).at(position.column); }

const BoardSquare& Board::at(const Board::Position& position) const {
    return this->squares.at(position.row).at(position.column);
}

bool Board::is_in_bounds(const Board::Position& position) const {
    return position.row < this->rows && position.column < this->columns;
}

bool Board::in_bounds_and_has_tile(const Position& position) const {
    return is_in_bounds(position) && at(position).has_tile();
}

void Board::print(ostream& out) const {
    // Draw horizontal number labels
    for (size_t i = 0; i < BOARD_TOP_MARGIN - 2; ++i) {
        out << std::endl;
    }
    out << FG_COLOR_LABEL << repeat(SPACE, BOARD_LEFT_MARGIN);
    const size_t right_number_space = (SQUARE_OUTER_WIDTH - 3) / 2;
    const size_t left_number_space = (SQUARE_OUTER_WIDTH - 3) - right_number_space;
    for (size_t column = 0; column < this->columns; ++column) {
        out << repeat(SPACE, left_number_space) << std::setw(2) << column + 1 << repeat(SPACE, right_number_space);
    }
    out << std::endl;

    // Draw top line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << endl;

    // Draw inner board
    for (size_t row = 0; row < this->rows; ++row) {
        if (row > 0) {
            out << repeat(SPACE, BOARD_LEFT_MARGIN);
            print_horizontal(this->columns, T_RIGHT, PLUS, T_LEFT, out);
            out << endl;
        }

        // Draw insides of squares
        for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
            out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD;

            // Output column number of left padding
            if (line == 1) {
                out << repeat(SPACE, BOARD_LEFT_MARGIN - 3);
                out << std::setw(2) << row + 1;
                out << SPACE;
            } else {
                out << repeat(SPACE, BOARD_LEFT_MARGIN);
            }

            // Iterate columns
            for (size_t column = 0; column < this->columns; ++column) {
                out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
                const BoardSquare& square = this->squares.at(row).at(column);
                bool is_start = this->start.row == row && this->start.column == column;

                // Figure out background color
                if (square.word_multiplier == 2) {
                    out << BG_COLOR_WORD_MULTIPLIER_2X;
                } else if (square.word_multiplier == 3) {
                    out << BG_COLOR_WORD_MULTIPLIER_3X;
                } else if (square.letter_multiplier == 2) {
                    out << BG_COLOR_LETTER_MULTIPLIER_2X;
                } else if (square.letter_multiplier == 3) {
                    out << BG_COLOR_LETTER_MULTIPLIER_3X;
                } else if (is_start) {
                    out << BG_COLOR_START_SQUARE;
                }

                // Text
                if (line == 0 && is_start) {
                    out << "  \u2605  ";
                } else if (line == 0 && square.word_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'W' << std::setw(1)
                        << square.word_multiplier;
                } else if (line == 0 && square.letter_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'L' << std::setw(1)
                        << square.letter_multiplier;
                } else if (line == 1 && square.has_tile()) {
                    char l = square.get_tile_kind().letter == TileKind::BLANK_LETTER ? square.get_tile_kind().assigned
                                                                                     : ' ';
                    out << repeat(SPACE, 2) << FG_COLOR_LETTER << square.get_tile_kind().letter << l
                        << repeat(SPACE, 1);
                } else if (line == SQUARE_INNER_HEIGHT - 1 && square.has_tile()) {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH - 1) << FG_COLOR_SCORE << square.get_points();
                } else {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH);
                }
            }

            // Add vertical line
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_OUTSIDE_BOARD << std::endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << endl << rang::style::reset << std::endl;
}
