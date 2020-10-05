#include "board.h"
#include "board_square.h"
#include "exceptions.h"
#include "formatting.h"
#include <iomanip>
#include <fstream>

using namespace std;


bool Board::Position::operator==(const Board::Position& other) const {
    return this->row == other.row && this->column == other.column;
}

bool Board::Position::operator!=(const Board::Position& other) const {
    return this->row != other.row || this->column != other.column;
}

Board::Position Board::Position::translate(Direction direction) const {
    return this->translate(direction, 1);
}

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

    //traverse through each square of board
    for(size_t i = 0; i < board.rows; i++){
    	//read in the current row from file
    	// string currRow;
    	// file >> currRow;
    	
    	board.squares.push_back(temp);
    	for(size_t j = 0; j < board.columns; j++){
    		char type;
    		file >> type;
    		//instantiate a boardsquare for each column of that row, with the correct letter/word multipliers
    		if(type == '.'){
    			BoardSquare currSquare(1,1);
    			//push the new BoardSquare onto the vector squares of board
    			board.squares[i].push_back(currSquare);
    		}
    		else if(type == '2'){
    			BoardSquare currSquare(2,1);
    			board.squares[i].push_back(currSquare);
    		}
    		else if(type == '3'){
    			BoardSquare currSquare(3,1);
    			board.squares[i].push_back(currSquare);
    		}
    		else if(type == 'd'){
    			BoardSquare currSquare(1,2);
    			board.squares[i].push_back(currSquare);
    		}
    		else if(type == 't'){
    			BoardSquare currSquare(1,3);
    			board.squares[i].push_back(currSquare);
    		}
    		else{
    			//handle this idk how yet oops
    		}
    	}
    }

    return board;
}

size_t Board::get_move_index() const {
    return this->move_index;
}

PlaceResult Board::test_place(const Move& move) const {
	//create a vector to keep track of all the positions that tiles will occupy
	vector<Board::Position> positions;
	string word;
	size_t wordlength = move.tiles.size();
	bool start_position = false;
	bool has_adjacent = false;
	size_t total_word_multiplier = 1;
	size_t total_points = 0;
	PlaceResult error("error");
	//set curr to where the first tile will be placed
	Board::Position curr(move.row, move.column);


	//loop to find any tiles before where the first tile will be placed
	curr = curr.translate(move.direction, -1);
	while(in_bounds_and_has_tile(curr)){
		curr = curr.translate(move.direction, -1);
		has_adjacent = true;
	}
	curr = curr.translate(move.direction);
	//fill in 'positions' with positions of tiles already on board at the start of the target word
	while(in_bounds_and_has_tile(curr)){
		positions.push_back(curr); //come back and fix indexing
		word += at(curr).get_tile_kind().letter;
		curr = curr.translate(move.direction, 1);
	}

	//go through all the tiles to be placed
	for(size_t i = 0; i < wordlength; i++){
		//if direction is vertical,
		if(move.direction == Direction::DOWN){
			//set curr to index of where this tile would be
			curr.row = move.row + i;
			curr.column = move.column;
		}
		else if(move.direction == Direction::ACROSS){
			//set curr to index of where this tile would be
			curr.column = move.column + i;
			curr.row = move.row;
		}

		//if this position is the starting position, set start_position to true for later
		if (curr == start)
			start_position = true;

		//check if curr is out of bounds
		if(!is_in_bounds(curr)){
			//handle this error, it's out of bounds (NEED TO COMPLETE)
			return error;
		}

		//if there is a tile currently in curr position
		if(at(curr).has_tile()){
			//if user is trying to place a word starting at a position that is occupied
			if(i == 0){
				//handle this error somehow (NEED TO COMPLETE)
				return error;
			}
			else{
				//add 1 to wordlength and add the letter to word string
				word += at(curr).get_tile_kind().letter;
				wordlength++;
				has_adjacent = true;
			}
		}
		else{
			//if there's not a tile in that position, then that's where our tile will be placed
			//so add the letter of the tile to be placed in word
			//wordlength-move.tiles.size() is how many tiles were already on the board
			word += move.tiles[i-(wordlength-move.tiles.size())].letter;
		}
			

		//set current position to index of where this tile would be
		positions.push_back(curr);

	}

	//check if there are any tiles after the end of inputted tiles
	curr = curr.translate(move.direction, 1);
	while(in_bounds_and_has_tile(curr)){
		//for every tile already on the board after the new tiles, add to positions
		positions.push_back(curr);
		word += at(curr).get_tile_kind().letter;
		has_adjacent = true;
		curr = curr.translate(move.direction, 1);
	}

	//at this point, words[0] should be the finished main word that the user was trying to make 
	//            and positions should hold all the positions for the word
	
	//if start is currently empty, we are on the first move
    if(!squares[start.row][start.column].has_tile()){
    	//if none of the tiles are being placed on start position, handle the error
    	if(!start_position){
    		//handle this error that it's not on the starting spot
    		return error;
    	}
    }	

	size_t currTile = 0;
	//go through the positions vector to find points
	for(size_t i = 0; i < positions.size(); i++){
		//add points from this letter to total_points
		if(at(positions[i]).has_tile()){
			total_points += at(positions[i]).get_points();
		}
		else{
			//if there's a word multiplier at a position, multiply it to total_word_multiplier
			total_word_multiplier *= at(positions[i]).word_multiplier;
			total_points += move.tiles[currTile].points * at(positions[i]).letter_multiplier;
			currTile++;
		}
	}

	total_points *= total_word_multiplier;

	//--------------------check other direction for other accidental words -------------------------
	PlaceResult extra_words = other_words(move);
	if((!has_adjacent && extra_words.words.size() == 0) && (!start_position)){
    	return error;
	}

    	
	vector<string> final_words = extra_words.words;
	final_words.push_back(word);
	total_points += extra_words.points;
	// Board::Position testing(8,8);
	// total_points = at(testing).letter_multiplier;


    

	//create a PlaceResult with final words vector and total points, and return
	PlaceResult result(final_words, total_points);
	return result;
}

