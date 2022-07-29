#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include "Block.h"
#include "Button.h"



#define MaxZoomInRatio 10
#define MinZoomOutRatio 0.2
#define ResolutionX 1080
#define ResolutionY 720
#define GuiBorder 0.2
#define GuiHeaderBorder 0.1





/*
64 bit system:
char 	  1
short     2
int 	  4
long      8
long long 8

float 	    4	
double 	    8	
long double 12	


File Structure of .infdm  V0
_______
Header|0x00~0x39
------------------------------------------------------------------------------
0x00      |unsigned char        |Version            |0x00
0x01~0x04 |unsigned short[2]    |MapDimInRoom       |[x,y]
    [0x01,0x01] for singular room
0x05~0x08 |unsigned short[2]    |RoomDimInBlock     |[x,y]
0x09~0x0C |unsigned short[2]    |SpawnRoomPos       |[x,y]   
0x0D~0X10 |unsigned short[2]    |SpawnPosInSpawnRoom|[x,y]

0x11~0x39 Unused



________
Content|0x40~  (32byte/block)
------------------------------------------------------------------------------
0x00      |unsigned char        |GeneratorTag   |
    IF uses Default Generator:

        0x00:Static Block -- Always use ContentA
        0x01:Down Block   -- Use ContentB if and only if the bottom of the room is open
        0x02:Right Block   -- Use ContentB if and only if the right side of the room is open
        0x03:Up Block   -- Use ContentB if and only if the top of the room is open
        0x04:Left Block   -- Use ContentB if and only if the left side of the room is open

ContentA: 0x01~0x0F

    0x01      |unsigned char     |TypeIDA    |

            0x00:Out of Bounds
            0x01:Wall
            0x02:Floor


    0x02~0x03      |unsigned short     |TextureIDA |

    0x04-0x0F   Unused

0X10 Unused

ContentB: 0x11~0x1F

    0x11      |unsigned char     |TypeIDB    |

            0x00:Out of Bounds
            0x01:Wall
            0x02:Floor

0x12~0x13      |unsigned short     |TextureIDB |


*/ 







