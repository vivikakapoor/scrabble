#include "scrabble.h"

#include "formatting.h"
#include <iomanip>
#include <iostream>
#include <map>

using namespace std;

// Given to you. this does not need to be changed
Scrabble::Scrabble(const ScrabbleConfig& config)
        : hand_size(config.hand_size),
          minimum_word_length(config.minimum_word_length),
          tile_bag(TileBag::read(config.tile_bag_file_path, config.seed)),
          board(Board::read(config.board_file_path)),
          dictionary(Dictionary::read(config.dictionary_file_path)) {}

void Scrabble::add_players() {
    size_t num_players = 0;
    string name;
    bool first = true;

    // query user about how many players are playing
    cout << "Please enter number of players: ";
    cin >> num_players;

    // if there are more than 8 players or less than 1, throw FileException
    if (num_players > 8 || num_players < 1) {
        throw FileException("Invalid number of players!");
    }

    // confirm the number of players
    cout << num_players << " players confirmed." << endl;

    for (size_t i = 0; i < num_players; i++) {
        // query user about name of each player
        cout << "Enter Name of Player " << i + 1 << ": ";

        // for the first getline, I needed to do cin.ignore() because otherwise it was skipping over the first input!
        if (first) {
            first = false;
            cin.ignore();
        }
        getline(cin, name);

        // construct HumanPlayer with name and hand_size and assign it to players vector
        shared_ptr<Player> new_player = make_shared<HumanPlayer>(name, this->hand_size);
        players.push_back(new_player);
        // print out player's name to show they have been added
        cout << "Player " << i + 1 << ", named "
             << "\"" << new_player->get_name() << "\""
             << " has been added." << endl;

        // give that player a starting hand with hand_size tiles from the tile bag
        vector<TileKind> new_tiles = tile_bag.remove_random_tiles(hand_size);
        players[i]->add_tiles(new_tiles);
    }
}

// Game Loop should cycle through players and get and execute that players move
// until the game is over.
void Scrabble::game_loop() {
    bool empty_hand = false;
    size_t passes = 0;

    // continues looping through the game until the game ends
    // game ends either when someone has an empty hand and there's no tiles left in the bag or everyone passes in a
    // round
    while (!empty_hand && passes != players.size()) {
        // loops through every player
        for (size_t i = 0; i < players.size(); i++) {
            // print board;
            board.print(cout);

            // get the player's points before they move
            size_t pre_points = players[i]->get_points();

            // call get_move for the player
            Move player_move = players[i]->get_move(board, dictionary);

            // if the player passes, increment passes counter
            if (player_move.kind == MoveKind::PASS) {
                passes++;
                // if the number of passes are equal to players vector size, break because the game is over
                if (passes == players.size())
                    break;
            }

            // else if the player wants to exchange tiles
            else if (player_move.kind == MoveKind::EXCHANGE) {
                // reset number of passes once someone does not pass
                passes = 0;

                // remove the tiles from player's hand
                players[i]->remove_tiles(player_move.tiles);

                // add those tiles to the tile bag
                for (size_t j = 0; j < player_move.tiles.size(); j++)
                    tile_bag.add_tile(player_move.tiles[j]);

                // if there are more tiles in the tile bag than the amount to be removed,
                if (player_move.tiles.size() < tile_bag.count_tiles()) {
                    // then remove random tiles from the tile bag and add them to player's hand
                    vector<TileKind> new_tiles = tile_bag.remove_random_tiles(player_move.tiles.size());
                    players[i]->add_tiles(new_tiles);
                }
                // if the tile bag is empty, can't add any tiles so they get no new tiles
                else if (tile_bag.count_tiles() == 0) {
                }
                // else if there are not enough tiles in the tile bag, give the player all the tiles left
                else {
                    vector<TileKind> new_tiles = tile_bag.remove_random_tiles(tile_bag.count_tiles());
                    players[i]->add_tiles(new_tiles);
                }

            }
            // else if player wants to place tiles
            else if (player_move.kind == MoveKind::PLACE) {
                // reset number of passes once someone does not pass
                passes = 0;

                // place tiles from player_move(we already tested if the player can place these tiles in
                // HumanPlayer::get_move)
                PlaceResult final_placeresult = board.place(player_move);

                // assign points from place_tiles to this player
                players[i]->add_points(final_placeresult.points);

                // if the player used all their tiles, add 50 to their score
                if (player_move.tiles.size() == players[i]->count_tiles())
                    players[i]->add_points(50);

                // get the points after the move and display how many points the player gained
                size_t post_points = players[i]->get_points();
                cout << "You gained " << SCORE_COLOR << post_points - pre_points << rang::style::reset << " points!"
                     << endl;

                // remove the tiles from player's hand
                players[i]->remove_tiles(player_move.tiles);

                // if there are more tiles in the tile bag than the amount to be removed,
                if (player_move.tiles.size() < tile_bag.count_tiles()) {
                    // then remove random tiles from the tile bag and add them to player's hand
                    vector<TileKind> new_tiles = tile_bag.remove_random_tiles(player_move.tiles.size());
                    players[i]->add_tiles(new_tiles);
                }
                // if the tile bag is empty, can't add any tiles so they get no new tiles
                else if (tile_bag.count_tiles() == 0) {
                }
                // else if there are not enough tiles in the tile bag, give the player all the tiles left
                else {
                    vector<TileKind> new_tiles = tile_bag.remove_random_tiles(tile_bag.count_tiles());
                    players[i]->add_tiles(new_tiles);
                }

                // if the player has no tiles left, then they have an empty_hand so break out of the loop
                if (players[i]->count_tiles() == 0) {
                    empty_hand = true;
                    break;
                }
            }

            // display the current score for the player and prompt them to continue
            cout << "Your current score: " << SCORE_COLOR << players[i]->get_points() << rang::style::reset << endl;
            cout << endl << "Press [enter] to continue.";
            cin.ignore();
        }
    }
}

void Scrabble::final_subtraction(vector<shared_ptr<Player>>& plrs) {
    size_t total_hand_value = 0;
    int winning_index = -1;

    // go through each player in plrs
    for (size_t i = 0; i < plrs.size(); i++) {
        // if their hand value is more than 0,
        if (plrs[i]->get_hand_value() > 0) {
            // add their hand value to the total_hand_value
            total_hand_value += plrs[i]->get_hand_value();
            // if their hand value is greater than their points, just set their points to 0
            if (plrs[i]->get_hand_value() > plrs[i]->get_points())
                plrs[i]->subtract_points(plrs[i]->get_points());
            // else, subtract their hand value from their points
            else
                plrs[i]->subtract_points(plrs[i]->get_hand_value());
        }
        // if their hand value is 0, set winning_index to this player's index
        else {
            winning_index = i;
        }
    }

    // if there is a winning_index, add the total_hand_value to their points
    if (winning_index >= 0) {
        plrs[winning_index]->add_points(total_hand_value);
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
        cout << SCORE_COLOR << player->get_points() << rang::style::reset << " | " << PLAYER_NAME_COLOR
             << player->get_name() << rang::style::reset << endl;
    }
}

// You should not need to change this.
void Scrabble::main() {
    add_players();
    game_loop();
    final_subtraction(this->players);
    print_result();
}
