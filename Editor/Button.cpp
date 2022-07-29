#include <SFML/Graphics.hpp>
#include "Button.h"


Button::Button(const sf::IntRect& DetectionRange,const sf::Texture& Texture):
DetectionRange(DetectionRange)
{
    this->setPosition(DetectionRange.left,DetectionRange.top);
    this->setTexture(Texture);
}

