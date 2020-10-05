#include <sstream>
#include <stdexcept>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "place_result.h"
#include "move.h"
#include "exceptions.h"
#include "human_player.h"
#include "tile_kind.h"
#include "formatting.h"
#include "rang.h"

using namespace std;


// This method is fully implemented.
inline string& to_upper(string& str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

Move HumanPlayer::get_move(const Board& board, const Dictionary& dictionary) const {
    cout << "Your hand: " << endl;
    this->print_hand(cout); 

    cout << "Enter your move: " << endl;
	string move_string;
    cin.ignore();
    getline(cin, move_string);
    cout << "STRING: " << move_string << endl;

    for(size_t i = 0; i < move_string.length(); i++)
        tolower(move_string[i]);

    //get the type of move that player is trying to play
    Move player_move = parse_move(move_string);

    if (player_move.kind == MoveKind::PASS){
        //if player passes, just return player_move
        return player_move;
    }
    else if (player_move.kind == MoveKind::EXCHANGE){
        //need to add the tiles from player_move.tiles to the tile bag
        //remove tiles from their hand - i think in scrabble.cpp

        //remove random tiles from tile bag and add those to player's hand
        

        return player_move;
    }
    else if (player_move.kind == MoveKind::PLACE){
        //test if we can place the word on the board
        PlaceResult test_placeresult = board.test_place(player_move);


        for(size_t i = 0; i < test_placeresult.words.size(); i++){
            if(!dictionary.is_word(test_placeresult.words[i])){
                //handle error
            }
        }
        if(!test_placeresult.valid){
            //handle error
        }
        return player_move;
    }
}

vector<TileKind> HumanPlayer::parse_tiles(string& letters) const{
    vector<TileKind> temp_tiles;

    for(size_t i = 0; i < letters.length(); i++){
        size_t num_chars = count(letters.begin(), letters.end(), letters[i]);
        try{
            this->tiles.lookup_tile(letters[i]);
        }
        catch(out_of_range){
            //handle this error
        }
        TileKind curr = this->tiles.lookup_tile(letters[i]);
        size_t num_tiles = this->tiles.count_tiles(curr);
        if(num_chars > num_tiles){
            //handle the error of not having the tiles
        }
        //if the current tile is '?'
        if(curr.letter == TileKind::BLANK_LETTER){
            curr.assigned = letters[i+1];
            i++;
        }
        //add curr tile to tiles vector
        temp_tiles.push_back(curr);
        
    }
    return temp_tiles;
}


Move HumanPlayer::parse_move(string& move_string) const {
    //create stringstream to read in parts of move_string
	stringstream ss(move_string);
    string eval;

    ss >> eval;

    //if first part of move_string is PASS, create default Move constructor
    if(eval == "pass"){
        Move player_move;
        return player_move;
    }
    //ele if first part of move_string is EXCHANGE, 
    else if(eval == "exchange"){
        //extract tiles to replace and convert to vector of TileKinds
        string removed;
        ss >> removed;

        vector<TileKind> replaced_tiles = parse_tiles(removed);
        //create Move with MoveKind exchange
        Move player_move(replaced_tiles);
        return player_move;
    }
    else if(eval == "place"){
        char direction;
        string tiles_input;
        size_t row, column;

        ss >> direction >> row >> column >> tiles_input;
        if(ss.fail()){
            //tell the user what's wrong and somehow ask for another move
        }
        vector<TileKind> new_tiles = parse_tiles(tiles_input);

        if(direction == '-'){
            Move player_move(new_tiles, row-1, column-1, Direction::ACROSS);
            return player_move;
        }
        else if(direction == '|'){
            Move player_move(new_tiles, row-1, column-1, Direction::DOWN);
            return player_move;
        }
        else{
            //tell the user what's wrong and somehow ask for another move
        }
        
    }
    
}


// This function is fully implemented.
void HumanPlayer::print_hand(ostream& out) const {
	const size_t tile_count = tiles.count_tiles();
	const size_t empty_tile_count = this->get_hand_size() - tile_count;
	const size_t empty_tile_width = empty_tile_count * (SQUARE_OUTER_WIDTH - 1);

	for(size_t i = 0; i < HAND_TOP_MARGIN - 2; ++i) {
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
