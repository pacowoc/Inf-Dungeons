#include <SFML/Graphics.hpp>
#include <vector>
#include "Block.h"


//Init Texture,Type:A
Block::Block(
    unsigned short TextureIDA,unsigned short TextureIDB,
    unsigned char Tag,unsigned char TypeA,unsigned char TypeB
): 
    TextureIDA(TextureIDA),TextureIDB(TextureIDB),CurrentTextureID(TextureIDA),
    TypeA(TypeA),TypeB(TypeB),CurrentType(TypeA)

{
}


void Block::switch_to_A(){
    CurrentType = TypeA;
    CurrentTextureID = TextureIDA;
}

void Block::switch_to_B(){
    CurrentType = TypeB;
    CurrentTextureID = TextureIDB;

}

void Block::changeTypeA(const unsigned char NewType){
    TypeA = NewType;
}

void Block::changeTypeB(const unsigned char NewType){
    TypeB = NewType;
}

void Block::changeTextureA(const unsigned char NewID){
    TextureIDA = NewID;
}

void Block::changeTextureB(const unsigned char NewID){
    TextureIDB = NewID;
}


void Block::refresh_texture(const std::vector<sf::Texture>& IDTable){
    setTexture(IDTable.at(CurrentTextureID));
}