int main(){

    sf::Vector2f Drag_initMousePos;
    sf::Vector2f Drag_initView;

    std::vector<sf::Texture> TexturesVector;
    sf::RenderWindow window(sf::VideoMode(ResolutionX, ResolutionY), "InfDungeon Room Editor");
    sf::View v_EditorArea;

    sf::RectangleShape GuiOverlay(sf::Vector2f(ResolutionX*GuiBorder,ResolutionY));

    //Setting up the Gui Area Background
    GuiOverlay.setFillColor(sf::Color(180,180,180));
    GuiOverlay.setOutlineColor(sf::Color(100,100,100));
    GuiOverlay.setOutlineThickness(7);

    sf::RectangleShape GuiHeaderOverlay(sf::Vector2f(ResolutionX*GuiBorder,ResolutionY*GuiHeaderBorder));
    GuiHeaderOverlay.setFillColor(sf::Color(220,220,220));
    GuiHeaderOverlay.setOutlineColor(sf::Color(100,100,100));
    GuiHeaderOverlay.setOutlineThickness(7);


    //Setting up the Editor Area
    v_EditorArea.reset(sf::FloatRect(0,0,ResolutionX,ResolutionY));
    v_EditorArea.setViewport(sf::FloatRect(GuiBorder,0,1,1));
    v_EditorArea.setCenter(648,360);
    float ZoomRatio = 1;

    //Loading in Button Textures
    sf::Texture File;
    File.loadFromFile("ETextures/File.png");
    sf::Texture Textures;
    Textures.loadFromFile("ETextures/Textures.png");
    sf::Texture Templates;
    Templates.loadFromFile("ETextures/Templates.png");
    sf::Texture Empty;
    Empty.loadFromFile("ETextures/Empty.png");
    sf::Texture New;
    New.loadFromFile("ETextures/New.png");
    sf::Texture Open;
    Open.loadFromFile("ETextures/Open.png");


    Button FileButton(sf::IntRect(0,0,108,36),File);
    Button TextureButton(sf::IntRect(108,0,108,36),Textures);
    Button TemplatesButton(sf::IntRect(0,36,108,36),Templates);
    Button PlaceHolder(sf::IntRect(108,36,108,36),Empty);
    Button OpenButton(sf::IntRect(0,108,108,36),Open);
    Button NewButton(sf::IntRect(108,108,108,36),New);
 




    bool Dragging = false;
    int GuiType = 0;

    std::array<std::array<Block,1>,1> BlockArray;

    


    
    while (window.isOpen()){

        sf::Vector2f MousePos = (sf::Vector2f)sf::Mouse::getPosition(window);

        sf::Event event;
        while (window.pollEvent(event)){
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::KeyPressed){
                if (event.key.code == sf::Keyboard::Down){
                    v_EditorArea.move(0,-80);
                }
                if (event.key.code == sf::Keyboard::Up){
                    v_EditorArea.move(0,80);
                }
                if (event.key.code == sf::Keyboard::Right){
                    v_EditorArea.move(-80,0);
                }
                if (event.key.code == sf::Keyboard::Left){
                    v_EditorArea.move(80,0);
                }
            
            }

            if (event.type == sf::Event::MouseButtonPressed){
                if (event.mouseButton.button == sf::Mouse::Left){
                    if (MousePos.x >= ResolutionX*GuiBorder){
                        Drag_initMousePos = MousePos;
                        Drag_initView = v_EditorArea.getCenter();
                        Dragging = true;
                    }
                    else{
                        std::cout<<"a"<<std::endl;
                        if(FileButton.get_overlay(MousePos)){
                            std::cout<<"b"<<std::endl;
                            GuiType = 0;
                        }
                        if(TextureButton.get_overlay(MousePos)){
                            GuiType = 1;
                        }
                        if(TemplatesButton.get_overlay(MousePos)){
                            GuiType = 2;
                        }
                        if(GuiType == 0){
                            if(OpenButton.get_overlay(MousePos)){
                                std::string MapName;
                                std::cout<<"Enter the Map Name:"<<std::endl;
                                std::cin>>MapName;
                                char ReadBuffer[50];
                                sf::Texture TextureBuffer;
                                std::fstream File;

                                //Reload Textures
                                TexturesVector.clear();
                                File.open("../Maps/"+MapName+"/Textures/"+"textureregister.txt",std::fstream::in);
                                while(!File.eof()){
                                    File.getline(ReadBuffer,50);
                                    //Maps/Default/Textures/floor.png
                                    TextureBuffer.loadFromFile("../Maps/Default/Textures/"+std::string(ReadBuffer));
                                    TexturesVector.push_back(TextureBuffer);
                                }
                                std::cout<<"Textures loaded"<<std::endl;
                                File.close();
                                BlockArray.at(0).at(0) = Block(0,0,0,0,0);
                                for(size_t i=0;i<BlockArray.size();i++){
                                    for(size_t j=0;j<BlockArray.at(i).size();j++){
                                        BlockArray.at(i).at(j).setPosition(i*80+ResolutionX/2,j*80+ResolutionY/2);
                                        BlockArray.at(i).at(j).refresh_texture(TexturesVector);
                                    }
                                }
                                

                            }
                        }
                    }

                }
            }

            if (event.type == sf::Event::MouseButtonReleased){
                if (event.mouseButton.button == sf::Mouse::Left){
                    if (Dragging == true){
                        Dragging = false;
                    }
                }
            }    

            if (event.type == sf::Event::MouseWheelScrolled){
                 ZoomRatio *= std::pow(1.1,-1*event.mouseWheelScroll.delta);
                 v_EditorArea.zoom(std::pow(1.1,-1*event.mouseWheelScroll.delta));
            }

        }

        if(Dragging == true){
            v_EditorArea.setCenter(ZoomRatio*(Drag_initMousePos-MousePos)+Drag_initView);
        }
        window.clear(sf::Color(0,0,0,255));
        window.setView(v_EditorArea);
        
        for(size_t i=0;i<BlockArray.size();i++){
            for(size_t j=0;j<BlockArray.at(i).size();j++){
                window.draw(BlockArray.at(i).at(j));
            }
        }
        window.setView(window.getDefaultView());

        //Draw the Bassic Gui Components
        window.draw(GuiOverlay);
        window.draw(GuiHeaderOverlay);
        window.draw(FileButton);
        window.draw(TextureButton);
        window.draw(TemplatesButton);
        window.draw(PlaceHolder);

        if(GuiType==0){
            window.draw(OpenButton);
            window.draw(NewButton);
        }

    
        window.display();
    }
            



}


