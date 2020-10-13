#include "human_player.h"

#include "exceptions.h"
#include "formatting.h"
#include "move.h"
#include "place_result.h"
#include "rang.h"
#include "tile_kind.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;

// This method is fully implemented.
inline string& to_upper(string& str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

Move HumanPlayer::get_move(const Board& board, const Dictionary& dictionary) const {
    // print out player's hand
    cout << "Your hand: " << endl;
    print_hand(cout);
    cout << endl;

    // ask for player's move
    cout << "Enter your move, " << get_name() << ": " << endl;
    string move_string;
    getline(cin, move_string);

    // convert their input into all lowercase
    for (size_t i = 0; i < move_string.length(); i++) {
        move_string[i] = tolower(move_string[i]);
    }

    // try parse_move and if it throws a commandexception, run get_move again to get a new move
    try {
        Move player_move = parse_move(move_string);
    } catch (CommandException) {
        return get_move(board, dictionary);
    }

    // get the type of move that player is trying to play
    Move player_move = parse_move(move_string);

    // if player passes, just return player_move
    if (player_move.kind == MoveKind::PASS) {
        return player_move;
    }

    // if player exchanges, just return player_move
    else if (player_move.kind == MoveKind::EXCHANGE) {
        return player_move;
    }

    // if player places,
    else if (player_move.kind == MoveKind::PLACE) {
        // test if we can place the word on the board
        PlaceResult test_placeresult = board.test_place(player_move);

        // if this placeresult isn't valid, restart the move
        if (!test_placeresult.valid) {
            return get_move(board, dictionary);
        }

        // if one of the words created by this move isn't a valid word, restart move
        for (size_t i = 0; i < test_placeresult.words.size(); i++) {
            if (!dictionary.is_word(test_placeresult.words[i])) {
                cerr << "Oops, you are trying to place an invalid word! Please enter a new move!" << endl;
                return get_move(board, dictionary);
            }
        }

        // return the move
        return player_move;
    }

    // shouldn't reach this point, but if so, run get_move again
    return get_move(board, dictionary);
}

vector<TileKind> HumanPlayer::parse_tiles(string& letters) const {
    vector<TileKind> temp_tiles;
    // a map that keeps track of letters that the user doesn't have enough of
    map<char, int> multiples;

    // parse through each tile that is being inputted through letters
    for (size_t i = 0; i < letters.length(); i++) {
        // count how many of that letter is present in the input string
        size_t num_chars = count(letters.begin(), letters.end(), letters[i]);

        // if lookup_tile throws an out_of_range error, throw a CommandException
        try {
            this->tiles.lookup_tile(letters[i]);
        } catch (out_of_range) {
            throw CommandException("don't have the correct tiles for the move!");
        }

        // lookup the tile and count how many of these tiles the player has
        TileKind curr = this->tiles.lookup_tile(letters[i]);
        size_t num_tiles = this->tiles.count_tiles(curr);

        // if there are more of this letter in the input string than the player's hand
        if (num_chars > num_tiles) {
            // if this letter is already in the multiples map, subtract the value by the number that they are lacking
            map<char, int>::iterator it = multiples.find(letters[i]);
            if (it != multiples.end()) {
                it->second -= (num_tiles - num_chars);
            }
            // else, add it to the map rather than immediately throwing an exception because there may be a blank tile
            // the player uses
            else {
                multiples.insert(make_pair(letters[i], num_tiles - num_chars));
            }
        }

        // if the current tile is '?'
        if (curr.letter == TileKind::BLANK_LETTER) {
            // make sure that there is something after ? if it's at the end of the input string
            if (i + 1 < letters.length()) {
                // set the assigned value of this tilekind to the following letter
                curr.assigned = letters[i + 1];

                // if this letter is already in the multiples map, increase the value by 1
                map<char, int>::iterator it = multiples.find(letters[i + 1]);
                if (it != multiples.end()) {
                    it->second += 1;
                }
                // else, add it to the map with a value of 1
                else {
                    multiples.insert(make_pair(letters[i + 1], 1));
                }
                // increment i to skip over the following letter (which we set to "assigned" for the ? tile)
                i++;
            }
            // else, throw CommandException
            else {
                cerr << "You must input a value for your blank tile, please try again!" << endl;
                throw CommandException("didn't have a value for the blank tile");
            }
        }

        // add curr tile to tiles vector that we will end up returning
        temp_tiles.push_back(curr);
    }

    // iterate through all of the multiples map
    map<char, int>::iterator end;
    for (end = multiples.begin(); end != multiples.end(); ++end) {
        // if any of the values are less than 1, that means they do not have enough of the tile they are trying to use
        // and they have not used enough blank tiles to take care of that issue
        if (end->second < 0)
            throw CommandException("don't have the correct tiles for the move!");
    }

    // return the tiles vector!
    return temp_tiles;
}

Move HumanPlayer::parse_move(string& move_string) const {
    // create stringstream to read in parts of move_string
    stringstream ss(move_string);
    string eval;

    // read in the first part of move_string
    ss >> eval;

    // if first part of move_string is PASS, create and return default Move constructor
    if (eval == "pass") {
        Move player_move;
        return player_move;
    }
    // else if first part of move_string is EXCHANGE,
    else if (eval == "exchange") {
        // extract tiles to replace and convert to vector of TileKinds
        string removed;
        ss >> removed;

        // if we get a CommandException from parse_tiles, cerr an error message and throw CommandException
        try {
            vector<TileKind> replaced_tiles = parse_tiles(removed);
        } catch (CommandException) {
            cerr << "Oops, you don't have the correct tiles for this move! Please enter a new move!" << endl;
            throw CommandException("don't have the correct tiles for the move!");
        }
        vector<TileKind> replaced_tiles = parse_tiles(removed);

        // if the player is trying to exchange no tiles, cerr an error message and throw CommandException
        if (replaced_tiles.empty()) {
            cerr << "Oops, you didn't input any tiles to exchange! Please try again!" << endl;
            throw CommandException("didn't input any tiles");
        }

        // create and return Move with MoveKind exchange
        Move player_move(replaced_tiles);
        return player_move;
    }
    // else if first part of move_string is place,
    else if (eval == "place") {
        char direction;
        string tiles_input;
        size_t row, column;
        // read in all the elements of a place move
        ss >> direction >> row >> column >> tiles_input;

        // if they didn't input these members correctly, cerr an error message and throw CommandException
        if (ss.fail()) {
            cerr << "Oops, you didn't input the move correctly! Please enter a new move!" << endl;
            throw CommandException("Invalid move input!");
        }

        // if we get a CommandException from parse_tiles, cerr an error message and throw CommandException
        try {
            vector<TileKind> new_tiles = parse_tiles(tiles_input);
        } catch (CommandException) {
            cerr << "Oops, you don't have the correct tiles for this move! Please enter a new move!" << endl;
            throw CommandException("don't have the correct tiles for the move!");
        }
        vector<TileKind> new_tiles = parse_tiles(tiles_input);

        // create and return Move with the correct direction based on the input
        if (direction == '-') {
            Move player_move(new_tiles, row - 1, column - 1, Direction::ACROSS);
            return player_move;
        } else if (direction == '|') {
            Move player_move(new_tiles, row - 1, column - 1, Direction::DOWN);
            return player_move;
        }
        // if they inputted an invalid direction, cerr an error message and throw CommandException
        else {
            cerr << "Oops, you didn't input a valid direction! Please enter a new move!" << endl;
            throw CommandException("no valid direction input!");
        }

    }

    // if they didn't put either place, exchange, or pass, cerr an error message and throw CommandException
    else {
        cerr << "Oops, you didn't input the move correctly! Please enter a new move!" << endl;
        throw CommandException("no valid input!");
    }
}

// This function is fully implemented.
void HumanPlayer::print_hand(ostream& out) const {
    const size_t tile_count = tiles.count_tiles();
    const size_t empty_tile_count = this->get_hand_size() - tile_count;
    const size_t empty_tile_width = empty_tile_count * (SQUARE_OUTER_WIDTH - 1);

    for (size_t i = 0; i < HAND_TOP_MARGIN - 2; ++i) {
        out << endl;
    }

    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_HEADING << "Your Hand: " << endl << endl;

    // Draw top line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;

    // Draw middle 3 lines
    for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
        out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD << repeat(SPACE, HAND_LEFT_MARGIN);
        for (auto it = tiles.cbegin(); it != tiles.cend(); ++it) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_PLAYER_HAND;

            // Print letter
            if (line == 1) {
                out << repeat(SPACE, 2) << FG_COLOR_LETTER << (char)toupper(it->letter) << repeat(SPACE, 2);

                // Print score in bottom right
            } else if (line == SQUARE_INNER_HEIGHT - 1) {
                out << FG_COLOR_SCORE << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << setw(2) << it->points;

            } else {
                out << repeat(SPACE, SQUARE_INNER_WIDTH);
            }
        }
        if (tiles.count_tiles() > 0) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
            out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << rang::style::reset << endl;
}
