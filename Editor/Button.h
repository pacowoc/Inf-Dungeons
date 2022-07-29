#include <SFML/Graphics.hpp>

#ifndef BUTTON_H
#define BUTTON_H

class Button : public sf::Sprite{
    private:
        sf::IntRect DetectionRange;



    public: 
        Button(const sf::IntRect& DetectionRange,const sf::Texture& Texture);

        bool get_overlay(const sf::Vector2f MousePos)const{
            return DetectionRange.contains(sf::Vector2i(MousePos));
        }


};



#endif