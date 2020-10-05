#include "player.h"

using namespace std;


void Player::add_points(size_t points){
	this->points += points;
}

void Player::subtract_points(size_t points){
	this->points -= points;
}

size_t Player::get_points() const{
	return this->points;
}

const std::string& Player::get_name() const{
	return this->name;
}

size_t Player::count_tiles() const{
	return this->tiles.count_tiles();
}

void Player::remove_tiles(const std::vector<TileKind>& tiles){
	for(size_t i = 0; i < tiles.size(); i++){
		//remove only 1 of those tiles from player's hand
		this->tiles.remove_tile(tiles[i]); 
	}
}

void Player::add_tiles(const std::vector<TileKind>& tiles){
	for(size_t i = 0; i < tiles.size(); i++){
		this->tiles.add_tile(tiles[i]);
	}
}

TileKind Player::lookup_tile(char letter){
	return this->tiles.lookup_tile(letter);
}

bool Player::has_tile(TileKind tile){
	// TileKind lookup = this->tiles.lookup_tile(tile.letter);
	try{
		this->tiles.lookup_tile(tile.letter);
	}
	catch(out_of_range){
		return false;
	}
	return true;
}

unsigned int Player::get_hand_value() const{
	return this->tiles.total_points();
}

size_t Player::get_hand_size() const{
	return this->hand_size;
}