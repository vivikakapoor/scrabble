#include "scrabble.h"
#include "formatting.h"
#include <iostream>
#include <iomanip>
#include <map>

using namespace std;


// Given to you. this does not need to be changed
Scrabble::Scrabble(const ScrabbleConfig& config)
    : hand_size(config.hand_size)
    , minimum_word_length(config.minimum_word_length)
    , tile_bag(TileBag::read(config.tile_bag_file_path, config.seed))
    , board(Board::read(config.board_file_path))
    , dictionary(Dictionary::read(config.dictionary_file_path)) {}

void Scrabble::add_players(){
    size_t num_players = 0;
    string name;

    //query user about how many players are playing
    cout << "How many players are playing?(1-8)" << endl;
    cin >> num_players;
    cout << num_players << " players confirmed." << endl;

    //if there are more than 8 layers or less than 1
    if(num_players > 8 || num_players < 1){
        cout << "Please enter a number between 1 - 8 inclusive." << endl;
        return add_players();
    }

    for(size_t i = 0; i < num_players; i++){
        //query user about name of each player
        cout << "Enter Name of Player " << i+1 << ": " << endl;
        cin >> name;
        // cin.getline(name); // confused idk how to use getline bc im dumb
        //construct HumanPlayer with name and hand_size and assign it to players vector
        shared_ptr<Player> new_player = make_shared<HumanPlayer>(name, this->hand_size);
        players.push_back(new_player);
        cout << "Player " << i << ", named " << "\"" << new_player->get_name() << "\"" << " has been added." << endl;
        vector<TileKind> new_tiles = tile_bag.remove_random_tiles(hand_size);
        players[i]->add_tiles(new_tiles);
    }
}

// Game Loop should cycle through players and get and execute that players move
// until the game is over.
void Scrabble::game_loop() {
    bool empty_hand = false;
    size_t passes = 0;
    int winner_index = -1;

    //continues looping through the game until the game ends
    //game ends either when someone has an empty hand and there's no tiles left in the bag or everyone passes in a round
    while(!empty_hand && passes != players.size()){
        //loops through every player
        for(size_t i = 0; i < players.size(); i++){
            //print board and hand:
            board.print(cout);
            // cout << "Your hand: " << endl;
            // players[i]->print_hand(cout); 

            size_t pre_points = players[i]->get_points();
            //call get_move for the player
            Move player_move = players[i]->get_move(board, dictionary);

            if(player_move.kind == MoveKind::PASS){
                passes++;
                if(passes == players.size())
                    break;
            }
            else if(player_move.kind == MoveKind::EXCHANGE){
                //reset number of passes once someone does not pass
                passes = 0;

                //remove the tiles from player's hand 
                players[i]->remove_tiles(player_move.tiles);

                //add those tiles to the tile bag
                for(size_t j = 0; j < player_move.tiles.size(); j++)
                    tile_bag.add_tile(player_move.tiles[j]);

                //remove random tiles from the tile bag and add them to player's hand
                //if there are more tiles in the tile bag than the amount to be removed,
                if(player_move.tiles.size() < tile_bag.count_tiles()){
                    vector<TileKind> new_tiles = tile_bag.remove_random_tiles(player_move.tiles.size());
                    players[i]->add_tiles(new_tiles);
                }
                //else if there are not enough tiles in the tile bag, give the player all the tiles left
                else{
                    vector<TileKind> new_tiles = tile_bag.remove_random_tiles(tile_bag.count_tiles());
                    players[i]->add_tiles(new_tiles);
                }

            }
            else if(player_move.kind == MoveKind::PLACE){
                //reset number of passes once someone does not pass
                passes = 0;

                PlaceResult final_placeresult = board.place(player_move);
                //assign points from place_tiles to this player
                players[i]->add_points(final_placeresult.points);
                if(player_move.tiles.size() == players[i]->get_hand_size())
                    players[i]->add_points(50);

                size_t post_points = players[i]->get_points();
                cout << "You gained " << post_points-pre_points << "!" << endl;
                //remove the tiles from player's hand
                players[i]->remove_tiles(player_move.tiles);

                //remove random tiles from the tile bag and add them to player's hand
                //if there are more tiles in the tile bag than the amount to be removed,
                if(player_move.tiles.size() < tile_bag.count_tiles()){
                    vector<TileKind> new_tiles = tile_bag.remove_random_tiles(player_move.tiles.size());
                    players[i]->add_tiles(new_tiles);
                }
                //if the tile bag is empty, can't add any tiles so they get no new tiles
                else if(tile_bag.count_tiles() == 0) {}
                //else if there are not enough tiles in the tile bag, give the player all the tiles left
                else{
                    vector<TileKind> new_tiles = tile_bag.remove_random_tiles(tile_bag.count_tiles());
                    players[i]->add_tiles(new_tiles);
                }
                if(players[i]->count_tiles() == 0){
                    empty_hand = true;
                    winner_index = i;
                    break;
                }

            } 

            cout << "Your current score: " << players[i]->get_points() << endl;
            // cout << "Press [enter] to continue.";
        }
    }

    if(empty_hand){
        for(size_t i = 0; i < players.size(); i++){
            //this will add the winning player's hand value to itself but that'll be taken care of in final_subtraction
            players[winner_index]->add_points(players[i]->get_hand_value());
        }
    }
    

    final_subtraction(players);

    // Useful cout expressions with fancy colors. Expressions in curly braces, indicate values you supply.
    // cout << "You gained " << SCORE_COLOR << {points} << rang::style::reset << " points!" << endl;
    // cout << "Your current score: " << SCORE_COLOR << {points} << rang::style::reset << endl;
    // cout << endl << "Press [enter] to continue.";
}

// Performs final score subtraction. Players lose points for each tile in their
// hand. The player who cleared their hand receives all the points lost by the
// other players.
void Scrabble::final_subtraction(vector<shared_ptr<Player>> & plrs) {
    for(size_t i = 0; i < plrs.size(); i++){
        plrs[i]->subtract_points(plrs[i]->get_hand_value());
    }
}

// You should not need to change this function.
void Scrabble::print_result() {
	// Determine highest score
    size_t max_points = 0;
	for (auto player : this->players) {
		if (player->get_points() > max_points) {
			max_points = player->get_points();
        }
	}

	// Determine the winner(s) indexes
	vector<shared_ptr<Player>> winners;
	for (auto player : this->players) {
		if (player->get_points() >= max_points) {
			winners.push_back(player);
        }
	}

    cout << (winners.size() == 1 ? "Winner:" : "Winners: ");
	for (auto player : winners) {
		cout << SPACE << PLAYER_NAME_COLOR << player->get_name();
	}
	cout << rang::style::reset << endl;

	// now print score table
    cout << "Scores: " << endl;
    cout << "---------------------------------" << endl;

	// Justify all integers printed to have the same amount of character as the high score, left-padding with spaces
    cout << setw(static_cast<uint32_t>(floor(log10(max_points) + 1)));

	for (auto player : this->players) {
		cout << SCORE_COLOR << player->get_points() << rang::style::reset << " | " << PLAYER_NAME_COLOR << player->get_name() << rang::style::reset << endl;
	}
}

// You should not need to change this.
void Scrabble::main() {
    add_players();
    game_loop();
    final_subtraction(this->players);
    print_result();
}
