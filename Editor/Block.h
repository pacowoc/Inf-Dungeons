#include <SFML/Graphics.hpp>
#include <vector>


#ifndef BLOCK_H
#define BLOCK_H

    class Block : public sf::Sprite{

        private:
            unsigned char CurrentType;
            unsigned char CurrentTextureID;;
            unsigned short TextureIDA;
            unsigned short TextureIDB;
            unsigned char Tag;
            unsigned char TypeA;
            unsigned char TypeB;
            unsigned short PosX;
            unsigned short PosY;
            
        public:
            Block(){ };
            Block(const unsigned short TextureIDA,const unsigned short TextureIDB,const unsigned char Tag,const unsigned char TypeA,const unsigned char TypeB);

        public:
            //getters

            unsigned char get_tag()const{
                return Tag;
            }

            unsigned char get_typeA()const{
                return TypeA;
            }

            unsigned char get_typeB()const{
                return TypeB;
            }

            unsigned char get_type()const{
                return CurrentType;
            }

            //definition in Block.cpp
            void switch_to_A();
            void switch_to_B();  
            void changeTypeA(const unsigned char NType);
            void changeTypeB(const unsigned char NType);
            void changeTextureA(const unsigned char NTexID);
            void changeTextureB(const unsigned char NTexID);
            void refresh_texture(const std::vector<sf::Texture>& IDTable);
              

            

    };



#endif