#include "player.h"

using namespace std;

void Player::add_points(size_t points) {
    // add points to this->points
    this->points += points;
}

void Player::subtract_points(size_t points) {
    // subtract points from this->points
    this->points -= points;
}

size_t Player::get_points() const {
    // just return the player's points
    return this->points;
}

const std::string& Player::get_name() const {
    // just return player's name
    return this->name;
}

size_t Player::count_tiles() const {
    // return count_tiles of tilecollection
    return this->tiles.count_tiles();
}

void Player::remove_tiles(const std::vector<TileKind>& tiles) {
    for (size_t i = 0; i < tiles.size(); i++) {
        // remove only 1 of those tiles from player's hand
        this->tiles.remove_tile(tiles[i]);
    }
}

void Player::add_tiles(const std::vector<TileKind>& tiles) {
    // for every tile from parameter, add it to player's tiles
    for (size_t i = 0; i < tiles.size(); i++) {
        this->tiles.add_tile(tiles[i]);
    }
}

TileKind Player::lookup_tile(char letter) { return this->tiles.lookup_tile(letter); }

bool Player::has_tile(TileKind tile) {
    // try looking up tile in player's tiles
    try {
        this->tiles.lookup_tile(tile.letter);
    }
    // if it throws out_of_range, return false
    catch (out_of_range) {
        return false;
    }
    return true;
}

unsigned int Player::get_hand_value() const {
    // return total_points() of tilecollection
    return this->tiles.total_points();
}

size_t Player::get_hand_size() const {
    // just return hand_size
    return this->hand_size;
}