PlaceResult Board::other_words(const Move& move) const{
	//set new_direction as the opposite of move.direction to test for adjacent cells in opposing direction
	Direction new_direction;
	if(move.direction == Direction::ACROSS)
		new_direction = Direction::DOWN;
	else if(move.direction == Direction::DOWN)
		new_direction = Direction::ACROSS;

	// size_t wordcount = 0;
	vector<string> words;
	size_t temp_total_points = 0;
	size_t total_points = 0;
	size_t temp_points = 0;
	size_t temp_length = move.tiles.size();
	//check for adjacent tiles on each tile to be placed in move
	for(size_t i = 0; i < temp_length; i++){
		string temp_word;
		size_t total_word_multiplier = 1; 
		//create temp position at current tile
		
		Board::Position orig(move.row, move.column);
		orig = orig.translate(move.direction, i);
		if(in_bounds_and_has_tile(orig)){
			temp_length++;
			break;
		}
		Board::Position temp = orig;
		//check if there are any adjacent tiles before this tile. This while loop finds the beginning of the possible new word
		//while temp either has a tile or is the tile to be placed for this iteration,
		while(in_bounds_and_has_tile(temp) || (temp == orig)){
			//translate temp up/left one
			temp = temp.translate(new_direction, -1);
		}
		//translate temp up/right one to be at the first position with a tile for that word
		temp = temp.translate(new_direction);

		//moves through all the positions for the new word
		while(in_bounds_and_has_tile(temp) || (temp == orig)){
			//if this position has a tile on the board, add it to words[wordcount]
			if(at(temp).has_tile()){
				// words[wordcount] += at(temp).get_tile_kind().letter;
				temp_word += at(temp).get_tile_kind().letter;
				//temp_points equals the points for the tile in this spot
				temp_points = at(temp).get_points();
			}
			//else if this is the position of the tile to be placed for this iteration, add the letter to words[wordcount]
			else if(temp == orig){
				// words[wordcount] += move.tiles[i].letter;
				temp_word += move.tiles[i].letter;
				//temp_points equals the points for this tilekind
				temp_points = move.tiles[i].points * at(temp).letter_multiplier;
				//updates total_word_multiplier
				total_word_multiplier *= at(temp).word_multiplier;
			}
			// temp_total_points += temp_points * at(temp).letter_multiplier;
			temp_total_points += temp_points;
			//translate temp up/right one more
			temp = temp.translate(new_direction);
			temp_points = 0;
		}
		//if this was just a singular tile/character, doesn't count as a word
		if(temp_word.length() == 1){
			// words.erase(words.begin() + wordcount);
		}
		else{
			// wordcount++;
			words.push_back(temp_word);
			total_points += temp_total_points * total_word_multiplier;
			temp_total_points = 0;
		}
	}
	PlaceResult result(words, total_points);
	return result;
}

PlaceResult Board::place(const Move& move) {
	//call test_place to get the result
    PlaceResult result = test_place(move);

    if (!result.valid){
    	//handle this error (NEED TO COMPLETE)
    }

    Position curr(move.row, move.column);
	size_t wordlength = move.tiles.size();
	for(size_t i = 0; i < wordlength; i++){
		//if there is already a tile at that position, decrement i so we don't skip over something in tiles
		if(at(curr).has_tile())
			i--;
		else{
			//set BoardSquare to the correct TileKind
			squares[curr.row][curr.column].set_tile_kind(move.tiles[i]);
			squares[curr.row][curr.column].letter_multiplier = 1;
			squares[curr.row][curr.column].word_multiplier = 1;
		}
		//translate curr down 1 or right 1 depending on direction
		 curr = curr.translate(move.direction, 1);
	}

	return result;

}

// The rest of this file is provided for you. No need to make changes.

BoardSquare& Board::at(const Board::Position& position) {
    return this->squares.at(position.row).at(position.column);
}

const BoardSquare& Board::at(const Board::Position& position) const {
    return this->squares.at(position.row).at(position.column);
}

bool Board::is_in_bounds(const Board::Position& position) const {
    return position.row < this->rows && position.column < this->columns;
}

bool Board::in_bounds_and_has_tile(const Position& position) const{
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
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'W' << std::setw(1) << square.word_multiplier;
                } else if (line == 0 && square.letter_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'L' << std::setw(1) << square.letter_multiplier;
				} else if (line == 1 && square.has_tile()) {
                    char l = square.get_tile_kind().letter == TileKind::BLANK_LETTER ? square.get_tile_kind().assigned : ' ';
                    out << repeat(SPACE, 2) << FG_COLOR_LETTER << square.get_tile_kind().letter << l << repeat(SPACE, 1);
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
