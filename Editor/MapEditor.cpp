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
#define GuiBorder 0.2f
#define GuiHeaderBorder 0.1f
#define FONT_A "ETextures/Fonts/SEGOELUI,TTF"   





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
0x09      |bool                 |doSpawn            |
0x0A~0x0D |unsigned short[2]    |SpawnRoomPos       |[x,y]   
0x0E~0X11 |unsigned short[2]    |SpawnPosInSpawnRoom|[x,y]

0x12~0x39 Unused



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

    0x14~0x1F Unused


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
    GuiOverlay.setOutlineThickness(2);

    sf::RectangleShape GuiHeaderOverlay(sf::Vector2f(ResolutionX*GuiBorder,ResolutionY*GuiHeaderBorder));
    GuiHeaderOverlay.setFillColor(sf::Color(220,220,220));
    GuiHeaderOverlay.setOutlineColor(sf::Color(100,100,100));
    GuiHeaderOverlay.setOutlineThickness(2);

    sf::RectangleShape* GuiBackground[2]= {&GuiOverlay,&GuiHeaderOverlay};

    sf::RectangleShape CursorBlockOutline(sf::Vector2f{80,80});
    CursorBlockOutline.setFillColor(sf::Color(0,0,0,0));
    CursorBlockOutline.setOutlineColor(sf::Color(255,255,0));
    CursorBlockOutline.setOutlineThickness(5);


    //Setting up the Editor Area
    v_EditorArea.reset(sf::FloatRect(0,0,ResolutionX,ResolutionY));
    v_EditorArea.setViewport(sf::FloatRect(GuiBorder,0,1,1));
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
    sf::Texture Save;
    Save.loadFromFile("ETextures/Save.png");
    sf::Texture OOBBlock;
    OOBBlock.loadFromFile("ETextures/OOB.png");


    Button FileButton(sf::IntRect(0,0,108,36),File);
    Button TextureButton(sf::IntRect(108,0,108,36),Textures);
    Button TemplatesButton(sf::IntRect(0,36,108,36),Templates);
    Button PlaceHolder(sf::IntRect(108,36,108,36),Empty);
    Button OpenButton(sf::IntRect(0,108,108,36),Open);
    Button NewButton(sf::IntRect(108,108,108,36),New);
    Button SaveButton(sf::IntRect(0,144,108,36),Save);

    Button* StaticButtons[4] = {&FileButton,&TextureButton,&TemplatesButton,&PlaceHolder};
    Button* Buttons0[3] = {&OpenButton,&NewButton,&SaveButton};
 

    Block** BlockArray = nullptr;
    unsigned short SizeX = 0;
    unsigned short SizeY = 0;
    bool  doSpawn = 0;
    unsigned short SpawnX = 0;
    unsigned short SpawnY = 0;
    char* ReadBuffer;
    char* WriteBuffer;


    bool Dragging = false;
    bool Selecting = false;
    bool CursorInBound = false;
    int GuiType = 0;


    


    
    while (window.isOpen()){

        sf::Vector2f MousePos = (sf::Vector2f)sf::Mouse::getPosition(window);
        sf::Vector2f MousePosE = v_EditorArea.getCenter()+(MousePos-sf::Vector2f(ResolutionX/2+ResolutionX*GuiBorder,ResolutionY*0.5))/ZoomRatio;
        sf::Vector2i MPosBlock = {(int)MousePosE.x/80,(int)MousePosE.y/80};
        
        if(MousePosE.x>=1&&MousePosE.y>=1&&MousePosE.x<=80*SizeX-1&&MousePosE.y<=80*SizeY-1){
            CursorBlockOutline.setPosition(MPosBlock.x*80,MPosBlock.y*80);
            CursorInBound = true;
        }
        else{
            CursorInBound = false;
        }
        sf::Event event;    
        while (window.pollEvent(event)&&window.hasFocus()){
            if (event.type == sf::Event::Closed){
                window.close(); 
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
                        if(FileButton.get_overlay(MousePos)){
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

                                //No Failsave yet
                                std::string MapName;
                                std::cout<<"Open>>Enter the Map Name:"<<std::endl;
                                std::cin>>MapName;
                                ReadBuffer = new char[50];
                                sf::Texture TextureBuffer;
                                std::fstream File;

                                //Reload Textures
                                TexturesVector.clear();
                                File.open("../Maps/"+MapName+"/Textures/"+"textureregister.txt",std::fstream::in);
                                while(!File.eof()){
                                    File.getline(ReadBuffer,50);
                                    TextureBuffer.loadFromFile("../Maps/Default/Textures/"+std::string(ReadBuffer));
                                    TexturesVector.push_back(TextureBuffer);
                                }
                                delete ReadBuffer;
                                ReadBuffer = nullptr;
                                std::cout<<"Textures loaded"<<std::endl;
                                File.close();
                                std::string RoomName;

                                //No Failsave yet
                                std::cout<<"Enter the Room Name(or type \"new\" to create a new one)"<<std::endl;
                                std::cin>>RoomName;
                                File.open("../Maps/"+MapName+"/Rooms/"+RoomName+".infdm",std::fstream::in|std::fstream::binary);
                                File.seekg (0, File.end);
                                int length = File.tellg();
                                File.seekg (0, File.beg);
                                ReadBuffer = new char[length];
                                File.read(ReadBuffer,length);

                                SizeX = ReadBuffer[5] + 0x100*ReadBuffer[6];
                                SizeY = ReadBuffer[7] + 0x100*ReadBuffer[8];
                                doSpawn = ReadBuffer[8];
                                SpawnX = ReadBuffer[13] + 0x100*ReadBuffer[14];
                                SpawnY = ReadBuffer[15] + 0x100*ReadBuffer[16];
                                if(BlockArray!=nullptr){
                                    for(size_t i=0;i<SizeX;i++){
                                        delete[] BlockArray[i];
                                    }
                                    BlockArray = nullptr;
                                }
                                BlockArray = new Block*[SizeX];
                                size_t index = 0;
                                for(size_t i = 0; i < SizeX; i++){
                                    BlockArray[i] = new Block[SizeY];
                                    for(size_t j = 0; j<SizeY;j++){
                                        int ReadPos = index*32+64;
                                        index++;
                                        BlockArray[i][j].changeTag(ReadBuffer[ReadPos]);
                                        BlockArray[i][j].changeTypeA(ReadBuffer[ReadPos+1]);
                                        BlockArray[i][j].changeTypeB(ReadBuffer[ReadPos+17]);
                                        BlockArray[i][j].changeTextureA(ReadBuffer[ReadPos+2]+ReadBuffer[ReadPos+3]*0x100);
                                        BlockArray[i][j].changeTextureB(ReadBuffer[ReadPos+18]+ReadBuffer[ReadPos+19]*0x100);
                                        BlockArray[i][j].switch_to_A();
                                        BlockArray[i][j].setPosition(80*i,80*j);
                                        BlockArray[i][j].refresh_texture(TexturesVector,OOBBlock);
                                    }

                                }
                                delete ReadBuffer;
                                ReadBuffer = nullptr;
                                File.close();


                            }
                            if(NewButton.get_overlay(MousePos)){

                                //No Failsave yet
                                std::string MapName;
                                std::cout<<"New>>Enter the Map Name:"<<std::endl;
                                std::cin>>MapName;
                                ReadBuffer = new char[50];
                                sf::Texture TextureBuffer;
                                std::fstream File;

                                //Reload Textures
                                TexturesVector.clear();
                                File.open("../Maps/"+MapName+"/Textures/"+"textureregister.txt",std::fstream::in);
                                while(!File.eof()){
                                    File.getline(ReadBuffer,50);
                                    std::cout<<ReadBuffer<<std::endl;
                                    TextureBuffer.loadFromFile("../Maps/Default/Textures/"+std::string(ReadBuffer));
                                    TexturesVector.push_back(TextureBuffer);
                                }
                                delete ReadBuffer;
                                ReadBuffer = nullptr;
                                File.close();

                                //No Failsave yet
                                std::cout<<"SizeX:"<<std::endl;
                                std::cin>>SizeX;
                                std::cout<<"SizeY:"<<std::endl;
                                std::cin>>SizeY;
                                std::cout<<"doSpawn"<<std::endl;
                                std::cin>>doSpawn;
                                if(doSpawn != 0){
                                    std::cout<<"SpawnX:"<<std::endl;
                                    std::cin>>SpawnX;
                                    std::cout<<"SpawnY:"<<std::endl;
                                    std::cin>>SpawnY;
                                }
                                std::cout<<"C";

                                //Delete the heap memory
                                if(BlockArray!= nullptr){
                                    for(size_t i=0;i<SizeX;i++){
                                        delete[] BlockArray[i];
                                    }
                                    delete[] BlockArray;
                                    BlockArray = nullptr;
                                }

                                //Generate Empty BlockArray
                                BlockArray = new Block*[SizeX];
                                for(size_t i = 0; i < SizeX; i++){
                                    BlockArray[i] = new Block[SizeY];
                                    for(size_t j = 0; j<SizeY;j++){
                                        BlockArray[i][j] = Block(0,0,0,0,0);
                                        BlockArray[i][j].setPosition(80*i,80*j);
                                        BlockArray[i][j].switch_to_A();
                                        BlockArray[i][j].refresh_texture(TexturesVector,OOBBlock);
                                    }
                                }


                            }
                            if(SaveButton.get_overlay(MousePos)){
                                //No Failsave yet
                                std::string MapName;
                                std::cout<<"Save>>Enter the Map Name:"<<std::endl;
                                std::cin>>MapName;
                                std::string FileName;
                                std::cout<<"Enter the File Name:"<<std::endl;
                                std::cin>>FileName;
                                std::fstream File;
                                WriteBuffer = new char[SizeX*SizeY*32+64]; 
                                for(size_t i=0;i<SizeX*SizeY*32+64;i++){
                                    WriteBuffer[i] = 0x00;
                                }
                                WriteBuffer[0] = 0x00;//Version
                                WriteBuffer[1] = 0x01;//DDimX
                                WriteBuffer[2] = 0x00;
                                WriteBuffer[3] = 0x01;//DDimY
                                WriteBuffer[4] = 0x00;
                                WriteBuffer[5] = SizeX%0x100;//RDimX
                                WriteBuffer[6] = SizeX/0x100;
                                WriteBuffer[7] = SizeY%0x100;//RDimY
                                WriteBuffer[8] = SizeY/0x100;
                                WriteBuffer[9] = doSpawn;//Spawn?
                                WriteBuffer[14] = SpawnX%0x100;//SPosX
                                WriteBuffer[15] = SpawnX/0x100;
                                WriteBuffer[16] = SpawnY%0x100;//SPosY
                                WriteBuffer[17] = SpawnY/0x100;
                                size_t index = 0;
                                for(size_t i = 0; i < SizeX; i++){
                                    for(size_t j = 0; j<SizeX;j++){
                                        int WritePos = index*32+64;
                                        index++;
                                        WriteBuffer[WritePos] = BlockArray[i][j].get_tag();
                                        WriteBuffer[WritePos+1] = BlockArray[i][j].get_typeA();
                                        WriteBuffer[WritePos+2] = BlockArray[i][j].get_textureIDA()%0X100;
                                        WriteBuffer[WritePos+3] = BlockArray[i][j].get_textureIDB()/0x100;
                                    }

                                }
                                File.open("../Maps/"+MapName+"/Rooms/"+FileName+".infdm",std::fstream::out|std::fstream::binary|std::fstream::trunc);
                                File.write((char*)WriteBuffer,SizeX*SizeY*32+64);
                                File.close();
                                delete WriteBuffer;
                                WriteBuffer = nullptr;
                                
                            }
                        }

                    }   
                }
                if(event.mouseButton.button == sf::Mouse::Right){
                    if (MousePos.x >= ResolutionX*GuiBorder){
                        
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
                 ZoomRatio *= std::pow(1.1,1*event.mouseWheelScroll.delta);
                 v_EditorArea.zoom(std::pow(1.1,-1*event.mouseWheelScroll.delta));
            }

        }

        if(Dragging == true){
            v_EditorArea.setCenter((Drag_initMousePos-MousePos)/ZoomRatio+Drag_initView);
        }
        /*
        
        DRAW PHASE
        
        */
        window.clear(sf::Color(0,0,0,255));

        //Change the View to Editor Window
        window.setView(v_EditorArea);

        //Draw the Blocks
        for(size_t i=0;i<SizeX;i++){
            for(size_t j=0;j<SizeY;j++){
                window.draw(BlockArray[i][j]);
            }
        }

        if(CursorInBound){
            window.draw(CursorBlockOutline);
        }

        //Reset to Default view
        window.setView(window.getDefaultView());

        //Draw the Basic Gui Components
        for(sf::RectangleShape* i:GuiBackground){
            window.draw(*i);
        }

        //Draw the Buttons on the Header
        for(Button* i:StaticButtons){
            window.draw(*i);
        }

        //Draw the File Buttons Conditionally
        if(GuiType==0){
            for(Button* i:Buttons0){
                window.draw(*i);
            }
        }

        //Update
        window.display();
    }
            



}

